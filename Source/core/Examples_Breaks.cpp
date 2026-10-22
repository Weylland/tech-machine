#include "ExampleTools.h"

// Breakbeat and bass music demos.
namespace techno::demos
{
void drumAndBass (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 174); p ("swing", 0.0f); p ("key", 5); p ("scale", 1); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 3); p ("echoFeedback", 0.4f); p ("echoLevel", 0.5f);
    p ("reverbType", 1); p ("reverbSize", 0.8f); p ("reverbLevel", 0.45f);
    p ("pumpMode", 1); p ("pump", 0.15f); p ("comp", 0.4f);

    w.show (BreakKit);
    p ("breaks_bd_level", 0.6f); p ("breaks_sd_level", 0.55f); p ("breaks_gs_level", 0.25f); p ("breaks_ch_level", 0.34f);
    p ("breaks_oh_level", 0.3f); p ("breaks_rd_level", 0.28f); p ("breaks_cr_level", 0.34f); p ("breaks_sd_reverb", 0.15f);

    w.show (Wobble);
    p ("wobble_wave", 1); p ("wobble_rate", 0); p ("wobble_depth", 0.3f); p ("wobble_cutoff", 0.3f); p ("wobble_reso", 0.25f);
    p ("wobble_detune", 0.55f); p ("wobble_overdrive", 0.45f); p ("wobble_glide", 0.25f); p ("wobble_volume", 0.5f);

    w.show (Juno106);
    p ("juno106_chord", 0); p ("juno106_saw", 0.6f); p ("juno106_pulse", 0.4f); p ("juno106_cutoff", 0.45f); p ("juno106_reso", 0.15f);
    p ("juno106_attack", 0.6f); p ("juno106_decay", 0.6f); p ("juno106_sustain", 0.8f); p ("juno106_release", 0.75f);
    p ("juno106_chorus", 3); p ("juno106_hpf", 2); p ("juno106_reverb", 0.6f); p ("juno106_volume", 0.7f);

    w.show (SoundFx);
    p ("fx_rise_decay", 0.62f); p ("fx_rise_level", 0.4f); p ("fx_imp_level", 0.55f);

    // F minor: Fm9 - Db - Ab - Eb pads, the Reese bass holds long notes that slide, the two-step break at 174.
    const juce::String none = "................";
    const juce::String kicks = bars ({ "x.........x.....", "x.........x..x..", "x.........x.....", "x.x.......x....." });
    const juce::String snares = bars ({ "....x.......x...", "....x.......x..x", "....x.......x...", "....x.....x.x..." });
    const juce::String ghosts = "..x....x.x....x...x..x..x.....x.";
    const juce::String pads = bars ({ held ("G#3+C4+D#4+G4", 16), held ("G#3+C#4+F4", 16), held ("G#3+C4+D#4", 16), held ("G3+A#3+D#4", 16) });
    const juce::String reese = bars ({ held ("F1", 10) + "G#1~ F1 - - - - ", held ("C#2", 12) + "C2~ C#2 - - ",
                                       held ("G#1", 10) + "A#1~ G#1 - - - - ", held ("D#1", 8) + ". . D#2~ D#1 - - . . " });

    auto beat = [&]
    {
        drums (w, BreakKit, "bd", kicks);
        drums (w, BreakKit, "sd", snares);
        drums (w, BreakKit, "gs", ghosts);
        drums (w, BreakKit, "ch", "x.x.x.x.x.x.x.x.");
        stepValues (w, BreakKit, "breaks_ch_level", "0.36 . 0.2 . ");
    };

    // A - intro: the pads and the hats.
    w.part (0);
    chords (w, Juno106, pads);
    drums (w, BreakKit, "ch", "x.x.x.x.x.x.x.x.");
    stepValues (w, BreakKit, "breaks_ch_level", "0.3 . 0.15 . ");
    drums (w, BreakKit, "rd", bars ({ none, none, "x...x...x...x...", "x...x...x...x..." }));

    // B - build: the break filtered, the riser.
    w.part (1);
    beat();
    stepValues (w, BreakKit, "breaks_filter", ramp (0.2f, 0.5f));
    chords (w, Juno106, pads);
    drums (w, SoundFx, "rise", bars ({ none, none, "x...............", none }));

    // C - drop: the Reese comes in under the break.
    w.part (2);
    beat();
    drums (w, BreakKit, "cr", "x...............................................................");
    drums (w, SoundFx, "imp", "x...............................................................");
    line (w, Wobble, reese);
    stepValues (w, Wobble, "wobble_cutoff", ramp (0.25f, 0.45f, 32) + ramp (0.45f, 0.28f, 32));

    // D - the drop goes on, ride and open hats, the pads come back.
    w.part (3);
    beat();
    drums (w, BreakKit, "rd", "x...x...x...x...");
    drums (w, BreakKit, "oh", "......x.......x.");
    line (w, Wobble, reese);
    stepValues (w, Wobble, "wobble_cutoff", ramp (0.3f, 0.5f, 32) + ramp (0.5f, 0.3f, 32));
    chords (w, Juno106, pads);
    stepValues (w, Juno106, "juno106_volume", hold (0.45f, 64));

    // E - break: the pads alone, the bass holds one note.
    w.part (4);
    chords (w, Juno106, pads);
    stepValues (w, Juno106, "juno106_cutoff", ramp (0.55f, 0.35f));
    line (w, Wobble, bars ({ held ("F1", 16), held ("C#2", 16), rest16, rest16 }));
    drums (w, BreakKit, "sd", bars ({ none, none, "x...x...x...x...", "x.x.x.x.xxxxxxxx" }));
    stepValues (w, BreakKit, "breaks_sd_level", hold (0.2f, 32) + ramp (0.2f, 0.5f, 32));

    w.song ({ { 0, 2 }, { 1, 1 }, { 2, 4 }, { 3, 4 }, { 4, 2 }, { 2, 4 }, { 3, 2 }, { 0, 1 } });
}

void jungle (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 165); p ("swing", 0.05f); p ("key", 7); p ("scale", 3); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 2); p ("echoFeedback", 0.45f); p ("echoLevel", 0.5f);
    p ("reverbType", 1); p ("reverbSize", 0.85f); p ("reverbLevel", 0.5f);
    p ("pumpMode", 1); p ("pump", 0.1f); p ("comp", 0.4f);

    w.show (BreakKit);
    p ("breaks_bd_level", 0.6f); p ("breaks_sd_level", 0.55f); p ("breaks_gs_level", 0.3f); p ("breaks_ch_level", 0.32f);
    p ("breaks_oh_level", 0.3f); p ("breaks_rd_level", 0.3f); p ("breaks_cr_level", 0.34f); p ("breaks_crush", 0.15f);

    w.show (ModelD);
    p ("modeld_wave", 2); p ("modeld_sub", 0.8f); p ("modeld_cutoff", 0.2f); p ("modeld_emphasis", 0.0f); p ("modeld_contour", 0.15f);
    p ("modeld_decay", 0.5f); p ("modeld_sustain", 0.6f); p ("modeld_glide", 0.15f); p ("modeld_overdrive", 0.15f); p ("modeld_volume", 0.9f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_attack", 0.7f); p ("strings_release", 0.8f); p ("strings_brightness", 0.4f);
    p ("strings_reverb", 0.6f); p ("strings_volume", 0.5f);

    w.show (Choir);
    p ("choir_chord", 0); p ("choir_vowel", 0); p ("choir_attack", 0.6f); p ("choir_release", 0.7f); p ("choir_reverb", 0.6f); p ("choir_volume", 0.45f);

    w.show (SoundFx);
    p ("fx_rwd_level", 0.45f); p ("fx_gun_level", 0.3f); p ("fx_gun_echo", 0.5f); p ("fx_sir_level", 0.3f);

    // G harmonic minor: the Amen break cut up and rolled, a deep sub, strings and choir far away, the sound system rewind.
    const juce::String none = "................";
    const juce::String amenK = bars ({ "x.x.......xx....", "x.x.......x.....", "x.x.......xx....", "x.........x.x..." });
    const juce::String amenS = bars ({ "....x..x.x..x..x", "....x..x.x..x...", "....x..x.x..x..x", ".x..x..x.x.x2244" });
    const juce::String amenH = "x.x.x.x.x.x.x.x.";
    const juce::String sub = bars ({ "G1 - - - - - - - . . G1 - D2 - - - ", "D#2 - - - - - - - . . D#2 - D2 - C2 - ",
                                     "G1 - - - - - - - . . G1 - A#1 - - - ", "D2 - - - - - - - . . A1 - F#1 - - - " });
    const juce::String pads = bars ({ held ("G3+A#3+D4", 16), held ("D#3+G3+A#3", 16), held ("G3+A#3+D4", 16), held ("F#3+A3+D4", 16) });

    auto amen = [&]
    {
        drums (w, BreakKit, "bd", amenK);
        drums (w, BreakKit, "sd", amenS);
        drums (w, BreakKit, "ch", amenH);
        drums (w, BreakKit, "rd", "x...x...x...x...");
    };

    // A - intro: strings and choir in the reverb.
    w.part (0);
    chords (w, Strings, pads);
    chords (w, Choir, bars ({ rest16, rest16, held ("D4+G4", 16), held ("D4+F#4", 16) }));

    // B - the Amen alone, filtered then open.
    w.part (1);
    amen();
    stepValues (w, BreakKit, "breaks_filter", ramp (0.25f, 0.5f));
    chords (w, Strings, pads);

    // C - drop: the sub comes in under the break.
    w.part (2);
    amen();
    drums (w, BreakKit, "cr", "x...............................................................");
    line (w, ModelD, sub);
    chords (w, Strings, pads);
    stepValues (w, Strings, "strings_volume", hold (0.35f, 64));

    // D - the break goes wild: more rolls, the gunshot, then the rewind.
    w.part (3);
    drums (w, BreakKit, "bd", amenK);
    drums (w, BreakKit, "sd", bars ({ "....x..x.x..x..x", "..2.x..x.x..x.22", "....x..x.x.2x..x", "2222444422224444" }));
    drums (w, BreakKit, "ch", amenH);
    drums (w, BreakKit, "gs", "..x....x...x..x.");
    line (w, ModelD, sub);
    drums (w, SoundFx, "gun", bars ({ none, "........x.......", none, none }));
    drums (w, SoundFx, "rwd", bars ({ none, none, none, "........x......." }));

    // E - break: the choir, the siren, the sub alone.
    w.part (4);
    chords (w, Choir, bars ({ held ("D4+G4", 16), held ("D#4+G4", 16), held ("D4+G4", 16), held ("D4+F#4", 16) }));
    chords (w, Strings, pads);
    line (w, ModelD, bars ({ held ("G1", 16), held ("D#2", 16), held ("G1", 16), held ("D2", 16) }));
    drums (w, SoundFx, "sir", bars ({ none, none, "x...............", none }));

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 4 }, { 3, 1 }, { 4, 2 }, { 2, 4 }, { 3, 1 }, { 1, 2 } });
}

void breakbeat (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 132); p ("swing", 0.1f); p ("key", 2); p ("scale", 15); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("echoLevel", 0.5f);
    p ("reverbType", 0); p ("reverbSize", 0.55f); p ("reverbLevel", 0.35f);
    p ("pumpMode", 1); p ("pump", 0.2f); p ("comp", 0.45f); p ("disto", 0.1f);

    w.show (BreakKit);
    p ("breaks_bd_level", 0.62f); p ("breaks_sd_level", 0.58f); p ("breaks_gs_level", 0.25f); p ("breaks_ch_level", 0.34f);
    p ("breaks_oh_level", 0.32f); p ("breaks_cr_level", 0.36f); p ("breaks_drive", 0.3f);

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_sub", 0.5f); p ("modeld_cutoff", 0.3f); p ("modeld_emphasis", 0.4f); p ("modeld_contour", 0.5f);
    p ("modeld_decay", 0.25f); p ("modeld_sustain", 0.2f); p ("modeld_glide", 0.1f); p ("modeld_overdrive", 0.5f); p ("modeld_volume", 0.8f);

    w.show (Hoover);
    p ("hoover_chord", 0); p ("hoover_detune", 0.7f); p ("hoover_dip", 0.6f); p ("hoover_cutoff", 0.65f); p ("hoover_attack", 0.05f);
    p ("hoover_release", 0.3f); p ("hoover_reverb", 0.3f); p ("hoover_volume", 0.6f);

    w.show (SH101);
    p ("sh101_octave", 3); p ("sh101_saw", 0.8f); p ("sh101_cutoff", 0.4f); p ("sh101_reso", 0.6f); p ("sh101_env", 0.6f);
    p ("sh101_decay", 0.25f); p ("sh101_sustain", 0.1f); p ("sh101_glide", 0.15f); p ("sh101_echo", 0.35f); p ("sh101_volume", 0.92f);

    w.show (SoundFx);
    p ("fx_scr_level", 0.4f); p ("fx_rise_decay", 0.72f); p ("fx_rise_level", 0.4f); p ("fx_imp_level", 0.5f);

    // D blues: a fat funky break, the hoover hits on the beat, the 101 plays a blues riff, the DJ scratches.
    const juce::String none = "................";
    const juce::String kicks = bars ({ "x.....x...x.....", "x.....x.x.......", "x.....x...x.....", "x.....x.x.....x." });
    const juce::String snares = bars ({ "....x.......x...", "....x.......x..x", "....x.......x...", "....x..x....x.xx" });
    const juce::String bass = bars ({ "D2 . . . . . D2 . . . F2 . G2 . . . ", "D2 . . . . . D2 . . . C2 . A1 . C2 . ",
                                      "D2 . . . . . D2 . . . F2 . G2 . G#2 . ", "A2 . . . G2 . . . F2 . . . D2 . C2 . " });
    const juce::String riff = bars ({ "D4 . F4 . D4 G4~ G#4 A4 . . C5 . A4 . F4 . ", "D4 . F4 . D4 G4~ G#4 A4 . . G4 . F4 . D4 . ",
                                      "D4 . F4 . D4 G4~ G#4 A4 . . C5 . D5 . C5 . ", "A4 . . G4 . . F4 . . D4 . . C4 . D4 . " });
    const juce::String hoover = bars ({ "D3+A3 - - - . . . . . . . . . . . . ", "D3+A3 - - - . . . . . . C3+G3 - . . . . ",
                                        "D3+A3 - - - . . . . . . . . . . . . ", "F3+C4 - - - . . . . G3+D4 - - - . . . . " });

    auto beat = [&]
    {
        drums (w, BreakKit, "bd", kicks);
        drums (w, BreakKit, "sd", snares);
        drums (w, BreakKit, "ch", "x.x.x.x.x.x.x.x.");
        drums (w, BreakKit, "gs", "..x......x....x.");
        drums (w, BreakKit, "oh", "..............x.");
    };

    // A - intro: the break alone, filtered, a scratch.
    w.part (0);
    beat();
    stepValues (w, BreakKit, "breaks_filter", ramp (0.22f, 0.45f));
    drums (w, SoundFx, "scr", bars ({ none, none, none, "........x...x..." }));

    // B - the bass and the hoover.
    w.part (1);
    beat();
    line (w, ModelD, bass);
    chords (w, Hoover, hoover);

    // C - the 101 riff on top, scratches between the phrases.
    w.part (2);
    beat();
    line (w, ModelD, bass);
    chords (w, Hoover, hoover);
    line (w, SH101, riff);
    drums (w, SoundFx, "scr", bars ({ none, "............x...", none, "............x.x." }));

    // D - break: the riff alone, its filter swept, the riser.
    w.part (3);
    drums (w, BreakKit, "ch", "x.x.x.x.x.x.x.x.");
    line (w, SH101, riff);
    stepValues (w, SH101, "sh101_cutoff", ramp (0.2f, 0.65f));
    drums (w, BreakKit, "sd", bars ({ none, none, "x...x...x...x...", "x.x.x.x.xxxx2244" }));
    stepValues (w, BreakKit, "breaks_sd_level", hold (0.2f, 32) + ramp (0.2f, 0.55f, 32));
    drums (w, SoundFx, "rise", bars ({ none, none, "x...............", none }));

    // E - drop: impact, everything, the crash.
    w.part (4);
    beat();
    drums (w, BreakKit, "cr", "x...............................................................");
    drums (w, SoundFx, "imp", "x...............................................................");
    line (w, ModelD, bass);
    chords (w, Hoover, hoover);
    line (w, SH101, riff);
    stepValues (w, SH101, "sh101_cutoff", hold (0.55f, 64));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 3 }, { 3, 1 }, { 4, 3 }, { 1, 1 }, { 0, 1 } });
}

void gabber (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 185); p ("swing", 0.0f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 1); p ("echoFeedback", 0.3f); p ("echoLevel", 0.4f);
    p ("reverbType", 1); p ("reverbSize", 0.75f); p ("reverbLevel", 0.4f);
    p ("pumpMode", 1); p ("pump", 0.45f); p ("comp", 0.55f); p ("disto", 0.2f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.5f); p ("tr909_bd_tone", 0.55f); p ("tr909_bd_drive", 0.95f); p ("tr909_bd_tune", 0.45f);
    p ("tr909_oh_level", 0.4f); p ("tr909_oh_decay", 0.3f); p ("tr909_cp_level", 0.42f); p ("tr909_cp_drive", 0.4f);
    p ("tr909_ch_level", 0.32f); p ("tr909_sd_level", 0.45f); p ("tr909_cr_level", 0.36f);

    w.show (Hoover);
    p ("hoover_chord", 0); p ("hoover_detune", 0.75f); p ("hoover_dip", 0.65f); p ("hoover_cutoff", 0.7f); p ("hoover_attack", 0.05f);
    p ("hoover_release", 0.35f); p ("hoover_chorus", 3); p ("hoover_reverb", 0.3f); p ("hoover_volume", 0.75f);

    w.show (SoundFx);
    p ("fx_horn_level", 0.4f); p ("fx_rise_decay", 0.6f); p ("fx_rise_level", 0.45f); p ("fx_imp_level", 0.55f);

    // A minor at 185: the kick distorted until it whistles, a hoover riff that moves down by half steps.
    const juce::String none = "................", kick = "x...x...x...x...";
    const juce::String riff = bars ({ "A3 - . A3 . . G3 - . . E3 - . . G3 . ", "A3 - . A3 . . C4 - . . B3 - . . G3 . ",
                                      "A3 - . A3 . . G3 - . . E3 - . . G3 . ", "F3 - - - . . E3 - - - . . D3 - E3 - " });

    // A - intro: the kick alone.
    w.part (0);
    drums (w, TR909, "bd", kick);

    // B - hats and clap.
    w.part (1);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "cp", "....x.......x...");

    // C - the hoover riff.
    w.part (2);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "ch", "xxxxxxxxxxxxxxxx");
    stepValues (w, TR909, "tr909_ch_level", "0.15 0.25 0.1 0.25 ");
    chords (w, Hoover, riff);

    // D - break: the hoover alone, the air horn, snare roll and riser.
    w.part (3);
    chords (w, Hoover, riff);
    stepValues (w, Hoover, "hoover_cutoff", ramp (0.4f, 0.8f));
    drums (w, SoundFx, "horn", bars ({ "x...............", none, "x...............", none }));
    drums (w, TR909, "sd", bars ({ none, "x...x...x...x...", "x.x.x.x.x.x.x.x.", "xxxxxxxx44444444" }));
    stepValues (w, TR909, "tr909_sd_level", ramp (0.15f, 0.6f));
    drums (w, SoundFx, "rise", bars ({ none, none, "x...............", none }));

    // E - drop: impact, crash, the kick doubles at the end of the phrase.
    w.part (4);
    drums (w, TR909, "bd", bars ({ kick, kick, kick, "x...x...x.x.xxxx" }));
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "ch", "xxxxxxxxxxxxxxxx");
    stepValues (w, TR909, "tr909_ch_level", "0.15 0.25 0.1 0.25 ");
    drums (w, TR909, "cr", "x...............................................................");
    drums (w, SoundFx, "imp", "x...............................................................");
    chords (w, Hoover, riff);

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 4 }, { 3, 1 }, { 4, 4 }, { 2, 2 }, { 3, 1 }, { 4, 3 }, { 1, 1 } });
}

void electro (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 126); p ("swing", 0.05f); p ("key", 2); p ("scale", 6); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.4f); p ("echoLevel", 0.5f);
    p ("reverbType", 2); p ("reverbSize", 0.5f); p ("reverbLevel", 0.35f);
    p ("pumpMode", 1); p ("pump", 0.15f); p ("comp", 0.35f);

    w.show (TR808);
    p ("tr808_bd_decay", 0.65f); p ("tr808_bd_level", 0.6f); p ("tr808_sd_level", 0.5f); p ("tr808_sd_tone", 0.6f);
    p ("tr808_cp_level", 0.42f); p ("tr808_cp_reverb", 0.3f); p ("tr808_ch_level", 0.36f); p ("tr808_oh_level", 0.3f);
    p ("tr808_cb_level", 0.3f); p ("tr808_cb_echo", 0.3f); p ("tr808_cl_level", 0.3f); p ("tr808_lt_level", 0.4f); p ("tr808_ht_level", 0.4f);

    w.show (SH101);
    p ("sh101_octave", 1); p ("sh101_saw", 0.0f); p ("sh101_square", 0.85f); p ("sh101_sub", 0.6f); p ("sh101_cutoff", 0.35f);
    p ("sh101_reso", 0.45f); p ("sh101_env", 0.55f); p ("sh101_decay", 0.18f); p ("sh101_sustain", 0.1f); p ("sh101_volume", 0.9f);

    w.show (JP8000);
    p ("jp8000_chord", 0); p ("jp8000_detune", 0.15f); p ("jp8000_mix", 0.3f); p ("jp8000_cutoff", 0.55f); p ("jp8000_reso", 0.3f);
    p ("jp8000_env", 0.4f); p ("jp8000_attack", 0.0f); p ("jp8000_decay", 0.38f); p ("jp8000_sustain", 0.4f); p ("jp8000_release", 0.2f);
    p ("jp8000_echo", 0.45f); p ("jp8000_reverb", 0.25f); p ("jp8000_volume", 1.0f);

    w.show (Vocoder);
    p ("vocoder_chord", 0); p ("vocoder_words", 2); p ("vocoder_speed", 0.45f); p ("vocoder_reverb", 0.25f); p ("vocoder_volume", 0.6f);

    // D Phrygian (the dark E flat): an 808 that never lands where you expect, a square bass like a machine, a cold lead.
    const juce::String none = "................";
    const juce::String kicks = bars ({ "x.....x...x.....", "x.....x.......x.", "x.....x...x.....", "x.....x...x..x.x" });
    const juce::String bass = bars ({ "D1 . D2 . . D1 . . D1 . . D2 . D1 . . ", "D1 . D2 . . D1 . . F1 . . D#1 . D1 . . ",
                                      "D1 . D2 . . D1 . . D1 . . D2 . D1 . . ", "A#0 . A#1 . . A#0 . . C1 . . C2 . D#1 . . " });
    const juce::String lead = bars ({ "D4 . . D4 . . F4 . . . D#4 . D4 . . . ", "A3 . . A3 . . C4 . . . A#3 . A3 . . . ",
                                      "D4 . . D4 . . F4 . . . G4 . A4 . . . ", "A#4 . . A4 . . G4 . . . F4 . D#4 . . . " });

    auto beat = [&] (bool full)
    {
        drums (w, TR808, "bd", kicks);
        drums (w, TR808, "sd", "....x.......x...");
        drums (w, TR808, "ch", "xxxxxxxxxxxxxxxx");
        stepValues (w, TR808, "tr808_ch_level", "0.3 0.15 0.22 0.15 ");
        if (! full) return;
        drums (w, TR808, "cb", "...x.....x....x....x.....x...x..");
        drums (w, TR808, "cl", "......x.......x.");
        drums (w, TR808, "cp", "....x.......x...");
    };

    // A - intro: the 808 alone.
    w.part (0);
    beat (false);

    // B - the robot bass.
    w.part (1);
    beat (false);
    line (w, SH101, bass);

    // C - cowbell and claves, the cold lead.
    w.part (2);
    beat (true);
    line (w, SH101, bass);
    chords (w, JP8000, lead);

    // D - break: the vocoder speaks over the bass, toms.
    w.part (3);
    drums (w, TR808, "ch", "x.x.x.x.x.x.x.x.");
    line (w, SH101, bass);
    stepValues (w, SH101, "sh101_cutoff", ramp (0.2f, 0.5f));
    chords (w, Vocoder, bars ({ "D3+A3 - - - - - - - . . . . . . . . ", rest16, "D3+A3 - - - - - - - . . . . . . . . ", "A#2+F3 - - - - - - - . . . . . . . . " }));
    drums (w, TR808, "ht", bars ({ none, none, none, "........x.x....." }));
    drums (w, TR808, "lt", bars ({ none, none, none, "............x.x." }));

    // E - everything with the vocoder.
    w.part (4);
    beat (true);
    line (w, SH101, bass);
    chords (w, JP8000, lead);
    chords (w, Vocoder, bars ({ "D3+A3 - - - - - - - . . . . . . . . ", rest16, "D3+A3 - - - - - - - . . . . . . . . ", "A#2+F3 - - - - - - - . . . . . . . . " }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 3 }, { 3, 2 }, { 4, 3 }, { 1, 1 }, { 0, 1 } });
}
} // namespace techno::demos
