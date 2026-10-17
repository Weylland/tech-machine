#pragma once

#include <juce_core/juce_core.h>
#include <vector>

namespace techno
{
enum MachineType
{
    TR909, TR808, BreakKit, TB303, SH101, ModelD, Juno106, JP8000,
    LinnDrum, CR78, MPC, Percussion, DX7, Strings, Organ, Sub808,
    Piano, Mallets, Pluck, Choir, SoundFx,
    numMachines
};

// Order of the machines in the rack and in the "add" menu.
inline constexpr int rackOrder[numMachines] {
    TR909, TR808, LinnDrum, CR78, MPC, BreakKit, Percussion, SoundFx,
    TB303, SH101, ModelD, Sub808,
    Juno106, JP8000, DX7, Strings, Organ, Piano, Mallets, Pluck, Choir
};

enum class MachineKind { Drum, Mono, Poly };

enum class Role
{
    Kick, Snare, Clap, Rim, ClosedHat, OpenHat, Ride, Crash,
    LowTom, MidTom, HighTom, Cowbell, Claves, Shaker, Ghost, Cymbal,
    LowConga, HighConga, LowBongo, HighBongo, LowTimbale, HighTimbale,
    Agogo, Guiro, Tambourine, Triangle, Snap, Perc
};

struct DrumRowDef
{
    const char* id;
    const char* label;
    Role role;
    float level, tune, decay, tone;
    const char* help;
    const char* toneHelp;
};

struct ParamDef
{
    const char* id;
    const char* label;
    float def;                         // 0..1 for knobs, index for choices
    const char* help;
    std::vector<const char*> choices;  // empty = continuous knob
};

struct MachineDef
{
    MachineType type;
    const char* id;
    const char* name;
    const char* subtitle;
    MachineKind kind;
    juce::uint32 colour;
    int baseOffset;                    // semitones above the key root for octave "0"
    const char* help;
    std::vector<DrumRowDef> rows;
    std::vector<ParamDef> params;
};

const MachineDef& machineDef (int type);

// Parameter order of each synth: must match the params vectors in MachineDefs.cpp.
namespace p303 { enum { Octave, Wave, Cutoff, Reso, EnvMod, Decay, Accent, count }; }
namespace p101 { enum { Octave, Saw, Square, Sub, Noise, Pwm, Cutoff, Reso, Env, Attack, Decay, Sustain, Release, Glide, count }; }
namespace pMD { enum { Octave, Wave, Detune, Sub, Cutoff, Emphasis, Contour, Attack, Decay, Sustain, Glide, Drive, count }; }
namespace p106 { enum { Octave, Chord, Saw, Pulse, Sub, Noise, Pwm, Hpf, Cutoff, Reso, Env, Attack, Decay, Sustain, Release, Chorus, count }; }
namespace p8000 { enum { Octave, Chord, Detune, Mix, Cutoff, Reso, Env, Attack, Decay, Sustain, Release, count }; }
namespace pDX7 { enum { Octave, Chord, Preset, Brightness, Decay, Release, Detune, Tremolo, count }; }
namespace pStrings { enum { Octave, Chord, Violin, Cello, Attack, Release, Ensemble, Brightness, count }; }
namespace pOrgan { enum { Octave, Chord, Type, Bass, Middle, High, Percussion, Leslie, Drive, Click, count }; }
namespace pSub { enum { Octave, Decay, Glide, Punch, Drive, Tone, count }; }
namespace pPiano { enum { Octave, Chord, Type, Brightness, Decay, Release, Hammer, Width, count }; }
namespace pMallets { enum { Octave, Chord, Type, Brightness, Decay, Release, Mallet, Vibrato, count }; }
namespace pPluck { enum { Octave, Chord, Type, Brightness, Decay, Release, Strum, Body, count }; }
namespace pChoir { enum { Octave, Chord, Vowel, Attack, Release, Ensemble, Brightness, Breath, count }; }

// Parameter IDs.
namespace pid
{
inline juce::String machine (int type, const char* suffix)
{
    return juce::String (machineDef (type).id) + "_" + suffix;
}

inline juce::String row (int type, int row, const char* suffix)
{
    return juce::String (machineDef (type).id) + "_" + machineDef (type).rows[(size_t) row].id + "_" + suffix;
}

inline juce::String synth (int type, int index)
{
    return juce::String (machineDef (type).id) + "_" + machineDef (type).params[(size_t) index].id;
}

inline constexpr const char* bpm = "bpm";
inline constexpr const char* swing = "swing";
inline constexpr const char* key = "key";
inline constexpr const char* scale = "scale";
inline constexpr const char* bars = "bars";
inline constexpr const char* echoTime = "echoTime";
inline constexpr const char* echoFeedback = "echoFeedback";
inline constexpr const char* echoLevel = "echoLevel";
inline constexpr const char* reverbSize = "reverbSize";
inline constexpr const char* reverbLevel = "reverbLevel";
inline constexpr const char* pump = "pump";
inline constexpr const char* djFilter = "djFilter";
inline constexpr const char* comp = "comp";
inline constexpr const char* disto = "disto";
inline constexpr const char* volume = "volume";
} // namespace pid

inline constexpr const char* echoTimeNames[] { "1/16", "1/8", "1/8 pointée", "1/4", "1/4 pointée" };
inline constexpr float echoTimeSteps[] { 1.0f, 2.0f, 3.0f, 4.0f, 6.0f };
} // namespace techno
