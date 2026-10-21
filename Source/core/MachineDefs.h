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
    EBass, Wobble, Hoover, Poly80, M1, Edm, Talkbox, Mellotron, EPiano, Brass, Winds, Orchestra,
    AcousticKit, TR707, Simmons, WorldPerc, Sampler,
    Chip, SemiMod, Wavetable, LaSynth, CS80, Ether, WestCoast, Vocoder,
    TR606, RhythmBox, DigitalKit, SynthDrums, SampleKeys,
    numMachines
};

// Order of the machines in the rack and in the "add" menu.
inline constexpr int rackOrder[numMachines] {
    TR909, TR808, TR606, LinnDrum, TR707, DigitalKit, CR78, RhythmBox, MPC, AcousticKit, Simmons, SynthDrums,
    BreakKit, Percussion, WorldPerc, Sampler, SampleKeys, SoundFx,
    TB303, SH101, SemiMod, ModelD, Sub808, EBass, Wobble,
    Hoover, Juno106, JP8000, Poly80, CS80, Edm, DX7, LaSynth, Wavetable, M1, Chip, Talkbox, Vocoder,
    Piano, EPiano, Organ, Mallets,
    Strings, Mellotron, Orchestra, Brass, Winds, Choir, Pluck,
    Ether, WestCoast
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

// Arpeggiator of the chord synths (every poly machine but the chip, which has its own).
inline bool hasArpeggiator (const MachineDef& def) { return def.kind == MachineKind::Poly && def.type != Chip; }
inline const std::vector<const char*>& arpModeNames()
{
    static const std::vector<const char*> names { "Off", "Montant", "Descendant", "Montant-descendant", "Au hasard", "Dans l'ordre joué" };
    return names;
}
inline const std::vector<const char*>& arpRateNames()
{
    static const std::vector<const char*> names { "1/8", "1/16", "1/16 triolet", "1/32" };
    return names;
}
inline const std::vector<const char*>& arpOctaveNames()
{
    static const std::vector<const char*> names { "1 octave", "2 octaves", "3 octaves" };
    return names;
}

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
namespace pBass { enum { Octave, Type, Tone, Decay, Muting, Glide, count }; }
namespace pWob { enum { Octave, Wave, Rate, Depth, Cutoff, Reso, Detune, Drive, Glide, count }; }
namespace pHoov { enum { Octave, Chord, Detune, Dip, Cutoff, Attack, Release, Chorus, count }; }
namespace pP80 { enum { Octave, Chord, Saw, Pulse, Detune, Cutoff, Reso, Env, Attack, Decay, Sustain, Release, Model, count }; }
namespace pEdm { enum { Octave, Chord, Detune, Mix, Cutoff, Reso, Env, Attack, Decay, Sustain, Release, Pump, count }; }
namespace pTalk { enum { Octave, Chord, From, To, Speed, Brightness, Release, count }; }
namespace pMello { enum { Octave, Chord, Type, Tone, Attack, Release, Wow, Noise, count }; }
namespace pEP { enum { Octave, Chord, Type, Tone, Decay, Release, Tremolo, Bark, count }; }
namespace pBrass { enum { Octave, Chord, Type, Brightness, Attack, Release, Vibrato, Growl, count }; }
namespace pWind { enum { Octave, Chord, Type, Breath, Vibrato, Attack, Release, Brightness, count }; }
namespace pOrch { enum { Octave, Chord, Type, Attack, Release, Brightness, Ensemble, count }; }
namespace pChip { enum { Octave, Chord, Wave, Arp, Pwm, Decay, Sustain, Release, Vibrato, LoFi, count }; }
namespace pSemi { enum { Octave, Model, Wave, Osc2, Cutoff, Reso, Env, Attack, Decay, Sustain, Release, Ring, Drive, Glide, count }; }
namespace pWave { enum { Octave, Chord, Table, Position, Sweep, Motion, Cutoff, Reso, Attack, Decay, Sustain, Release, count }; }
namespace pLA { enum { Octave, Chord, Type, Layer, Brightness, Attack, Release, Chorus, count }; }
namespace pCS { enum { Octave, Chord, Type, Brilliance, Reso, Attack, Release, Touch, Bend, Vibrato, count }; }
namespace pEther { enum { Octave, Type, Glide, Vibrato, Rate, Tone, Attack, Release, count }; }
namespace pWest { enum { Octave, Chord, Ratio, Gate, Fm, Fold, Symmetry, Decay, count }; }
namespace pVoc { enum { Octave, Chord, Words, Speed, Carrier, Sibilance, Shift, Release, Source, count }; }
namespace pSampler { enum { Mode, Texture, count }; }
namespace pKeys { enum { Octave, Chord, Root, Attack, Release, Loop, Tone, count }; }
inline constexpr int keysLowestRoot = 36;   // the ROOT choices start on C2

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
inline constexpr const char* echoType = "echoType";
inline constexpr const char* reverbType = "reverbType";
inline constexpr const char* pumpMode = "pumpMode";
inline constexpr const char* djFilter = "djFilter";
inline constexpr const char* comp = "comp";
inline constexpr const char* disto = "disto";
inline constexpr const char* volume = "volume";
} // namespace pid

inline constexpr const char* echoTimeNames[] { "1/16", "1/8", "1/8 pointée", "1/4", "1/4 pointée" };
inline constexpr float echoTimeSteps[] { 1.0f, 2.0f, 3.0f, 4.0f, 6.0f };
inline constexpr const char* echoTypeNames[] { "Ping-pong", "Bande" };
inline constexpr const char* reverbTypeNames[] { "Pièce", "Hall", "Plate" };
inline constexpr const char* pumpModeNames[] { "Pompe", "Sidechain" };
} // namespace techno
