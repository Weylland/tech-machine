#pragma once

#include "MachineDefs.h"
#include "Track.h"
#include <juce_core/juce_core.h>
#include <vector>

namespace techno
{
// Pattern strings are 16 steps long (one bar):
//   'X' = hit + accent, 'x' = hit, 'o' = 50% chance, 'r' = 20% chance, '.' = nothing,
//   '-' = (melodies) continue the previous note: glide for mono synths, hold for chords.
struct DrumPart
{
    Role role;
    std::vector<const char*> variants;
    bool firstBarOnly = false;
};

struct MelodyPart
{
    std::vector<const char*> rhythms;
    std::vector<std::pair<int, int>> notes;    // scale degree, weight
    float accent = 0.2f, slide = 0.1f, octaveJump = 0.1f;
    int motif = 16;                            // steps before the motif repeats
    bool followChords = true;                  // transpose with the chord progression
};

struct ChordPart
{
    std::vector<const char*> rhythms;
    bool pad = false;                          // long held chords instead of stabs
};

struct StyleDef
{
    const char* name;
    const char* help;
    float bpm, swing;
    int scale;
    std::vector<int> progression;              // chord root degrees, one per bar
    std::vector<DrumPart> drums;
    std::vector<DrumPart> fill;                // added on the last bar of multi-bar patterns
    MelodyPart acid, bass, lead;
    ChordPart chords;
    bool leadOn101, leadOn8000;
    std::vector<int> recommended;
    std::vector<std::pair<const char*, float>> presets;   // parameter id, value
};

const std::vector<StyleDef>& styles();

// Writes a new part for one machine in the given style.
void generate (Track&, const MachineDef&, const StyleDef&, int steps, int scale, juce::Random&);
} // namespace techno
