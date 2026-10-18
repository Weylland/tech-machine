#include "../PluginProcessor.h"

using namespace techno;

namespace
{
template <size_t N>
juce::String encodeBools (const std::array<std::atomic<bool>, N>& cells)
{
    juce::String s;
    for (auto& c : cells) s << (c.load() ? "1" : "0");
    return s;
}

template <size_t N>
void decodeBools (const juce::String& s, std::array<std::atomic<bool>, N>& cells)
{
    for (size_t i = 0; i < N; ++i) cells[i] = s[(int) i] == '1';
}
} // namespace

//==============================================================================
// Projects

void TechMachineProcessor::resetParameters()
{
    for (auto* param : getParameters())
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (param))
            p->setValueNotifyingHost (p->getDefaultValue());
}

void TechMachineProcessor::newProject()
{
    playing = false;
    samples.clear();
    resetParameters();
    for (auto& m : machines) m->track.clear();
    genreIndex = 0;
    progression = { 0 };
    name = utf8 ("Sans titre");
    file = juce::File();
    touchPattern();
    undoManager.clearUndoHistory();
    dirty = false;
}

void TechMachineProcessor::loadExample (int index)
{
    newProject();

    struct Writer : ExampleWriter
    {
        TechMachineProcessor& p;
        explicit Writer (TechMachineProcessor& owner) : p (owner) {}
        void setParam (const juce::String& id, float value) override { p.setParam (id, value); }
        void show (int type) override { p.setShown (type, true); }
        Track& track (int type) override { return p.machine (type).track; }
        int baseNote (int type) override
        {
            const auto& def = machineDef (type);
            const int octave = def.params.empty() ? 2 : (int) p.apvts.getRawParameterValue (pid::synth (type, 0))->load();
            return machineBaseNote (def, p.currentKey(), octave);
        }
    } writer (*this);

    buildExample (index, writer);
    const auto& info = examples()[(size_t) index];
    for (int g = 0; g < (int) styles().size(); ++g)
        if (utf8 (styles()[(size_t) g].name) == utf8 (info.genre)) genreIndex = g;
    chooseProgression();
    name = utf8 (info.name);
    touchPattern();
    undoManager.clearUndoHistory();
    dirty = false;
}

std::unique_ptr<juce::XmlElement> TechMachineProcessor::createProjectXml()
{
    auto xml = apvts.copyState().createXml();
    auto* patterns = xml->createNewChildElement ("Patterns");
    patterns->setAttribute ("genre", genreIndex);
    patterns->setAttribute ("name", name);
    juce::StringArray prog;
    for (int d : progression) prog.add (juce::String (d));
    patterns->setAttribute ("progression", prog.joinIntoString (","));

    for (auto& m : machines)
    {
        auto* e = patterns->createNewChildElement ("Machine");
        e->setAttribute ("id", m->def.id);
        for (size_t r = 0; r < m->def.rows.size(); ++r)
        {
            e->setAttribute ("hits" + juce::String ((int) r), encodeBools (m->track.hits[r]));
            juce::String rolls;
            bool any = false;
            for (auto& v : m->track.rolls[r])
            {
                rolls << juce::String ((int) v.load());
                any = any || v.load() > 0;
            }
            if (any) e->setAttribute ("rolls" + juce::String ((int) r), rolls);
        }
        e->setAttribute ("accent", encodeBools (m->track.accent));
        e->setAttribute ("on", encodeBools (m->track.on));
        e->setAttribute ("slide", encodeBools (m->track.slide));

        juce::StringArray notes, chords, ties;
        bool anyChord = false;
        for (auto& n : m->track.note) notes.add (juce::String (n.load()));
        for (int i = 0; i < maxSteps; ++i)
        {
            chords.add (juce::String::toHexString ((juce::int64) m->track.chord[(size_t) i].load()));
            ties.add (juce::String::toHexString ((juce::int64) m->track.tie[(size_t) i].load()));
            anyChord = anyChord || m->track.chord[(size_t) i].load() != 0;
        }
        e->setAttribute ("note", notes.joinIntoString (","));
        if (anyChord)
        {
            e->setAttribute ("chord", chords.joinIntoString (","));
            e->setAttribute ("tie", ties.joinIntoString (","));
        }

        if (m->type == Sampler)
            for (int i = 0; i < numPads; ++i)
                if (const auto pad = samples.describe (i); pad.isNotEmpty()) e->setAttribute ("pad" + juce::String (i), pad);
    }
    return xml;
}

void TechMachineProcessor::restoreProjectXml (const juce::XmlElement& source)
{
    juce::XmlElement xml (source);
    auto* patterns = xml.getChildByName ("Patterns");
    if (patterns == nullptr) return;

    genreIndex = juce::jlimit (0, (int) styles().size() - 1, patterns->getIntAttribute ("genre"));
    name = patterns->getStringAttribute ("name", utf8 ("Sans titre"));
    progression.clear();
    for (auto& d : juce::StringArray::fromTokens (patterns->getStringAttribute ("progression", "0"), ",", {}))
        progression.push_back (juce::jlimit (0, 13, d.getIntValue()));
    if (progression.empty()) progression = { 0 };

    // What the file does not mention starts empty (older files have fewer machines and no sampler).
    for (auto& m : machines) m->track.clear();
    samples.clear();

    for (auto* e : patterns->getChildIterator())
    {
        for (auto& m : machines)
        {
            if (e->getStringAttribute ("id") != m->def.id) continue;
            for (size_t r = 0; r < m->def.rows.size(); ++r)
            {
                decodeBools (e->getStringAttribute ("hits" + juce::String ((int) r)), m->track.hits[r]);
                const auto rolls = e->getStringAttribute ("rolls" + juce::String ((int) r));
                for (int s = 0; s < maxSteps && s < rolls.length(); ++s)
                    m->track.rolls[r][(size_t) s] = (std::uint8_t) juce::jlimit (0, 4, (int) (rolls[s] - (juce::juce_wchar) 48));
            }
            decodeBools (e->getStringAttribute ("accent"), m->track.accent);
            decodeBools (e->getStringAttribute ("on"), m->track.on);
            decodeBools (e->getStringAttribute ("slide"), m->track.slide);
            auto notes = juce::StringArray::fromTokens (e->getStringAttribute ("note"), ",", {});
            for (int i = 0; i < maxSteps; ++i)
                m->track.note[(size_t) i] = juce::jlimit (0, noteRange - 1, notes[i].getIntValue());
            if (m->type == Sampler)
            {
                juce::StringArray pads;
                for (int i = 0; i < numPads; ++i) pads.add (e->getStringAttribute ("pad" + juce::String (i)));
                samples.restore (pads);
            }
            auto chords = juce::StringArray::fromTokens (e->getStringAttribute ("chord"), ",", {});
            auto ties = juce::StringArray::fromTokens (e->getStringAttribute ("tie"), ",", {});
            for (int i = 0; i < maxSteps && i < chords.size(); ++i)
                m->track.chord[(size_t) i] = (std::uint64_t) chords[i].getHexValue64() & ((1ull << noteRange) - 1);
            for (int i = 0; i < maxSteps && i < ties.size(); ++i)
                m->track.tie[(size_t) i] = (std::uint64_t) ties[i].getHexValue64() & ((1ull << noteRange) - 1);
        }
    }
    xml.removeChildElement (patterns, true);
    xml.removeAttribute ("app");
    xml.removeAttribute ("version");

    // Parameters missing from the file (older versions) go back to their default.
    resetParameters();
    apvts.replaceState (juce::ValueTree::fromXml (xml));
    touchPattern();
}

juce::File TechMachineProcessor::projectsFolder()
{
    auto folder = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Tech Machine").getChildFile ("Projets");
    folder.createDirectory();
    return folder;
}


juce::StringArray TechMachineProcessor::recentProjects()
{
    const auto list = juce::StringArray::fromLines (settings().getValue ("recent"));
    juce::StringArray existing;
    for (auto& path : list)
        if (path.isNotEmpty() && juce::File (path).existsAsFile()) existing.add (path);
    return existing;
}

void TechMachineProcessor::addRecent (const juce::File& f)
{
    auto list = recentProjects();
    list.removeString (f.getFullPathName());
    list.insert (0, f.getFullPathName());
    while (list.size() > 8) list.remove (list.size() - 1);
    settings().setValue ("recent", list.joinIntoString ("\n"));
    settings().saveIfNeeded();
}

bool TechMachineProcessor::saveProject (const juce::File& target)
{
    name = target.getFileNameWithoutExtension();
    auto xml = createProjectXml();
    xml->setAttribute ("app", "Tech Machine");
    xml->setAttribute ("version", JucePlugin_VersionString);
    if (! xml->writeTo (target)) return false;
    file = target;
    dirty = false;
    addRecent (target);
    return true;
}

bool TechMachineProcessor::loadProject (const juce::File& source)
{
    auto xml = juce::XmlDocument::parse (source);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()) || xml->getChildByName ("Patterns") == nullptr)
        return false;

    playing = false;
    restoreProjectXml (*xml);
    name = source.getFileNameWithoutExtension();
    file = source;
    undoManager.clearUndoHistory();
    dirty = false;
    addRecent (source);
    return true;
}

bool TechMachineProcessor::exportWav (const juce::XmlElement& project, const juce::File& target, int loops,
                                      const std::function<bool (float)>& progress)
{
    constexpr double rate = 48000.0;
    constexpr int block = 512;

    TechMachineProcessor offline;
    offline.restoreProjectXml (project);
    offline.setPlayConfigDetails (0, 2, rate, block);
    offline.prepareToPlay (rate, block);
    offline.playing = true;

    const double loopSeconds = offline.numSteps() * 15.0 / (double) offline.bpm->load();
    const int total = (int) ((loopSeconds * loops + 2.5) * rate);
    const int musicEnd = (int) (loopSeconds * loops * rate);

    target.deleteFile();
    std::unique_ptr<juce::OutputStream> stream = target.createOutputStream();
    if (stream == nullptr) return false;
    juce::WavAudioFormat wav;
    auto writer = wav.createWriterFor (stream, juce::AudioFormatWriterOptions {}.withSampleRate (rate).withNumChannels (2).withBitsPerSample (24));
    if (writer == nullptr) return false;

    juce::AudioBuffer<float> buffer (2, block);
    juce::MidiBuffer midi;
    for (int pos = 0; pos < total; pos += block)
    {
        const int n = juce::jmin (block, total - pos);
        buffer.setSize (2, n, false, false, true);
        if (pos >= musicEnd) offline.playing = false;   // let echoes and reverb ring out
        offline.processBlock (buffer, midi);
        writer->writeFromAudioSampleBuffer (buffer, 0, n);
        if (progress && ! progress ((float) pos / (float) total)) return false;
    }
    return true;
}

void TechMachineProcessor::startRecording()
{
    stopRecording();

    auto folder = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Tech Machine");
    folder.createDirectory();
    auto target = folder.getNonexistentChildFile ("TechMachine_" + juce::Time::getCurrentTime().formatted ("%Y-%m-%d_%H-%M-%S"), ".wav");

    std::unique_ptr<juce::OutputStream> stream = target.createOutputStream();
    if (stream == nullptr) return;

    juce::WavAudioFormat wav;
    auto writer = wav.createWriterFor (stream, juce::AudioFormatWriterOptions {}
                                                   .withSampleRate (sampleRate)
                                                   .withNumChannels (2)
                                                   .withBitsPerSample (24));
    if (writer == nullptr) return;

    writerThread.startThread();
    threadedWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter> (writer.release(), writerThread, 32768);
    lastRecording = target;

    const juce::ScopedLock sl (writerLock);
    activeWriter = threadedWriter.get();
}

void TechMachineProcessor::stopRecording()
{
    {
        const juce::ScopedLock sl (writerLock);
        activeWriter = nullptr;
    }
    threadedWriter.reset();
}

//==============================================================================
// Sampler pads

bool TechMachineProcessor::loadSamplePad (int pad, const juce::File& source)
{
    if (! samples.load (pad, source)) return false;
    dirty = true;
    return true;
}

// Cuts a loop into 8 equal slices, one per pad, and writes the steps that play it back as it was (one bar, slices on eighth notes).
bool TechMachineProcessor::chopLoop (const juce::File& loop)
{
    if (! samples.chop (loop)) return false;

    undoManager.beginNewTransaction();
    beginPatternEdit();
    auto& track = machines[(size_t) Sampler]->track;
    track.clear();
    for (int i = 0; i < numPads; ++i) track.hits[(size_t) i][(size_t) (i * 2)] = true;
    track.tile (pageSteps);
    setShown (Sampler, true);
    touchPattern();
    return true;
}

void TechMachineProcessor::clearSamplePads()
{
    samples.clear();
    dirty = true;
}
