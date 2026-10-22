#include "ExampleTools.h"

// Demos played like a band: disco, funk, pop, reggae, synthwave, ambient.
namespace techno::demos
{
void disco (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 120); p ("swing", 0.05f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.3f); p ("echoLevel", 0.4f);
    p ("reverbType", 2); p ("reverbSize", 0.55f); p ("reverbLevel", 0.4f);
    p ("comp", 0.35f);

    w.show (LinnDrum);
    p ("linn_bd_level", 0.6f); p ("linn_sd_level", 0.5f); p ("linn_sd_reverb", 0.25f); p ("linn_cp_level", 0.38f);
    p ("linn_ch_level", 0.32f); p ("linn_oh_level", 0.36f); p ("linn_tb_level", 0.25f); p ("linn_cg_level", 0.28f);
    p ("linn_lt_level", 0.42f); p ("linn_ht_level", 0.42f); p ("linn_cr_level", 0.32f);

    w.show (EBass);
    p ("ebass_type", 0); p ("ebass_tone", 0.55f); p ("ebass_decay", 0.4f); p ("ebass_muting", 0.45f); p ("ebass_volume", 0.85f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_violin", 0.85f); p ("strings_cello", 0.4f); p ("strings_attack", 0.25f); p ("strings_release", 0.45f);
    p ("strings_ensemble", 0.75f); p ("strings_brightness", 0.6f); p ("strings_reverb", 0.4f); p ("strings_volume", 0.6f);

    w.show (Organ);
    p ("organ_chord", 0); p ("organ_type", 0); p ("organ_percussion", 0.5f); p ("organ_leslie", 0.5f); p ("organ_click", 0.4f);
    p ("organ_volume", 0.6f);

    // A minor: Am7 - Dm7 - G - C. Four on the floor, the bass in octaves, the organ on the off-beats, the strings sweep up.
    const juce::String none = "................", kick = "x...x...x...x...";
    auto oct = [] (const juce::String& lo, const juce::String& hi) { return lo + " . " + hi + " . " + lo + " . " + hi + " . " + lo + " . " + hi + " . " + lo + " . " + hi + " . "; };
    const juce::String bass = bars ({ oct ("A1", "A2"), oct ("D2", "D3"), oct ("G1", "G2"), oct ("C2", "C3") });
    auto offs = [] (const juce::String& c) { return ". . " + c + " . . . " + c + " . . . " + c + " . . . " + c + " . "; };
    const juce::String organ = bars ({ offs ("G3+C4+E4"), offs ("F3+A3+C4"), offs ("F3+B3+D4"), offs ("E3+G3+C4") });
    const juce::String pads = bars ({ held ("C4+E4+G4", 16), held ("C4+F4+A4", 16), held ("B3+D4+G4", 16), held ("C4+E4+G4", 16) });
    const juce::String theme = bars ({ "E5 - - - . . A5 - G5 - E5 - . . C5 - ", "D5 - - - . . F5 - E5 - D5 - . . A4 - ",
                                       "B4 - - - . . D5 - G5 - - - F5 - E5 - ", "E5 - - - - - - - . . A4 B4 C5 D5 E5 G5 " });

    auto beat = [&] (bool full)
    {
        drums (w, LinnDrum, "bd", kick);
        drums (w, LinnDrum, "sd", "....x.......x...");
        drums (w, LinnDrum, "oh", "..x...x...x...x.");
        drums (w, LinnDrum, "ch", "x...x...x...x...");
        if (! full) return;
        drums (w, LinnDrum, "tb", "xxxxxxxxxxxxxxxx");
        stepValues (w, LinnDrum, "linn_tb_level", "0.1 0.15 0.25 0.15 ");
        drums (w, LinnDrum, "cg", "...x..x....x..x.");
    };

    // A - intro: drums and bass.
    w.part (0);
    beat (false);
    line (w, EBass, bass);

    // B - verse: the organ on the off-beats, the strings hold the chords.
    w.part (1);
    beat (true);
    line (w, EBass, bass);
    chords (w, Organ, organ);
    chords (w, Strings, pads);

    // C - chorus: the strings play the theme, crash.
    w.part (2);
    beat (true);
    drums (w, LinnDrum, "cr", "x...............................................................");
    line (w, EBass, bass);
    chords (w, Organ, organ);
    chords (w, Strings, theme);

    // D - break: the drums alone with the tom fill, then the strings climb back.
    w.part (3);
    drums (w, LinnDrum, "bd", kick);
    drums (w, LinnDrum, "oh", "..x...x...x...x.");
    drums (w, LinnDrum, "cg", "...x..x....x..x.");
    drums (w, LinnDrum, "ht", bars ({ none, none, none, "........x.x....." }));
    drums (w, LinnDrum, "lt", bars ({ none, none, none, "............x.x." }));
    chords (w, Strings, bars ({ rest16, rest16, "A4 . B4 . C5 . D5 . E5 . F5 . G5 . A5 . ", "B5 - - - - - - - - - - - - - - - " }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 2 }, { 1, 1 }, { 3, 1 }, { 2, 3 }, { 0, 1 } });
}

void funk (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 105); p ("swing", 0.1f); p ("key", 4); p ("scale", 0); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 1); p ("echoFeedback", 0.2f); p ("echoLevel", 0.3f);
    p ("reverbType", 0); p ("reverbSize", 0.45f); p ("reverbLevel", 0.3f);
    p ("comp", 0.4f);

    w.show (AcousticKit);
    p ("kit_bd_level", 0.6f); p ("kit_sd_level", 0.55f); p ("kit_gs_level", 0.22f); p ("kit_ch_level", 0.34f); p ("kit_oh_level", 0.3f);
    p ("kit_cr_level", 0.3f); p ("kit_ht_level", 0.4f); p ("kit_mt_level", 0.4f); p ("kit_lt_level", 0.4f);

    w.show (EBass);
    p ("ebass_type", 1); p ("ebass_tone", 0.6f); p ("ebass_decay", 0.35f); p ("ebass_muting", 0.5f); p ("ebass_volume", 0.85f);

    w.show (EPiano);
    p ("epiano_chord", 0); p ("epiano_type", 2); p ("epiano_tone", 0.6f); p ("epiano_decay", 0.3f); p ("epiano_release", 0.15f);
    p ("epiano_volume", 0.95f);

    w.show (Pluck);
    p ("pluck_chord", 0); p ("pluck_type", 2); p ("pluck_brightness", 0.7f); p ("pluck_decay", 0.3f); p ("pluck_strum", 0.1f);
    p ("pluck_volume", 1.0f);

    w.show (Brass);
    p ("brass_chord", 0); p ("brass_type", 3); p ("brass_brightness", 0.6f); p ("brass_attack", 0.1f); p ("brass_release", 0.2f);
    p ("brass_reverb", 0.25f); p ("brass_volume", 0.7f);

    // E7 (Mixolydian, played freely for the blue notes): one E7 groove, then A7 for the bridge. Ghost notes on the snare, slap bass, clavinet and guitar
    // that answer each other, horns that punch.
    const juce::String none = "................";
    const juce::String kicks = bars ({ "x.x.......x..x..", "x.x.......x.....", "x.x.......x..x..", "x.x....x..x....." });
    const juce::String ghosts = ".x..x..x.x..x.xx";
    const juce::String bass = bars ({ "E1 . E2 . . E1 . D2 . E1 . . G1 . G#1 . ", "E1 . E2 . . E1 . D2 . B1 . . A1 . B1 . ",
                                      "E1 . E2 . . E1 . D2 . E1 . . G1 . G#1 . ", "A1 . A2 . . A1 . G2 . A1 . . C#2 . E2 . " });
    const juce::String clav = ". G#3+D4 . G#3+D4 . . G#3+D4 . . G#3+D4 . . . G#3+D4 . G#3+D4 ";
    const juce::String guitar = "E4+G#4+D5 . . . . . . E4+G#4+D5 . . . . E4+G#4+D5 . . . ";
    const juce::String horns = bars ({ rest16, ". . . . . . . . . . G#4+B4+D5 . . G#4+B4+D5 - . ", rest16,
                                       ". . . . . . . . A4+C#5+E5 . A4+C#5+E5 . . B4+D5+F#5 - - " });

    auto beat = [&]
    {
        drums (w, AcousticKit, "bd", kicks);
        drums (w, AcousticKit, "sd", "....x.......x...");
        drums (w, AcousticKit, "gs", ghosts);
        drums (w, AcousticKit, "ch", "xxxxxxxxxxxxxxxx");
        stepValues (w, AcousticKit, "kit_ch_level", "0.4 0.18 0.28 0.18 ");
        drums (w, AcousticKit, "oh", ".............x..");
    };

    // A - intro: the drums alone.
    w.part (0);
    beat();

    // B - the slap bass and the clavinet.
    w.part (1);
    beat();
    line (w, EBass, bass);
    chords (w, EPiano, clav);

    // C - the guitar answers, the horns punch.
    w.part (2);
    beat();
    line (w, EBass, bass);
    chords (w, EPiano, clav);
    chords (w, Pluck, guitar);
    chords (w, Brass, horns);

    // D - bridge: A7, the drums open up, tom fill.
    w.part (3);
    drums (w, AcousticKit, "bd", kicks);
    drums (w, AcousticKit, "sd", "....x.......x...");
    drums (w, AcousticKit, "gs", ghosts);
    drums (w, AcousticKit, "oh", "x.x.x.x.x.x.x.x.");
    drums (w, AcousticKit, "ht", bars ({ none, none, none, "........xx......" }));
    drums (w, AcousticKit, "mt", bars ({ none, none, none, "..........xx...." }));
    drums (w, AcousticKit, "lt", bars ({ none, none, none, "............xx.." }));
    line (w, EBass, bars ({ "A1 . A2 . . A1 . G2 . A1 . . C2 . C#2 . ", "A1 . A2 . . A1 . G2 . E2 . . D2 . E2 . ",
                            "A1 . A2 . . A1 . G2 . A1 . . C2 . C#2 . ", "B1 - - - . . B1 . . . D2 . . E2 - - " }));
    chords (w, Brass, bars ({ "A4+C#5+E5 - . . . . . . . . . . . . . . ", ". . . . . . . . . . A4+C#5+G5 . . A4+C#5+G5 - . ",
                              "A4+C#5+E5 - . . . . . . . . . . . . . . ", "B4+D5+F#5 - - - . . . . . . . . . . . . " }));
    chords (w, Pluck, "C#4+G4+A4 . . . . . . C#4+G4+A4 . . . . C#4+G4+A4 . . . ");

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 1 }, { 2, 3 }, { 1, 1 } });
}

void pop (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 110); p ("swing", 0.0f); p ("key", 0); p ("scale", 2); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.3f); p ("echoLevel", 0.4f);
    p ("reverbType", 2); p ("reverbSize", 0.55f); p ("reverbLevel", 0.4f);
    p ("comp", 0.35f);

    w.show (AcousticKit);
    p ("kit_bd_level", 0.6f); p ("kit_sd_level", 0.55f); p ("kit_sd_reverb", 0.3f); p ("kit_ch_level", 0.32f); p ("kit_oh_level", 0.3f);
    p ("kit_cr_level", 0.32f); p ("kit_rd_level", 0.28f); p ("kit_xs_level", 0.3f); p ("kit_lt_level", 0.4f); p ("kit_mt_level", 0.4f);

    w.show (EBass);
    p ("ebass_type", 2); p ("ebass_tone", 0.5f); p ("ebass_decay", 0.5f); p ("ebass_muting", 0.35f); p ("ebass_volume", 0.85f);

    w.show (Piano);
    p ("piano_chord", 0); p ("piano_type", 3); p ("piano_brightness", 0.55f); p ("piano_decay", 0.55f); p ("piano_release", 0.4f);
    p ("piano_reverb", 0.3f); p ("piano_volume", 0.75f);

    w.show (SH101);
    p ("sh101_octave", 3); p ("sh101_saw", 0.5f); p ("sh101_square", 0.5f); p ("sh101_cutoff", 0.55f); p ("sh101_reso", 0.15f);
    p ("sh101_env", 0.3f); p ("sh101_attack", 0.05f); p ("sh101_decay", 0.5f); p ("sh101_sustain", 0.6f); p ("sh101_release", 0.3f);
    p ("sh101_glide", 0.1f); p ("sh101_echo", 0.3f); p ("sh101_reverb", 0.3f); p ("sh101_volume", 0.7f);

    // C major, I - V - vi - IV: C, G, Am, F. Verse and chorus with a melody that is sung, not played.
    const juce::String none = "................";
    const juce::String c = "E3+G3+C4", g = "D3+G3+B3", am = "E3+A3+C4", f = "F3+A3+C4";
    auto strum = [] (const juce::String& ch) { return ch + " - - . . . " + ch + " - . . " + ch + " - . . . . "; };
    auto pulse = [] (const juce::String& ch) { return ch + " . " + ch + " . " + ch + " . " + ch + " . " + ch + " . " + ch + " . " + ch + " . " + ch + " . "; };
    const juce::String verseKeys = bars ({ strum (c), strum (g), strum (am), strum (f) });
    const juce::String chorusKeys = bars ({ pulse (c), pulse (g), pulse (am), pulse (f) });
    const juce::String bass = bars ({ "C2 - - . . . C2 . . . G1 . C2 . . . ", "G1 - - . . . G1 . . . D2 . G1 . . . ",
                                      "A1 - - . . . A1 . . . E2 . A1 . . . ", "F1 - - . . . F1 . . . C2 . G1 . B1 . " });
    const juce::String verse = bars ({ ". . E4 - G4 - E4 - D4 - C4 - - - . . ", ". . D4 - G4 - D4 - B3 - G3 - - - . . ",
                                       ". . E4 - A4 - G4 - E4 - C4 - - - . . ", ". . F4 - A4 - G4 - F4 - E4 - D4 - . . " });
    const juce::String chorus = bars ({ "G4 - - - E5 - - - D5 - C5 - - - . . ", "D5 - - - B4 - - - G4 - - - A4 - B4 - ",
                                        "C5 - - - E5 - - - A5 - G5 - E5 - . . ", "F5 - E5 - D5 - C5 - D5 - - - - - . . " });

    auto beat = [&] (bool chorusBeat)
    {
        drums (w, AcousticKit, "bd", chorusBeat ? "x.......x.x....." : "x.......x.......");
        drums (w, AcousticKit, "sd", "....x.......x...");
        if (chorusBeat) drums (w, AcousticKit, "oh", "x.x.x.x.x.x.x.x.");
        else drums (w, AcousticKit, "ch", "x.x.x.x.x.x.x.x.");
    };

    // A - intro: the piano alone.
    w.part (0);
    chords (w, Piano, verseKeys);

    // B - verse: side stick, bass and the melody low.
    w.part (1);
    drums (w, AcousticKit, "bd", "x.......x.......");
    drums (w, AcousticKit, "xs", "....x.......x...");
    drums (w, AcousticKit, "ch", "x.x.x.x.x.x.x.x.");
    line (w, EBass, bass);
    chords (w, Piano, verseKeys);
    line (w, SH101, verse);

    // C - pre-chorus: the snare comes in, the piano pulses, tom fill at the end.
    w.part (2);
    beat (false);
    drums (w, AcousticKit, "lt", bars ({ none, none, none, "........x.x.x.x." }));
    line (w, EBass, bass);
    chords (w, Piano, chorusKeys);
    stepValues (w, Piano, "piano_volume", ramp (0.6f, 0.8f));
    line (w, SH101, verse);

    // D - chorus: crash, open hats, the melody goes up.
    w.part (3);
    beat (true);
    drums (w, AcousticKit, "cr", "x...............................................................");
    line (w, EBass, bass);
    chords (w, Piano, chorusKeys);
    line (w, SH101, chorus);

    // E - bridge: Am - F - C - G, the drums half, the piano alone holds.
    w.part (4);
    drums (w, AcousticKit, "rd", "x...x...x...x...");
    drums (w, AcousticKit, "bd", "x...............");
    chords (w, Piano, bars ({ held (am, 16), held (f, 16), held (c, 16), held (g, 16) }));
    line (w, EBass, bars ({ held ("A1", 16), held ("F1", 16), held ("C2", 16), held ("G1", 12) + "G1 . B1 . " }));
    line (w, SH101, bars ({ ". . C5 - B4 - A4 - - - - - . . . . ", ". . A4 - G4 - F4 - - - - - . . . . ",
                            ". . G4 - A4 - C5 - - - E5 - - - . . ", "D5 - - - - - - - - - - - . . . . " }));

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 1 }, { 3, 2 }, { 1, 1 }, { 2, 1 }, { 3, 2 }, { 4, 1 }, { 3, 2 }, { 0, 1 } });
}

void reggae (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 76); p ("swing", 0.2f); p ("key", 7); p ("scale", 2); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 2); p ("echoFeedback", 0.5f); p ("echoLevel", 0.55f);
    p ("reverbType", 2); p ("reverbSize", 0.6f); p ("reverbLevel", 0.4f);
    p ("comp", 0.3f);

    w.show (AcousticKit);
    p ("kit_bd_level", 0.7f); p ("kit_xs_level", 0.5f); p ("kit_xs_echo", 0.3f); p ("kit_sd_level", 0.45f); p ("kit_ch_level", 0.3f);
    p ("kit_oh_level", 0.26f); p ("kit_rb_level", 0.25f); p ("kit_ht_level", 0.38f); p ("kit_lt_level", 0.38f);

    w.show (EBass);
    p ("ebass_type", 0); p ("ebass_tone", 0.3f); p ("ebass_decay", 0.6f); p ("ebass_muting", 0.2f); p ("ebass_volume", 0.95f);

    w.show (Pluck);
    p ("pluck_chord", 0); p ("pluck_type", 10); p ("pluck_brightness", 0.55f); p ("pluck_decay", 0.2f); p ("pluck_release", 0.1f);
    p ("pluck_strum", 0.15f); p ("pluck_echo", 0.3f); p ("pluck_volume", 1.0f);

    w.show (Organ);
    p ("organ_chord", 0); p ("organ_type", 0); p ("organ_percussion", 0.2f); p ("organ_leslie", 0.4f); p ("organ_volume", 0.5f);

    w.show (SoundFx);
    p ("fx_sir_level", 0.3f); p ("fx_sir_echo", 0.5f);

    // G major: G - Em - C - D. One drop: nothing on the one, kick and side stick together on the third beat.
    // The guitar skanks on the off-beats, the organ bubbles, the bass is the melody.
    const juce::String none = "................";
    const juce::String drop = "........x.......";
    auto skank = [] (const juce::String& c) { return ". . . . " + c + " . . . . . . . " + c + " . . . "; };
    auto bubble = [] (const juce::String& c) { return ". . " + c + " " + c + " . . " + c + " " + c + " . . " + c + " " + c + " . . " + c + " " + c + " "; };
    const juce::String guitar = bars ({ skank ("G3+B3+D4"), skank ("G3+B3+E4"), skank ("G3+C4+E4"), skank ("F#3+A3+D4") });
    const juce::String organ = bars ({ bubble ("B3+D4"), bubble ("B3+E4"), bubble ("C4+E4"), bubble ("A3+D4") });
    const juce::String bass = bars ({ "G1 - . . D2 . . . . . G1 . B1 - D2 . ", "E2 - . . B1 . . . . . E2 . D2 - B1 . ",
                                      "C2 - . . G1 . . . . . C2 . E2 - C2 . ", "D2 - . . A1 . . . . . F#1 . A1 - C2 . " });

    auto beat = [&]
    {
        drums (w, AcousticKit, "bd", drop);
        drums (w, AcousticKit, "xs", drop);
        drums (w, AcousticKit, "ch", "x.x.x.x.x.x.x.x.");
        stepValues (w, AcousticKit, "kit_ch_level", "0.32 . 0.2 . ");
    };

    // A - intro: guitar and organ.
    w.part (0);
    chords (w, Pluck, guitar);
    chords (w, Organ, organ);

    // B - the one drop and the bass.
    w.part (1);
    beat();
    line (w, EBass, bass);
    chords (w, Pluck, guitar);
    chords (w, Organ, organ);

    // C - dub: the guitar in the echo, the siren, the drums play with the space.
    w.part (2);
    beat();
    drums (w, AcousticKit, "rb", "x...x...x...x...");
    drums (w, AcousticKit, "ht", bars ({ none, none, none, "..........x....." }));
    drums (w, AcousticKit, "lt", bars ({ none, none, none, "............x.x." }));
    line (w, EBass, bass);
    chords (w, Pluck, bars ({ skank ("G3+B3+D4"), rest16, skank ("G3+C4+E4"), rest16 }));
    stepValues (w, Pluck, "pluck_echo", hold (0.8f, 64));
    drums (w, SoundFx, "sir", bars ({ "x...............", none, none, none }));

    // D - the bass alone with the drums: the dub break.
    w.part (3);
    beat();
    line (w, EBass, bass);
    drums (w, AcousticKit, "oh", "..............x.");

    w.song ({ { 0, 1 }, { 1, 3 }, { 2, 1 }, { 1, 2 }, { 3, 1 }, { 2, 1 }, { 1, 2 } });
}

void synthwave (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 100); p ("swing", 0.0f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.4f); p ("echoLevel", 0.5f);
    p ("reverbType", 1); p ("reverbSize", 0.8f); p ("reverbLevel", 0.5f);
    p ("pumpMode", 1); p ("pump", 0.15f); p ("comp", 0.35f);

    w.show (LinnDrum);
    p ("linn_bd_level", 0.6f); p ("linn_sd_level", 0.6f); p ("linn_sd_reverb", 0.6f); p ("linn_cp_level", 0.35f); p ("linn_cp_reverb", 0.5f);
    p ("linn_ch_level", 0.32f); p ("linn_oh_level", 0.28f); p ("linn_lt_level", 0.45f); p ("linn_ht_level", 0.45f); p ("linn_cr_level", 0.32f);

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_detune", 0.1f); p ("modeld_sub", 0.4f); p ("modeld_cutoff", 0.35f); p ("modeld_emphasis", 0.25f);
    p ("modeld_contour", 0.4f); p ("modeld_decay", 0.2f); p ("modeld_sustain", 0.1f); p ("modeld_overdrive", 0.2f); p ("modeld_volume", 0.8f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_attack", 0.6f); p ("strings_release", 0.7f); p ("strings_brightness", 0.45f);
    p ("strings_ensemble", 0.85f); p ("strings_reverb", 0.5f); p ("strings_volume", 0.55f);

    w.show (JP8000);
    p ("jp8000_chord", 0); p ("jp8000_detune", 0.3f); p ("jp8000_mix", 0.5f); p ("jp8000_cutoff", 0.6f); p ("jp8000_attack", 0.05f);
    p ("jp8000_decay", 0.5f); p ("jp8000_sustain", 0.7f); p ("jp8000_release", 0.4f); p ("jp8000_echo", 0.45f); p ("jp8000_reverb", 0.4f);
    p ("jp8000_volume", 0.75f);

    // A minor: Am - Em - F - G. The bass pulses in eighths, the big gated snare, a nostalgic lead in the echo.
    const juce::String none = "................";
    auto eighths = [] (const juce::String& n) { return n + " . " + n + " . " + n + " . " + n + " . " + n + " . " + n + " . " + n + " . " + n + " . "; };
    const juce::String bass = bars ({ eighths ("A1"), eighths ("E1"), eighths ("F1"), eighths ("G1") });
    const juce::String pads = bars ({ held ("A3+C4+E4", 16), held ("G3+B3+E4", 16), held ("A3+C4+F4", 16), held ("G3+B3+D4", 16) });
    const juce::String lead = bars ({ "E5 - - - - - D5 - C5 - - - B4 - C5 - ", "B4 - - - - - - - G4 - - - - - - - ",
                                      "A4 - - - C5 - - - F5 - - - E5 - D5 - ", "D5 - - - - - - - B4 - C5 - D5 - - - " });

    auto beat = [&] (bool fill)
    {
        drums (w, LinnDrum, "bd", "x.......x.x.....");
        drums (w, LinnDrum, "sd", "....x.......x...");
        drums (w, LinnDrum, "ch", "x.x.x.x.x.x.x.x.");
        if (fill)
        {
            drums (w, LinnDrum, "ht", bars ({ none, none, none, "........x.x....." }));
            drums (w, LinnDrum, "lt", bars ({ none, none, none, "............x.x." }));
        }
    };

    // A - intro: the pads and the pulse of the bass.
    w.part (0);
    chords (w, Strings, pads);
    line (w, ModelD, bass);
    stepValues (w, ModelD, "modeld_cutoff", ramp (0.18f, 0.35f));

    // B - verse: the drums.
    w.part (1);
    beat (false);
    chords (w, Strings, pads);
    line (w, ModelD, bass);

    // C - chorus: the lead plays the theme, toms at the end.
    w.part (2);
    beat (true);
    drums (w, LinnDrum, "cr", "x...............................................................");
    chords (w, Strings, pads);
    line (w, ModelD, bass);
    chords (w, JP8000, lead);

    // D - break: the drums out, the lead alone in the reverb, the bass holds.
    w.part (3);
    chords (w, Strings, pads);
    chords (w, JP8000, lead);
    stepValues (w, JP8000, "jp8000_reverb", hold (0.75f, 64));
    line (w, ModelD, bars ({ held ("A1", 16), held ("E1", 16), held ("F1", 16), held ("G1", 16) }));
    drums (w, LinnDrum, "sd", bars ({ none, none, none, "x...x...x.x.xxxx" }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 2 }, { 1, 1 }, { 3, 1 }, { 2, 3 }, { 0, 1 } });
}

void ambient (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 80); p ("swing", 0.0f); p ("key", 2); p ("scale", 8); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 4); p ("echoFeedback", 0.55f); p ("echoLevel", 0.6f);
    p ("reverbType", 1); p ("reverbSize", 0.95f); p ("reverbLevel", 0.6f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_attack", 0.9f); p ("strings_release", 0.9f); p ("strings_brightness", 0.35f);
    p ("strings_ensemble", 0.9f); p ("strings_reverb", 0.7f); p ("strings_volume", 0.6f);

    w.show (Choir);
    p ("choir_chord", 0); p ("choir_vowel", 1); p ("choir_attack", 0.85f); p ("choir_release", 0.9f); p ("choir_ensemble", 0.8f);
    p ("choir_breath", 0.3f); p ("choir_reverb", 0.7f); p ("choir_volume", 0.5f);

    w.show (Mallets);
    p ("mallets_chord", 0); p ("mallets_type", 9); p ("mallets_brightness", 0.6f); p ("mallets_decay", 0.8f); p ("mallets_release", 0.8f);
    p ("mallets_echo", 0.6f); p ("mallets_reverb", 0.6f); p ("mallets_volume", 0.65f);

    w.show (Mellotron);
    p ("mellotron_chord", 0); p ("mellotron_type", 0); p ("mellotron_attack", 0.5f); p ("mellotron_release", 0.6f); p ("mellotron_wow", 0.5f);
    p ("mellotron_reverb", 0.6f); p ("mellotron_volume", 0.5f);

    w.show (SoundFx);
    p ("fx_rain_decay", 1.0f); p ("fx_rain_level", 0.25f);

    // D Lydian (the G# that floats): D - E/D - Bm - A, each chord held for a bar, the crotales fall like drops,
    // the flutes come in only halfway.
    const juce::String d = "D3+A3+F#4", e = "D3+B3+G#4", bm = "D3+B3+F#4", a = "C#3+A3+E4";
    const juce::String pads = bars ({ held (d, 16), held (e, 16), held (bm, 16), held (a, 16) });
    const juce::String voices = bars ({ held ("F#4+A4", 16), held ("G#4+B4", 16), held ("F#4+B4", 16), held ("E4+A4", 16) });
    const juce::String drops = bars ({ ". . . . A5 . . . . . . . E6 . . . ", ". . . . . . G#5 . . . . . . . B5 . ",
                                       ". . F#5 . . . . . . . D6 . . . . . ", ". . . . . . . . C#6 . . . A5 . . . " });
    const juce::String flutes = bars ({ "A4 - - - - - - - B4 - - - C#5 - - - ", "B4 - - - - - - - G#4 - - - - - - - ",
                                        "F#4 - - - - - - - D5 - - - C#5 - - - ", "A4 - - - - - - - - - - - - - - - " });
    auto rain = [&] { drums (w, SoundFx, "rain", "x..............................................................."); };

    // A - the rain and the strings.
    w.part (0);
    rain();
    chords (w, Strings, pads);

    // B - the choir and the crotales.
    w.part (1);
    rain();
    chords (w, Strings, pads);
    chords (w, Choir, voices);
    chords (w, Mallets, drops);

    // C - the Mellotron flutes sing.
    w.part (2);
    chords (w, Strings, pads);
    chords (w, Choir, voices);
    chords (w, Mallets, drops);
    chords (w, Mellotron, flutes);

    // D - only the crotales and the choir, the strings breathe out.
    w.part (3);
    rain();
    chords (w, Choir, voices);
    chords (w, Mallets, drops);
    stepValues (w, Mallets, "mallets_echo", hold (0.8f, 64));

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 2 }, { 3, 1 }, { 2, 1 }, { 1, 1 }, { 0, 1 } });
}
} // namespace techno::demos
