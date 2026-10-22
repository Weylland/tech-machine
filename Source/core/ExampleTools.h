#pragma once

#include "Examples.h"
#include "Automation.h"

// Tools to write the demos (patterns as text, chords, automation), and the demos themselves (Examples_*.cpp).
namespace techno::demos
{
inline const juce::String rest16 = ". . . . . . . . . . . . . . . . ";

inline juce::String bars (std::initializer_list<juce::String> parts)
{
    juce::String s;
    for (auto& p : parts) s << p << " ";
    return s;
}

inline juce::String held (const juce::String& note, int steps)
{
    juce::String s = note + " ";
    for (int i = 1; i < steps; ++i) s << "- ";
    return s;
}

inline int parseNote (juce::String token)
{
    static const char* names[] { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const bool sharp = token.length() > 1 && token[1] == '#';
    const juce::String name = token.substring (0, sharp ? 2 : 1);
    int pc = 0;
    for (int i = 0; i < 12; ++i)
        if (name == names[i]) pc = i;
    const int octave = token.substring (sharp ? 2 : 1).getIntValue();
    return 12 * (octave + 1) + pc;
}

// Drum pattern: 'x' hit, 'X' hit + accent, '2'-'4' roll, '.' nothing. 16 to 64 characters, repeated to fill 64 steps.
inline void drums (ExampleWriter& w, int type, const char* rowId, const juce::String& pattern)
{
    const auto& def = machineDef (type);
    int row = -1;
    for (size_t i = 0; i < def.rows.size(); ++i)
        if (juce::String (def.rows[i].id) == rowId) row = (int) i;
    jassert (row >= 0);
    if (row < 0) return;

    const juce::String p = pattern.removeCharacters (" ");
    auto& t = w.track (type);
    for (int s = 0; s < maxSteps; ++s)
    {
        const auto c = p[s % p.length()];
        if (c == 'x' || c == 'X' || (c >= '2' && c <= '4')) t.hits[(size_t) row][(size_t) s] = true;
        if (c == 'X') t.accent[(size_t) s] = true;
        if (c >= '2' && c <= '4') t.rolls[(size_t) row][(size_t) s] = (std::uint8_t) (c - '0');
    }
}

// Note line: one token per step. "A1" note, "A1!" accented, "A1~" glides to the next note,
// "-" holds the previous note, "." silence. Repeated to fill 64 steps.
inline void line (ExampleWriter& w, int type, const juce::String& text)
{
    auto tokens = juce::StringArray::fromTokens (text, " ", "");
    tokens.removeEmptyStrings();
    jassert (tokens.size() > 0 && maxSteps % tokens.size() == 0);
    if (tokens.isEmpty()) return;

    auto& t = w.track (type);
    const int base = w.baseNote (type);
    for (int s = 0; s < maxSteps; ++s)
    {
        const auto i = (size_t) s;
        auto token = tokens[s % tokens.size()];
        if (token == ".") continue;
        if (token == "-")
        {
            const auto prev = (size_t) ((s + maxSteps - 1) % maxSteps);
            t.on[i] = true;
            t.note[i] = t.note[prev].load();
            t.slide[prev] = true;
            continue;
        }

        const bool accent = token.containsChar ('!'), glide = token.containsChar ('~');
        int offset = parseNote (token.removeCharacters ("!~")) - base;
        while (offset < 0) offset += 12;
        while (offset >= noteRange) offset -= 12;
        t.on[i] = true;
        t.note[i] = offset;
        t.accent[i] = accent;
        if (glide) t.slide[i] = true;
    }
}

struct P
{
    ExampleWriter& w;
    void operator() (const juce::String& id, float value) const { w.setParam (id, value); }
};

// A setting of the steps themselves ("chance", "condition", "delay", "velocity"): one token per step, "." = none.
inline void stepValues (ExampleWriter& w, int type, const char* setting, const juce::String& text)
{
    auto tokens = juce::StringArray::fromTokens (text, " ", "");
    tokens.removeEmptyStrings();
    if (tokens.isEmpty()) return;
    auto& t = w.track (type);
    const int lane = t.laneFor (automation::targetFor (type, setting), true);
    if (lane < 0) return;
    for (int st = 0; st < maxSteps; ++st)
    {
        const auto& token = tokens[st % tokens.size()];
        t.lockValue[(size_t) lane][(size_t) st] = token == "." ? noLock : token.getFloatValue();
    }
}

// Notes written one by one, as in the piano roll of a poly synth: "A3+C4+E4" plays these notes together, "-" holds
// them one more step, "." silence. One token per step, repeated to fill 64 steps.
inline void chords (ExampleWriter& w, int type, const juce::String& text)
{
    auto tokens = juce::StringArray::fromTokens (text, " ", "");
    tokens.removeEmptyStrings();
    jassert (tokens.size() > 0 && maxSteps % tokens.size() == 0);
    if (tokens.isEmpty()) return;

    auto& t = w.track (type);
    const int base = w.baseNote (type);
    std::uint64_t sounding = 0;
    for (int s = 0; s < maxSteps; ++s)
    {
        const auto i = (size_t) s;
        const auto& token = tokens[s % tokens.size()];
        if (token == ".") { sounding = 0; continue; }
        if (token == "-")
        {
            if (sounding == 0) continue;
            t.on[i] = true;
            t.chord[i] = sounding;
            t.tie[(size_t) ((s + maxSteps - 1) % maxSteps)] = sounding;
            continue;
        }
        std::uint64_t mask = 0;
        for (auto& name : juce::StringArray::fromTokens (token.removeCharacters ("!"), "+", ""))
        {
            int offset = parseNote (name) - base;
            while (offset < 0) offset += 12;
            while (offset >= noteRange) offset -= 12;
            mask |= std::uint64_t (1) << offset;
        }
        t.on[i] = true;
        t.chord[i] = mask;
        t.accent[i] = token.containsChar ('!');
        sounding = mask;
    }
}

// Automation values going from one to the other over a number of steps (a filter that opens, a fade...).
inline juce::String ramp (float from, float to, int steps = maxSteps)
{
    juce::String s;
    for (int i = 0; i < steps; ++i) s << juce::String (from + (to - from) * (float) i / (float) juce::jmax (1, steps - 1), 3) << " ";
    return s;
}

// The same automation value for a number of steps.
inline juce::String hold (float value, int steps)
{
    juce::String s;
    for (int i = 0; i < steps; ++i) s << juce::String (value, 3) << " ";
    return s;
}

void techno (ExampleWriter&);
void acid (ExampleWriter&);
void detroit (ExampleWriter&);
void house (ExampleWriter&);
void berlin (ExampleWriter&);
void trance (ExampleWriter&);
void psytrance (ExampleWriter&);
void drumAndBass (ExampleWriter&);
void dubTechno (ExampleWriter&);
void electro (ExampleWriter&);
void boomBap (ExampleWriter&);
void trap (ExampleWriter&);
void lofi (ExampleWriter&);
void disco (ExampleWriter&);
void reggaeton (ExampleWriter&);
void pianoHouse (ExampleWriter&);
void minimal (ExampleWriter&);
void hardTechno (ExampleWriter&);
void jungle (ExampleWriter&);
void breakbeat (ExampleWriter&);
void ukGarage (ExampleWriter&);
void gabber (ExampleWriter&);
void drill (ExampleWriter&);
void rnb (ExampleWriter&);
void funk (ExampleWriter&);
void pop (ExampleWriter&);
void afroHouse (ExampleWriter&);
void reggae (ExampleWriter&);
void ambient (ExampleWriter&);
void synthwave (ExampleWriter&);
void bongoLab (ExampleWriter&);
void pixelQuest (ExampleWriter&);
void robotDisco (ExampleWriter&);
void tearsInRain (ExampleWriter&);
void euclidGarden (ExampleWriter&);
void arpHorizon (ExampleWriter&);
} // namespace techno::demos
