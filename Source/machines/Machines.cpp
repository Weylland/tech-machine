#include "Machines.h"
#include "../core/Styles.h"
#include "../dsp/Effects.h"

namespace techno
{
namespace
{
using APVTS = juce::AudioProcessorValueTreeState;

class DrumMachine : public Machine
{
public:
    explicit DrumMachine (int machineType) : Machine (machineType)
    {
        numRows = (int) def.rows.size();
        for (int i = 0; i < numRows; ++i)
        {
            voices.push_back (createDrumVoice (def.id, def.rows[(size_t) i].id));
            trims[i] = drumVoiceTrim (def.id, def.rows[(size_t) i].id);
            if (def.rows[(size_t) i].role == Role::OpenHat) openHatRow = i;
        }
    }

    void bind (APVTS& state) override
    {
        for (int i = 0; i < numRows; ++i)
        {
            levelParam[i] = state.getRawParameterValue (pid::row (type, i, "level"));
            tuneParam[i] = state.getRawParameterValue (pid::row (type, i, "tune"));
            decayParam[i] = state.getRawParameterValue (pid::row (type, i, "decay"));
            toneParam[i] = state.getRawParameterValue (pid::row (type, i, "tone"));
            muteParam[i] = state.getRawParameterValue (pid::row (type, i, "mute"));
            auto& fx = rowFx[i];
            fx.drive = state.getRawParameterValue (pid::row (type, i, "drive"));
            fx.filter = state.getRawParameterValue (pid::row (type, i, "filter"));
            fx.pan = state.getRawParameterValue (pid::row (type, i, "pan"));
            fx.echo = state.getRawParameterValue (pid::row (type, i, "echo"));
            fx.reverb = state.getRawParameterValue (pid::row (type, i, "reverb"));
        }
        accentParam = state.getRawParameterValue (pid::machine (type, "accent"));
    }

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        for (auto& v : voices) v->prepare (sampleRate);
    }

    void readParams() override
    {
        for (int i = 0; i < numRows; ++i)
        {
            settings[i] = { tuneParam[i]->load(), decayParam[i]->load(), toneParam[i]->load() };
            const float level = levelParam[i]->load();
            levels[i] = level * 1.25f * trims[i];
            mutes[i] = muteParam[i]->load() > 0.5f;

            auto& fx = rowFx[i];
            fx.driveAmount = fx.drive->load();
            fx.echoSend = fx.echo->load();
            fx.reverbSend = fx.reverb->load();
            const float angle = fx.pan->load() * juce::MathConstants<float>::halfPi;
            fx.panLeft = std::cos (angle) * juce::MathConstants<float>::sqrt2;
            fx.panRight = std::sin (angle) * juce::MathConstants<float>::sqrt2;
            fx.dsp.set (fx.filter->load(), fs);
        }
        accentAmount = accentParam->load();
    }

    void onStep (const StepInfo& s) override
    {
        const auto st = (size_t) s.step;
        const float base = track.accent[st] ? 1.0f : 1.0f - 0.5f * accentAmount;

        for (int i = 0; i < numRows; ++i)
        {
            if (! track.hits[(size_t) i][st] || mutes[i]) continue;

            const Role role = def.rows[(size_t) i].role;
            float velocity = base;
            if (role == Role::ClosedHat || role == Role::OpenHat || role == Role::Shaker || role == Role::Ride)
                velocity *= 0.85f + 0.15f * random.nextFloat();
            if (role == Role::ClosedHat && openHatRow >= 0)
                voices[(size_t) openHatRow]->choke();

            voices[(size_t) i]->trigger (velocity, settings[i]);
            if (role == Role::Kick) kickHit = true;

            // Roll: the sound is repeated 2 to 4 times inside the step.
            const int roll = track.rolls[(size_t) i][st].load();
            if (roll >= 2)
            {
                rollInterval[i] = (float) s.stepSamples / (float) roll;
                rollCountdown[i] = rollInterval[i];
                rollsLeft[i] = roll - 1;
                rollVelocity[i] = velocity;
            }
            else
            {
                rollsLeft[i] = 0;
            }
        }
    }

    void tick() override
    {
        for (int i = 0; i < numRows; ++i)
        {
            if (rollsLeft[i] <= 0 || (rollCountdown[i] -= 1.0f) > 0.0f) continue;
            rollCountdown[i] += rollInterval[i];
            --rollsLeft[i];
            rollVelocity[i] *= 0.9f;
            if (def.rows[(size_t) i].role == Role::ClosedHat && openHatRow >= 0)
                voices[(size_t) openHatRow]->choke();
            voices[(size_t) i]->trigger (rollVelocity[i], settings[i]);
        }
    }

    void stop() override
    {
        for (auto& n : rollsLeft) n = 0;
    }

    // General MIDI drum map first (what pads and keyboards send), then the rows in order from note 36.
    int rowForMidiNote (int note) const override
    {
        struct Gm { int note; Role role; };
        static constexpr Gm map[] {
            { 35, Role::Kick }, { 36, Role::Kick }, { 37, Role::Rim }, { 38, Role::Snare }, { 39, Role::Clap }, { 40, Role::Ghost },
            { 41, Role::LowTom }, { 42, Role::ClosedHat }, { 43, Role::LowTom }, { 44, Role::ClosedHat }, { 45, Role::MidTom },
            { 46, Role::OpenHat }, { 47, Role::MidTom }, { 48, Role::HighTom }, { 49, Role::Crash }, { 50, Role::HighTom },
            { 51, Role::Ride }, { 52, Role::Cymbal }, { 54, Role::Tambourine }, { 55, Role::Cymbal }, { 56, Role::Cowbell },
            { 57, Role::Crash }, { 59, Role::Ride }, { 60, Role::HighBongo }, { 61, Role::LowBongo }, { 62, Role::HighConga },
            { 63, Role::HighConga }, { 64, Role::LowConga }, { 65, Role::HighTimbale }, { 66, Role::LowTimbale }, { 67, Role::Agogo },
            { 68, Role::Agogo }, { 70, Role::Shaker }, { 73, Role::Guiro }, { 74, Role::Guiro }, { 75, Role::Claves },
            { 76, Role::Perc }, { 77, Role::Perc }, { 80, Role::Triangle }, { 81, Role::Triangle }, { 82, Role::Shaker },
        };
        for (auto& g : map)
            if (g.note == note)
                if (const int row = rowForRole (def, g.role); row >= 0) return row;
        return numRows > 0 ? ((note - 36) % numRows + numRows) % numRows : -1;
    }

    void setSampleSlot (int pad, const std::atomic<const SampleData*>* slot) override
    {
        if (pad >= 0 && pad < numRows)
            if (auto* sample = dynamic_cast<SampleVoice*> (voices[(size_t) pad].get())) sample->setSlot (slot);
    }

    void liveNoteOn (int note, float velocity) override
    {
        const int row = rowForMidiNote (note);
        if (row < 0) return;
        if (def.rows[(size_t) row].role == Role::ClosedHat && openHatRow >= 0) voices[(size_t) openHatRow]->choke();
        voices[(size_t) row]->trigger (0.4f + velocity * 0.6f, settings[row]);
        if (def.rows[(size_t) row].role == Role::Kick) kickHit = true;
    }

    void render (Frame& f) override
    {
        for (int i = 0; i < numRows; ++i)
        {
            auto& fx = rowFx[i];
            float v = voices[(size_t) i]->process() * levels[i];
            if (fx.driveAmount > 0.001f)
            {
                const float g = 1.0f + fx.driveAmount * 12.0f;
                v += fx.driveAmount * (fastTanh (v * g) * 0.85f - v);
            }
            v = fx.dsp.process (v);
            f.left += v * fx.panLeft;
            f.right += v * fx.panRight;
            f.echo += v * fx.echoSend;
            f.reverb += v * fx.reverbSend;
        }
    }

private:
    std::vector<std::unique_ptr<DrumVoice>> voices;
    int numRows = 0, openHatRow = -1;
    std::atomic<float>* levelParam[maxDrumRows] {};
    std::atomic<float>* tuneParam[maxDrumRows] {};
    std::atomic<float>* decayParam[maxDrumRows] {};
    std::atomic<float>* toneParam[maxDrumRows] {};
    std::atomic<float>* muteParam[maxDrumRows] {};
    std::atomic<float>* accentParam = nullptr;

    struct RowFx
    {
        std::atomic<float>* drive = nullptr, *filter = nullptr, *pan = nullptr, *echo = nullptr, *reverb = nullptr;
        float driveAmount = 0, echoSend = 0, reverbSend = 0, panLeft = 1, panRight = 1;
        MonoDjFilter dsp;
    };
    RowFx rowFx[maxDrumRows];
    float fs = 44100.0f;
    DrumSettings settings[maxDrumRows];
    float levels[maxDrumRows] {}, trims[maxDrumRows] {};
    int rollsLeft[maxDrumRows] {};
    float rollInterval[maxDrumRows] {}, rollCountdown[maxDrumRows] {}, rollVelocity[maxDrumRows] {};
    bool mutes[maxDrumRows] {};
    float accentAmount = 0.5f;
    juce::Random random;
};

class SynthMachine : public Machine
{
public:
    using Machine::Machine;

    void bind (APVTS& state) override
    {
        for (size_t i = 0; i < def.params.size(); ++i)
            params.push_back (state.getRawParameterValue (pid::synth (type, (int) i)));
        values.resize (def.params.size());
    }

    void readParams() override
    {
        for (size_t i = 0; i < params.size(); ++i)
            values[i] = params[i]->load();
        octave = (int) values[0];
        applyParams();
    }

protected:
    virtual void applyParams() = 0;

    int gateSamples (const StepInfo& s) const
    {
        const auto st = (size_t) s.step;
        return track.slide[st] ? (int) (s.stepSamples + s.nominalStepSamples * 0.5)
                               : std::max (1, (int) (s.stepSamples * gateFraction));
    }

    std::vector<std::atomic<float>*> params;
    std::vector<float> values;
    int octave = 2, gateCountdown = 0;
    float gateFraction = 0.5f;
};

class MonoMachine : public SynthMachine
{
public:
    MonoMachine (int machineType, std::unique_ptr<MonoVoice> v, float gateLength = 0.5f) : SynthMachine (machineType), voice (std::move (v))
    {
        gateFraction = gateLength;
    }

    void prepare (float sampleRate) override { voice->prepare (sampleRate * 2.0f); }

    void onStep (const StepInfo& s) override
    {
        voice->syncStep (s.step, s.nominalStepSamples);
        const auto st = (size_t) s.step, prev = (size_t) s.prevStep;
        if (! track.on[st]) return;

        const bool glide = track.on[prev] && track.slide[prev];
        const int semitone = snapToScale (s.scale, track.note[st]);
        voice->noteOn (machineBaseNote (def, s.key, octave) + semitone, track.accent[st], glide);
        gateCountdown = gateSamples (s);
    }

    void tick() override
    {
        if (gateCountdown > 0 && --gateCountdown == 0) voice->gateOff();
    }

    // The voice runs at twice the sample rate to keep the resonant filter and oscillators clean.
    void render (Frame& f) override
    {
        const float a = voice->process();
        const float b = voice->process();
        f.left = f.right = decimator.process (a, b);
    }

    void stop() override
    {
        voice->gateOff();
        gateCountdown = 0;
        heldCount = 0;
    }

    // Mono: the last key held plays; releasing it returns to the previous one with a glide (legato).
    void liveNoteOn (int note, float velocity) override
    {
        const bool legato = heldCount > 0;
        if (heldCount < (int) std::size (held)) held[heldCount++] = note;
        voice->noteOn (note, velocity > 0.85f, legato);
        gateCountdown = 0;
    }

    void liveNoteOff (int note) override
    {
        int k = 0;
        for (int i = 0; i < heldCount; ++i)
            if (held[i] != note) held[k++] = held[i];
        heldCount = k;
        if (heldCount > 0) voice->noteOn (held[heldCount - 1], false, true);
        else voice->gateOff();
    }

private:
    void applyParams() override { voice->setParams (values.data()); }

    std::unique_ptr<MonoVoice> voice;
    Decimator2x decimator;
    int held[16] {}, heldCount = 0;
};

class PolyMachine : public SynthMachine
{
public:
    using SynthMachine::SynthMachine;

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        for (auto& v : voices) v->prepare (sampleRate * 2.0f);
        previousChord = {};
    }

    void onStep (const StepInfo& s) override
    {
        const auto st = (size_t) s.step, prev = (size_t) s.prevStep;

        // Notes written in the piano roll: each one starts, goes on (tie) and stops on its own.
        const std::uint64_t mask = track.on[st] ? track.chord[st].load() : 0;
        if (mask != 0 || written != 0)
        {
            const std::uint64_t continuing = written & mask & (track.on[prev] ? track.tie[prev].load() : 0);
            releaseNotes (written & ~continuing);
            written = 0;
            if (mask != 0)
            {
                const int base = machineBaseNote (def, s.key, octave);
                int count = 0;
                for (int n = 0; n < noteRange; ++n) count += (int) ((mask >> n) & 1u);
                const float velocity = (track.accent[st] ? 1.0f : 0.8f) * 1.6f / std::sqrt ((float) juce::jmax (1, count));
                for (int n = 0; n < noteRange; ++n)
                {
                    if (! ((mask >> n) & 1u) || ((continuing >> n) & 1u)) continue;
                    writtenNote[n] = base + snapToScale (s.scale, n);
                    startVoice (writtenNote[n], velocity);
                }
                written = mask;
                endingAtGate = mask & ~track.tie[st].load();
                gateOpen = false;
                gateCountdown = juce::jmax (1, (int) (s.stepSamples * gateFraction));
                return;
            }
        }

        const bool held = gateOpen && track.on[prev] && track.slide[prev] && track.on[st]
                       && track.note[prev].load() == track.note[st].load() && track.chord[prev].load() == track.chord[st].load();
        if (held)
        {
            gateCountdown = gateSamples (s);
            return;
        }
        if (! track.on[st]) return;

        releaseAll();
        Chord chord;
        if (const auto mask = track.chord[st].load(); mask != 0)
        {
            // Notes written in the piano roll: played exactly as written.
            const int base = machineBaseNote (def, s.key, octave);
            for (int n = 0; n < noteRange && chord.count < Chord::maxNotes; ++n)
                if ((mask >> n) & 1u) chord.notes[chord.count++] = base + snapToScale (s.scale, n);
        }
        else
        {
            const int root = snapToScale (s.scale, track.note[st]);
            int intervals[5];
            const int count = chordIntervals (s.scale, root, (int) values[1], intervals);
            const int base = machineBaseNote (def, s.key, octave) + root;
            chord.count = count;
            for (int k = 0; k < count; ++k) chord.notes[k] = base + intervals[k];
            if (count >= 3) chord = closestVoicing (chord);
        }
        const int count = chord.count;
        previousChord = chord;

        // Same loudness whether one note or a five-note chord is played.
        const float velocity = (track.accent[st] ? 1.0f : 0.8f) * 1.6f / std::sqrt ((float) count);
        for (int k = 0; k < chord.count; ++k) startVoice (chord.notes[k], velocity);

        gateOpen = true;
        gateCountdown = gateSamples (s);
    }

    void tick() override
    {
        if (gateCountdown > 0 && --gateCountdown == 0)
        {
            if (written != 0)
            {
                releaseNotes (endingAtGate);
                written &= ~endingAtGate;
                return;
            }
            releaseAll();
            gateOpen = false;
        }
    }

    void stop() override
    {
        releaseAll();
        gateOpen = false;
        gateCountdown = 0;
        written = 0;
    }

    void render (Frame& f) override
    {
        float l0 = 0, r0 = 0, l1 = 0, r1 = 0;
        for (auto& v : voices)
        {
            if (! v->isActive()) continue;
            v->process (l0, r0);
            v->process (l1, r1);
        }
        f.left = decimatorL.process (l0, l1);
        f.right = decimatorR.process (r0, r1);
        postProcess (f.left, f.right);
    }

    void liveNoteOn (int note, float velocity) override { startVoice (note, 0.5f + velocity * 0.7f); }

    void liveNoteOff (int note) override
    {
        for (auto& v : voices)
            if (v->isActive() && v->note == note) v->release();
    }

protected:
    virtual void postProcess (float&, float&) {}

    std::vector<std::unique_ptr<PolyVoice>> voices;
    float fs = 44100.0f;

private:
    struct Chord
    {
        static constexpr int maxNotes = 8;
        int notes[maxNotes] {};
        int count = 0;

        float mean() const
        {
            float m = 0;
            for (int i = 0; i < count; ++i) m += (float) notes[i];
            return count > 0 ? m / (float) count : 0.0f;
        }
    };

    // Picks the inversion of the chord that moves the least from the previous one (smooth voice leading),
    // without drifting away from the register of the root position. No allocation: runs on the audio thread.
    Chord closestVoicing (const Chord& rootPosition) const
    {
        if (previousChord.count == 0) return rootPosition;

        const float centre = rootPosition.mean();
        Chord best = rootPosition;
        float bestCost = 1.0e9f;

        for (int shift : { -12, 0, 12 })
        {
            Chord candidate = rootPosition;
            for (int i = 0; i < candidate.count; ++i) candidate.notes[i] += shift;
            for (int rotation = 0; rotation < candidate.count; ++rotation)
            {
                std::sort (candidate.notes, candidate.notes + candidate.count);
                float cost = std::abs (candidate.mean() - centre) * 0.6f;
                for (int i = 0; i < candidate.count; ++i)
                    cost += (float) std::abs (candidate.notes[i] - previousChord.notes[juce::jmin (i, previousChord.count - 1)]);
                if (cost < bestCost)
                {
                    bestCost = cost;
                    best = candidate;
                }
                candidate.notes[0] += 12;
            }
        }
        return best;
    }

    void startVoice (int note, float velocity)
    {
        PolyVoice* chosen = nullptr;
        for (auto& v : voices)
            if (! v->isActive()) { chosen = v.get(); break; }
        if (chosen == nullptr)
            for (auto& v : voices)
                if (chosen == nullptr || v->age < chosen->age) chosen = v.get();

        chosen->age = ++ageCounter;
        chosen->start (note, velocity);
    }

    void releaseAll()
    {
        for (auto& v : voices)
            if (v->isActive()) v->release();
    }

    void releaseNotes (std::uint64_t bits)
    {
        for (int n = 0; n < noteRange; ++n)
            if ((bits >> n) & 1u)
                for (auto& v : voices)
                    if (v->isActive() && v->note == writtenNote[n]) v->release();
    }

    std::uint64_t written = 0, endingAtGate = 0;   // piano roll notes sounding, and those that stop at the end of this step
    int writtenNote[noteRange] {};

    bool gateOpen = false;
    unsigned int ageCounter = 0;
    Chord previousChord;
    Decimator2x decimatorL, decimatorR;
};

class JunoMachine : public PolyMachine
{
public:
    JunoMachine() : PolyMachine (Juno106)
    {
        gateFraction = 0.75f;
        for (int i = 0; i < 8; ++i)
        {
            auto v = std::make_unique<JunoVoice> (shared);
            junoVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

    void prepare (float sampleRate) override
    {
        PolyMachine::prepare (sampleRate);
        chorus.prepare (sampleRate);
    }

private:
    void applyParams() override
    {
        shared.saw = values[p106::Saw];
        shared.pulse = values[p106::Pulse];
        shared.sub = values[p106::Sub];
        shared.noise = values[p106::Noise];
        shared.pwm = values[p106::Pwm];
        shared.hpf = (int) values[p106::Hpf];
        shared.cutoff = values[p106::Cutoff];
        shared.reso = values[p106::Reso];
        shared.env = values[p106::Env];
        chorusMode = (int) values[p106::Chorus];
        for (auto* v : junoVoices)
            v->setEnvelope (knobTime (values[p106::Attack], 0.001f, 3.0f), knobTime (values[p106::Decay], 0.005f, 4.0f),
                            values[p106::Sustain], knobTime (values[p106::Release], 0.005f, 5.0f));
    }

    void postProcess (float& left, float& right) override
    {
        advance (lfoPhase, 0.6f / fs);
        shared.lfo = 1.0f - 4.0f * std::abs (lfoPhase - 0.5f);
        chorus.process (left * 2.2f, chorusMode, left, right);
    }

    JunoParams shared;
    std::vector<JunoVoice*> junoVoices;
    BbdChorus chorus;
    int chorusMode = 1;
    float lfoPhase = 0;
};

class SuperMachine : public PolyMachine
{
public:
    SuperMachine() : PolyMachine (JP8000)
    {
        gateFraction = 0.75f;
        for (int i = 0; i < 8; ++i)
        {
            auto v = std::make_unique<SuperVoice> (shared);
            superVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

private:
    void applyParams() override
    {
        shared.detune = values[p8000::Detune];
        shared.mix = values[p8000::Mix];
        shared.cutoff = values[p8000::Cutoff];
        shared.reso = values[p8000::Reso];
        shared.env = values[p8000::Env];
        for (auto* v : superVoices)
            v->setEnvelope (knobTime (values[p8000::Attack], 0.001f, 3.0f), knobTime (values[p8000::Decay], 0.005f, 4.0f),
                            values[p8000::Sustain], knobTime (values[p8000::Release], 0.005f, 5.0f));
    }

    void postProcess (float& left, float& right) override
    {
        left *= 1.8f;
        right *= 1.8f;
    }

    SuperParams shared;
    std::vector<SuperVoice*> superVoices;
};
class FmMachine : public PolyMachine
{
public:
    explicit FmMachine (int machineType = DX7, int firstPreset = 0) : PolyMachine (machineType), presetOffset (firstPreset)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<FmVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.preset = presetOffset + (int) values[pDX7::Preset];
        shared.brightness = values[pDX7::Brightness];
        shared.decay = values[pDX7::Decay];
        shared.release = values[pDX7::Release];
        shared.detune = values[pDX7::Detune];
        shared.tremolo = values[pDX7::Tremolo];
    }

    void postProcess (float& left, float& right) override
    {
        advance (lfoPhase, 5.2f / fs);
        shared.lfo = std::sin (2.0f * pi * lfoPhase);
        left *= 0.85f;
        right *= 0.85f;
    }

    FmParams shared;
    int presetOffset = 0;
    float lfoPhase = 0;
};

class StringMachine : public PolyMachine
{
public:
    StringMachine() : PolyMachine (Strings)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i)
        {
            auto v = std::make_unique<StringVoice> (shared);
            stringVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

    void prepare (float sampleRate) override
    {
        PolyMachine::prepare (sampleRate);
        ensemble.prepare (sampleRate);
    }

private:
    void applyParams() override
    {
        shared.violin = values[pStrings::Violin];
        shared.cello = values[pStrings::Cello];
        shared.brightness = values[pStrings::Brightness];
        ensembleAmount = values[pStrings::Ensemble];
        for (auto* v : stringVoices)
            v->setEnvelope (knobTime (values[pStrings::Attack], 0.005f, 3.0f), knobTime (values[pStrings::Release], 0.02f, 5.0f));
    }

    void postProcess (float& left, float& right) override
    {
        ensemble.process (left * 1.8f, ensembleAmount, left, right);
    }

    StringParams shared;
    std::vector<StringVoice*> stringVoices;
    EnsembleChorus ensemble;
    float ensembleAmount = 0.7f;
};

class OrganMachine : public PolyMachine
{
public:
    OrganMachine() : PolyMachine (Organ)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<OrganVoice> (shared));
        buildOrganTable (shared, 0.8f, 0.5f, 0.3f, false);
    }

    void prepare (float sampleRate) override
    {
        PolyMachine::prepare (sampleRate);
        leslie.prepare (sampleRate);
    }

private:
    void applyParams() override
    {
        const float bass = values[pOrgan::Bass], middle = values[pOrgan::Middle], high = values[pOrgan::High];
        const bool house = values[pOrgan::Type] > 0.5f;
        if (bass != lastBass || middle != lastMiddle || high != lastHigh || house != lastHouse)
        {
            buildOrganTable (shared, bass, middle, high, house);
            lastBass = bass;
            lastMiddle = middle;
            lastHigh = high;
            lastHouse = house;
        }
        shared.percussion = values[pOrgan::Percussion];
        shared.click = values[pOrgan::Click];
        leslieAmount = values[pOrgan::Leslie];
        drive = 1.0f + values[pOrgan::Drive] * 8.0f;
    }

    void postProcess (float& left, float& right) override
    {
        const float x = fastTanh (left * 1.6f * drive) / fastTanh (drive);
        leslie.process (x * 0.55f, leslieAmount, left, right);
    }

    OrganParams shared;
    Leslie leslie;
    float lastBass = -1, lastMiddle = -1, lastHigh = -1, leslieAmount = 0.4f, drive = 1.0f;
    bool lastHouse = false;
};

class PianoMachine : public PolyMachine
{
public:
    PianoMachine() : PolyMachine (Piano)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<PianoVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pPiano::Type];
        shared.brightness = values[pPiano::Brightness];
        shared.decay = values[pPiano::Decay];
        shared.release = values[pPiano::Release];
        shared.hammer = values[pPiano::Hammer];
        shared.width = values[pPiano::Width];
    }

    PianoParams shared;
};

class MalletMachine : public PolyMachine
{
public:
    MalletMachine() : PolyMachine (Mallets)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<MalletVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pMallets::Type];
        shared.brightness = values[pMallets::Brightness];
        shared.decay = values[pMallets::Decay];
        shared.release = values[pMallets::Release];
        shared.mallet = values[pMallets::Mallet];
        shared.vibrato = values[pMallets::Vibrato];
    }

    void postProcess (float&, float&) override
    {
        advance (lfoPhase, 5.5f / fs);
        shared.lfo = std::sin (2.0f * pi * lfoPhase);
    }

    MalletParams shared;
    float lfoPhase = 0;
};

class PluckMachine : public PolyMachine
{
public:
    PluckMachine() : PolyMachine (Pluck)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 8; ++i) voices.push_back (std::make_unique<PluckVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pPluck::Type];
        shared.brightness = values[pPluck::Brightness];
        shared.decay = values[pPluck::Decay];
        shared.release = values[pPluck::Release];
        shared.strum = values[pPluck::Strum];
        shared.body = values[pPluck::Body];
    }

    // All the notes of a chord start during the same step, before the next sample: the strum restarts here.
    void postProcess (float&, float&) override { shared.strumIndex = 0; }

    PluckParams shared;
};

class ChoirMachine : public PolyMachine
{
public:
    ChoirMachine() : PolyMachine (Choir)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 8; ++i)
        {
            auto v = std::make_unique<ChoirVoice> (shared);
            choirVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

    void prepare (float sampleRate) override
    {
        PolyMachine::prepare (sampleRate);
        ensemble.prepare (sampleRate);
    }

private:
    void applyParams() override
    {
        shared.vowel = (int) values[pChoir::Vowel];
        shared.ensemble = values[pChoir::Ensemble];
        shared.brightness = values[pChoir::Brightness];
        shared.breath = values[pChoir::Breath];
        for (auto* v : choirVoices)
            v->setEnvelope (knobTime (values[pChoir::Attack], 0.01f, 3.0f), knobTime (values[pChoir::Release], 0.05f, 5.0f));
    }

    void postProcess (float& left, float& right) override { ensemble.process (left, 0.3f + shared.ensemble * 0.6f, left, right); }

    ChoirParams shared;
    std::vector<ChoirVoice*> choirVoices;
    EnsembleChorus ensemble;
};

class HooverMachine : public PolyMachine
{
public:
    HooverMachine() : PolyMachine (Hoover)
    {
        gateFraction = 0.85f;
        for (int i = 0; i < 8; ++i) voices.push_back (std::make_unique<HooverVoice> (shared));
    }

    void prepare (float sampleRate) override
    {
        PolyMachine::prepare (sampleRate);
        chorus.prepare (sampleRate);
    }

private:
    void applyParams() override
    {
        shared.detune = values[pHoov::Detune];
        shared.dip = values[pHoov::Dip];
        shared.cutoff = values[pHoov::Cutoff];
        shared.attack = knobTime (values[pHoov::Attack], 0.002f, 1.5f);
        shared.release = knobTime (values[pHoov::Release], 0.02f, 3.0f);
        chorusMode = (int) values[pHoov::Chorus];
    }

    void postProcess (float& left, float& right) override
    {
        const float mono = (left + right) * 0.5f;
        if (chorusMode > 0) chorus.process (mono * 2.0f, chorusMode, left, right);
        else left = right = mono * 2.0f;
    }

    HooverParams shared;
    BbdChorus chorus;
    int chorusMode = 3;
};

class Poly80Machine : public PolyMachine
{
public:
    Poly80Machine() : PolyMachine (Poly80)
    {
        gateFraction = 0.75f;
        for (int i = 0; i < 8; ++i)
        {
            auto v = std::make_unique<Poly80Voice> (shared);
            polyVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

private:
    void applyParams() override
    {
        shared.saw = values[pP80::Saw];
        shared.pulse = values[pP80::Pulse];
        shared.detune = values[pP80::Detune];
        shared.cutoff = values[pP80::Cutoff];
        shared.reso = values[pP80::Reso];
        shared.env = values[pP80::Env];
        for (auto* v : polyVoices)
            v->setEnvelope (knobTime (values[pP80::Attack], 0.001f, 3.0f), knobTime (values[pP80::Decay], 0.005f, 4.0f),
                            values[pP80::Sustain], knobTime (values[pP80::Release], 0.005f, 5.0f));
    }

    Poly80Params shared;
    std::vector<Poly80Voice*> polyVoices;
};

// Supersaw voices with a sidechain-like volume pump locked to the beats.
class EdmMachine : public PolyMachine
{
public:
    EdmMachine() : PolyMachine (Edm)
    {
        gateFraction = 0.8f;
        for (int i = 0; i < 8; ++i)
        {
            auto v = std::make_unique<SuperVoice> (shared);
            superVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

    void onStep (const StepInfo& s) override
    {
        PolyMachine::onStep (s);
        beatSamples = (float) s.nominalStepSamples * 4.0f;
        if (s.step % 4 == 0) sinceBeat = 0.0f;
    }

private:
    void applyParams() override
    {
        shared.detune = values[pEdm::Detune];
        shared.mix = values[pEdm::Mix];
        shared.cutoff = values[pEdm::Cutoff];
        shared.reso = values[pEdm::Reso];
        shared.env = values[pEdm::Env];
        pump = values[pEdm::Pump];
        for (auto* v : superVoices)
            v->setEnvelope (knobTime (values[pEdm::Attack], 0.001f, 3.0f), knobTime (values[pEdm::Decay], 0.005f, 4.0f),
                            values[pEdm::Sustain], knobTime (values[pEdm::Release], 0.005f, 5.0f));
    }

    void postProcess (float& left, float& right) override
    {
        sinceBeat += 1.0f;
        const float x = juce::jmin (1.0f, sinceBeat / (beatSamples * 0.6f));
        const float duck = 1.0f - pump * (1.0f - x) * (1.0f - x);
        left *= 2.5f * duck;
        right *= 2.5f * duck;
    }

    SuperParams shared;
    std::vector<SuperVoice*> superVoices;
    float pump = 0.6f, beatSamples = 24000.0f, sinceBeat = 0.0f;
};

class TalkboxMachine : public PolyMachine
{
public:
    TalkboxMachine() : PolyMachine (Talkbox)
    {
        gateFraction = 0.85f;
        for (int i = 0; i < 6; ++i) voices.push_back (std::make_unique<TalkboxVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.from = (int) values[pTalk::From];
        shared.to = (int) values[pTalk::To];
        shared.speed = values[pTalk::Speed];
        shared.brightness = values[pTalk::Brightness];
        shared.release = values[pTalk::Release];
    }

    TalkboxParams shared;
};

class MellotronMachine : public PolyMachine
{
public:
    MellotronMachine() : PolyMachine (Mellotron)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i)
        {
            auto v = std::make_unique<MellotronVoice> (shared);
            melloVoices.push_back (v.get());
            voices.push_back (std::move (v));
        }
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pMello::Type];
        shared.tone = values[pMello::Tone];
        shared.wow = values[pMello::Wow];
        hiss = values[pMello::Noise];
        for (auto* v : melloVoices)
            v->setEnvelope (knobTime (values[pMello::Attack], 0.01f, 1.5f), knobTime (values[pMello::Release], 0.03f, 2.0f));
    }

    void postProcess (float& left, float& right) override
    {
        advance (slow, 0.55f / fs);
        advance (fast, 6.8f / fs);
        shared.wobble = std::sin (2.0f * pi * slow) + 0.3f * std::sin (2.0f * pi * fast);
        hissFilter += 0.2f * (noise.next() - hissFilter);
        const float h = hissFilter * hiss * 0.01f;
        left = left * 0.65f + h;
        right = right * 0.65f + h;
    }

    MellotronParams shared;
    std::vector<MellotronVoice*> melloVoices;
    Noise noise;
    float slow = 0, fast = 0, hiss = 0.3f, hissFilter = 0;
};

class EPianoMachine : public PolyMachine
{
public:
    EPianoMachine() : PolyMachine (EPiano)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<EPianoVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pEP::Type];
        shared.tone = values[pEP::Tone];
        shared.decay = values[pEP::Decay];
        shared.release = values[pEP::Release];
        shared.bark = values[pEP::Bark];
        tremolo = values[pEP::Tremolo];
    }

    // Stereo tremolo: the sound moves from one side to the other.
    void postProcess (float& left, float& right) override
    {
        advance (lfo, 4.6f / fs);
        const float pan = std::sin (2.0f * pi * lfo) * tremolo * 0.7f;
        const float mono = (left + right) * 0.5f * 0.65f;
        left = mono * (1.0f - pan);
        right = mono * (1.0f + pan);
    }

    EPianoParams shared;
    float tremolo = 0.3f, lfo = 0;
};

class BrassMachine : public PolyMachine
{
public:
    BrassMachine() : PolyMachine (Brass)
    {
        gateFraction = 0.75f;
        for (int i = 0; i < 8; ++i) voices.push_back (std::make_unique<BrassVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pBrass::Type];
        shared.brightness = values[pBrass::Brightness];
        shared.attack = values[pBrass::Attack];
        shared.release = values[pBrass::Release];
        shared.vibrato = values[pBrass::Vibrato];
        shared.growl = values[pBrass::Growl];
    }

    BrassParams shared;
};

class WindMachine : public PolyMachine
{
public:
    WindMachine() : PolyMachine (Winds)
    {
        gateFraction = 0.85f;
        for (int i = 0; i < 8; ++i) voices.push_back (std::make_unique<WindVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pWind::Type];
        shared.breath = values[pWind::Breath];
        shared.vibrato = values[pWind::Vibrato];
        shared.attack = values[pWind::Attack];
        shared.release = values[pWind::Release];
        shared.brightness = values[pWind::Brightness];
    }

    WindParams shared;
};

class OrchestraMachine : public PolyMachine
{
public:
    OrchestraMachine() : PolyMachine (Orchestra)
    {
        gateFraction = 0.9f;
        for (int i = 0; i < 10; ++i) voices.push_back (std::make_unique<OrchestraVoice> (shared));
    }

private:
    void applyParams() override
    {
        shared.type = (int) values[pOrch::Type];
        shared.attack = values[pOrch::Attack];
        shared.release = values[pOrch::Release];
        shared.brightness = values[pOrch::Brightness];
        shared.ensemble = values[pOrch::Ensemble];
    }

    OrchestraParams shared;
};
} // namespace

std::unique_ptr<Machine> createMachine (int type)
{
    switch (type)
    {
        case TR909:
        case TR808:
        case BreakKit: return std::make_unique<DrumMachine> (type);
        case TB303:    return std::make_unique<MonoMachine> (type, std::make_unique<Acid303>());
        case SH101:    return std::make_unique<MonoMachine> (type, std::make_unique<Mono101>());
        case ModelD:   return std::make_unique<MonoMachine> (type, std::make_unique<ModelDVoice>());
        case Juno106:  return std::make_unique<JunoMachine>();
        case JP8000:   return std::make_unique<SuperMachine>();
        case LinnDrum:
        case CR78:
        case MPC:
        case Percussion:
        case SoundFx:
        case AcousticKit:
        case TR707:
        case Simmons:
        case WorldPerc:
        case Sampler:  return std::make_unique<DrumMachine> (type);
        case EBass:    return std::make_unique<MonoMachine> (type, std::make_unique<ElectricBassVoice>(), 0.85f);
        case Wobble:   return std::make_unique<MonoMachine> (type, std::make_unique<WobbleVoice>(), 0.9f);
        case Hoover:   return std::make_unique<HooverMachine>();
        case Poly80:   return std::make_unique<Poly80Machine>();
        case M1:       return std::make_unique<FmMachine> (M1, 30);
        case Edm:      return std::make_unique<EdmMachine>();
        case Talkbox:  return std::make_unique<TalkboxMachine>();
        case Mellotron: return std::make_unique<MellotronMachine>();
        case EPiano:   return std::make_unique<EPianoMachine>();
        case Brass:    return std::make_unique<BrassMachine>();
        case Winds:    return std::make_unique<WindMachine>();
        case Orchestra: return std::make_unique<OrchestraMachine>();
        case DX7:      return std::make_unique<FmMachine>();
        case Strings:  return std::make_unique<StringMachine>();
        case Organ:    return std::make_unique<OrganMachine>();
        case Piano:    return std::make_unique<PianoMachine>();
        case Mallets:  return std::make_unique<MalletMachine>();
        case Pluck:    return std::make_unique<PluckMachine>();
        case Choir:    return std::make_unique<ChoirMachine>();
        case Sub808:   return std::make_unique<MonoMachine> (type, std::make_unique<Sub808Voice>(), 14.0f);
        default:       break;
    }
    jassertfalse;
    return nullptr;
}
} // namespace techno
