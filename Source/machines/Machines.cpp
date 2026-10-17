#include "Machines.h"

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
        }
        accentParam = state.getRawParameterValue (pid::machine (type, "accent"));
    }

    void prepare (float sampleRate) override
    {
        for (auto& v : voices) v->prepare (sampleRate);
    }

    void readParams() override
    {
        for (int i = 0; i < numRows; ++i)
        {
            settings[i] = { tuneParam[i]->load(), decayParam[i]->load(), toneParam[i]->load() };
            levels[i] = levelParam[i]->load();
            mutes[i] = muteParam[i]->load() > 0.5f;
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

    void render (float& left, float& right) override
    {
        float sum = 0.0f;
        for (int i = 0; i < numRows; ++i)
            sum += voices[(size_t) i]->process() * levels[i];
        left = right = sum;
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
    DrumSettings settings[maxDrumRows];
    float levels[maxDrumRows] {};
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

    void prepare (float sampleRate) override { voice->prepare (sampleRate); }

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

    void render (float& left, float& right) override { left = right = voice->process(); }

    void stop() override
    {
        voice->gateOff();
        gateCountdown = 0;
    }

private:
    void applyParams() override { voice->setParams (values.data()); }

    std::unique_ptr<MonoVoice> voice;
};

class PolyMachine : public SynthMachine
{
public:
    using SynthMachine::SynthMachine;

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        for (auto& v : voices) v->prepare (sampleRate);
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
        for (int k = 0; k < count; ++k)
            startVoice (base + intervals[k], track.accent[st] ? 1.0f : 0.8f);

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

    void render (float& left, float& right) override
    {
        left = right = 0.0f;
        for (auto& v : voices)
            if (v->isActive()) v->process (left, right);
        postProcess (left, right);
    }

protected:
    virtual void postProcess (float&, float&) {}

    std::vector<std::unique_ptr<PolyVoice>> voices;
    float fs = 44100.0f;

private:
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
