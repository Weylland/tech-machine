#include "Machines.h"
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
        }
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
    MonoMachine (int machineType, std::unique_ptr<MonoVoice> v) : SynthMachine (machineType), voice (std::move (v)) {}

    void prepare (float sampleRate) override { voice->prepare (sampleRate * 2.0f); }

    void onStep (const StepInfo& s) override
    {
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
    }

private:
    void applyParams() override { voice->setParams (values.data()); }

    std::unique_ptr<MonoVoice> voice;
    Decimator2x decimator;
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
        const bool held = gateOpen && track.on[prev] && track.slide[prev] && track.on[st]
                       && track.note[prev].load() == track.note[st].load();
        if (held)
        {
            gateCountdown = gateSamples (s);
            return;
        }
        if (! track.on[st]) return;

        releaseAll();
        const int root = snapToScale (s.scale, track.note[st]);
        int intervals[5];
        const int count = chordIntervals (s.scale, root, (int) values[1], intervals);
        const int base = machineBaseNote (def, s.key, octave) + root;

        Chord chord;
        chord.count = count;
        for (int k = 0; k < count; ++k) chord.notes[k] = base + intervals[k];
        if (count >= 3) chord = closestVoicing (chord);
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
            releaseAll();
            gateOpen = false;
        }
    }

    void stop() override
    {
        releaseAll();
        gateOpen = false;
        gateCountdown = 0;
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

protected:
    virtual void postProcess (float&, float&) {}

    std::vector<std::unique_ptr<PolyVoice>> voices;
    float fs = 44100.0f;

private:
    struct Chord
    {
        int notes[5] {};
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
        default:       break;
    }
    jassertfalse;
    return nullptr;
}
} // namespace techno
