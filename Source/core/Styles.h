#pragma once

#include "MachineDefs.h"
#include "Track.h"
#include <juce_core/juce_core.h>
#include <vector>

namespace techno
{
// Drum pattern strings are 16 steps long (one bar):
//   'X' = hit + accent, 'x' = hit, 'o' = 50% chance, 'r' = 20% chance, '.' = nothing.
// Percussion written only with 'r' / '.' is replaced by a Euclidean rhythm (evenly spread hits).
struct DrumPart
{
    Role role;
    std::vector<const char*> variants;
    bool firstBarOnly = false;
};

enum class BassMode { Offbeat, Rolling, Groove, Long, Psy, Sub808, Disco, Funk, Reggae, Tresillo };
enum class LeadMode { Motif, Arp, Riff, Sparse };
enum class ChordMode { Stabs, Pad, Sparse, Skank, Comping };

// A genre: what the dice and SURPRISE ! write for each machine.
struct StyleDef
{
    const char* name;
    const char* help;
    float bpm, swing;                              // suggested tempo and swing
    int scale;                                     // suggested scale
    std::vector<std::vector<int>> progressions;    // chord root degrees, one per bar
    std::vector<DrumPart> drums;
    std::vector<DrumPart> fill;                    // added on the last bar of multi-bar patterns
    std::vector<BassMode> bassModes;
    std::vector<LeadMode> leadModes;
    std::vector<ChordMode> chordModes;
    std::vector<const char*> chordRhythms;         // used by stabs
    bool leadOn101, leadOn8000;                    // otherwise the 101 plays bass and the 8000 chords
};

const std::vector<StyleDef>& styles();

enum class GenerateMode { NewIdea, Vary, Denser, Sparser };

struct GenerateContext
{
    const StyleDef& style;
    int steps, scale;
    const std::vector<int>& progression;           // chord root degree per bar
    std::array<bool, maxSteps> kick {};            // where the kick plays, so bass lines can avoid it
};

// Writes or transforms the part of one machine.
void generate (Track&, const MachineDef&, const GenerateContext&, GenerateMode, juce::Random&);

// Rotates a whole part by one step (direction = +1 or -1) over the pattern length.
void shiftTrack (Track&, int steps, int direction);

// The row of a kit that plays a role (or the closest sound it has), -1 when none fits.
int rowForRole (const MachineDef&, Role);

// Chord root degree at a step of the current progression.
int chordRootAt (const GenerateContext&, int step);
} // namespace techno
