#pragma once

#include "Styles.h"

namespace techno
{
// Famous ready-made parts, picked from the PATTERNS menu of each machine.
//
// Drum strings are 16 steps (one bar) or 32 steps (two bars):
//   'X' = hit + accent, 'x' = hit, 'g' = ghost note (on the GHOST row when the kit has one),
//   '2' / '3' / '4' = roll, '.' = nothing.
// Melodic strings: '0'-'9' = scale degree above the current chord root (7 = one octave up),
//   'a' / 'b' / 'c' = one / two / three degrees below, '-' = hold the previous note (glide on mono synths), '.' = rest.
enum class PatternKind { Drums, Bass, Chords, Lead };

struct DrumLine
{
    Role role;
    const char* steps;
};

struct Pattern
{
    PatternKind kind;
    const char* category;
    const char* name;
    int bpm;
    std::vector<DrumLine> drums;
    const char* line = nullptr;
};

const std::vector<Pattern>& patterns();

// Categories of one kind, in menu order.
std::vector<const char*> patternCategories (PatternKind kind);

// Which kinds make sense on a machine (drums on drum machines, bass / chords / leads on synths).
std::vector<PatternKind> patternKindsFor (const MachineDef& def);

void applyPattern (Track& track, const MachineDef& def, const GenerateContext& ctx, const Pattern& pattern);
} // namespace techno
