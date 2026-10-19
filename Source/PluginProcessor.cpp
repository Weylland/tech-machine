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
    if (std::abs (v - 0.5f) < 0.02f) return techno::utf8 ("Centre");
    return techno::utf8 (v < 0.5f ? "G " : "D ") + juce::String (juce::roundToInt (std::abs (v - 0.5f) * 200.0f));
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
} // namespace

TechMachineProcessor::TechMachineProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "TechMachine", createLayout())
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

    for (int t = 0; t < numMachines; ++t)
        for (int target = 0; target < automation::targetCount (t); ++target)
            automationIndex[automation::paramId (t, target)] = { t, target };

    name = utf8 ("Sans titre");
    apvts.state.addListener (this);
    for (auto& r : recordStart) r = -1;
    for (int i = 0; i < numPads; ++i) machines[(size_t) Sampler]->setSampleSlot (i, samples.slot (i));
}

TechMachineProcessor::~TechMachineProcessor()
{
    apvts.state.removeListener (this);
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
    layout.add (knob (pid::pump, "Pump", 0.0f));
    layout.add (knob (pid::djFilter, "Master filter", 0.5f, filterText));
    layout.add (knob (pid::comp, "Compressor", 0.0f));
    layout.add (knob (pid::disto, "Distortion", 0.0f));
    layout.add (knob (pid::volume, "Volume", 0.8f));

    for (int t = 0; t < numMachines; ++t)
    {
        const auto& def = machineDef (t);
        const juce::String name = utf8 (def.name);
        const bool drum = def.kind == MachineKind::Drum;

        layout.add (toggle (pid::machine (t, "show"), name + " visible"));
        layout.add (toggle (pid::machine (t, "mute"), name + " mute"));
        layout.add (toggle (pid::machine (t, "solo"), name + " solo"));
        layout.add (knob (pid::machine (t, "drive"), name + " drive", 0.0f));
        layout.add (knob (pid::machine (t, "crush"), name + " crush", 0.0f));
        layout.add (knob (pid::machine (t, "filter"), name + " filter", 0.5f, filterText));
        layout.add (knob (pid::machine (t, "echo"), name + " echo", 0.0f));
        layout.add (knob (pid::machine (t, "reverb"), name + " reverb", 0.0f));
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
                layout.add (knob (pid::row (t, r, "drive"), rowName + " drive", 0.0f));
                layout.add (knob (pid::row (t, r, "filter"), rowName + " filter", 0.5f, filterText));
                layout.add (knob (pid::row (t, r, "pan"), rowName + " pan", 0.5f, panText));
                layout.add (knob (pid::row (t, r, "echo"), rowName + " echo", 0.0f));
                layout.add (knob (pid::row (t, r, "reverb"), rowName + " reverb", 0.0f));
            }
        }

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
    setLatencySamples (techno::PeakLimiter::lookahead - 1);

    reverb.prepare (sampleRate);
    lastReverbSize = -1.0f;
    juce::ignoreUnused (samplesPerBlock);

    muteCoef = 1.0f - decayCoef (0.006f, sampleRate);
}

void TechMachineProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    bpmValue = bpm->load();
    lengthSteps = numSteps();

    // As a plugin, tempo, start/stop and position come from the host.
    bool play = playing.load();
    juce::Optional<double> hostPpq;
    if (isHosted())
    {
        play = false;
        if (auto* head = getPlayHead())
        {
            if (auto position = head->getPosition())
            {
                if (auto hostBpm = position->getBpm()) bpmValue = juce::jlimit (20.0f, 400.0f, (float) *hostBpm);
                hostPpq = position->getPpqPosition();
                play = position->getIsPlaying() && hostPpq.hasValue() && *hostPpq >= 0.0;
            }
        }
        playing = play;
    }
    displayBpm = bpmValue;

    if (play && ! wasPlaying)
    {
        stepIndex = -1;
        samplesToNextStep = 0.0;
    }
    if (! play && wasPlaying)
    {
        for (auto& m : machines)
        {
            m->stop();
            m->releaseLocks();
        }
        currentStep = -1;
    }
    wasPlaying = play;

    if (play && hostPpq.hasValue())
        syncToHost (*hostPpq);

    {
        int start1, size1, start2, size2;
        previewFifo.prepareToRead (previewFifo.getNumReady(), start1, size1, start2, size2);
        auto playPreview = [this] (const PreviewNote& n)
        {
            if (n.on) machines[(size_t) n.type]->liveNoteOn (n.note, 0.8f);
            else machines[(size_t) n.type]->liveNoteOff (n.note);
        };
        for (int i = 0; i < size1; ++i) playPreview (previewQueue[(size_t) (start1 + i)]);
        for (int i = 0; i < size2; ++i) playPreview (previewQueue[(size_t) (start2 + i)]);
        previewFifo.finishedRead (size1 + size2);
    }
    handleMidi (midi);
    midi.clear();

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
        readStrip (t);
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
        reverb.setSize (size);
        lastReverbSize = size;
    }

    const int numSamples = buffer.getNumSamples();
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getWritePointer (1);
    render (left, right, numSamples);

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
        left[i] = std::isfinite (l) ? juce::jlimit (-1.0f, 1.0f, l) : 0.0f;
        right[i] = std::isfinite (r) ? juce::jlimit (-1.0f, 1.0f, r) : 0.0f;
        peak = std::max (peak, std::max (std::abs (l), std::abs (r)));
    }
    outputPeak = std::max (outputPeak.load(), peak);

    const juce::ScopedLock sl (writerLock);
    if (auto* writer = activeWriter.load())
        writer->write (buffer.getArrayOfReadPointers(), numSamples);
}

// The effects of a machine's strip, as they sound now (automation included).
void TechMachineProcessor::readStrip (int t)
{
    using namespace automation;
    auto& s = strips[(size_t) t];
    const auto& m = *machines[(size_t) t];
    const float vol = m.setting (Volume, s.volume->load());
    s.target = s.audible ? vol * vol * 1.6f : 0.0f;
    const float angle = m.setting (Pan, s.pan->load()) * juce::MathConstants<float>::halfPi;
    s.panLeft = std::cos (angle) * juce::MathConstants<float>::sqrt2;
    s.panRight = std::sin (angle) * juce::MathConstants<float>::sqrt2;
    s.driveAmount = m.setting (Drive, s.drive->load());
    s.crushAmount = m.setting (Crush, s.crush->load());
    s.echoSend = m.setting (Echo, s.echo->load());
    s.reverbSend = m.setting (Reverb, s.reverb->load());
    s.djFilter.set (m.setting (Filter, s.filter->load()), sampleRate);
}

void TechMachineProcessor::render (float* left, float* right, int numSamples)
{
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
            Frame frame;
            m.render (frame);
            float l = frame.left, r = frame.right;

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
            echoIn += mono * s.echoSend + frame.echo * s.gain;
            reverbIn += mono * s.reverbSend + frame.reverb * s.gain;
        }

        float dl, dr;
        delay.process (echoIn, feedbackValue, dl, dr);
        dl *= echoLevelValue;
        dr *= echoLevelValue;

        float rl, rr;
        reverb.process (reverbIn + 0.15f * (dl + dr), rl, rr);
        left[i] = sumL + dl + rl * reverbLevelValue;
        right[i] = sumR + dr + rr * reverbLevelValue;
    }
}

// Aligns the step clock on the host position (in quarter notes) at the start of a block.
// Steps are sixteenths; with swing the first step of each pair is longer, as in advanceStep().
void TechMachineProcessor::syncToHost (double ppq)
{
    const double sw = swing->load() * 0.33;
    const double sixteenths = ppq * 4.0;
    const double pair = std::floor (sixteenths / 2.0);
    const double inPair = sixteenths - pair * 2.0;
    const bool secondOfPair = inPair >= 1.0 + sw;
    const double sinceStart = secondOfPair ? inPair - (1.0 + sw) : inPair;
    const double untilNext = secondOfPair ? 2.0 - inPair : (1.0 + sw) - inPair;
    const double samplesPerStep = sampleRate * stepSeconds();
    const int hostStep = (int) (((long long) pair * 2 + (secondOfPair ? 1 : 0)) % lengthSteps);

    if (hostStep == stepIndex)
    {
        samplesToNextStep = untilNext * samplesPerStep;
    }
    else if (sinceStart * samplesPerStep < 1.0)
    {
        // A step starts right now: let render() trigger it on the first sample.
        stepIndex = (hostStep + lengthSteps - 1) % lengthSteps;
        samplesToNextStep = 0.0;
    }
    else
    {
        // Started or jumped in the middle of a step: wait for the next one instead of playing late.
        stepIndex = hostStep;
        currentStep = hostStep;
        samplesToNextStep = untilNext * samplesPerStep;
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
        auto& m = *machines[(size_t) t];
        if (m.applyLocks (stepIndex))
        {
            m.readParams();
            readStrip (t);
        }
        m.onStep (info);
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
}

void TechMachineProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary (*createProjectXml(), destData);
}

void TechMachineProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType())) return;

    // States saved by v0.1 (no "Patterns" element) use other parameter meanings: start fresh instead.
    if (xml->getChildByName ("Patterns") == nullptr) return;

    restoreProjectXml (*xml);
    undoManager.clearUndoHistory();
    dirty = false;
}

juce::AudioProcessorEditor* TechMachineProcessor::createEditor()
{
    return new TechMachineEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TechMachineProcessor();
}
