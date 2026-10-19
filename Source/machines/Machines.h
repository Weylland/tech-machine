#pragma once

#include "../core/Automation.h"
#include "../core/MachineDefs.h"
#include "../core/Music.h"
#include "../core/Track.h"
#include "../dsp/DrumVoices.h"
#include "../dsp/SamplerVoice.h"
#include "../dsp/AcousticVoices.h"
#include "../dsp/BandVoices.h"
#include "../dsp/KeysVoices.h"
#include "../dsp/SynthVoices.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace techno
{
// One sample of a machine's output: stereo dry signal plus what its sounds send to the effects.
struct Frame
{
    float left = 0, right = 0, echo = 0, reverb = 0;
};

struct StepInfo
{
    int step, prevStep, key, scale;
    double stepSamples;         // length of this step, swing included
    double nominalStepSamples;  // length of a step without swing
};

class Machine
{
public:
    explicit Machine (int machineType) : type (machineType), def (machineDef (machineType))
    {
        lockOverride.assign ((size_t) automation::targetCount (machineType), noLock);
        applied.fill (-1);
    }
    virtual ~Machine() = default;

    virtual void bind (juce::AudioProcessorValueTreeState&) = 0;
    virtual void prepare (float sampleRate) = 0;
    virtual void readParams() = 0;
    virtual void onStep (const StepInfo&) = 0;
    virtual void tick() {}
    virtual void render (Frame&) = 0;
    virtual void stop() {}

    // Played live from a MIDI keyboard or pads (audio thread).
    virtual void liveNoteOn (int /*midiNote*/, float /*velocity*/) {}
    virtual void liveNoteOff (int /*midiNote*/) {}
    virtual int rowForMidiNote (int /*midiNote*/) const { return -1; }

    // Sampler: where each pad finds its sound.
    virtual void setSampleSlot (int /*pad*/, const std::atomic<const SampleData*>* /*slot*/) {}

    // Automation: the values of this step replace the knobs. True when a setting changed (audio thread).
    bool applyLocks (int step)
    {
        bool changed = false;
        for (size_t l = 0; l < (size_t) maxLockLanes; ++l)
        {
            const int target = track.lockTarget[l].load();
            if (applied[l] >= 0 && applied[l] != target)
            {
                lockOverride[(size_t) applied[l]] = noLock;
                changed = true;
            }
            applied[l] = target;
            if (target < 0 || target >= (int) lockOverride.size()) continue;
            const float v = track.lockValue[l][(size_t) step].load();
            if (! juce::exactlyEqual (lockOverride[(size_t) target], v))
            {
                lockOverride[(size_t) target] = v;
                changed = true;
            }
        }
        return changed;
    }

    // Back to the knobs (the sequencer stopped).
    void releaseLocks()
    {
        std::fill (lockOverride.begin(), lockOverride.end(), noLock);
        applied.fill (-1);
    }

    // A setting as it sounds now: the step's value if it has one, otherwise the knob.
    float setting (int target, float knob) const
    {
        const float v = lockOverride[(size_t) target];
        return v >= 0.0f ? v : knob;
    }

    bool takeKick()
    {
        const bool hit = kickHit;
        kickHit = false;
        return hit;
    }

    const int type;
    const MachineDef& def;
    Track track;

protected:
    bool kickHit = false;

private:
    std::vector<float> lockOverride;
    std::array<int, maxLockLanes> applied {};
};

std::unique_ptr<Machine> createMachine (int type);

// MIDI note of semitone 0 for a synth machine, given the key and its octave setting (index 2 = "0").
inline int machineBaseNote (const MachineDef& def, int key, int octaveIndex)
{
    return keyRoot (key) + def.baseOffset + 12 * (octaveIndex - 2);
}
} // namespace techno
