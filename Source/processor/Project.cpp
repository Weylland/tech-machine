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
    activePart = 0;
    cuedPart = -1;
    for (auto& part : *partStore)
        for (auto& track : part) track.clear();
    setSong ({});
    songMode = false;
    for (auto& stored : sceneStored) stored = false;
    cuedScene = currentScene = -1;
    fillArmed = false;
    resetOrder();
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
        int current = 0;
        explicit Writer (TechMachineProcessor& owner) : p (owner) {}
        void setParam (const juce::String& id, float value) override { p.setParam (id, value); }
        void show (int type) override { p.setShown (type, true); }
        Track& track (int type) override { return p.partTrack (current, type); }
        void part (int index) override { current = juce::jlimit (0, numParts - 1, index); }
        void song (const std::vector<std::pair<int, int>>& entries) override
        {
            std::vector<SongEntry> list;
            for (const auto& [partIndex, repeats] : entries) list.push_back ({ partIndex, repeats });
            p.setSong (list);
            p.setSongMode (true);
        }
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

// One machine's pattern as XML attributes (and <Lock> children for its automation).
void TechMachineProcessor::writeTrackXml (juce::XmlElement& e, int type, const Track& track) const
{
    const auto& def = machineDef (type);
    e.setAttribute ("id", def.id);
    for (size_t r = 0; r < def.rows.size(); ++r)
    {
        e.setAttribute ("hits" + juce::String ((int) r), encodeBools (track.hits[r]));
        juce::String rolls;
        bool any = false;
        for (auto& v : track.rolls[r])
        {
            rolls << juce::String ((int) v.load());
            any = any || v.load() > 0;
        }
        if (any) e.setAttribute ("rolls" + juce::String ((int) r), rolls);
    }
    e.setAttribute ("accent", encodeBools (track.accent));
    e.setAttribute ("on", encodeBools (track.on));
    if (track.length.load() > 0) e.setAttribute ("length", track.length.load());
    if (track.speed.load() > 0) e.setAttribute ("speed", track.speed.load());
    e.setAttribute ("slide", encodeBools (track.slide));

    juce::StringArray notes, chords, ties;
    bool anyChord = false;
    for (auto& n : track.note) notes.add (juce::String (n.load()));
    for (int i = 0; i < maxSteps; ++i)
    {
        chords.add (juce::String::toHexString ((juce::int64) track.chord[(size_t) i].load()));
        ties.add (juce::String::toHexString ((juce::int64) track.tie[(size_t) i].load()));
        anyChord = anyChord || track.chord[(size_t) i].load() != 0;
    }
    e.setAttribute ("note", notes.joinIntoString (","));
    if (anyChord)
    {
        e.setAttribute ("chord", chords.joinIntoString (","));
        e.setAttribute ("tie", ties.joinIntoString (","));
    }

    for (size_t l = 0; l < (size_t) maxLockLanes; ++l)
    {
        const int target = track.lockTarget[l].load();
        if (target < 0) continue;
        juce::StringArray values;
        for (auto& v : track.lockValue[l]) values.add (v.load() >= 0.0f ? juce::String ((double) v.load(), 9) : juce::String ("-"));   // 9 digits: read back exactly
        auto* lock = e.createNewChildElement ("Lock");
        lock->setAttribute ("param", automation::paramId (type, target));
        lock->setAttribute ("values", values.joinIntoString (","));
    }
}

void TechMachineProcessor::readTrackXml (const juce::XmlElement& e, int type, Track& track)
{
    const auto& def = machineDef (type);
    track.clear();
    for (size_t r = 0; r < def.rows.size(); ++r)
    {
        decodeBools (e.getStringAttribute ("hits" + juce::String ((int) r)), track.hits[r]);
        const auto rolls = e.getStringAttribute ("rolls" + juce::String ((int) r));
        for (int s = 0; s < maxSteps && s < rolls.length(); ++s)
            track.rolls[r][(size_t) s] = (std::uint8_t) juce::jlimit (0, 4, (int) (rolls[s] - (juce::juce_wchar) 48));
    }
    decodeBools (e.getStringAttribute ("accent"), track.accent);
    decodeBools (e.getStringAttribute ("on"), track.on);
    track.length = juce::jlimit (0, maxSteps, e.getIntAttribute ("length", 0));
    track.speed = juce::jlimit (0, numTrackSpeeds - 1, e.getIntAttribute ("speed", 0));
    decodeBools (e.getStringAttribute ("slide"), track.slide);
    auto notes = juce::StringArray::fromTokens (e.getStringAttribute ("note"), ",", {});
    for (int i = 0; i < maxSteps; ++i)
        track.note[(size_t) i] = juce::jlimit (0, noteRange - 1, notes[i].getIntValue());
    for (auto* lock : e.getChildWithTagNameIterator ("Lock"))
    {
        const int target = automation::targetFor (type, lock->getStringAttribute ("param"));
        const int lane = target >= 0 ? track.laneFor (target, true) : -1;
        if (lane < 0) continue;
        const auto values = juce::StringArray::fromTokens (lock->getStringAttribute ("values"), ",", {});
        for (int s = 0; s < maxSteps && s < values.size(); ++s)
            track.lockValue[(size_t) lane][(size_t) s] = values[s] == "-" ? noLock : values[s].getFloatValue();
    }
    auto chords = juce::StringArray::fromTokens (e.getStringAttribute ("chord"), ",", {});
    auto ties = juce::StringArray::fromTokens (e.getStringAttribute ("tie"), ",", {});
    for (int i = 0; i < maxSteps && i < chords.size(); ++i)
        track.chord[(size_t) i] = (std::uint64_t) chords[i].getHexValue64() & ((1ull << noteRange) - 1);
    for (int i = 0; i < maxSteps && i < ties.size(); ++i)
        track.tie[(size_t) i] = (std::uint64_t) ties[i].getHexValue64() & ((1ull << noteRange) - 1);
}

// Part A is written as the machines of <Patterns> (as before the song mode), parts B to H as <Part> elements.
std::unique_ptr<juce::XmlElement> TechMachineProcessor::createProjectXml()
{
    auto xml = apvts.copyState().createXml();
    auto* patterns = xml->createNewChildElement ("Patterns");
    patterns->setAttribute ("genre", genreIndex);
    patterns->setAttribute ("name", name);
    juce::StringArray prog;
    for (int d : progression) prog.add (juce::String (d));
    patterns->setAttribute ("progression", prog.joinIntoString (","));
    patterns->setAttribute ("part", activePart.load());
    juce::StringArray orderIds;
    for (int t : order) orderIds.add (machineDef (t).id);
    patterns->setAttribute ("order", orderIds.joinIntoString (","));

    for (int t = 0; t < numMachines; ++t)
    {
        auto* e = patterns->createNewChildElement ("Machine");
        writeTrackXml (*e, t, partTrack (0, t));
        if (t == Sampler)
            for (int i = 0; i < numPads; ++i)
                if (const auto pad = samples.describe (i); pad.isNotEmpty()) e->setAttribute ("pad" + juce::String (i), pad);
        if (t == SampleKeys)
            if (const auto sound = samples.describe (keysSlot); sound.isNotEmpty()) e->setAttribute ("sample", sound);
    }
    for (int part = 1; part < numParts; ++part)
    {
        if (isPartEmpty (part)) continue;
        auto* p = patterns->createNewChildElement ("Part");
        p->setAttribute ("index", part);
        for (int t = 0; t < numMachines; ++t)
            if (! partTrack (part, t).isEmpty())
                writeTrackXml (*p->createNewChildElement ("Machine"), t, partTrack (part, t));
    }

    juce::StringArray entries;
    for (auto& entry : song()) entries.add (juce::String (entry.part) + "x" + juce::String (entry.repeats));
    if (! entries.isEmpty() || songMode.load())
    {
        auto* songXml = patterns->createNewChildElement ("Song");
        songXml->setAttribute ("entries", entries.joinIntoString (","));
        songXml->setAttribute ("on", songMode.load());
    }

    for (int scene = 0; scene < numScenes; ++scene)
    {
        if (! hasScene (scene)) continue;
        juce::StringArray cut;
        for (int t = 0; t < numMachines; ++t)
            if (sceneMutes[(size_t) scene][(size_t) t].load()) cut.add (machineDef (t).id);
        auto* e = patterns->createNewChildElement ("Scene");
        e->setAttribute ("index", scene);
        e->setAttribute ("cut", cut.joinIntoString (","));
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

    // The rack order: the machines the file names first, the others after in their usual place.
    {
        std::vector<int> list;
        for (auto& id : juce::StringArray::fromTokens (patterns->getStringAttribute ("order"), ",", {}))
            for (int t = 0; t < numMachines; ++t)
                if (id == machineDef (t).id && std::find (list.begin(), list.end(), t) == list.end()) list.push_back (t);
        for (int t : rackOrder)
            if (std::find (list.begin(), list.end(), t) == list.end()) list.push_back (t);
        std::copy (list.begin(), list.end(), order.begin());
        ++orderVersion;
    }

    // What the file does not mention starts empty (older files have fewer machines, no sampler and no parts).
    activePart = 0;
    cuedPart = -1;
    for (auto& m : machines) m->track.clear();
    for (auto& part : *partStore)
        for (auto& track : part) track.clear();
    samples.clear();

    auto machineOf = [] (const juce::XmlElement& e)
    {
        for (int t = 0; t < numMachines; ++t)
            if (e.getStringAttribute ("id") == machineDef (t).id) return t;
        return -1;
    };
    juce::StringArray sounds;
    for (int i = 0; i < techno::SampleBank::numSlots; ++i) sounds.add ({});
    for (auto* e : patterns->getChildWithTagNameIterator ("Machine"))
    {
        const int t = machineOf (*e);
        if (t < 0) continue;
        readTrackXml (*e, t, machines[(size_t) t]->track);
        if (t == Sampler)
            for (int i = 0; i < numPads; ++i) sounds.set (i, e->getStringAttribute ("pad" + juce::String (i)));
        if (t == SampleKeys) sounds.set (keysSlot, e->getStringAttribute ("sample"));
    }
    samples.restore (sounds);
    for (auto* p : patterns->getChildWithTagNameIterator ("Part"))
    {
        const int part = p->getIntAttribute ("index", -1);
        if (part <= 0 || part >= numParts) continue;
        for (auto* e : p->getChildWithTagNameIterator ("Machine"))
            if (const int t = machineOf (*e); t >= 0) readTrackXml (*e, t, (*partStore)[(size_t) part][(size_t) t]);
    }

    std::vector<SongEntry> entries;
    bool songOn = false;
    if (auto* songXml = patterns->getChildByName ("Song"))
    {
        for (auto& token : juce::StringArray::fromTokens (songXml->getStringAttribute ("entries"), ",", {}))
            entries.push_back ({ token.upToFirstOccurrenceOf ("x", false, false).getIntValue(),
                                 token.fromFirstOccurrenceOf ("x", false, false).getIntValue() });
        songOn = songXml->getBoolAttribute ("on");
    }
    setSong (entries);
    songMode = songOn && ! entries.empty();

    for (auto& stored : sceneStored) stored = false;
    cuedScene = currentScene = -1;
    for (auto* e : patterns->getChildWithTagNameIterator ("Scene"))
    {
        const int scene = e->getIntAttribute ("index", -1);
        if (scene < 0 || scene >= numScenes) continue;
        const auto cut = juce::StringArray::fromTokens (e->getStringAttribute ("cut"), ",", {});
        for (int t = 0; t < numMachines; ++t) sceneMutes[(size_t) scene][(size_t) t] = cut.contains (machineDef (t).id);
        sceneStored[(size_t) scene] = true;
    }
    songEntry = -1;
    switchPart (juce::jlimit (0, numParts - 1, patterns->getIntAttribute ("part", 0)));

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

    const double loopSeconds = offline.isSongMode() && offline.songLength.load() > 0
                                   ? offline.songSeconds()
                                   : offline.numSteps() * 15.0 / (double) offline.bpm->load();
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

void TechMachineProcessor::setSampleRange (int slot, int start, int end, int loopStart, int loopEnd)
{
    samples.setRange (slot, start, end, loopStart, loopEnd);
    dirty = true;
}

juce::File TechMachineProcessor::recordingsFolder()
{
    // The tests keep their recordings apart from the user's.
    const auto base = juce::SystemStats::getEnvironmentVariable ("TECHMACHINE_TEST_SETTINGS", {}).isNotEmpty()
                          ? juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("Tech Machine Tests")
                          : juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Tech Machine");
    return base.getChildFile ("Micro");
}

bool TechMachineProcessor::startSampling (int slot)
{
    if (! hasAudioInput() || slot < 0 || slot >= techno::SampleBank::numSlots) return false;
    sampling = false;
    samplingBuffer.assign ((size_t) (sampleRate * 30.0f), 0.0f);
    samplingCount = 0;
    samplingPeak = 0.0f;
    samplingSlot = slot;
    sampling = true;
    return true;
}

juce::File TechMachineProcessor::stopSampling()
{
    if (! sampling.exchange (false)) return {};
    while (samplingWrite.load()) juce::Thread::yield();   // the audio thread finishes its block

    // From the first sound heard to the end, brought up to a good level.
    const int count = samplingCount.load();
    float peak = 0.0f;
    for (int i = 0; i < count; ++i) peak = std::max (peak, std::abs (samplingBuffer[(size_t) i]));
    if (peak < 0.003f) return {};
    int first = 0;
    while (first < count && std::abs (samplingBuffer[(size_t) first]) < peak * 0.05f) ++first;
    first = juce::jmax (0, first - (int) (sampleRate * 0.01f));
    juce::AudioBuffer<float> audio (1, count - first);
    for (int i = first; i < count; ++i) audio.setSample (0, i - first, samplingBuffer[(size_t) i] * 0.9f / peak);

    const auto folder = recordingsFolder();
    folder.createDirectory();
    const auto recording = folder.getNonexistentChildFile ("Micro " + juce::Time::getCurrentTime().formatted ("%Y-%m-%d %H-%M-%S"), ".wav", false);
    {
        std::unique_ptr<juce::OutputStream> stream = recording.createOutputStream();
        if (stream == nullptr) return {};
        juce::WavAudioFormat wav;
        auto writer = wav.createWriterFor (stream, juce::AudioFormatWriterOptions {}.withSampleRate (sampleRate).withNumChannels (1).withBitsPerSample (24));
        if (writer == nullptr) return {};
        writer->writeFromAudioSampleBuffer (audio, 0, audio.getNumSamples());
    }
    samplingBuffer.clear();
    samplingBuffer.shrink_to_fit();
    return loadSamplePad (samplingSlot.load(), recording) ? recording : juce::File();
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
    samples.clearPads();
    dirty = true;
}
