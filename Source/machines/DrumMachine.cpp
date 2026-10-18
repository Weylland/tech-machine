#include "Factories.h"
#include "../core/Styles.h"
#include "../dsp/Effects.h"

namespace techno
{
namespace
{
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

    void bind (juce::AudioProcessorValueTreeState& state) override
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
    juce::Random random { 0x7ec4 };   // fixed seed: the same project always sounds the same
};
} // namespace

std::unique_ptr<Machine> createDrumMachine (int type) { return std::make_unique<DrumMachine> (type); }
} // namespace techno
