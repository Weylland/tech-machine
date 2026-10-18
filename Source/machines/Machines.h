#pragma once

#include "../core/MachineDefs.h"
#include "../core/Music.h"
#include "../core/Track.h"
#include "../dsp/DrumVoices.h"
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
    explicit Machine (int machineType) : type (machineType), def (machineDef (machineType)) {}
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
};

std::unique_ptr<Machine> createMachine (int type);

// MIDI note of semitone 0 for a synth machine, given the key and its octave setting (index 2 = "0").
inline int machineBaseNote (const MachineDef& def, int key, int octaveIndex)
{
    return keyRoot (key) + def.baseOffset + 12 * (octaveIndex - 2);
}
} // namespace techno
