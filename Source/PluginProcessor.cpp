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

    name = utf8 ("Sans titre");
    apvts.state.addListener (this);
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
    setLatencySamples (techno::PeakLimiter::lookahead - 1);

    reverb.prepare (sampleRate);
    lastReverbSize = -1.0f;
    juce::ignoreUnused (samplesPerBlock);

    muteCoef = 1.0f - decayCoef (0.006f, sampleRate);
}

void TechMachineProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
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
        for (auto& m : machines) m->stop();
        currentStep = -1;
    }
    wasPlaying = play;

    if (play && hostPpq.hasValue())
        syncToHost (*hostPpq);

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
}

//==============================================================================
// Undo of note edits: a snapshot of every part, taken just before the change.

class TechMachineProcessor::PatternAction : public juce::UndoableAction
{
public:
    explicit PatternAction (TechMachineProcessor& p) : processor (p), before (p.snapshot()) {}

    bool perform() override
    {
        if (! performed)
        {
            performed = true;
            return true;
        }
        processor.restore (after);
        return true;
    }

    bool undo() override
    {
        after = processor.snapshot();
        processor.restore (before);
        return true;
    }

    int getSizeInUnits() override { return 100; }

private:
    TechMachineProcessor& processor;
    Snapshot before, after;
    bool performed = false;
};

TechMachineProcessor::Snapshot TechMachineProcessor::snapshot() const
{
    Snapshot s;
    for (size_t t = 0; t < machines.size(); ++t) s[t] = machines[t]->track.snapshot();
    return s;
}

void TechMachineProcessor::restore (const Snapshot& s)
{
    for (size_t t = 0; t < machines.size(); ++t) machines[t]->track.restore (s[t]);
    touchPattern();
}

void TechMachineProcessor::beginPatternEdit()
{
    undoManager.perform (new PatternAction (*this));
    dirty = true;
}

//==============================================================================
// Generation

GenerateContext TechMachineProcessor::context() const
{
    GenerateContext ctx { styles()[(size_t) genreIndex], numSteps(), currentScale(), progression, {} };

    // Where the kick plays: taken from the drum machines in the rack, otherwise four on the floor.
    bool anyKick = false;
    for (int t = 0; t < numMachines; ++t)
    {
        const auto& def = machineDef (t);
        if (! isShown (t) || def.kind != MachineKind::Drum) continue;
        for (size_t r = 0; r < def.rows.size(); ++r)
        {
            if (def.rows[r].role != Role::Kick) continue;
            for (int s = 0; s < ctx.steps; ++s)
            {
                if (machines[(size_t) t]->track.hits[r][(size_t) s])
                {
                    ctx.kick[(size_t) s] = true;
                    anyKick = true;
                }
            }
        }
    }
    if (! anyKick)
        for (int s = 0; s < ctx.steps; s += 4) ctx.kick[(size_t) s] = true;
    return ctx;
}

void TechMachineProcessor::chooseProgression()
{
    const auto& options = styles()[(size_t) genreIndex].progressions;
    progression = options[(size_t) random.nextInt ((int) options.size())];
}

void TechMachineProcessor::setGenre (int index)
{
    genreIndex = juce::jlimit (0, (int) styles().size() - 1, index);
    chooseProgression();

    // On an empty project, the genre also brings its usual tempo, swing and scale.
    if (isProjectEmpty())
    {
        const auto& style = styles()[(size_t) genreIndex];
        setParam (pid::bpm, style.bpm);
        setParam (pid::swing, style.swing);
        setParam (pid::scale, (float) style.scale);
    }
    dirty = true;
}

void TechMachineProcessor::generate (int type, GenerateMode mode)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    techno::generate (machines[(size_t) type]->track, machineDef (type), context(), mode, random);
    touchPattern();
}

void TechMachineProcessor::generateAll()
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    chooseProgression();

    // Empty rack: set up the usual band of the genre, with its tempo, swing and scale, over 4 bars.
    bool anyShown = false;
    for (int t = 0; t < numMachines; ++t) anyShown = anyShown || isShown (t);
    if (! anyShown)
    {
        const auto& style = styles()[(size_t) genreIndex];
        setParam (pid::bpm, style.bpm);
        setParam (pid::swing, style.swing);
        setParam (pid::scale, (float) style.scale);
        setBars (4);
        for (auto& member : defaultBand (style))
        {
            setShown (member.machine, true);
            if (member.preset == nullptr) continue;
            for (auto& preset : presetsFor (member.machine))
                if (std::strcmp (preset.name, member.preset) == 0) applyPreset (member.machine, preset);
        }
    }

    // Drums first, so the bass lines know where the kicks are.
    for (int pass = 0; pass < 2; ++pass)
    {
        for (int t = 0; t < numMachines; ++t)
        {
            const bool drum = machineDef (t).kind == MachineKind::Drum;
            if (isShown (t) && drum == (pass == 0))
                techno::generate (machines[(size_t) t]->track, machineDef (t), context(), GenerateMode::NewIdea, random);
        }
    }
    touchPattern();
}

void TechMachineProcessor::clearMachine (int type)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    machines[(size_t) type]->track.clear();
    touchPattern();
}

void TechMachineProcessor::applyPattern (int type, const Pattern& pattern)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    techno::applyPattern (machines[(size_t) type]->track, machineDef (type), context(), pattern);
    touchPattern();
}

juce::RangedAudioParameter* TechMachineProcessor::presetParameter (int type, const char* key)
{
    juce::String k (key);
    if (k.startsWithChar ('@')) return apvts.getParameter (pid::machine (type, k.substring (1).toRawUTF8()));
    return apvts.getParameter (juce::String (machineDef (type).id) + "_" + k.replaceCharacter ('.', '_'));
}

void TechMachineProcessor::applyPreset (int type, const SoundPreset& preset)
{
    const auto& def = machineDef (type);
    auto reset = [this] (const juce::String& id)
    {
        if (auto* p = apvts.getParameter (id)) p->setValueNotifyingHost (p->getDefaultValue());
    };

    // Back to the factory sound first, so a preset only has to list what it changes.
    for (const char* fx : { "drive", "crush", "filter", "echo", "reverb" }) reset (pid::machine (type, fx));
    if (def.kind == MachineKind::Drum)
    {
        for (int r = 0; r < (int) def.rows.size(); ++r)
            for (const char* p : { "level", "tune", "decay", "tone", "drive", "filter", "pan", "echo", "reverb" })
                reset (pid::row (type, r, p));
    }
    else
    {
        for (int i = 0; i < (int) def.params.size(); ++i) reset (pid::synth (type, i));
    }

    for (auto& [key, value] : preset.values)
        if (auto* p = presetParameter (type, key))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
    dirty = true;
}

void TechMachineProcessor::shiftMachine (int type, int direction)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    shiftTrack (machines[(size_t) type]->track, numSteps(), direction);
    touchPattern();
}

void TechMachineProcessor::duplicateFirstBar (int type)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    machines[(size_t) type]->track.tile (pageSteps);
    touchPattern();
}

void TechMachineProcessor::copyMachine (int type)
{
    clipboard = machines[(size_t) type]->track.snapshot();
    clipboardKind = type;
}

bool TechMachineProcessor::canPaste (int type) const
{
    return clipboard.has_value() && clipboardKind == type;
}

void TechMachineProcessor::pasteMachine (int type)
{
    if (! canPaste (type)) return;
    undoManager.beginNewTransaction();
    beginPatternEdit();
    machines[(size_t) type]->track.restore (*clipboard);
    touchPattern();
}

bool TechMachineProcessor::isProjectEmpty() const
{
    for (auto& m : machines)
        if (! m->track.isEmpty()) return false;
    return true;
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

        juce::StringArray notes;
        for (auto& n : m->track.note) notes.add (juce::String (n.load()));
        e->setAttribute ("note", notes.joinIntoString (","));
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

    for (auto* e : patterns->getChildIterator())
    {
        for (auto& m : machines)
        {
            if (e->getStringAttribute ("id") != m->def.id) continue;
            m->track.clear();
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
