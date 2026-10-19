#include "Factories.h"
#include "../core/MidiMap.h"
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
            const Role role = def.rows[(size_t) i].role;
            if (role == Role::OpenHat) openHatRow = i;
            auto first = [i] (int& row) { if (row < 0) row = i; };
            if (role == Role::Snare) first (snareRow);
            if (role == Role::Clap) first (clapRow);
            if (role == Role::HighTom) first (highTomRow);
            if (role == Role::MidTom) first (midTomRow);
            if (role == Role::LowTom) first (lowTomRow);
            if (role == Role::Crash) first (crashRow);
            if (role == Role::Cymbal) first (cymbalRow);
        }
        if (snareRow < 0) snareRow = clapRow;
        if (crashRow < 0) crashRow = cymbalRow;
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
        for (int i = 0; i < (int) def.params.size(); ++i) machineParams.push_back (state.getRawParameterValue (pid::synth (type, i)));
        machineValues.resize (machineParams.size());
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
            using namespace automation;
            auto row = [this, i] (int s, std::atomic<float>* knob) { return setting (rowTarget (def, i, s), knob->load()); };
            settings[i] = { row (Tune, tuneParam[i]), row (Decay, decayParam[i]), row (Tone, toneParam[i]) };
            const float level = row (Level, levelParam[i]);
            levels[i] = level * 1.25f * trims[i];
            mutes[i] = muteParam[i]->load() > 0.5f;

            auto& fx = rowFx[i];
            fx.driveAmount = row (RowDrive, fx.drive);
            fx.echoSend = row (RowEcho, fx.echo);
            fx.reverbSend = row (RowReverb, fx.reverb);
            const float angle = row (RowPan, fx.pan) * juce::MathConstants<float>::halfPi;
            fx.panLeft = std::cos (angle) * juce::MathConstants<float>::sqrt2;
            fx.panRight = std::sin (angle) * juce::MathConstants<float>::sqrt2;
            fx.dsp.set (row (RowFilter, fx.filter), fs);
        }
        accentAmount = accentParam->load();
        if (! machineParams.empty())
        {
            for (size_t i = 0; i < machineParams.size(); ++i) machineValues[i] = setting (automation::synthTarget ((int) i), machineParams[i]->load());
            for (auto& v : voices) v->setMachineParams (machineValues.data());
        }
    }

    void onStep (const StepInfo& s) override
    {
        const auto st = (size_t) s.step;
        const float base = (track.accent[st] ? 1.0f : 1.0f - 0.5f * accentAmount) * velocityScale();

        // FILL: the last beat of the bar is played by the snare and the toms (unless the steps have their own fill).
        int fillRow = -1, fillRoll = 0;
        const bool fill = fillPosition >= 12 && snareRow >= 0 && ! hasFillSteps();
        if (fill) chooseFill (fillPosition - 12, fillRow, fillRoll);
        if (crashNow && crashRow >= 0 && ! mutes[crashRow]) voices[(size_t) crashRow]->trigger (base, settings[crashRow]);

        for (int i = 0; i < numRows; ++i)
        {
            const Role role = def.rows[(size_t) i].role;
            bool hit = track.hits[(size_t) i][st] && ! silentStep;
            int roll = track.rolls[(size_t) i][st].load();
            if (fill)
            {
                const bool drumsOfTheFill = i == snareRow || i == highTomRow || i == midTomRow || i == lowTomRow;
                const bool cymbal = role == Role::ClosedHat || role == Role::OpenHat || role == Role::Ride;
                if (drumsOfTheFill || cymbal) hit = i == fillRow;
                if (i == fillRow) roll = fillRoll;
            }
            if (! hit || mutes[i]) continue;

            float velocity = base;
            if (fill && i == fillRow) velocity = juce::jmin (1.0f, velocity * (0.8f + 0.1f * (float) (fillPosition - 12)));   // louder towards the end
            if (role == Role::ClosedHat || role == Role::OpenHat || role == Role::Shaker || role == Role::Ride)
                velocity *= 0.85f + 0.15f * random.nextFloat();
            if (role == Role::ClosedHat && openHatRow >= 0)
                voices[(size_t) openHatRow]->choke();

            voices[(size_t) i]->trigger (velocity, settings[i]);
            if (role == Role::Kick) kickHit = true;

            // Roll: the sound is repeated 2 to 4 times inside the step.
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
    int rowForMidiNote (int note) const override { return rowForGmNote (def, note); }

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
    // The drum of each step of the fill (0..3): snare, snare roll, then down the toms; a snare crescendo without toms.
    void chooseFill (int position, int& row, int& roll) const
    {
        const int toms[] { highTomRow >= 0 ? highTomRow : midTomRow, lowTomRow >= 0 ? lowTomRow : midTomRow };
        const bool withToms = toms[0] >= 0 && toms[1] >= 0 && toms[0] != toms[1];
        static constexpr int snareRolls[] { 0, 2, 2, 4 };
        row = snareRow;
        roll = withToms ? (position == 1 ? 2 : 0) : snareRolls[position];
        if (withToms && position >= 2) row = toms[position - 2];
    }

    // Steps programmed to play only during a FILL replace the automatic one.
    bool hasFillSteps() const
    {
        const int lane = track.findLane (automation::stepTarget (def, automation::Condition));
        if (lane < 0) return false;
        for (auto& v : track.lockValue[(size_t) lane])
            if (juce::roundToInt (v.load()) == automation::fillCondition) return true;
        return false;
    }

    std::vector<std::unique_ptr<DrumVoice>> voices;
    int numRows = 0, openHatRow = -1;
    int snareRow = -1, clapRow = -1, highTomRow = -1, midTomRow = -1, lowTomRow = -1, crashRow = -1, cymbalRow = -1;
    std::vector<std::atomic<float>*> machineParams;
    std::vector<float> machineValues;
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
