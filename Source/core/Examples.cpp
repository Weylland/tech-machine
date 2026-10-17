#include "Examples.h"

namespace techno
{
namespace
{
const juce::String rest16 = ". . . . . . . . . . . . . . . . ";

juce::String bars (std::initializer_list<juce::String> parts)
{
    juce::String s;
    for (auto& p : parts) s << p << " ";
    return s;
}

juce::String held (const juce::String& note, int steps)
{
    juce::String s = note + " ";
    for (int i = 1; i < steps; ++i) s << "- ";
    return s;
}

int parseNote (juce::String token)
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
void drums (ExampleWriter& w, int type, const char* rowId, const juce::String& pattern)
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
void line (ExampleWriter& w, int type, const juce::String& text)
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

void techno (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 130); p ("swing", 0.05f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.45f); p ("echoLevel", 0.7f); p ("reverbSize", 0.6f); p ("reverbLevel", 0.5f);
    p ("pump", 0.25f); p ("comp", 0.3f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.5f); p ("tr909_bd_tone", 0.6f); p ("tr909_bd_drive", 0.15f);
    p ("tr909_ch_level", 0.45f); p ("tr909_ch_decay", 0.3f); p ("tr909_oh_level", 0.4f); p ("tr909_oh_decay", 0.35f);
    p ("tr909_cp_reverb", 0.2f); p ("tr909_rs_level", 0.4f); p ("tr909_rs_echo", 0.25f); p ("tr909_sd_level", 0.45f);
    p ("tr909_cr_level", 0.35f);
    drums (w, TR909, "bd", "X...x...x...x...");
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "ch", "xx.xxx.xxx.xxx.x");
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "rs", "...x..x....x....");
    drums (w, TR909, "sd", "................................................" "............x.xx");
    drums (w, TR909, "cr", "x..............................................................." );

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_detune", 0.1f); p ("modeld_sub", 0.5f); p ("modeld_cutoff", 0.28f);
    p ("modeld_emphasis", 0.35f); p ("modeld_contour", 0.45f); p ("modeld_decay", 0.25f); p ("modeld_sustain", 0.2f);
    p ("modeld_drive", 0.35f); p ("modeld_volume", 0.7f);
    const juce::String bass = ". . A1 . . . A1 A2 . . A1 . . . A1 A1 ";
    line (w, ModelD, bars ({ bass, bass, bass, ". . A1 . . . A1 A2 . . A1 . . G1 A1 C2" }));

    w.show (Juno106);
    p ("juno106_chord", 1); p ("juno106_cutoff", 0.45f); p ("juno106_reso", 0.25f); p ("juno106_env", 0.5f);
    p ("juno106_decay", 0.28f); p ("juno106_sustain", 0.1f); p ("juno106_release", 0.3f); p ("juno106_chorus", 1);
    p ("juno106_hpf", 2); p ("juno106_echo", 0.45f); p ("juno106_reverb", 0.3f); p ("juno106_volume", 0.6f);
    line (w, Juno106, bars ({ ". . . A3 . . . . . . A3 . . . . .", ". . . A3 . . . . . . A3 . . . . .",
                              ". . . F3 . . . . . . F3 . . . . .", ". . . G3 . . . . . . G3 . . . . ." }));
}

void acid (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 128); p ("swing", 0.05f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("echoLevel", 0.6f); p ("reverbLevel", 0.4f); p ("comp", 0.25f); p ("pump", 0.1f);

    w.show (TR909);
    p ("tr909_ch_level", 0.35f); p ("tr909_ch_decay", 0.25f); p ("tr909_oh_level", 0.4f); p ("tr909_cp_reverb", 0.15f);
    drums (w, TR909, "bd", "X...x...x...x...");
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "ch", "xxxxxxxxxxxxxxxx");
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "cr", "x..............................................................." );

    w.show (TB303);
    p ("tb303_wave", 0); p ("tb303_cutoff", 0.22f); p ("tb303_reso", 0.82f); p ("tb303_env", 0.62f);
    p ("tb303_decay", 0.42f); p ("tb303_accent", 0.75f); p ("tb303_drive", 0.3f); p ("tb303_echo", 0.2f); p ("tb303_reverb", 0.05f);
    const juce::String riff = "A1! A1 A2~ A1 . C2 A1 D2! A1 . A2 G1~ A1 C2! E2 A1 ";
    line (w, TB303, bars ({ riff, riff, riff, "A1! A1 A2~ A1 . C2 A1 D2! A1 . A2 G1~ A1 C2! D2~ E2!" }));
}

void detroit (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 125); p ("swing", 0.15f); p ("key", 5); p ("scale", 1); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.4f); p ("echoLevel", 0.6f); p ("reverbSize", 0.55f); p ("reverbLevel", 0.45f);
    p ("comp", 0.25f); p ("pump", 0.2f);

    w.show (TR909);
    p ("tr909_ch_level", 0.4f); p ("tr909_oh_level", 0.38f); p ("tr909_rs_level", 0.35f); p ("tr909_rd_level", 0.3f);
    p ("tr909_cp_reverb", 0.15f);
    drums (w, TR909, "bd", "x...x...x...x...");
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "ch", "x.xxx.xxx.xxx.xx");
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "rs", "......x.......x.");
    drums (w, TR909, "rd", "................................" "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.");
    drums (w, TR909, "cr", "x..............................................................." );

    w.show (Juno106);
    p ("juno106_chord", 1); p ("juno106_cutoff", 0.5f); p ("juno106_reso", 0.2f); p ("juno106_env", 0.45f);
    p ("juno106_decay", 0.3f); p ("juno106_sustain", 0.25f); p ("juno106_release", 0.35f); p ("juno106_chorus", 2);
    p ("juno106_echo", 0.4f); p ("juno106_reverb", 0.3f); p ("juno106_volume", 0.6f);
    auto stab = [] (const char* n) { const juce::String c (n); return c + " . . " + c + " . . " + c + " . . . " + c + " . . " + c + " . ."; };
    line (w, Juno106, bars ({ stab ("F3"), stab ("C#3"), stab ("D#3"), stab ("C3") }));

    w.show (ModelD);
    p ("modeld_wave", 1); p ("modeld_sub", 0.4f); p ("modeld_cutoff", 0.32f); p ("modeld_contour", 0.4f);
    p ("modeld_decay", 0.3f); p ("modeld_sustain", 0.3f); p ("modeld_glide", 0.1f); p ("modeld_volume", 0.7f);
    line (w, ModelD, bars ({ "F2 . . F2 . . F3 . F2 . . D#2 . . F2 .", "C#2 . . C#2 . . C#3 . C#2 . . C2 . . C#2 .",
                             "D#2 . . D#2 . . D#3 . D#2 . . C#2 . . D#2 .", "C2 . . C2 . . C3 . C2 . . A#2 . . C2 D#2" }));
}

void house (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 124); p ("swing", 0.25f); p ("key", 9); p ("scale", 5); p ("bars", 1);
    p ("echoTime", 2); p ("echoFeedback", 0.3f); p ("echoLevel", 0.5f); p ("reverbLevel", 0.4f); p ("comp", 0.35f); p ("pump", 0.35f);

    w.show (TR909);
    p ("tr909_cp_level", 0.55f); p ("tr909_cp_reverb", 0.15f); p ("tr909_ch_level", 0.3f); p ("tr909_ch_decay", 0.2f);
    p ("tr909_oh_level", 0.4f); p ("tr909_oh_decay", 0.4f); p ("tr909_rs_level", 0.35f);
    drums (w, TR909, "bd", "x...x...x...x...");
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "ch", "xxxxxxxxxxxxxxxx");
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "rs", "...x......x.x...");
    drums (w, TR909, "cr", "x..............................." );

    w.show (Juno106);
    p ("juno106_chord", 1); p ("juno106_cutoff", 0.58f); p ("juno106_reso", 0.15f); p ("juno106_env", 0.35f);
    p ("juno106_decay", 0.35f); p ("juno106_sustain", 0.2f); p ("juno106_release", 0.3f); p ("juno106_chorus", 1);
    p ("juno106_echo", 0.25f); p ("juno106_reverb", 0.3f); p ("juno106_volume", 0.55f);
    line (w, Juno106, bars ({ ". . A3 . . A3 . . . . A3 . . A3 . .", ". . D3 . . D3 . . . . D3 . . D3 . ." }));

    w.show (ModelD);
    p ("modeld_wave", 1); p ("modeld_cutoff", 0.3f); p ("modeld_contour", 0.35f); p ("modeld_decay", 0.35f);
    p ("modeld_sustain", 0.4f); p ("modeld_sub", 0.45f); p ("modeld_volume", 0.7f);
    line (w, ModelD, bars ({ "A1 . . A1 . . A2 . . A1 . . G1 . A1 .", "D2 . . D2 . . D3 . . D2 . . C2 . D2 ." }));
}

void berlin (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 133); p ("swing", 0.0f); p ("key", 9); p ("scale", 6); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.6f); p ("echoLevel", 0.65f); p ("reverbSize", 0.8f); p ("reverbLevel", 0.55f);
    p ("comp", 0.35f); p ("pump", 0.3f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.7f); p ("tr909_bd_tone", 0.5f); p ("tr909_bd_reverb", 0.2f); p ("tr909_bd_drive", 0.1f);
    p ("tr909_ch_level", 0.35f); p ("tr909_ch_decay", 0.22f); p ("tr909_oh_level", 0.38f);
    p ("tr909_rs_level", 0.4f); p ("tr909_rs_echo", 0.35f); p ("tr909_rd_level", 0.3f); p ("tr909_cp_level", 0.5f); p ("tr909_cp_reverb", 0.25f);
    drums (w, TR909, "bd", "X...x...x...x...");
    drums (w, TR909, "ch", "xxxxxxxxxxxxxxxx");
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "rs", "..x..x.x...x..x.");
    drums (w, TR909, "cp", "............x...");
    drums (w, TR909, "rd", "................................" "..x...x...x...x...x...x...x...x.");

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_detune", 0.15f); p ("modeld_sub", 0.55f); p ("modeld_cutoff", 0.22f);
    p ("modeld_emphasis", 0.4f); p ("modeld_contour", 0.5f); p ("modeld_decay", 0.18f); p ("modeld_sustain", 0.05f);
    p ("modeld_drive", 0.4f); p ("modeld_volume", 0.7f);
    const juce::String roll = ". A1 . A1 . A1 . A1 . A1 . A1 . A1 . A1 ";
    line (w, ModelD, bars ({ roll, roll, roll, ". A1 . A1 . A1 . A1 . A#1 . A#1 . A1 . A1" }));

    w.show (Juno106);
    p ("juno106_chord", 2); p ("juno106_cutoff", 0.35f); p ("juno106_reso", 0.35f); p ("juno106_env", 0.5f);
    p ("juno106_decay", 0.4f); p ("juno106_sustain", 0.1f); p ("juno106_release", 0.5f); p ("juno106_chorus", 1);
    p ("juno106_hpf", 2); p ("juno106_echo", 0.7f); p ("juno106_reverb", 0.5f); p ("juno106_volume", 0.5f);
    const juce::String hit = ". . . . . . . . . . A3 . . . . . ";
    line (w, Juno106, bars ({ rest16, hit, rest16, hit }));
}

void trance (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 138); p ("swing", 0.0f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.45f); p ("echoLevel", 0.6f); p ("reverbSize", 0.7f); p ("reverbLevel", 0.55f);
    p ("comp", 0.35f); p ("pump", 0.55f);

    w.show (TR909);
    p ("tr909_ch_level", 0.3f); p ("tr909_sd_level", 0.4f); p ("tr909_cp_reverb", 0.2f);
    drums (w, TR909, "bd", "X...x...x...x...");
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "ch", "xxx.xxx.xxx.xxx.");
    drums (w, TR909, "sd", "................................................" "........xxxxxxxx");
    drums (w, TR909, "cr", "x..............................................................." );

    w.show (JP8000);
    p ("jp8000_chord", 1); p ("jp8000_attack", 0.35f); p ("jp8000_decay", 0.5f); p ("jp8000_sustain", 0.85f);
    p ("jp8000_release", 0.6f); p ("jp8000_cutoff", 0.6f); p ("jp8000_reso", 0.15f); p ("jp8000_env", 0.2f);
    p ("jp8000_detune", 0.45f); p ("jp8000_mix", 0.65f); p ("jp8000_reverb", 0.45f); p ("jp8000_echo", 0.2f); p ("jp8000_volume", 0.55f);
    line (w, JP8000, bars ({ held ("A3", 16), held ("F3", 16), held ("C4", 16), held ("G3", 16) }));

    w.show (SH101);
    p ("sh101_saw", 0.8f); p ("sh101_square", 0.3f); p ("sh101_sub", 0.0f); p ("sh101_cutoff", 0.55f); p ("sh101_reso", 0.3f);
    p ("sh101_env", 0.4f); p ("sh101_attack", 0.0f); p ("sh101_decay", 0.18f); p ("sh101_sustain", 0.0f); p ("sh101_release", 0.15f);
    p ("sh101_echo", 0.35f); p ("sh101_reverb", 0.25f); p ("sh101_volume", 0.5f);
    line (w, SH101, bars ({ "A3 C4 E4 A4 E4 C4 A3 C4 E4 A4 E4 C4 A3 C4 E4 C4", "F3 A3 C4 F4 C4 A3 F3 A3 C4 F4 C4 A3 F3 A3 C4 A3",
                            "C4 E4 G4 C5 G4 E4 C4 E4 G4 C5 G4 E4 C4 E4 G4 E4", "G3 B3 D4 G4 D4 B3 G3 B3 D4 G4 D4 B3 G3 B3 D4 B3" }));

    w.show (ModelD);
    p ("modeld_cutoff", 0.3f); p ("modeld_decay", 0.2f); p ("modeld_sustain", 0.1f); p ("modeld_volume", 0.65f);
    auto off = [] (const char* n) { const juce::String c (n); return ". . " + c + " . . . " + c + " . . . " + c + " . . . " + c + " ."; };
    line (w, ModelD, bars ({ off ("A1"), off ("F1"), off ("C2"), off ("G1") }));
}

void psytrance (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 145); p ("swing", 0.0f); p ("key", 4); p ("scale", 7); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.55f); p ("echoLevel", 0.6f); p ("reverbLevel", 0.4f); p ("pump", 0.15f); p ("comp", 0.3f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.35f); p ("tr909_bd_tone", 0.7f); p ("tr909_oh_level", 0.38f); p ("tr909_ch_level", 0.32f); p ("tr909_cp_level", 0.45f);
    drums (w, TR909, "bd", "x...x...x...x...");
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "ch", "x.xxx.xxx.xxx.xx");
    drums (w, TR909, "cp", "............x...");
    drums (w, TR909, "cr", "x..............................................................." );

    w.show (SH101);
    p ("sh101_octave", 1); p ("sh101_saw", 1.0f); p ("sh101_sub", 0.25f); p ("sh101_cutoff", 0.32f); p ("sh101_reso", 0.4f);
    p ("sh101_env", 0.55f); p ("sh101_decay", 0.12f); p ("sh101_sustain", 0.0f); p ("sh101_release", 0.05f); p ("sh101_volume", 0.75f);
    const juce::String rolling = ". E2 E2 E2 . E2 E2 E2 . E2 E2 E2 . E2 E2 E2 ";
    line (w, SH101, bars ({ rolling, rolling, rolling, ". E2 E2 E2 . E2 E2 E2 . E2 E2 E2 . F2 G#2 B2" }));

    w.show (JP8000);
    p ("jp8000_chord", 0); p ("jp8000_detune", 0.3f); p ("jp8000_mix", 0.4f); p ("jp8000_cutoff", 0.55f); p ("jp8000_env", 0.35f);
    p ("jp8000_decay", 0.25f); p ("jp8000_sustain", 0.25f); p ("jp8000_release", 0.2f); p ("jp8000_echo", 0.4f);
    p ("jp8000_reverb", 0.3f); p ("jp8000_volume", 0.45f);
    const juce::String riff = "E4 . E4 F4 . E4 . G#4 E4 . B3 . E4 F4 . G#4 ";
    line (w, JP8000, bars ({ rest16, rest16, riff, riff }));
}

void drumAndBass (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 174); p ("swing", 0.0f); p ("key", 5); p ("scale", 1); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("reverbSize", 0.65f); p ("reverbLevel", 0.45f); p ("comp", 0.3f);

    w.show (BreakKit);
    p ("breaks_crush", 0.25f); p ("breaks_sd_level", 0.7f); p ("breaks_gs_level", 0.3f); p ("breaks_ch_level", 0.4f);
    p ("breaks_sd_reverb", 0.15f);
    drums (w, BreakKit, "bd", "x.........x....." "x.........x..x..");
    drums (w, BreakKit, "sd", "....x.......x...");
    drums (w, BreakKit, "gs", "......x..x....x.");
    drums (w, BreakKit, "ch", "x.x.x.x.x.x.x.x.");
    drums (w, BreakKit, "oh", "..x.......x.....");
    drums (w, BreakKit, "cr", "x..............................................................." );

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_detune", 0.75f); p ("modeld_sub", 0.55f); p ("modeld_cutoff", 0.3f); p ("modeld_emphasis", 0.2f);
    p ("modeld_contour", 0.15f); p ("modeld_attack", 0.05f); p ("modeld_decay", 0.6f); p ("modeld_sustain", 0.9f);
    p ("modeld_glide", 0.3f); p ("modeld_drive", 0.45f); p ("modeld_volume", 0.7f);
    line (w, ModelD, bars ({ held ("F2", 16), held ("F2", 8) + held ("G#2", 4) + held ("D#2", 4),
                             held ("C#2", 16), held ("C#2", 8) + held ("D#2", 8) }));

    w.show (Juno106);
    p ("juno106_chord", 1); p ("juno106_attack", 0.45f); p ("juno106_release", 0.6f); p ("juno106_sustain", 0.8f);
    p ("juno106_cutoff", 0.5f); p ("juno106_hpf", 2); p ("juno106_chorus", 2); p ("juno106_reverb", 0.5f);
    p ("juno106_echo", 0.2f); p ("juno106_volume", 0.45f);
    line (w, Juno106, bars ({ held ("F3", 32), held ("C#3", 32) }));
}

void dubTechno (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 118); p ("swing", 0.1f); p ("key", 0); p ("scale", 1); p ("bars", 1);
    p ("echoTime", 2); p ("echoFeedback", 0.7f); p ("echoLevel", 0.75f); p ("reverbSize", 0.85f); p ("reverbLevel", 0.6f); p ("comp", 0.2f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.4f); p ("tr909_bd_tone", 0.3f); p ("tr909_bd_level", 0.8f);
    p ("tr909_oh_level", 0.3f); p ("tr909_oh_decay", 0.3f); p ("tr909_rs_level", 0.35f); p ("tr909_rs_echo", 0.45f);
    drums (w, TR909, "bd", "x...x...x...x...");
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "rs", ".......x......x.");

    w.show (Juno106);
    p ("juno106_chord", 4); p ("juno106_cutoff", 0.38f); p ("juno106_reso", 0.3f); p ("juno106_env", 0.3f); p ("juno106_decay", 0.22f);
    p ("juno106_sustain", 0.05f); p ("juno106_release", 0.4f); p ("juno106_chorus", 1); p ("juno106_hpf", 2);
    p ("juno106_echo", 0.85f); p ("juno106_reverb", 0.6f); p ("juno106_filter", 0.42f); p ("juno106_volume", 0.6f);
    line (w, Juno106, ". . C3 . . . . . . . C3 . . . . .");

    w.show (ModelD);
    p ("modeld_wave", 2); p ("modeld_cutoff", 0.25f); p ("modeld_sub", 0.6f); p ("modeld_sustain", 0.8f); p ("modeld_decay", 0.5f);
    p ("modeld_volume", 0.7f);
    line (w, ModelD, held ("C2", 6) + ". . " + held ("C2", 4) + ". . . .");
}

void electro (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 126); p ("swing", 0.05f); p ("key", 2); p ("scale", 6); p ("bars", 1);
    p ("echoTime", 2); p ("echoFeedback", 0.45f); p ("echoLevel", 0.55f); p ("reverbLevel", 0.4f); p ("comp", 0.25f);

    w.show (TR808);
    p ("tr808_bd_decay", 0.6f); p ("tr808_ch_level", 0.45f); p ("tr808_cb_level", 0.35f); p ("tr808_cl_level", 0.35f);
    p ("tr808_sd_reverb", 0.15f);
    drums (w, TR808, "bd", "x.....x...x..x..");
    drums (w, TR808, "sd", "....x.......x...");
    drums (w, TR808, "ch", "x.x.x.x.x.x.x.x.");
    drums (w, TR808, "cb", "..x.......x.....");
    drums (w, TR808, "cl", "...x..x....x....");
    drums (w, TR808, "cy", "x..............................." );

    w.show (SH101);
    p ("sh101_octave", 1); p ("sh101_square", 0.8f); p ("sh101_pwm", 0.5f); p ("sh101_saw", 0.3f); p ("sh101_cutoff", 0.38f);
    p ("sh101_reso", 0.45f); p ("sh101_env", 0.55f); p ("sh101_decay", 0.2f); p ("sh101_sustain", 0.1f); p ("sh101_volume", 0.7f);
    line (w, SH101, "D2 . . D2 . . D3 . D2 . . F2 . . D2 C3");

    w.show (JP8000);
    p ("jp8000_chord", 0); p ("jp8000_detune", 0.15f); p ("jp8000_mix", 0.3f); p ("jp8000_cutoff", 0.5f); p ("jp8000_env", 0.4f);
    p ("jp8000_decay", 0.3f); p ("jp8000_sustain", 0.3f); p ("jp8000_release", 0.25f); p ("jp8000_echo", 0.4f);
    p ("jp8000_reverb", 0.3f); p ("jp8000_volume", 0.4f);
    line (w, JP8000, bars ({ "D4 - - . . . F4 . . . A4 - . . . .", "D4 - - . . . F4 . . . C5 - - . . ." }));
}

void boomBap (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 90); p ("swing", 0.35f); p ("key", 2); p ("scale", 5); p ("bars", 1);
    p ("echoTime", 2); p ("echoFeedback", 0.3f); p ("reverbSize", 0.4f); p ("reverbLevel", 0.35f); p ("comp", 0.35f);

    w.show (MPC);
    p ("mpc_bd_level", 0.9f); p ("mpc_sd_level", 0.75f); p ("mpc_ch_level", 0.4f); p ("mpc_sh_level", 0.25f);
    p ("mpc_sd_reverb", 0.15f);
    drums (w, MPC, "bd", "x......x..x....." "x......xx.....x.");
    drums (w, MPC, "sd", "....x.......x...");
    drums (w, MPC, "ch", "x.x.x.x.x.x.x.x.");
    drums (w, MPC, "sh", "..x...x...x...x.");
    drums (w, MPC, "rs", "................" "..........x.....");

    w.show (DX7);
    p ("dx7_octave", 2); p ("dx7_chord", 1); p ("dx7_preset", 0); p ("dx7_brightness", 0.35f); p ("dx7_decay", 0.6f);
    p ("dx7_release", 0.35f); p ("dx7_tremolo", 0.3f); p ("dx7_reverb", 0.3f); p ("dx7_volume", 0.6f);
    line (w, DX7, bars ({ "D3 - - - - - . . . . D3 - - . . .", "G3 - - - - - . . . . A3 - - . . ." }));

    w.show (ModelD);
    p ("modeld_wave", 2); p ("modeld_cutoff", 0.3f); p ("modeld_sub", 0.5f); p ("modeld_contour", 0.2f); p ("modeld_decay", 0.5f);
    p ("modeld_sustain", 0.6f); p ("modeld_volume", 0.7f);
    line (w, ModelD, bars ({ "D2 - - . . . . D2 . . F2 - . . . .", "G2 - - . . . . G2 . . A2 . C3 . . ." }));
}

void trap (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 140); p ("swing", 0.0f); p ("key", 4); p ("scale", 3); p ("bars", 1);
    p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("echoLevel", 0.5f); p ("reverbSize", 0.6f); p ("reverbLevel", 0.4f); p ("comp", 0.3f);

    w.show (TR808);
    p ("tr808_bd_decay", 0.35f); p ("tr808_ch_level", 0.4f); p ("tr808_ch_decay", 0.2f); p ("tr808_cp_level", 0.6f);
    p ("tr808_cp_reverb", 0.2f); p ("tr808_sd_level", 0.45f);
    drums (w, TR808, "bd", "x......x..x....." "x.....x...x..x..");
    drums (w, TR808, "cp", "........x.......");
    drums (w, TR808, "sd", "........x.......");
    drums (w, TR808, "ch", "x.x.x.x.x.x3x.x." "x.x.x.x2x.x.x4x2");
    drums (w, TR808, "oh", "................" ".......x........");

    w.show (Sub808);
    p ("sub808_octave", 2); p ("sub808_decay", 0.75f); p ("sub808_glide", 0.35f); p ("sub808_punch", 0.5f);
    p ("sub808_drive", 0.35f); p ("sub808_volume", 0.75f);
    line (w, Sub808, bars ({ "E1 - - - - - - E1 - - E1 - - - - -", "E1 - - - - - C2 - - - B1 - - G1 - -" }));

    w.show (DX7);
    p ("dx7_octave", 3); p ("dx7_chord", 0); p ("dx7_preset", 2); p ("dx7_brightness", 0.5f); p ("dx7_decay", 0.45f);
    p ("dx7_release", 0.4f); p ("dx7_echo", 0.4f); p ("dx7_reverb", 0.35f); p ("dx7_volume", 0.5f);
    line (w, DX7, bars ({ "E5 . . B4 . . G4 . . . . . F#4 . G4 .", "E5 . . B4 . . C5 . . . B4 . D#5 . . ." }));

    w.show (Strings);
    p ("strings_octave", 2); p ("strings_chord", 1); p ("strings_attack", 0.6f); p ("strings_release", 0.6f);
    p ("strings_brightness", 0.35f); p ("strings_reverb", 0.4f); p ("strings_volume", 0.35f);
    line (w, Strings, bars ({ held ("E3", 16), held ("C3", 16) }));
}

void lofi (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 80); p ("swing", 0.45f); p ("key", 0); p ("scale", 2); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("echoLevel", 0.4f); p ("reverbSize", 0.55f); p ("reverbLevel", 0.4f);
    p ("comp", 0.3f); p ("djFilter", 0.42f);

    w.show (CR78);
    p ("cr78_bd_level", 0.9f); p ("cr78_sd_level", 0.7f); p ("cr78_ch_level", 0.4f); p ("cr78_rs_level", 0.4f);
    p ("cr78_ma_level", 0.3f); p ("cr78_sd_reverb", 0.2f);
    drums (w, CR78, "bd", "x......x.x......" "x.......x.x.....");
    drums (w, CR78, "sd", "....x.......x...");
    drums (w, CR78, "ch", "x.x.x.x.x.x.x.x.");
    drums (w, CR78, "rs", "..........x.....");
    drums (w, CR78, "ma", "..x...x...x...x.");

    w.show (SoundFx);
    p ("fx_vin_level", 0.5f); p ("fx_vin_decay", 0.92f);
    drums (w, SoundFx, "vin", "x...............................................................");

    w.show (DX7);
    p ("dx7_octave", 2); p ("dx7_chord", 1); p ("dx7_preset", 0); p ("dx7_brightness", 0.3f); p ("dx7_decay", 0.7f);
    p ("dx7_release", 0.45f); p ("dx7_tremolo", 0.45f); p ("dx7_echo", 0.2f); p ("dx7_reverb", 0.35f); p ("dx7_volume", 0.6f);
    line (w, DX7, bars ({ "C3 - - - - - - . . . C3 - - . . .", "A2 - - - - - - . . . A2 - - . . .",
                          "D3 - - - - - - . . . D3 - - . . .", "G2 - - - - - - . . . G2 - B2 - . ." }));

    w.show (ModelD);
    p ("modeld_wave", 2); p ("modeld_cutoff", 0.25f); p ("modeld_sub", 0.4f); p ("modeld_contour", 0.15f); p ("modeld_decay", 0.5f);
    p ("modeld_sustain", 0.6f); p ("modeld_volume", 0.6f);
    line (w, ModelD, bars ({ "C2 - - . . . . . . . G1 - . . . .", "A1 - - . . . . . . . E2 - . . . .",
                             "D2 - - . . . . . . . A1 - . . . .", "G1 - - . . . . . . . B1 - . . . ." }));
}

void disco (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 120); p ("swing", 0.05f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.3f); p ("echoLevel", 0.4f); p ("reverbSize", 0.6f); p ("reverbLevel", 0.4f); p ("comp", 0.3f);

    w.show (LinnDrum);
    p ("linn_bd_level", 0.85f); p ("linn_sd_level", 0.7f); p ("linn_oh_level", 0.4f); p ("linn_ch_level", 0.35f);
    p ("linn_tb_level", 0.3f); p ("linn_sd_reverb", 0.2f);
    drums (w, LinnDrum, "bd", "x...x...x...x...");
    drums (w, LinnDrum, "sd", "....x.......x...");
    drums (w, LinnDrum, "oh", "..x...x...x...x.");
    drums (w, LinnDrum, "ch", "x.x.x.x.x.x.x.x.");
    drums (w, LinnDrum, "tb", "xxxxxxxxxxxxxxxx");
    drums (w, LinnDrum, "cr", "x..............................................................." );

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_cutoff", 0.35f); p ("modeld_emphasis", 0.25f); p ("modeld_contour", 0.4f);
    p ("modeld_decay", 0.25f); p ("modeld_sustain", 0.3f); p ("modeld_volume", 0.7f);
    line (w, ModelD, bars ({ "A1 . A2 . A1 . A2 . A1 . A2 . A1 . A2 .", "D2 . D3 . D2 . D3 . D2 . D3 . D2 . D3 .",
                             "G1 . G2 . G1 . G2 . G1 . G2 . G1 . G2 .", "C2 . C3 . C2 . C3 . E2 . E3 . E2 . E3 ." }));

    w.show (Strings);
    p ("strings_octave", 2); p ("strings_chord", 1); p ("strings_attack", 0.35f); p ("strings_release", 0.5f);
    p ("strings_ensemble", 0.8f); p ("strings_reverb", 0.35f); p ("strings_volume", 0.45f);
    line (w, Strings, bars ({ held ("A3", 16), held ("D3", 16), held ("G3", 16), held ("C3", 8) + held ("E3", 8) }));

    w.show (Organ);
    p ("organ_octave", 2); p ("organ_chord", 1); p ("organ_type", 0); p ("organ_percussion", 0.5f); p ("organ_leslie", 0.5f);
    p ("organ_volume", 0.4f);
    line (w, Organ, bars ({ ". . A3 . . . A3 . . . A3 . . . A3 .", ". . D3 . . . D3 . . . D3 . . . D3 .",
                            ". . G3 . . . G3 . . . G3 . . . G3 .", ". . C3 . . . C3 . . . E3 . . . E3 ." }));
}

void reggaeton (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 95); p ("swing", 0.0f); p ("key", 7); p ("scale", 1); p ("bars", 1);
    p ("echoTime", 2); p ("echoFeedback", 0.3f); p ("echoLevel", 0.4f); p ("reverbLevel", 0.3f); p ("comp", 0.35f);

    w.show (TR808);
    p ("tr808_bd_decay", 0.4f); p ("tr808_sd_level", 0.55f); p ("tr808_ch_level", 0.35f); p ("tr808_rs_level", 0.4f);
    drums (w, TR808, "bd", "x...x...x...x...");
    drums (w, TR808, "sd", "...x..x....x..x.");
    drums (w, TR808, "ch", "x.x.x.x.x.x.x.x.");

    w.show (Percussion);
    p ("perc_th_level", 0.45f); p ("perc_cgh_level", 0.45f); p ("perc_sh_level", 0.3f);
    drums (w, Percussion, "th", "................" "............x.xx");
    drums (w, Percussion, "cgh", "..x.......x.....");
    drums (w, Percussion, "sh", "x.xxx.xxx.xxx.xx");

    w.show (Sub808);
    p ("sub808_octave", 2); p ("sub808_decay", 0.55f); p ("sub808_glide", 0.25f); p ("sub808_punch", 0.5f);
    p ("sub808_drive", 0.25f); p ("sub808_volume", 0.75f);
    line (w, Sub808, bars ({ "G1 - . G1 - . G1 - D#1 - . D#1 - . D#1 -", "A#1 - . A#1 - . A#1 - F1 - . F1 - . F1 -" }));

    w.show (DX7);
    p ("dx7_octave", 2); p ("dx7_chord", 1); p ("dx7_preset", 4); p ("dx7_brightness", 0.45f); p ("dx7_decay", 0.4f);
    p ("dx7_echo", 0.3f); p ("dx7_reverb", 0.25f); p ("dx7_volume", 0.55f);
    line (w, DX7, bars ({ ". . G3 . . . G3 . . . D#3 . . . D#3 .", ". . A#3 . . . A#3 . . . F3 . . . F3 ." }));
}

void synthwave (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 100); p ("swing", 0.0f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.45f); p ("echoLevel", 0.5f); p ("reverbSize", 0.8f); p ("reverbLevel", 0.5f); p ("comp", 0.3f);

    w.show (LinnDrum);
    p ("linn_bd_level", 0.85f); p ("linn_sd_level", 0.75f); p ("linn_sd_reverb", 0.45f); p ("linn_ch_level", 0.35f);
    p ("linn_lt_level", 0.5f); p ("linn_ht_level", 0.5f);
    drums (w, LinnDrum, "bd", "x...x...x...x...");
    drums (w, LinnDrum, "sd", "....X.......X...");
    drums (w, LinnDrum, "ch", "x.x.x.x.x.x.x.x.");
    drums (w, LinnDrum, "ht", "................................................" "............x...");
    drums (w, LinnDrum, "lt", "................................................" "..............x.");
    drums (w, LinnDrum, "cr", "x..............................................................." );

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_cutoff", 0.3f); p ("modeld_emphasis", 0.2f); p ("modeld_contour", 0.35f);
    p ("modeld_decay", 0.2f); p ("modeld_sustain", 0.2f); p ("modeld_volume", 0.7f);
    line (w, ModelD, bars ({ "A1 . A1 . A1 . A1 . A1 . A1 . A1 . A1 .", "F1 . F1 . F1 . F1 . F1 . F1 . F1 . F1 .",
                             "C2 . C2 . C2 . C2 . C2 . C2 . C2 . C2 .", "G1 . G1 . G1 . G1 . G1 . G1 . G1 . G1 ." }));

    w.show (Strings);
    p ("strings_octave", 2); p ("strings_chord", 1); p ("strings_attack", 0.5f); p ("strings_release", 0.6f);
    p ("strings_ensemble", 0.85f); p ("strings_reverb", 0.4f); p ("strings_volume", 0.4f);
    line (w, Strings, bars ({ held ("A3", 16), held ("F3", 16), held ("C3", 16), held ("G3", 16) }));

    w.show (JP8000);
    p ("jp8000_chord", 0); p ("jp8000_detune", 0.25f); p ("jp8000_mix", 0.4f); p ("jp8000_cutoff", 0.55f); p ("jp8000_env", 0.3f);
    p ("jp8000_decay", 0.4f); p ("jp8000_sustain", 0.6f); p ("jp8000_release", 0.35f); p ("jp8000_echo", 0.5f);
    p ("jp8000_reverb", 0.4f); p ("jp8000_volume", 0.4f);
    line (w, JP8000, bars ({ "E5 - - - D5 - C5 - D5 - - - . . . .", "C5 - - - A4 - - - . . . . . . . .",
                             "E5 - - - G5 - - - E5 - D5 - C5 - - -", "D5 - - - - - - - B4 - - - . . . ." }));
}

void pianoHouse (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 124); p ("swing", 0.15f); p ("key", 5); p ("scale", 2); p ("bars", 3);
    p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("echoLevel", 0.45f); p ("reverbSize", 0.65f); p ("reverbLevel", 0.45f);
    p ("pump", 0.2f); p ("comp", 0.3f);

    w.show (TR909);
    p ("tr909_ch_level", 0.4f); p ("tr909_oh_level", 0.4f); p ("tr909_cp_reverb", 0.2f); p ("tr909_cr_level", 0.35f);
    drums (w, TR909, "bd", "x...x...x...x...");
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "ch", "xx.xxx.xxx.xxx.x");
    drums (w, TR909, "cr", "x..............................................................." );

    w.show (Piano);
    p ("piano_octave", 2); p ("piano_chord", 1); p ("piano_type", 3); p ("piano_brightness", 0.6f); p ("piano_decay", 0.4f);
    p ("piano_release", 0.25f); p ("piano_hammer", 0.5f); p ("piano_reverb", 0.25f); p ("piano_volume", 0.6f);
    line (w, Piano, bars ({ "F3 . . F3 . . F3 . . . F3 . . F3 . .", "D3 . . D3 . . D3 . . . D3 . . D3 . .",
                            "A#2 . . A#2 . . A#2 . . . A#2 . . A#2 . .", "C3 . . C3 . . C3 . . . C3 . . C3 . ." }));

    w.show (Choir);
    p ("choir_octave", 2); p ("choir_chord", 1); p ("choir_vowel", 0); p ("choir_attack", 0.6f); p ("choir_release", 0.6f);
    p ("choir_reverb", 0.45f); p ("choir_volume", 0.4f);
    line (w, Choir, bars ({ held ("F3", 16), held ("D3", 16), held ("A#2", 16), held ("C3", 16) }));

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_cutoff", 0.3f); p ("modeld_contour", 0.35f); p ("modeld_decay", 0.25f);
    p ("modeld_sustain", 0.3f); p ("modeld_volume", 0.65f);
    line (w, ModelD, bars ({ ". . F1 . . . F1 . . . F1 . . . F1 .", ". . D2 . . . D2 . . . D2 . . . D2 .",
                             ". . A#1 . . . A#1 . . . A#1 . . . A#1 .", ". . C2 . . . C2 . . . C2 . . . C2 ." }));

    w.show (SoundFx);
    p ("fx_imp_level", 0.6f); p ("fx_rise_level", 0.5f); p ("fx_rise_decay", 0.4f);
    drums (w, SoundFx, "imp", "x...............................................................");
    drums (w, SoundFx, "rise", "................................................x...............");
}
} // namespace

const std::vector<ExampleInfo>& examples()
{
    static const std::vector<ExampleInfo> list {
        { "Warehouse", "Techno", "Techno hypnotique : kick 909, basse qui roule, stabs d'accords dans l'écho.", 130 },
        { "Acid Rain", "Acid", "Une ligne de 303 qui miaule avec accents et glissés, sur un groove 909.", 128 },
        { "Motor City", "Detroit", "Accords chaleureux du 106, basse funky et ride : l'esprit de Detroit.", 125 },
        { "Deep Sunday", "House", "House profonde et chaloupée : accords Am7 / D7, basse ronde, swing.", 124 },
        { "Basement", "Berlin", "Techno sombre : kick qui gronde, rimshots dans l'écho, sub qui roule.", 133 },
        { "Euphoria", "Trance", "Nappes supersaw sur 4 accords, arpège du 101, basse sur les contretemps.", 138 },
        { "Goa Sunrise", "Psytrance", "Basse « kick-basse-basse-basse » et riff oriental qui arrive à la 3e mesure.", 145 },
        { "Night Bus", "Drum & Bass", "Breakbeat rapide, basse Reese qui gronde et nappes planantes.", 174 },
        { "Fog", "Dub Techno", "Un accord Cm7 qui se perd dans l'écho et la reverb, sub profonde.", 118 },
        { "Robot Funk", "Électro", "Rythme 808 syncopé, cowbell, basse robotique et lead froid.", 126 },
        { "Golden Era", "Hip-Hop", "Boom bap des années 90 : kit MPC swingué, piano électrique DX7 jazzy, basse ronde.", 90 },
        { "Midnight Trap", "Trap", "Charlestons en roulements, grosse basse 808 qui glisse, cloche FM et nappe de cordes.", 140 },
        { "Rainy Window", "Lo-fi", "Lo-fi tranquille : CR-78 feutrée, piano électrique, craquements de vinyle, beaucoup de swing.", 80 },
        { "Studio 54", "Disco", "Disco : kick sur chaque temps, basse en octaves, cordes et orgue sur les contretemps.", 120 },
        { "Perreo", "Reggaeton", "Le dembow à la 808, percussions latines, basse 808 sur le tresillo et marimba FM.", 95 },
        { "Piano Paradise", "House", "Piano house des années 90 : stabs de piano, chœurs « aah », basse sur les contretemps, riser et impact.", 124 },
        { "Neon Drive", "Synthwave", "Années 80 : LinnDrum à grosse snare, basse qui pulse, nappes de cordes et lead nostalgique.", 100 },
    };
    return list;
}

void buildExample (int index, ExampleWriter& w)
{
    switch (index)
    {
        case 0: techno (w); break;
        case 1: acid (w); break;
        case 2: detroit (w); break;
        case 3: house (w); break;
        case 4: berlin (w); break;
        case 5: trance (w); break;
        case 6: psytrance (w); break;
        case 7: drumAndBass (w); break;
        case 8: dubTechno (w); break;
        case 9: electro (w); break;
        case 10: boomBap (w); break;
        case 11: trap (w); break;
        case 12: lofi (w); break;
        case 13: disco (w); break;
        case 14: reggaeton (w); break;
        case 15: pianoHouse (w); break;
        case 16: synthwave (w); break;
        default: break;
    }
}
} // namespace techno
