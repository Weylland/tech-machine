#pragma once

#include "Language.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace techno
{
// User-facing text: written in French in the code, translated when the interface is in English.
inline juce::String utf8 (const char* text) { return tr (text); }

struct ScaleDef
{
    const char* name;
    std::vector<int> intervals;
    const char* help;
};

const std::vector<ScaleDef>& scales();

inline constexpr int chromaticScale = 0;
inline constexpr const char* keyNames[12] { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

// Root of the bass register, between A1 and G#2 whatever the key.
inline int keyRoot (int key) { return 33 + (key + 3) % 12; }

bool inScale (int scale, int semitone);
int snapToScale (int scale, int semitone);
int moveInScale (int scale, int semitone, int delta);
int degreeToSemitone (int scale, int degree);
juce::String noteName (int midiNote, bool withOctave = true);

enum ChordType
{
    ChordSingle, ChordAuto, ChordMinor, ChordMajor, ChordMin7, ChordMaj7, ChordDom7,
    ChordSus2, ChordSus4, ChordMin9, ChordPower, ChordOctave, numChordTypes
};

inline constexpr const char* chordTypeNames[numChordTypes] {
    "Note seule", "Auto (gamme)", "Mineur", "Majeur", "Mineur 7", "Majeur 7", "7e",
    "Sus2", "Sus4", "Mineur 9", "Quinte", "Octave"
};

// Fills intervals (relative to the chord root) and returns how many notes the chord has (max 5).
int chordIntervals (int scale, int rootSemitone, int type, int* intervals);
juce::String chordLabel (int scale, int rootMidi, int rootSemitone, int type);
} // namespace techno
