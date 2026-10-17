#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace techno;

namespace
{
using Attributes = juce::AudioParameterFloatAttributes;

juce::String percentText (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + "%"; }

juce::String filterText (float v, int)
{
    if (std::abs (v - 0.5f) < 0.02f) return "OFF";
    return (v < 0.5f ? "LP " : "HP ") + juce::String (juce::roundToInt (std::abs (v - 0.5f) * 200.0f)) + "%";
}

juce::String panText (float v, int)
{
    if (std::abs (v - 0.5f) < 0.02f) return "Centre";
    return (v < 0.5f ? "G " : "D ") + juce::String (juce::roundToInt (std::abs (v - 0.5f) * 200.0f));
}

std::unique_ptr<juce::RangedAudioParameter> knob (const juce::String& id, const juce::String& name, float def,
                                                   juce::String (*text) (float, int) = percentText)
{
    return std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 }, name, juce::NormalisableRange<float> (0.0f, 1.0f),
                                                        def, Attributes().withStringFromValueFunction (text));
}

std::unique_ptr<juce::RangedAudioParameter> toggle (const juce::String& id, const juce::String& name, bool def = false)
{
    return std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, 1 }, name, def);
}

std::unique_ptr<juce::RangedAudioParameter> choice (const juce::String& id, const juce::String& name,
                                                     const std::vector<const char*>& items, int def)
{
    juce::StringArray names;
    for (auto* item : items) names.add (utf8 (item));
    return std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id, 1 }, name, names, def);
}

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

TechMachineProcessor::TechMachineProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "TechMachine", createLayout())
{
    auto raw = [this] (const juce::String& id) { return apvts.getRawParameterValue (id); };

    bpm = raw (pid::bpm);
    swing = raw (pid::swing);
    keyParam = raw (pid::key);
    scaleParam = raw (pid::scale);
    bars = raw (pid::bars);
    echoTime = raw (pid::echoTime);
    echoFeedback = raw (pid::echoFeedback);
    echoLevel = raw (pid::echoLevel);
    reverbSize = raw (pid::reverbSize);
    reverbLevel = raw (pid::reverbLevel);
    pumpParam = raw (pid::pump);
    djFilterParam = raw (pid::djFilter);
    comp = raw (pid::comp);
    disto = raw (pid::disto);
    volume = raw (pid::volume);

    for (int t = 0; t < numMachines; ++t)
    {
        machines.push_back (createMachine (t));
        machines.back()->bind (apvts);

        auto& s = strips[(size_t) t];
        s.show = raw (pid::machine (t, "show"));
        s.mute = raw (pid::machine (t, "mute"));
        s.solo = raw (pid::machine (t, "solo"));
        s.drive = raw (pid::machine (t, "drive"));
        s.crush = raw (pid::machine (t, "crush"));
        s.filter = raw (pid::machine (t, "filter"));
        s.echo = raw (pid::machine (t, "echo"));
        s.reverb = raw (pid::machine (t, "reverb"));
        s.volume = raw (pid::machine (t, "volume"));
        s.pan = raw (pid::machine (t, "pan"));
    }

    applyStyle (0);
}

TechMachineProcessor::~TechMachineProcessor()
{
    stopRecording();
}

juce::AudioProcessorValueTreeState::ParameterLayout TechMachineProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { pid::bpm, 1 }, "Tempo", juce::NormalisableRange<float> (60.0f, 200.0f, 1.0f), 130.0f,
        Attributes().withStringFromValueFunction ([] (float v, int) { return juce::String (juce::roundToInt (v)) + " BPM"; })));
    layout.add (knob (pid::swing, "Swing", 0.05f));
    layout.add (choice (pid::key, "Key", { std::begin (keyNames), std::end (keyNames) }, 9));

    std::vector<const char*> scaleNames;
    for (auto& s : scales()) scaleNames.push_back (s.name);
    layout.add (choice (pid::scale, "Scale", scaleNames, 1));
    layout.add (choice (pid::bars, "Bars", { "1", "2", "3", "4" }, 0));

    layout.add (choice (pid::echoTime, "Echo time", { std::begin (echoTimeNames), std::end (echoTimeNames) }, 2));
    layout.add (knob (pid::echoFeedback, "Echo feedback", 0.45f));
    layout.add (knob (pid::echoLevel, "Echo level", 0.7f));
    layout.add (knob (pid::reverbSize, "Reverb size", 0.6f));
    layout.add (knob (pid::reverbLevel, "Reverb level", 0.6f));
    layout.add (knob (pid::pump, "Pump", 0.3f));
    layout.add (knob (pid::djFilter, "Master filter", 0.5f, filterText));
    layout.add (knob (pid::comp, "Compressor", 0.25f));
    layout.add (knob (pid::disto, "Distortion", 0.0f));
    layout.add (knob (pid::volume, "Volume", 0.8f));

    for (int t = 0; t < numMachines; ++t)
    {
        const auto& def = machineDef (t);
        const juce::String name = utf8 (def.name);
        const bool drum = def.kind == MachineKind::Drum;

        layout.add (toggle (pid::machine (t, "show"), name + " visible", t == TR909 || t == TB303 || t == Juno106));
        layout.add (toggle (pid::machine (t, "mute"), name + " mute"));
        layout.add (toggle (pid::machine (t, "solo"), name + " solo"));
        layout.add (knob (pid::machine (t, "drive"), name + " drive", 0.0f));
        layout.add (knob (pid::machine (t, "crush"), name + " crush", t == BreakKit ? 0.25f : 0.0f));
        layout.add (knob (pid::machine (t, "filter"), name + " filter", 0.5f, filterText));
        layout.add (knob (pid::machine (t, "echo"), name + " echo", drum ? 0.0f : 0.15f));
        layout.add (knob (pid::machine (t, "reverb"), name + " reverb", drum ? 0.08f : 0.15f));
        layout.add (knob (pid::machine (t, "volume"), name + " volume", 0.75f));
        layout.add (knob (pid::machine (t, "pan"), name + " pan", 0.5f, panText));

        if (drum)
        {
            layout.add (knob (pid::machine (t, "accent"), name + " accent", 0.5f));
            for (int r = 0; r < (int) def.rows.size(); ++r)
            {
                const auto& row = def.rows[(size_t) r];
                const juce::String rowName = name + " " + utf8 (row.label);
                layout.add (knob (pid::row (t, r, "level"), rowName + " level", row.level));
                layout.add (knob (pid::row (t, r, "tune"), rowName + " tune", row.tune));
                layout.add (knob (pid::row (t, r, "decay"), rowName + " decay", row.decay));
                layout.add (knob (pid::row (t, r, "tone"), rowName + " tone", row.tone));
                layout.add (toggle (pid::row (t, r, "mute"), rowName + " mute"));
            }
        }
        else
        {
            for (int i = 0; i < (int) def.params.size(); ++i)
            {
                const auto& p = def.params[(size_t) i];
                const juce::String paramName = name + " " + utf8 (p.label);
                if (p.choices.empty())
                    layout.add (knob (pid::synth (t, i), paramName, p.def));
                else
                    layout.add (choice (pid::synth (t, i), paramName, p.choices, (int) p.def));
            }
        }
    }

    return layout;
}

bool TechMachineProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void TechMachineProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    sampleRate = (float) newSampleRate;

    for (auto& m : machines) m->prepare (sampleRate);
    delay.prepare (sampleRate);
    pump.prepare (sampleRate);
    compressor.prepare (sampleRate);
    limiter.prepare (sampleRate);

    reverbBuffer.setSize (2, juce::jmax (samplesPerBlock, 512));
    reverb.setSampleRate (newSampleRate);
    reverb.reset();
    lastReverbSize = -1.0f;

    muteCoef = 1.0f - decayCoef (0.006f, sampleRate);
}

void TechMachineProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    bpmValue = bpm->load();
    lengthSteps = numSteps();

    const bool play = playing.load();
    if (play && ! wasPlaying)
    {
        stepIndex = -1;
        samplesToNextStep = 0.0;
    }
    if (! play && wasPlaying)
    {
        for (auto& m : machines) m->stop();
        currentStep = -1;
    }
    wasPlaying = play;

    bool anySolo = false;
    for (auto& s : strips)
        anySolo = anySolo || (s.show->load() > 0.5f && s.solo->load() > 0.5f);

    for (int t = 0; t < numMachines; ++t)
    {
        auto& s = strips[(size_t) t];
        s.shown = s.show->load() > 0.5f;
        if (! s.shown)
        {
            s.gain = 0.0f;
            continue;
        }

        s.audible = s.mute->load() < 0.5f && (! anySolo || s.solo->load() > 0.5f);
        const float vol = s.volume->load();
        s.target = s.audible ? vol * vol * 1.6f : 0.0f;
        const float angle = s.pan->load() * juce::MathConstants<float>::halfPi;
        s.panLeft = std::cos (angle) * juce::MathConstants<float>::sqrt2;
        s.panRight = std::sin (angle) * juce::MathConstants<float>::sqrt2;
        s.driveAmount = s.drive->load();
        s.crushAmount = s.crush->load();
        s.echoSend = s.echo->load();
        s.reverbSend = s.reverb->load();
        s.djFilter.set (s.filter->load(), sampleRate);
        machines[(size_t) t]->readParams();
    }

    feedbackValue = echoFeedback->load() * 0.85f;
    echoLevelValue = echoLevel->load();
    reverbLevelValue = reverbLevel->load() * 1.2f;
    pumpDepth = pumpParam->load();
    distoValue = disto->load();
    volumeValue = volume->load() * volume->load() * 1.2f;
    delay.setTime (stepSeconds() * echoTimeSteps[juce::jlimit (0, 4, (int) echoTime->load())]);
    masterFilter.set (djFilterParam->load(), sampleRate);
    compressor.setAmount (comp->load());

    if (const float size = reverbSize->load(); std::abs (size - lastReverbSize) > 0.001f)
    {
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.3f + size * 0.68f;
        rp.damping = 0.5f - size * 0.2f;
        rp.wetLevel = 1.0f;
        rp.dryLevel = 0.0f;
        rp.width = 1.0f;
        reverb.setParameters (rp);
        lastReverbSize = size;
    }

    const int numSamples = buffer.getNumSamples();
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getWritePointer (1);
    const int chunk = reverbBuffer.getNumSamples();
    for (int pos = 0; pos < numSamples; pos += chunk)
        render (left + pos, right + pos, juce::jmin (chunk, numSamples - pos));

    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        float l = left[i], r = right[i];
        masterFilter.process (l, r);
        compressor.process (l, r);
        l += distoValue * (fastTanh (l * 3.0f) - l);
        r += distoValue * (fastTanh (r * 3.0f) - r);
        l *= volumeValue;
        r *= volumeValue;
        limiter.process (l, r);
        left[i] = l;
        right[i] = r;
        peak = std::max (peak, std::max (std::abs (l), std::abs (r)));
    }
    outputPeak = std::max (outputPeak.load(), peak);

    const juce::ScopedLock sl (writerLock);
    if (auto* writer = activeWriter.load())
        writer->write (buffer.getArrayOfReadPointers(), numSamples);
}

void TechMachineProcessor::render (float* left, float* right, int numSamples)
{
    auto* revL = reverbBuffer.getWritePointer (0);
    auto* revR = reverbBuffer.getWritePointer (1);

    for (int i = 0; i < numSamples; ++i)
    {
        if (wasPlaying)
        {
            if (samplesToNextStep <= 0.0) advanceStep();
            samplesToNextStep -= 1.0;
            for (int t = 0; t < numMachines; ++t)
                if (strips[(size_t) t].shown) machines[(size_t) t]->tick();
        }

        const float duck = pump.process (pumpDepth);
        float sumL = 0, sumR = 0, echoIn = 0, reverbIn = 0;

        for (int t = 0; t < numMachines; ++t)
        {
            auto& s = strips[(size_t) t];
            if (! s.shown) continue;

            auto& m = *machines[(size_t) t];
            float l, r;
            m.render (l, r);

            if (s.driveAmount > 0.001f)
            {
                const float g = 1.0f + s.driveAmount * 10.0f;
                l += s.driveAmount * (fastTanh (l * g) * 0.8f - l);
                r += s.driveAmount * (fastTanh (r * g) * 0.8f - r);
            }
            s.crusher.process (s.crushAmount, sampleRate, l, r);
            s.djFilter.process (l, r);

            s.gain += (s.target - s.gain) * muteCoef;
            l *= s.gain * s.panLeft;
            r *= s.gain * s.panRight;
            if (m.def.kind != MachineKind::Drum)
            {
                l *= duck;
                r *= duck;
            }

            sumL += l;
            sumR += r;
            const float mono = 0.5f * (l + r);
            echoIn += mono * s.echoSend;
            reverbIn += mono * s.reverbSend;
        }

        float dl, dr;
        delay.process (echoIn, feedbackValue, dl, dr);
        dl *= echoLevelValue;
        dr *= echoLevelValue;

        revL[i] = revR[i] = reverbIn + 0.15f * (dl + dr);
        left[i] = sumL + dl;
        right[i] = sumR + dr;
    }

    reverb.processStereo (revL, revR, numSamples);
    for (int i = 0; i < numSamples; ++i)
    {
        left[i] += revL[i] * reverbLevelValue;
        right[i] += revR[i] * reverbLevelValue;
    }
}

void TechMachineProcessor::advanceStep()
{
    stepIndex = (stepIndex + 1) % lengthSteps;
    currentStep = stepIndex;

    const double nominal = sampleRate * stepSeconds();
    const double sw = swing->load() * 0.33;
    const double thisStep = stepIndex % 2 == 0 ? nominal * (1.0 + sw) : nominal * (1.0 - sw);
    samplesToNextStep += thisStep;

    const StepInfo info { stepIndex, (stepIndex + lengthSteps - 1) % lengthSteps, currentKey(), currentScale(), thisStep, nominal };
    for (int t = 0; t < numMachines; ++t)
    {
        if (! strips[(size_t) t].shown) continue;
        machines[(size_t) t]->onStep (info);
        if (machines[(size_t) t]->takeKick() && strips[(size_t) t].audible)
            pump.trigger (stepSeconds() * 4.0f);
    }
}

void TechMachineProcessor::setParam (const juce::String& id, float value)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (value));
    else
        jassertfalse;
}

void TechMachineProcessor::setShown (int type, bool shown)
{
    setParam (pid::machine (type, "show"), shown ? 1.0f : 0.0f);
    if (! shown) return;

    auto& track = machines[(size_t) type]->track;
    bool empty = true;
    for (int s = 0; s < maxSteps && empty; ++s)
    {
        empty = ! track.on[(size_t) s];
        for (auto& row : track.hits) empty = empty && ! row[(size_t) s];
    }
    if (empty) regenerate (type);
}

void TechMachineProcessor::applyStyle (int newStyle)
{
    styleIndex = juce::jlimit (0, (int) styles().size() - 1, newStyle);
    const auto& style = styles()[(size_t) styleIndex];

    for (auto* param : getParameters())
    {
        auto* p = dynamic_cast<juce::RangedAudioParameter*> (param);
        if (p == nullptr) continue;
        const auto& id = p->paramID;
        if (id.endsWith ("_show") || id.endsWith ("_mute") || id.endsWith ("_solo") || id == pid::key || id == pid::bars
            || id == pid::volume)
            continue;
        p->setValueNotifyingHost (p->getDefaultValue());
    }

    setParam (pid::bpm, style.bpm);
    setParam (pid::swing, style.swing);
    setParam (pid::scale, (float) style.scale);
    for (auto& [id, value] : style.presets)
        setParam (utf8 (id), value);

    regenerateAll();
}

void TechMachineProcessor::regenerate (int type)
{
    auto& m = *machines[(size_t) type];
    const int steps = numSteps();
    generate (m.track, m.def, styles()[(size_t) styleIndex], steps, currentScale(), random);
    m.track.tile (steps);
    touchPattern();
}

void TechMachineProcessor::regenerateAll()
{
    for (int t = 0; t < numMachines; ++t)
        if (isShown (t)) regenerate (t);
}

void TechMachineProcessor::addRecommendedMachines()
{
    for (int t : styles()[(size_t) styleIndex].recommended)
    {
        if (isShown (t)) continue;
        setParam (pid::machine (t, "show"), 1.0f);
        regenerate (t);
    }
}

void TechMachineProcessor::clearMachine (int type)
{
    machines[(size_t) type]->track.clear();
    touchPattern();
}

void TechMachineProcessor::setBars (int newBars)
{
    const int oldSteps = numSteps();
    const int newSteps = newBars * pageSteps;
    if (newSteps > oldSteps)
        for (auto& m : machines) m->track.tile (oldSteps);
    setParam (pid::bars, (float) (newBars - 1));
    touchPattern();
}

void TechMachineProcessor::startRecording()
{
    stopRecording();

    auto folder = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Tech Machine");
    folder.createDirectory();
    auto file = folder.getNonexistentChildFile ("TechMachine_" + juce::Time::getCurrentTime().formatted ("%Y-%m-%d_%H-%M-%S"), ".wav");

    std::unique_ptr<juce::OutputStream> stream = file.createOutputStream();
    if (stream == nullptr) return;

    juce::WavAudioFormat wav;
    auto writer = wav.createWriterFor (stream, juce::AudioFormatWriterOptions {}
                                                   .withSampleRate (sampleRate)
                                                   .withNumChannels (2)
                                                   .withBitsPerSample (24));
    if (writer == nullptr) return;

    writerThread.startThread();
    threadedWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter> (writer.release(), writerThread, 32768);
    lastRecording = file;

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

void TechMachineProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto xml = apvts.copyState().createXml();
    auto* patterns = xml->createNewChildElement ("Patterns");
    patterns->setAttribute ("style", styleIndex);

    for (auto& m : machines)
    {
        auto* e = patterns->createNewChildElement ("Machine");
        e->setAttribute ("id", m->def.id);
        for (size_t r = 0; r < m->def.rows.size(); ++r)
            e->setAttribute ("hits" + juce::String ((int) r), encodeBools (m->track.hits[r]));
        e->setAttribute ("accent", encodeBools (m->track.accent));
        e->setAttribute ("on", encodeBools (m->track.on));
        e->setAttribute ("slide", encodeBools (m->track.slide));

        juce::StringArray notes;
        for (auto& n : m->track.note) notes.add (juce::String (n.load()));
        e->setAttribute ("note", notes.joinIntoString (","));
    }

    copyXmlToBinary (*xml, destData);
}

void TechMachineProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType())) return;

    // States saved by v0.1 (no "Patterns" element) use other parameter meanings: start fresh instead.
    if (xml->getChildByName ("Patterns") == nullptr) return;

    if (auto* patterns = xml->getChildByName ("Patterns"))
    {
        styleIndex = juce::jlimit (0, (int) styles().size() - 1, patterns->getIntAttribute ("style"));
        for (auto* e : patterns->getChildIterator())
        {
            for (auto& m : machines)
            {
                if (e->getStringAttribute ("id") != m->def.id) continue;
                m->track.clear();
                for (size_t r = 0; r < m->def.rows.size(); ++r)
                    decodeBools (e->getStringAttribute ("hits" + juce::String ((int) r)), m->track.hits[r]);
                decodeBools (e->getStringAttribute ("accent"), m->track.accent);
                decodeBools (e->getStringAttribute ("on"), m->track.on);
                decodeBools (e->getStringAttribute ("slide"), m->track.slide);
                auto notes = juce::StringArray::fromTokens (e->getStringAttribute ("note"), ",", {});
                for (int i = 0; i < maxSteps; ++i)
                    m->track.note[(size_t) i] = juce::jlimit (0, noteRange - 1, notes[i].getIntValue());
            }
        }
        xml->removeChildElement (patterns, true);
        touchPattern();
    }

    apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* TechMachineProcessor::createEditor()
{
    return new TechMachineEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TechMachineProcessor();
}
