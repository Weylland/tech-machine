#include "ExampleTools.h"

// Club demos: techno, house and their cousins.
namespace techno::demos
{
void techno (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 130); p ("swing", 0.04f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 2); p ("echoFeedback", 0.42f); p ("echoLevel", 0.6f);
    p ("reverbType", 1); p ("reverbSize", 0.75f); p ("reverbLevel", 0.45f);
    p ("pumpMode", 1); p ("pump", 0.3f); p ("comp", 0.35f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.55f); p ("tr909_bd_tone", 0.55f); p ("tr909_bd_drive", 0.2f);
    p ("tr909_ch_level", 0.45f); p ("tr909_ch_decay", 0.22f); p ("tr909_oh_level", 0.36f); p ("tr909_oh_decay", 0.4f);
    p ("tr909_cp_level", 0.5f); p ("tr909_cp_reverb", 0.25f); p ("tr909_rs_level", 0.35f); p ("tr909_rs_echo", 0.4f);
    p ("tr909_sd_level", 0.45f); p ("tr909_sd_reverb", 0.2f); p ("tr909_rd_level", 0.28f); p ("tr909_cr_level", 0.38f);
    p ("tr909_lt_level", 0.5f); p ("tr909_mt_level", 0.5f); p ("tr909_ht_level", 0.5f);

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_detune", 0.08f); p ("modeld_sub", 0.55f); p ("modeld_cutoff", 0.3f);
    p ("modeld_emphasis", 0.3f); p ("modeld_contour", 0.4f); p ("modeld_decay", 0.2f); p ("modeld_sustain", 0.1f);
    p ("modeld_overdrive", 0.3f); p ("modeld_volume", 0.8f);

    w.show (Juno106);
    p ("juno106_chord", 0); p ("juno106_saw", 0.75f); p ("juno106_pulse", 0.3f); p ("juno106_cutoff", 0.52f); p ("juno106_reso", 0.25f);
    p ("juno106_env", 0.5f); p ("juno106_decay", 0.3f); p ("juno106_sustain", 0.05f); p ("juno106_release", 0.3f);
    p ("juno106_chorus", 2); p ("juno106_hpf", 1); p ("juno106_echo", 0.5f); p ("juno106_reverb", 0.25f); p ("juno106_volume", 0.95f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_violin", 0.7f); p ("strings_cello", 0.55f); p ("strings_attack", 0.65f); p ("strings_release", 0.7f);
    p ("strings_ensemble", 0.8f); p ("strings_brightness", 0.4f); p ("strings_reverb", 0.5f); p ("strings_volume", 0.5f);

    w.show (TB303);
    p ("tb303_cutoff", 0.24f); p ("tb303_reso", 0.78f); p ("tb303_env", 0.55f); p ("tb303_decay", 0.38f); p ("tb303_accent", 0.7f);
    p ("tb303_drive", 0.35f); p ("tb303_echo", 0.3f); p ("tb303_volume", 0.5f);

    w.show (SoundFx);
    p ("fx_rise_decay", 0.72f); p ("fx_rise_level", 0.45f); p ("fx_rev_decay", 0.43f); p ("fx_rev_level", 0.4f);
    p ("fx_imp_level", 0.55f); p ("fx_swp_level", 0.35f); p ("fx_down_decay", 0.6f); p ("fx_down_level", 0.35f);

    // A minor: Am7 for two bars, Fmaj7, G6. Stabs on the off-beats (3-3-4), the bass rolls between the kicks.
    const juce::String none = "................", kick = "X...x...x...x...", hats = "xxxxxxxxxxxxxxxx",
                       offbeats = "..x...x...x...x.", claps = "....x.......x...", rims = "......x.....x..x";
    const juce::String hatLevels = "0.22 0.3 0.52 0.3 ";   // the off-beat sixteenth leads the groove
    const juce::String am7 = "A3+C4+E4+G4", fmaj7 = "F3+A3+C4+E4", g6 = "G3+B3+D4+E4", c = "G3+C4+E4";
    auto roll = [] (const juce::String& n) { return ". . " + n + " " + n + " . . " + n + " " + n + " . . " + n + " " + n + " . . " + n + " " + n + " "; };
    auto stab = [] (const juce::String& chord) { return ". . . " + chord + " . . " + chord + " . . . " + chord + " . . . . . "; };
    const juce::String acid = "A1! A1 A2~ A1 . C2 A1 D2! A1 . A2 G1~ A1 C2! E2 A1 ";

    auto groove = [&] (const juce::String& kicks, const juce::String& clapLine)
    {
        drums (w, TR909, "bd", kicks);
        drums (w, TR909, "ch", hats);
        stepValues (w, TR909, "tr909_ch_level", hatLevels);
        drums (w, TR909, "oh", offbeats);
        drums (w, TR909, "cp", clapLine);
        drums (w, TR909, "rs", rims);
    };
    const juce::String lastBeatOff = bars ({ kick, kick, kick, "x...x...x......." });
    auto dropMusic = [&]
    {
        line (w, ModelD, bars ({ roll ("A1"), roll ("A1"), roll ("F1"), roll ("G1") }));
        chords (w, Juno106, bars ({ stab (am7), stab (am7), stab (fmaj7), stab (g6) }));
    };

    // A - intro: kick, off-beat hats, then the rimshots.
    w.part (0);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "ch", offbeats);
    drums (w, TR909, "rs", bars ({ none, none, rims, rims }));
    drums (w, SoundFx, "swp", bars ({ none, none, none, "........x......." }));

    // B - the groove: sixteenth hats, clap, the bass comes in and opens.
    w.part (1);
    groove (kick, claps);
    line (w, ModelD, bars ({ roll ("A1"), roll ("A1"), roll ("A1"), ". . A1 A1 . . A1 A1 . . A1 A1 . C2 A1 G1 " }));
    stepValues (w, ModelD, "modeld_cutoff", ramp (0.18f, 0.32f));

    // C - build: the stabs open up, snare roll, riser, the kick stops on the last beat.
    w.part (2);
    groove (lastBeatOff, bars ({ claps, claps, claps, none }));
    drums (w, TR909, "sd", bars ({ none, none, "x...x...x...x...", "x.x.x.x.xxxx2244" }));
    stepValues (w, TR909, "tr909_sd_level", hold (0.25f, 32) + ramp (0.25f, 0.6f, 32));
    line (w, ModelD, bars ({ roll ("A1"), roll ("A1"), roll ("A1"), ". . A1 A1 . . A1 A1 . . . . . . . . " }));
    chords (w, Juno106, bars ({ stab (am7), stab (am7), stab (am7), stab (am7) }));
    stepValues (w, Juno106, "juno106_cutoff", ramp (0.3f, 0.6f));
    drums (w, SoundFx, "rise", bars ({ none, none, "x...............", none }));
    drums (w, SoundFx, "rev", bars ({ none, none, none, "........x......." }));

    // D - drop: crash and impact, the chords move.
    w.part (3);
    groove (kick, claps);
    drums (w, TR909, "cr", "x..............................................................." );
    drums (w, SoundFx, "imp", "x..............................................................." );
    dropMusic();

    // E - the drop goes on.
    w.part (4);
    groove (kick, claps);
    dropMusic();

    // F - end of the drop: tom fill and reverse cymbal.
    w.part (5);
    groove (lastBeatOff, claps);
    drums (w, TR909, "sd", bars ({ none, none, none, "........x......." }));
    drums (w, TR909, "ht", bars ({ none, none, none, "..........xx...." }));
    drums (w, TR909, "mt", bars ({ none, none, none, "............xx.." }));
    drums (w, TR909, "lt", bars ({ none, none, none, "..............xx" }));
    line (w, ModelD, bars ({ roll ("A1"), roll ("A1"), roll ("F1"), ". . G1 G1 . . G1 G1 . . . . . . . . " }));
    chords (w, Juno106, bars ({ stab (am7), stab (am7), stab (fmaj7), stab (g6) }));
    drums (w, SoundFx, "rev", bars ({ none, none, none, "........x......." }));

    // G - break: no kick, strings, the stabs drown in the reverb, the 303 is heard in the distance.
    w.part (6);
    drums (w, SoundFx, "down", "x..............................................................." );
    drums (w, TR909, "rs", bars ({ "......x.........", "......x.........", "......x.........", "......x...x....." }));
    chords (w, Strings, bars ({ held ("A3+C4+E4", 16), held ("F3+A3+C4", 16), held (c, 16), held ("G3+B3+D4", 16) }));
    chords (w, Juno106, bars ({ stab (am7), stab (fmaj7), stab (c), stab (g6) }));
    stepValues (w, Juno106, "juno106_reverb", hold (0.7f, 64));
    stepValues (w, Juno106, "juno106_cutoff", hold (0.42f, 64));
    line (w, TB303, bars ({ rest16, rest16, acid, acid }));
    stepValues (w, TB303, "tb303_cutoff", hold (0.1f, 32) + ramp (0.1f, 0.2f, 32));

    // H - second drop with the acid line and the ride.
    w.part (7);
    groove (kick, claps);
    drums (w, TR909, "cr", "x..............................................................." );
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    dropMusic();
    line (w, TB303, bars ({ acid, acid, acid, "A1! A1 A2~ A1 . C2 A1 D2! A1 . A2 G1~ A1 C2! D2~ E2! " }));
    stepValues (w, TB303, "tb303_cutoff", ramp (0.2f, 0.42f, 32) + ramp (0.42f, 0.2f, 32));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 1 }, { 3, 1 }, { 4, 2 }, { 5, 1 }, { 6, 2 }, { 2, 1 }, { 7, 3 }, { 5, 1 }, { 1, 1 }, { 0, 1 } });
}

void acid (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 128); p ("swing", 0.05f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.4f); p ("echoLevel", 0.6f);
    p ("reverbType", 0); p ("reverbSize", 0.55f); p ("reverbLevel", 0.35f);
    p ("pumpMode", 1); p ("pump", 0.2f); p ("comp", 0.3f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.5f); p ("tr909_bd_tone", 0.6f); p ("tr909_bd_drive", 0.15f);
    p ("tr909_ch_level", 0.38f); p ("tr909_ch_decay", 0.22f); p ("tr909_oh_level", 0.38f); p ("tr909_oh_decay", 0.38f);
    p ("tr909_cp_level", 0.48f); p ("tr909_cp_reverb", 0.2f); p ("tr909_rd_level", 0.26f); p ("tr909_cr_level", 0.36f);
    p ("tr909_sd_level", 0.45f); p ("tr909_lt_level", 0.48f); p ("tr909_mt_level", 0.48f); p ("tr909_ht_level", 0.48f);

    w.show (TB303);
    p ("tb303_wave", 0); p ("tb303_cutoff", 0.22f); p ("tb303_reso", 0.8f); p ("tb303_env", 0.55f);
    p ("tb303_decay", 0.4f); p ("tb303_accent", 0.75f); p ("tb303_drive", 0.3f); p ("tb303_echo", 0.22f);
    p ("tb303_reverb", 0.05f); p ("tb303_volume", 0.6f);

    w.show (SoundFx);
    p ("fx_rise_decay", 0.72f); p ("fx_rise_level", 0.4f); p ("fx_imp_level", 0.5f); p ("fx_rev_decay", 0.43f); p ("fx_rev_level", 0.38f);

    // One riff that lives through the knobs: the filter opens, the resonance screams, the accents push.
    const juce::String none = "................", kick = "X...x...x...x...", claps = "....x.......x...";
    const juce::String riff = "A1! A1 A2~ A1 . C2 A1 D2! A1 . A2 G1~ A1 C2! E2 A1 ";
    const juce::String turn = "A1! A1 A2~ A1 . C2 A1 D2! A1 . A2 G1~ A1 C2! D2~ E2! ";
    const juce::String riffB = "A1! . A2 A1~ C2 A1 . E2! A1 G1 . A2~ G2 A1 C2! D2~ ";
    const juce::String acid = bars ({ riff, riff, riff, turn }), acidB = bars ({ riffB, riff, riffB, turn });

    auto beat = [&] (const juce::String& kicks, bool full)
    {
        drums (w, TR909, "bd", kicks);
        drums (w, TR909, "ch", "xxxxxxxxxxxxxxxx");
        stepValues (w, TR909, "tr909_ch_level", "0.2 0.28 0.5 0.28 ");
        if (! full) return;
        drums (w, TR909, "cp", claps);
        drums (w, TR909, "oh", "..x...x...x...x.");
    };
    const juce::String endFill = bars ({ kick, kick, kick, "x...x...x......." });

    // A - intro: the 303 is closed, almost a click.
    w.part (0);
    beat (kick, false);
    line (w, TB303, acid);
    stepValues (w, TB303, "tb303_cutoff", hold (0.1f, 64));
    stepValues (w, TB303, "tb303_env", hold (0.3f, 64));

    // B - the clap comes in, the filter starts to open.
    w.part (1);
    beat (kick, true);
    line (w, TB303, acid);
    stepValues (w, TB303, "tb303_cutoff", ramp (0.12f, 0.24f));

    // C - acid: the filter breathes over four bars, the ride comes in.
    w.part (2);
    beat (kick, true);
    drums (w, TR909, "rd", "..x...x...x...x.");
    line (w, TB303, acid);
    stepValues (w, TB303, "tb303_cutoff", ramp (0.22f, 0.4f, 32) + ramp (0.4f, 0.25f, 32));

    // D - peak: wide open, more envelope, crash.
    w.part (3);
    beat (kick, true);
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    drums (w, TR909, "cr", "x...............................x...............................");
    line (w, TB303, acidB);
    stepValues (w, TB303, "tb303_cutoff", ramp (0.42f, 0.58f, 32) + ramp (0.58f, 0.42f, 32));
    stepValues (w, TB303, "tb303_env", hold (0.72f, 64));
    stepValues (w, TB303, "tb303_drive", hold (0.5f, 64));

    // E - break: the drums step back, the 303 alone sweeps from dark to screaming, snare roll.
    w.part (4);
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "cp", bars ({ claps, claps, claps, none }));
    drums (w, TR909, "sd", bars ({ none, none, "x...x...x...x...", "x.x.x.x.xxxx2244" }));
    stepValues (w, TR909, "tr909_sd_level", hold (0.22f, 32) + ramp (0.22f, 0.6f, 32));
    line (w, TB303, acid);
    stepValues (w, TB303, "tb303_cutoff", ramp (0.12f, 0.72f));
    stepValues (w, TB303, "tb303_reso", hold (0.9f, 64));
    drums (w, SoundFx, "rise", bars ({ none, none, "x...............", none }));

    // F - drop: impact, everything open.
    w.part (5);
    beat (kick, true);
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    drums (w, TR909, "cr", "x...............................................................");
    drums (w, SoundFx, "imp", "x...............................................................");
    line (w, TB303, acidB);
    stepValues (w, TB303, "tb303_cutoff", ramp (0.5f, 0.62f, 32) + ramp (0.62f, 0.45f, 32));
    stepValues (w, TB303, "tb303_env", hold (0.7f, 64));

    // G - end of the drop: tom fill, reverse cymbal into the outro.
    w.part (6);
    beat (endFill, true);
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    drums (w, TR909, "sd", bars ({ none, none, none, "........x......." }));
    drums (w, TR909, "ht", bars ({ none, none, none, "..........xx...." }));
    drums (w, TR909, "mt", bars ({ none, none, none, "............xx.." }));
    drums (w, TR909, "lt", bars ({ none, none, none, "..............xx" }));
    drums (w, SoundFx, "rev", bars ({ none, none, none, "........x......." }));
    line (w, TB303, bars ({ riffB, riff, riffB, ". . . . . . . . . . . . . . . . " }));
    stepValues (w, TB303, "tb303_cutoff", ramp (0.55f, 0.3f));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 }, { 4, 1 }, { 5, 3 }, { 6, 1 }, { 1, 2 }, { 0, 1 } });
}

void detroit (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 125); p ("swing", 0.15f); p ("key", 5); p ("scale", 1); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.4f); p ("echoLevel", 0.55f);
    p ("reverbType", 1); p ("reverbSize", 0.6f); p ("reverbLevel", 0.45f);
    p ("pumpMode", 1); p ("pump", 0.15f); p ("comp", 0.3f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.5f); p ("tr909_bd_tone", 0.5f); p ("tr909_cp_level", 0.42f); p ("tr909_cp_reverb", 0.3f);
    p ("tr909_ch_level", 0.36f); p ("tr909_ch_decay", 0.2f); p ("tr909_oh_level", 0.3f); p ("tr909_rd_level", 0.32f); p ("tr909_rd_decay", 0.6f);
    p ("tr909_rs_level", 0.32f); p ("tr909_rs_echo", 0.3f); p ("tr909_cr_level", 0.32f);
    p ("tr909_lt_level", 0.45f); p ("tr909_mt_level", 0.45f); p ("tr909_ht_level", 0.45f); p ("tr909_sd_level", 0.4f);

    w.show (Juno106);
    p ("juno106_chord", 0); p ("juno106_saw", 0.6f); p ("juno106_pulse", 0.45f); p ("juno106_pwm", 0.4f); p ("juno106_cutoff", 0.5f);
    p ("juno106_reso", 0.2f); p ("juno106_env", 0.35f); p ("juno106_decay", 0.45f); p ("juno106_sustain", 0.25f);
    p ("juno106_release", 0.45f); p ("juno106_chorus", 3); p ("juno106_hpf", 1); p ("juno106_echo", 0.35f);
    p ("juno106_reverb", 0.3f); p ("juno106_volume", 0.72f);

    w.show (ModelD);
    p ("modeld_wave", 1); p ("modeld_detune", 0.05f); p ("modeld_sub", 0.4f); p ("modeld_cutoff", 0.38f);
    p ("modeld_emphasis", 0.3f); p ("modeld_contour", 0.45f); p ("modeld_decay", 0.25f); p ("modeld_sustain", 0.2f);
    p ("modeld_glide", 0.08f); p ("modeld_overdrive", 0.25f); p ("modeld_volume", 0.8f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_attack", 0.7f); p ("strings_release", 0.75f); p ("strings_ensemble", 0.8f);
    p ("strings_brightness", 0.45f); p ("strings_reverb", 0.5f); p ("strings_volume", 0.5f);

    // F minor, soul chords: Fm9, Dbmaj7, Eb (add 9), Cm7. The bass jumps octaves like a funk bassist.
    const juce::String none = "................", kick = "x...x...x...x...", claps = "....x.......x...";
    const juce::String fm9 = "G#3+C4+D#4+G4", dbmaj7 = "G#3+C4+C#4+F4", eb = "G3+A#3+D#4+F4", cm7 = "G3+A#3+C4+D#4";
    auto stab = [] (const juce::String& c) { return c + " - . " + c + " - . " + c + " . . . " + c + " - . . . . "; };
    auto bass = [] (const juce::String& lo, const juce::String& hi, const juce::String& pick)
    { return lo + " . . " + hi + " . . " + lo + " . . . " + lo + " . " + pick + " . " + hi + " . "; };
    const juce::String bassLine = bars ({ bass ("F1", "F2", "D#2"), bass ("C#2", "C#2", "C2"), bass ("D#1", "D#2", "F2"), bass ("C2", "C2", "A#1") });
    const juce::String keys = bars ({ stab (fm9), stab (dbmaj7), stab (eb), stab (cm7) });

    auto beat = [&] (const juce::String& kicks, bool ride)
    {
        drums (w, TR909, "bd", kicks);
        drums (w, TR909, "cp", claps);
        drums (w, TR909, "ch", "x.xxx.xxx.xxx.xx");
        stepValues (w, TR909, "tr909_ch_level", "0.3 0.15 0.45 0.2 ");
        drums (w, TR909, "oh", "..x...x...x...x.");
        if (ride) drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    };

    // A - intro: kick, ride and rimshots, the chords filtered.
    w.part (0);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    drums (w, TR909, "rs", "...x..x.......x.");
    chords (w, Juno106, keys);
    stepValues (w, Juno106, "juno106_cutoff", ramp (0.25f, 0.42f));

    // B - the bass and the clap.
    w.part (1);
    beat (kick, true);
    line (w, ModelD, bassLine);
    chords (w, Juno106, keys);

    // C - full groove: the chords open, the strings hold the harmony.
    w.part (2);
    beat (kick, true);
    drums (w, TR909, "rs", "...x..x.......x.");
    line (w, ModelD, bassLine);
    chords (w, Juno106, keys);
    stepValues (w, Juno106, "juno106_cutoff", hold (0.58f, 64));
    chords (w, Strings, bars ({ held (fm9, 16), held (dbmaj7, 16), held (eb, 16), held (cm7, 16) }));

    // D - break: no kick, the chords drift in the reverb, the bass holds the roots.
    w.part (3);
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    stepValues (w, TR909, "tr909_rd_level", ramp (0.15f, 0.35f));
    chords (w, Strings, bars ({ held (fm9, 16), held (dbmaj7, 16), held (eb, 16), held (cm7, 16) }));
    chords (w, Juno106, bars ({ held (fm9, 8) + ". . . . . . . . ", held (dbmaj7, 8) + ". . . . . . . . ",
                                held (eb, 8) + ". . . . . . . . ", held (cm7, 8) + ". . . . . . . . " }));
    stepValues (w, Juno106, "juno106_reverb", hold (0.7f, 64));
    line (w, ModelD, bars ({ held ("F1", 16), held ("C#2", 16), held ("D#1", 16), held ("C2", 12) + ". . . . " }));
    stepValues (w, ModelD, "modeld_volume", hold (0.55f, 64));
    drums (w, TR909, "sd", bars ({ none, none, none, "x.x.x.x.xxxxxxxx" }));
    stepValues (w, TR909, "tr909_sd_level", hold (0.2f, 48) + ramp (0.2f, 0.55f, 16));

    // E - back with a crash and a tom fill at the end of the phrase.
    w.part (4);
    beat (bars ({ kick, kick, kick, "x...x...x......." }), true);
    drums (w, TR909, "cr", "x...............................................................");
    drums (w, TR909, "ht", bars ({ none, none, none, "..........xx...." }));
    drums (w, TR909, "mt", bars ({ none, none, none, "............xx.." }));
    drums (w, TR909, "lt", bars ({ none, none, none, "..............xx" }));
    line (w, ModelD, bassLine);
    chords (w, Juno106, keys);
    stepValues (w, Juno106, "juno106_cutoff", hold (0.62f, 64));
    chords (w, Strings, bars ({ held (fm9, 16), held (dbmaj7, 16), held (eb, 16), held (cm7, 16) }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 3 }, { 4, 1 }, { 3, 2 }, { 2, 3 }, { 4, 1 }, { 1, 1 }, { 0, 1 } });
}

void house (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 122); p ("swing", 0.18f); p ("key", 9); p ("scale", 5); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.38f); p ("echoLevel", 0.55f);
    p ("reverbType", 2); p ("reverbSize", 0.6f); p ("reverbLevel", 0.45f);
    p ("pumpMode", 1); p ("pump", 0.2f); p ("comp", 0.3f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.6f); p ("tr909_bd_tone", 0.4f); p ("tr909_bd_drive", 0.05f);
    p ("tr909_cp_level", 0.42f); p ("tr909_cp_reverb", 0.35f); p ("tr909_ch_level", 0.38f); p ("tr909_ch_decay", 0.18f);
    p ("tr909_oh_level", 0.3f); p ("tr909_oh_decay", 0.3f); p ("tr909_rs_level", 0.32f); p ("tr909_rs_echo", 0.3f);

    w.show (Percussion);
    p ("perc_sh_level", 0.32f); p ("perc_cgh_level", 0.26f); p ("perc_cgl_level", 0.28f); p ("perc_tb_level", 0.25f);
    p ("perc_cgh_reverb", 0.2f); p ("perc_cgl_reverb", 0.2f);

    w.show (ModelD);
    p ("modeld_wave", 2); p ("modeld_detune", 0.0f); p ("modeld_sub", 0.6f); p ("modeld_cutoff", 0.32f);
    p ("modeld_emphasis", 0.15f); p ("modeld_contour", 0.25f); p ("modeld_decay", 0.35f); p ("modeld_sustain", 0.35f);
    p ("modeld_glide", 0.05f); p ("modeld_overdrive", 0.15f); p ("modeld_volume", 0.85f);

    w.show (EPiano);
    p ("epiano_chord", 0); p ("epiano_type", 0); p ("epiano_tone", 0.45f); p ("epiano_decay", 0.55f); p ("epiano_release", 0.35f);
    p ("epiano_tremolo", 0.35f); p ("epiano_bark", 0.3f); p ("epiano_echo", 0.15f); p ("epiano_reverb", 0.3f); p ("epiano_volume", 0.72f);

    w.show (Mallets);
    p ("mallets_chord", 0); p ("mallets_type", 1); p ("mallets_brightness", 0.5f); p ("mallets_decay", 0.6f);
    p ("mallets_release", 0.5f); p ("mallets_mallet", 0.45f); p ("mallets_vibrato", 0.35f);
    p ("mallets_echo", 0.4f); p ("mallets_reverb", 0.35f); p ("mallets_volume", 0.9f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_violin", 0.65f); p ("strings_cello", 0.6f); p ("strings_attack", 0.7f); p ("strings_release", 0.8f);
    p ("strings_ensemble", 0.85f); p ("strings_brightness", 0.35f); p ("strings_reverb", 0.5f); p ("strings_volume", 0.55f);

    w.show (SoundFx);
    p ("fx_swp_level", 0.3f); p ("fx_rise_decay", 0.72f); p ("fx_rise_level", 0.35f); p ("fx_rise_tone", 0.2f);

    // A Dorian. Verse: Am9 / D9 (rootless voicings, the F# of the Dorian mode). Break: Cmaj7 / Bm7 / Am9 / D9.
    const juce::String none = "................", kick = "x...x...x...x...", claps = "....x.......x...",
                       hats = "xxxxxxxxxxxxxxxx", offbeats = "..x...x...x...x.", rims = "..........x..x..";
    const juce::String am9 = "G3+B3+C4+E4", d9 = "F#3+A3+C4+E4", cmaj7 = "E3+G3+B3+C4", bm7 = "F#3+A3+B3+D4";
    auto comp = [] (const juce::String& c) { return c + " - - . . . " + c + " - . . " + c + " - - - . . "; };
    const juce::String bassAm = ". . A1 . . . A1 . . . . A2 . . G1 . ", bassD = ". . D2 . . . D2 . . . . A1 . . C2 . ";
    const juce::String hook = bars ({ ". . E5 . G5 . A5 - . . G5 . E5 - . . ", ". . F#5 . A5 . B5 - . . A5 . F#5 - E5 . ",
                                      ". . E5 . G5 . A5 - . . C6 . B5 - A5 . ", ". . G5 . F#5 . E5 - - - . . D5 . E5 . " });

    auto drumsUp = [&] (bool full)
    {
        drums (w, TR909, "bd", kick);
        drums (w, TR909, "cp", claps);
        drums (w, TR909, "ch", hats);
        stepValues (w, TR909, "tr909_ch_level", "0.12 0.22 0.42 0.22 ");
        drums (w, TR909, "rs", rims);
        drums (w, Percussion, "sh", hats);
        stepValues (w, Percussion, "perc_sh_level", "0.18 0.3 0.45 0.3 ");
        if (! full) return;
        drums (w, TR909, "oh", offbeats);
        drums (w, Percussion, "cgh", "...x..x.......x....x..x....x..x.");
        drums (w, Percussion, "cgl", "........x..x............x.....x.");
        drums (w, Percussion, "tb", "....x.......x...");
    };
    auto groove = [&] { line (w, ModelD, bars ({ bassAm, bassD, bassAm, bassD })); };
    auto verseKeys = [&] { chords (w, EPiano, bars ({ comp (am9), comp (d9), comp (am9), comp (d9) })); };

    // A - intro: kick, shaker, rimshots; the Rhodes far away.
    w.part (0);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "rs", rims);
    drums (w, Percussion, "sh", hats);
    stepValues (w, Percussion, "perc_sh_level", "0.18 0.3 0.45 0.3 ");
    chords (w, EPiano, bars ({ comp (am9), rest16, comp (d9), rest16 }));
    stepValues (w, EPiano, "epiano_filter", hold (0.25f, 64));
    drums (w, SoundFx, "swp", bars ({ none, none, none, "........x......." }));

    // B - the groove: clap, hats, the bass.
    w.part (1);
    drumsUp (false);
    groove();

    // C - verse: the Rhodes plays Am9 / D9.
    w.part (2);
    drumsUp (false);
    groove();
    verseKeys();

    // D - hook: the vibraphone sings, open hats and congas.
    w.part (3);
    drumsUp (true);
    groove();
    verseKeys();
    chords (w, Mallets, hook);

    // E - break: no kick, strings, the vibraphone answers in the echo, a riser brings the hook back.
    w.part (4);
    drums (w, Percussion, "sh", hats);
    stepValues (w, Percussion, "perc_sh_level", "0.12 0.2 0.32 0.2 ");
    drums (w, TR909, "rs", rims);
    chords (w, Strings, bars ({ held (cmaj7, 16), held (bm7, 16), held (am9, 16), held (d9, 16) }));
    chords (w, EPiano, bars ({ held (cmaj7, 8) + rest16.substring (0, 16), held (bm7, 8) + rest16.substring (0, 16),
                               held (am9, 8) + rest16.substring (0, 16), held (d9, 8) + rest16.substring (0, 16) }));
    chords (w, Mallets, bars ({ ". . E5 . G5 . B5 - - - . . . . . . ", rest16, ". . E5 . G5 . A5 - - - . . . . . . ", rest16 }));
    stepValues (w, Mallets, "mallets_echo", hold (0.7f, 64));
    line (w, ModelD, bars ({ "C2 - - - - - - - . . . . . . . . ", "B1 - - - - - - - . . . . . . . . ", rest16, rest16 }));
    drums (w, SoundFx, "rise", bars ({ none, none, "x...............", none }));

    // F - last hook: everything, with the strings under it.
    w.part (5);
    drumsUp (true);
    groove();
    verseKeys();
    chords (w, Mallets, hook);
    chords (w, Strings, bars ({ held (cmaj7, 16), held (d9, 16), held (cmaj7, 16), held (d9, 16) }));
    stepValues (w, Strings, "strings_volume", hold (0.4f, 64));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 }, { 4, 2 }, { 5, 3 }, { 2, 1 }, { 1, 1 }, { 0, 1 } });
}

void berlin (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 133); p ("swing", 0.0f); p ("key", 9); p ("scale", 6); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 2); p ("echoFeedback", 0.55f); p ("echoLevel", 0.6f);
    p ("reverbType", 1); p ("reverbSize", 0.85f); p ("reverbLevel", 0.5f);
    p ("pumpMode", 1); p ("pump", 0.35f); p ("comp", 0.4f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.7f); p ("tr909_bd_tone", 0.35f); p ("tr909_bd_drive", 0.35f); p ("tr909_bd_reverb", 0.3f);
    p ("tr909_ch_level", 0.34f); p ("tr909_ch_decay", 0.15f); p ("tr909_oh_level", 0.3f); p ("tr909_oh_decay", 0.3f);
    p ("tr909_rs_level", 0.4f); p ("tr909_rs_echo", 0.6f); p ("tr909_rs_reverb", 0.3f); p ("tr909_cp_level", 0.4f); p ("tr909_cp_reverb", 0.5f);
    p ("tr909_rd_level", 0.25f); p ("tr909_sd_level", 0.4f);

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_sub", 0.7f); p ("modeld_cutoff", 0.22f); p ("modeld_emphasis", 0.2f);
    p ("modeld_contour", 0.3f); p ("modeld_decay", 0.15f); p ("modeld_sustain", 0.0f); p ("modeld_overdrive", 0.45f); p ("modeld_volume", 0.8f);

    w.show (Juno106);
    p ("juno106_chord", 0); p ("juno106_saw", 0.8f); p ("juno106_pulse", 0.2f); p ("juno106_cutoff", 0.35f); p ("juno106_reso", 0.4f);
    p ("juno106_env", 0.3f); p ("juno106_attack", 0.5f); p ("juno106_decay", 0.6f); p ("juno106_sustain", 0.6f); p ("juno106_release", 0.7f);
    p ("juno106_chorus", 1); p ("juno106_hpf", 2); p ("juno106_echo", 0.3f); p ("juno106_reverb", 0.5f); p ("juno106_volume", 0.8f);

    // A Phrygian: the drone A - E with the dark B flat, the rimshots thrown into a long tape echo.
    const juce::String none = "................", kick = "x...x...x...x...";
    const juce::String rumble = ". A1 A1 A1 . A1 A1 A1 . A1 A1 A1 . A1 A1 A1 ";
    const juce::String rumbleTurn = ". A1 A1 A1 . A1 A1 A1 . A#1 A#1 A#1 . A1 G1 A1 ";
    const juce::String bass = bars ({ rumble, rumble, rumble, rumbleTurn });
    const juce::String drone = bars ({ held ("A2+E3+A3", 32) , held ("A#2+E3+A3", 16), held ("A2+E3+G3", 16) });
    const juce::String stab = bars ({ ". . . . . . A#3+E4 . . . . . . . . . ", ". . . . . . . . . . A#3+E4 . . . . . ",
                                      ". . . . . . A#3+E4 . . . . . . . . . ", ". . . . . . . . . . . . . . A#3+F4 . " });
    const juce::String rims = "...x.....x....x....x...x......x.";

    auto beat = [&] (const juce::String& kicks)
    {
        drums (w, TR909, "bd", kicks);
        drums (w, TR909, "ch", "..x...x...x...x.");
        drums (w, TR909, "oh", "....x.......x...");
        drums (w, TR909, "rs", rims);
    };

    // A - intro: kick in the reverb, rimshots in the echo.
    w.part (0);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "rs", bars ({ none, none, rims.substring (0, 16), rims.substring (16) }));

    // B - the rumble: the sub rolls between the kicks.
    w.part (1);
    beat (kick);
    line (w, ModelD, bass);

    // C - the drone rises out of the dark.
    w.part (2);
    beat (kick);
    drums (w, TR909, "ch", "xxxxxxxxxxxxxxxx");
    stepValues (w, TR909, "tr909_ch_level", "0.12 0.18 0.36 0.18 ");
    line (w, ModelD, bass);
    chords (w, Juno106, drone);
    stepValues (w, Juno106, "juno106_cutoff", ramp (0.15f, 0.4f));

    // D - peak: clap in a big room, ride, the dissonant stab.
    w.part (3);
    beat (kick);
    drums (w, TR909, "ch", "xxxxxxxxxxxxxxxx");
    stepValues (w, TR909, "tr909_ch_level", "0.12 0.18 0.36 0.18 ");
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    line (w, ModelD, bass);
    chords (w, Juno106, stab);
    stepValues (w, Juno106, "juno106_attack", hold (0.0f, 64));
    stepValues (w, Juno106, "juno106_decay", hold (0.3f, 64));
    stepValues (w, Juno106, "juno106_sustain", hold (0.0f, 64));
    stepValues (w, Juno106, "juno106_echo", hold (0.6f, 64));
    stepValues (w, Juno106, "juno106_cutoff", hold (0.6f, 64));
    stepValues (w, Juno106, "juno106_hpf", hold (1.0f, 64));

    // E - break: the kick stops, the drone and the echoes, then the snare climbs.
    w.part (4);
    drums (w, TR909, "rs", rims);
    chords (w, Juno106, drone);
    stepValues (w, Juno106, "juno106_cutoff", ramp (0.3f, 0.55f));
    drums (w, TR909, "sd", bars ({ none, none, "x.......x.......", "x...x...x.x.xxxx" }));
    stepValues (w, TR909, "tr909_sd_level", hold (0.2f, 32) + ramp (0.2f, 0.5f, 32));
    line (w, ModelD, bars ({ rest16, rest16, rest16, ". . . . . . . . . . . . . A1 A1 A1 " }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 3 }, { 4, 1 }, { 3, 3 }, { 2, 1 }, { 1, 1 } });
}

void trance (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 138); p ("swing", 0.0f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.45f); p ("echoLevel", 0.6f);
    p ("reverbType", 1); p ("reverbSize", 0.8f); p ("reverbLevel", 0.5f);
    p ("pumpMode", 1); p ("pump", 0.35f); p ("comp", 0.35f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.45f); p ("tr909_bd_tone", 0.6f); p ("tr909_bd_drive", 0.15f);
    p ("tr909_ch_level", 0.36f); p ("tr909_ch_decay", 0.2f); p ("tr909_oh_level", 0.4f); p ("tr909_oh_decay", 0.35f);
    p ("tr909_cp_level", 0.45f); p ("tr909_cp_reverb", 0.3f); p ("tr909_sd_level", 0.42f); p ("tr909_sd_reverb", 0.25f);
    p ("tr909_cr_level", 0.38f); p ("tr909_rd_level", 0.25f);

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_detune", 0.12f); p ("modeld_sub", 0.5f); p ("modeld_cutoff", 0.35f); p ("modeld_emphasis", 0.25f);
    p ("modeld_contour", 0.45f); p ("modeld_decay", 0.18f); p ("modeld_sustain", 0.1f); p ("modeld_overdrive", 0.2f); p ("modeld_volume", 0.92f);

    w.show (SH101);
    p ("sh101_octave", 3); p ("sh101_saw", 0.3f); p ("sh101_square", 0.7f); p ("sh101_sub", 0.0f); p ("sh101_cutoff", 0.5f);
    p ("sh101_reso", 0.35f); p ("sh101_env", 0.45f); p ("sh101_decay", 0.22f); p ("sh101_sustain", 0.0f); p ("sh101_release", 0.1f);
    p ("sh101_glide", 0.0f); p ("sh101_echo", 0.4f); p ("sh101_reverb", 0.2f); p ("sh101_volume", 0.85f);

    w.show (JP8000);
    p ("jp8000_chord", 0); p ("jp8000_detune", 0.6f); p ("jp8000_mix", 0.7f); p ("jp8000_cutoff", 0.55f); p ("jp8000_reso", 0.1f);
    p ("jp8000_attack", 0.3f); p ("jp8000_decay", 0.6f); p ("jp8000_sustain", 0.85f); p ("jp8000_release", 0.6f);
    p ("jp8000_reverb", 0.45f); p ("jp8000_volume", 0.65f);

    w.show (Edm);
    p ("edm_chord", 0); p ("edm_octave", 2); p ("edm_detune", 0.45f); p ("edm_mix", 0.6f); p ("edm_cutoff", 0.65f);
    p ("edm_attack", 0.02f); p ("edm_decay", 0.5f); p ("edm_sustain", 0.75f); p ("edm_release", 0.35f); p ("edm_pump", 0.4f);
    p ("edm_echo", 0.35f); p ("edm_reverb", 0.4f); p ("edm_volume", 0.6f);

    w.show (SoundFx);
    p ("fx_rise_decay", 0.75f); p ("fx_rise_level", 0.45f); p ("fx_imp_level", 0.55f); p ("fx_down_decay", 0.6f); p ("fx_down_level", 0.35f);

    // A minor: Am - F - C - G, one bar each. Supersaw pads, the 101 arpeggiates the chord, the bass on the off-beats.
    const juce::String none = "................", kick = "X...x...x...x...", offs = "..x...x...x...x.";
    auto off = [] (const juce::String& n) { return ". . " + n + " . . . " + n + " . . . " + n + " . . . " + n + " . "; };
    const juce::String bass = bars ({ off ("A1"), off ("F1"), off ("C2"), off ("G1") });
    const juce::String arp = bars ({ "A3 C4 E4 A4 C5 A4 E4 C4 A3 C4 E4 A4 C5 A4 E4 C4 ", "F3 A3 C4 F4 A4 F4 C4 A3 F3 A3 C4 F4 A4 F4 C4 A3 ",
                                     "C4 E4 G4 C5 E5 C5 G4 E4 C4 E4 G4 C5 E5 C5 G4 E4 ", "G3 B3 D4 G4 B4 G4 D4 B3 G3 B3 D4 G4 B4 G4 D4 B3 " });
    const juce::String pads = bars ({ held ("A3+E4+A4+C5", 16), held ("F3+C4+F4+A4", 16), held ("G3+C4+E4+G4", 16), held ("G3+B3+D4+G4", 16) });
    const juce::String lead = bars ({ "A4 - - . C5 - - . E5 - - . D5 - C5 - ", "C5 - - . A4 - - . F4 - - . A4 - C5 - ",
                                      "G4 - - . C5 - - . E5 - - . G5 - E5 - ", "D5 - - - - - - - B4 - - . D5 - - - " });

    auto beat = [&] (const juce::String& kicks, bool clap)
    {
        drums (w, TR909, "bd", kicks);
        drums (w, TR909, "oh", offs);
        drums (w, TR909, "ch", "xxxxxxxxxxxxxxxx");
        stepValues (w, TR909, "tr909_ch_level", "0.18 0.25 0.1 0.25 ");
        if (clap) drums (w, TR909, "cp", "....x.......x...");
    };

    // A - intro: kick, hats, the arpeggio behind a filter.
    w.part (0);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "oh", offs);
    line (w, SH101, arp);
    stepValues (w, SH101, "sh101_cutoff", ramp (0.2f, 0.4f));

    // B - the bass on the off-beats, the clap.
    w.part (1);
    beat (kick, true);
    line (w, ModelD, bass);
    line (w, SH101, arp);
    stepValues (w, SH101, "sh101_cutoff", hold (0.42f, 64));

    // C - the pads arrive under the arpeggio.
    w.part (2);
    beat (kick, true);
    drums (w, TR909, "rd", offs);
    line (w, ModelD, bass);
    line (w, SH101, arp);
    chords (w, JP8000, pads);
    stepValues (w, JP8000, "jp8000_cutoff", ramp (0.3f, 0.55f));

    // D - breakdown: no drums, the pads open, the lead plays the theme alone.
    w.part (3);
    drums (w, SoundFx, "down", "x...............................................................");
    chords (w, JP8000, pads);
    stepValues (w, JP8000, "jp8000_cutoff", hold (0.6f, 64));
    chords (w, Edm, lead);
    stepValues (w, Edm, "edm_reverb", hold (0.7f, 64));
    stepValues (w, Edm, "edm_pump", hold (0.0f, 64));

    // E - build: the arpeggio opens, snare roll, riser; the kick stops for the last beat.
    w.part (4);
    drums (w, TR909, "sd", bars ({ "x...x...x...x...", "x...x...x...x...", "x.x.x.x.x.x.x.x.", "xxxxxxxx22224444" }));
    stepValues (w, TR909, "tr909_sd_level", ramp (0.15f, 0.6f));
    drums (w, TR909, "bd", bars ({ none, none, kick, "x...x...x......." }));
    chords (w, JP8000, pads);
    line (w, SH101, arp);
    stepValues (w, SH101, "sh101_cutoff", ramp (0.25f, 0.7f));
    drums (w, SoundFx, "rise", bars ({ none, "x...............", none, none }));

    // F - drop: everything, the lead on top.
    w.part (5);
    beat (kick, true);
    drums (w, TR909, "cr", "x...............................................................");
    drums (w, SoundFx, "imp", "x...............................................................");
    line (w, ModelD, bass);
    line (w, SH101, arp);
    chords (w, JP8000, pads);
    chords (w, Edm, lead);

    // G - second half of the drop, the ride drives it.
    w.part (6);
    beat (bars ({ kick, kick, kick, "x...x...x...x.x." }), true);
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    line (w, ModelD, bass);
    line (w, SH101, arp);
    chords (w, JP8000, pads);
    chords (w, Edm, lead);
    drums (w, TR909, "sd", bars ({ none, none, none, "........x.x.xxxx" }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 }, { 4, 1 }, { 5, 2 }, { 6, 2 }, { 2, 1 }, { 1, 1 } });
}

void psytrance (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 145); p ("swing", 0.0f); p ("key", 4); p ("scale", 7); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.5f); p ("echoLevel", 0.6f);
    p ("reverbType", 1); p ("reverbSize", 0.7f); p ("reverbLevel", 0.4f);
    p ("pumpMode", 1); p ("pump", 0.15f); p ("comp", 0.35f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.3f); p ("tr909_bd_tone", 0.7f); p ("tr909_bd_drive", 0.25f);
    p ("tr909_ch_level", 0.34f); p ("tr909_ch_decay", 0.15f); p ("tr909_oh_level", 0.38f); p ("tr909_oh_decay", 0.3f);
    p ("tr909_cp_level", 0.4f); p ("tr909_cp_reverb", 0.3f); p ("tr909_sd_level", 0.4f); p ("tr909_cr_level", 0.35f);

    w.show (SH101);
    p ("sh101_octave", 1); p ("sh101_saw", 0.9f); p ("sh101_square", 0.0f); p ("sh101_sub", 0.4f); p ("sh101_cutoff", 0.32f);
    p ("sh101_reso", 0.4f); p ("sh101_env", 0.55f); p ("sh101_decay", 0.14f); p ("sh101_sustain", 0.0f); p ("sh101_release", 0.05f);
    p ("sh101_glide", 0.0f); p ("sh101_volume", 1.0f);

    w.show (JP8000);
    p ("jp8000_chord", 0); p ("jp8000_detune", 0.25f); p ("jp8000_mix", 0.4f); p ("jp8000_cutoff", 0.5f); p ("jp8000_reso", 0.35f);
    p ("jp8000_env", 0.5f); p ("jp8000_attack", 0.0f); p ("jp8000_decay", 0.35f); p ("jp8000_sustain", 0.35f); p ("jp8000_release", 0.2f);
    p ("jp8000_echo", 0.55f); p ("jp8000_reverb", 0.3f); p ("jp8000_volume", 0.95f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_attack", 0.8f); p ("strings_release", 0.8f); p ("strings_brightness", 0.4f);
    p ("strings_reverb", 0.6f); p ("strings_volume", 0.5f);

    w.show (SoundFx);
    p ("fx_rise_decay", 0.72f); p ("fx_rise_level", 0.42f); p ("fx_zap_level", 0.3f); p ("fx_zap_echo", 0.5f); p ("fx_imp_level", 0.5f);

    // E Phrygian dominant (E F G# A B C D): the oriental colour. Kick on the beat, three bass notes after it.
    const juce::String none = "................", kick = "x...x...x...x...";
    const juce::String rolling = ". E1 E1 E1 . E1 E1 E1 . E1 E1 E1 . E1 E1 E1 ";
    const juce::String bass = bars ({ rolling, rolling, rolling, ". E1 E1 E1 . E1 E1 E1 . F1 F1 F1 . G#1 G#1 G#1 " });
    const juce::String r1 = "E4 F4 G#4 E4 . B4 A4 G#4 F4 . E4 F4 G#4 A4 G#4 F4 ", r2 = "E4 F4 G#4 E4 . B4 A4 G#4 F4 . E4 F4 G#4 A4 B4 C5 ",
                       r4 = "D5 C5 B4 A4 G#4 A4 B4 G#4 F4 E4 F4 G#4 E4 . . . ";
    const juce::String riff = bars ({ r1, r2, r1, r4 });
    const juce::String drone = bars ({ held ("E3+B3+E4", 32), held ("F3+C4+F4", 16), held ("E3+G#3+B3", 16) });

    auto beat = [&] (const juce::String& kicks)
    {
        drums (w, TR909, "bd", kicks);
        drums (w, TR909, "oh", "..x...x...x...x.");
        drums (w, TR909, "ch", "x.xxx.xxx.xxx.xx");
        stepValues (w, TR909, "tr909_ch_level", "0.3 0.15 0.2 0.15 ");
        drums (w, TR909, "cp", "....x.......x...");
    };

    // A - intro: kick and the rolling bass, closed.
    w.part (0);
    drums (w, TR909, "bd", kick);
    line (w, SH101, bass);
    stepValues (w, SH101, "sh101_cutoff", ramp (0.15f, 0.3f));

    // B - the groove opens.
    w.part (1);
    beat (kick);
    line (w, SH101, bass);
    stepValues (w, SH101, "sh101_cutoff", ramp (0.28f, 0.4f, 32) + ramp (0.4f, 0.3f, 32));

    // C - the riff comes in at the third bar.
    w.part (2);
    beat (kick);
    line (w, SH101, bass);
    chords (w, JP8000, bars ({ rest16, rest16, r1, r4 }));
    drums (w, SoundFx, "zap", bars ({ none, "..............x.", none, "..............x." }));

    // D - the riff all the way, its filter moving.
    w.part (3);
    beat (kick);
    line (w, SH101, bass);
    chords (w, JP8000, riff);
    stepValues (w, JP8000, "jp8000_cutoff", ramp (0.35f, 0.65f, 32) + ramp (0.65f, 0.4f, 32));

    // E - break: no kick, the strings drone, the riff lost in the echo, then the riser.
    w.part (4);
    chords (w, Strings, drone);
    chords (w, JP8000, riff);
    stepValues (w, JP8000, "jp8000_echo", hold (0.85f, 64));
    stepValues (w, JP8000, "jp8000_cutoff", ramp (0.25f, 0.55f));
    drums (w, TR909, "sd", bars ({ none, none, "x...x...x...x...", "x.x.x.x.xxxx2244" }));
    stepValues (w, TR909, "tr909_sd_level", hold (0.2f, 32) + ramp (0.2f, 0.55f, 32));
    drums (w, SoundFx, "rise", bars ({ none, none, "x...............", none }));

    // F - drop: everything with the strings under it.
    w.part (5);
    beat (kick);
    drums (w, TR909, "cr", "x...............................................................");
    drums (w, SoundFx, "imp", "x...............................................................");
    line (w, SH101, bass);
    chords (w, JP8000, riff);
    stepValues (w, JP8000, "jp8000_cutoff", hold (0.62f, 64));
    chords (w, Strings, drone);
    stepValues (w, Strings, "strings_volume", hold (0.35f, 64));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 1 }, { 3, 3 }, { 4, 2 }, { 5, 3 }, { 3, 1 }, { 1, 1 } });
}

void dubTechno (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 118); p ("swing", 0.1f); p ("key", 0); p ("scale", 1); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 2); p ("echoFeedback", 0.62f); p ("echoLevel", 0.7f);
    p ("reverbType", 1); p ("reverbSize", 0.9f); p ("reverbLevel", 0.55f);
    p ("pumpMode", 1); p ("pump", 0.2f); p ("comp", 0.25f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.55f); p ("tr909_bd_tone", 0.3f); p ("tr909_ch_level", 0.3f); p ("tr909_ch_decay", 0.35f);
    p ("tr909_rs_level", 0.3f); p ("tr909_rs_echo", 0.5f); p ("tr909_cp_level", 0.3f); p ("tr909_cp_reverb", 0.6f);
    p ("tr909_oh_level", 0.22f); p ("tr909_oh_decay", 0.5f);

    w.show (Juno106);
    p ("juno106_chord", 0); p ("juno106_saw", 0.65f); p ("juno106_pulse", 0.35f); p ("juno106_cutoff", 0.35f); p ("juno106_reso", 0.3f);
    p ("juno106_env", 0.35f); p ("juno106_decay", 0.45f); p ("juno106_sustain", 0.1f); p ("juno106_release", 0.45f);
    p ("juno106_chorus", 2); p ("juno106_hpf", 1); p ("juno106_echo", 0.8f); p ("juno106_reverb", 0.45f); p ("juno106_volume", 1.0f);

    w.show (ModelD);
    p ("modeld_wave", 2); p ("modeld_sub", 0.8f); p ("modeld_cutoff", 0.2f); p ("modeld_emphasis", 0.1f);
    p ("modeld_contour", 0.15f); p ("modeld_decay", 0.4f); p ("modeld_sustain", 0.4f); p ("modeld_overdrive", 0.1f); p ("modeld_volume", 0.85f);

    // C minor: one Cm7 chord that never stops moving - its filter and its echo do the music. Fm9 in the second half.
    const juce::String none = "................", kick = "x...x...x...x...";
    const juce::String cm7 = "G3+A#3+C4+D#4", fm9 = "G#3+C4+D#4+G4";
    auto chordBar = [] (const juce::String& c) { return ". . . " + c + " . . . . . . " + c + " . . . . . "; };
    const juce::String chordsA = bars ({ chordBar (cm7), chordBar (cm7), chordBar (cm7), chordBar (cm7) });
    const juce::String chordsB = bars ({ chordBar (cm7), chordBar (cm7), chordBar (fm9), chordBar (fm9) });
    const juce::String sub = bars ({ ". . C2 . . . C2 . . . C2 . . . C2 . ", ". . C2 . . . C2 . . . C2 . . . C2 . ",
                                     ". . C2 . . . C2 . . . C2 . . . C2 . ", ". . C2 . . . C2 . . . A#1 . . . G1 . " });
    const juce::String subB = bars ({ ". . C2 . . . C2 . . . C2 . . . C2 . ", ". . C2 . . . C2 . . . C2 . . . C2 . ",
                                      ". . F1 . . . F1 . . . F1 . . . F1 . ", ". . F1 . . . F1 . . . G1 . . . G1 . " });
    const juce::String breathe = ramp (0.38f, 0.62f, 32) + ramp (0.62f, 0.42f, 32);

    auto beat = [&]
    {
        drums (w, TR909, "bd", kick);
        drums (w, TR909, "ch", "..x...x...x...x.");
        drums (w, TR909, "rs", "......x.........");
    };

    // A - intro: the kick and the chord far away.
    w.part (0);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "rs", "......x.........");
    chords (w, Juno106, chordsA);
    stepValues (w, Juno106, "juno106_cutoff", hold (0.32f, 64));

    // B - the sub, the hats; the chord starts to breathe.
    w.part (1);
    beat();
    line (w, ModelD, sub);
    chords (w, Juno106, chordsA);
    stepValues (w, Juno106, "juno106_cutoff", breathe);

    // C - deeper: the clap far back, the open hat, the second chord.
    w.part (2);
    beat();
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "oh", "..............x.");
    line (w, ModelD, subB);
    chords (w, Juno106, chordsB);
    stepValues (w, Juno106, "juno106_cutoff", ramp (0.42f, 0.7f, 32) + ramp (0.7f, 0.45f, 32));

    // D - break: no kick, the echo swallows the chord.
    w.part (3);
    drums (w, TR909, "ch", "..x...x...x...x.");
    chords (w, Juno106, bars ({ chordBar (cm7), rest16, chordBar (fm9), rest16 }));
    stepValues (w, Juno106, "juno106_echo", hold (1.0f, 64));
    stepValues (w, Juno106, "juno106_cutoff", ramp (0.65f, 0.4f));
    line (w, ModelD, bars ({ held ("C2", 16), rest16, held ("F1", 16), rest16 }));

    w.song ({ { 0, 2 }, { 1, 3 }, { 2, 3 }, { 3, 2 }, { 2, 3 }, { 1, 2 }, { 0, 1 } });
}

void pianoHouse (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 124); p ("swing", 0.15f); p ("key", 5); p ("scale", 2); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("echoLevel", 0.5f);
    p ("reverbType", 2); p ("reverbSize", 0.6f); p ("reverbLevel", 0.45f);
    p ("pumpMode", 1); p ("pump", 0.25f); p ("comp", 0.35f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.55f); p ("tr909_bd_tone", 0.5f); p ("tr909_cp_level", 0.45f); p ("tr909_cp_reverb", 0.3f);
    p ("tr909_ch_level", 0.34f); p ("tr909_oh_level", 0.36f); p ("tr909_oh_decay", 0.35f); p ("tr909_cr_level", 0.36f);
    p ("tr909_sd_level", 0.42f); p ("tr909_rd_level", 0.25f);

    w.show (Piano);
    p ("piano_chord", 0); p ("piano_type", 3); p ("piano_brightness", 0.65f); p ("piano_decay", 0.45f); p ("piano_release", 0.3f);
    p ("piano_hammer", 0.55f); p ("piano_width", 0.6f); p ("piano_reverb", 0.3f); p ("piano_volume", 0.8f);

    w.show (Choir);
    p ("choir_chord", 0); p ("choir_vowel", 0); p ("choir_attack", 0.45f); p ("choir_release", 0.6f); p ("choir_ensemble", 0.7f);
    p ("choir_brightness", 0.55f); p ("choir_reverb", 0.5f); p ("choir_volume", 0.45f);

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_sub", 0.5f); p ("modeld_cutoff", 0.38f); p ("modeld_emphasis", 0.2f); p ("modeld_contour", 0.4f);
    p ("modeld_decay", 0.2f); p ("modeld_sustain", 0.15f); p ("modeld_overdrive", 0.2f); p ("modeld_volume", 0.8f);

    w.show (SoundFx);
    p ("fx_rise_decay", 0.72f); p ("fx_rise_level", 0.42f); p ("fx_imp_level", 0.55f);

    // F major, vi - IV - I - V: Dm, Bb, F, C. Piano stabs that move by one note, a bass that jumps on the off-beats.
    const juce::String none = "................", kick = "x...x...x...x...";
    const juce::String dm = "A3+D4+F4", bb = "A#3+D4+F4", f = "A3+C4+F4", c = "G3+C4+E4";
    auto stab = [] (const juce::String& ch) { return ch + " . . " + ch + " . . " + ch + " . . . " + ch + " . " + ch + " . . . "; };
    auto off = [] (const juce::String& lo, const juce::String& hi) { return ". . " + lo + " . . . " + hi + " . . . " + lo + " . . . " + hi + " . "; };
    const juce::String keys = bars ({ stab (dm), stab (bb), stab (f), stab (c) });
    const juce::String bass = bars ({ off ("D2", "D3"), off ("A#1", "A#2"), off ("F1", "F2"), off ("C2", "C3") });
    const juce::String voices = bars ({ held ("D4+F4+A4", 16), held ("D4+F4+A#4", 16), held ("C4+F4+A4", 16), held ("C4+E4+G4", 16) });
    const juce::String hook = bars ({ "D5 . . F5 . . A5 . . . G5 . F5 . . . ", "F5 . . D5 . . F5 . . . A#5 . A5 . . . ",
                                      "A5 . . C6 . . A5 . . . F5 . G5 . . . ", "G5 . . E5 . . C5 . . . E5 . G5 . . . " });

    auto beat = [&] (const juce::String& kicks)
    {
        drums (w, TR909, "bd", kicks);
        drums (w, TR909, "cp", "....x.......x...");
        drums (w, TR909, "oh", "..x...x...x...x.");
        drums (w, TR909, "ch", "x...x...x...x...");
    };

    // A - intro: kick, hats, the piano filtered.
    w.part (0);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "oh", "..x...x...x...x.");
    chords (w, Piano, keys);
    stepValues (w, Piano, "piano_filter", ramp (0.25f, 0.45f));

    // B - the bass and the clap: the groove.
    w.part (1);
    beat (kick);
    line (w, ModelD, bass);
    chords (w, Piano, keys);

    // C - the choir sings the chords.
    w.part (2);
    beat (kick);
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    line (w, ModelD, bass);
    chords (w, Piano, keys);
    chords (w, Choir, voices);

    // D - break: piano alone with the hook in the right hand, the choir, then the riser.
    w.part (3);
    chords (w, Piano, bars ({ held (dm, 8) + ". . . . . . . . ", held (bb, 8) + ". . . . . . . . ", held (f, 8) + ". . . . . . . . ", held (c, 8) + ". . . . . . . . " }));
    chords (w, Choir, voices);
    drums (w, TR909, "sd", bars ({ none, none, "x...x...x...x...", "x.x.x.x.xxxx2244" }));
    stepValues (w, TR909, "tr909_sd_level", hold (0.2f, 32) + ramp (0.2f, 0.55f, 32));
    drums (w, SoundFx, "rise", bars ({ none, none, "x...............", none }));

    // E - drop: impact, the stabs, the hook played high on the piano, the choir.
    w.part (4);
    beat (kick);
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    drums (w, TR909, "cr", "x...............................................................");
    drums (w, SoundFx, "imp", "x...............................................................");
    line (w, ModelD, bass);
    chords (w, Piano, hook);
    chords (w, Choir, voices);

    // F - the drop goes on with the stabs; fill at the end.
    w.part (5);
    beat (bars ({ kick, kick, kick, "x...x...x......." }));
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    drums (w, TR909, "sd", bars ({ none, none, none, "........x.x.xxxx" }));
    line (w, ModelD, bass);
    chords (w, Piano, keys);
    chords (w, Choir, voices);

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 }, { 4, 2 }, { 5, 1 }, { 4, 1 }, { 5, 1 }, { 1, 1 }, { 0, 1 } });
}

void minimal (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 125); p ("swing", 0.3f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 2); p ("echoFeedback", 0.5f); p ("echoLevel", 0.6f);
    p ("reverbType", 0); p ("reverbSize", 0.5f); p ("reverbLevel", 0.35f);
    p ("pumpMode", 1); p ("pump", 0.15f); p ("comp", 0.3f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.4f); p ("tr909_bd_tone", 0.45f); p ("tr909_rs_level", 0.42f); p ("tr909_rs_echo", 0.55f);
    p ("tr909_ch_level", 0.3f); p ("tr909_ch_decay", 0.12f); p ("tr909_cp_level", 0.36f); p ("tr909_cp_reverb", 0.25f);
    p ("tr909_oh_level", 0.25f); p ("tr909_oh_decay", 0.22f);

    w.show (Percussion);
    p ("perc_bl_level", 0.35f); p ("perc_bh_level", 0.32f); p ("perc_cl_level", 0.3f); p ("perc_cl_echo", 0.4f);
    p ("perc_sh_level", 0.22f); p ("perc_tr_level", 0.2f); p ("perc_cb_level", 0.25f); p ("perc_cb_echo", 0.4f);

    w.show (ModelD);
    p ("modeld_wave", 1); p ("modeld_sub", 0.6f); p ("modeld_cutoff", 0.25f); p ("modeld_emphasis", 0.3f); p ("modeld_contour", 0.35f);
    p ("modeld_decay", 0.16f); p ("modeld_sustain", 0.0f); p ("modeld_overdrive", 0.2f); p ("modeld_volume", 0.95f);

    w.show (SH101);
    p ("sh101_octave", 3); p ("sh101_saw", 0.0f); p ("sh101_square", 0.9f); p ("sh101_cutoff", 0.55f); p ("sh101_reso", 0.5f);
    p ("sh101_env", 0.5f); p ("sh101_decay", 0.16f); p ("sh101_sustain", 0.0f); p ("sh101_echo", 0.6f); p ("sh101_volume", 0.9f);

    w.show (SoundFx);
    p ("fx_rev_decay", 0.43f); p ("fx_rev_level", 0.35f); p ("fx_swp_level", 0.3f);

    // Very little, all of it placed: each element bounces against the others, the echo fills the gaps.
    const juce::String none = "................", kick = "x...x...x...x...";
    const juce::String rims = "...x..x.......x....x..x...x.....";
    const juce::String bass = bars ({ ". . . . . . A1 . . . . . . . A1 . ", ". . . A1 . . A1 . . . . . . . . . ",
                                      ". . . . . . A1 . . . . . . . A1 . ", ". . . A1 . . A1 . . . C2 . . . G1 . " });
    const juce::String bleeps = bars ({ ". . . . . . . . . . E5 . . . . . ", ". . . . . . A4 . . . . . . . . . ",
                                        ". . . . . . . . . . E5 . . . . C5 ", ". . . . . . . . . . . . . . . . " });

    auto beat = [&]
    {
        drums (w, TR909, "bd", kick);
        drums (w, TR909, "ch", "..x...x...x...x.");
        drums (w, TR909, "rs", rims);
    };

    // A - intro: the kick and the rimshots that bounce in the echo.
    w.part (0);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "rs", rims);

    // B - the bass, two notes, and the bongos.
    w.part (1);
    beat();
    line (w, ModelD, bass);
    drums (w, Percussion, "bh", "......x.......x.........x.....x.");
    drums (w, Percussion, "bl", "..x.........x.......x........x..");

    // C - the clap, the shaker, the bleeps of the 101.
    w.part (2);
    beat();
    drums (w, TR909, "cp", "....x.......x...");
    line (w, ModelD, bass);
    drums (w, Percussion, "bh", "......x.......x.........x.....x.");
    drums (w, Percussion, "bl", "..x.........x.......x........x..");
    drums (w, Percussion, "sh", "xxxxxxxxxxxxxxxx");
    stepValues (w, Percussion, "perc_sh_level", "0.1 0.16 0.24 0.16 ");
    line (w, SH101, bleeps);

    // D - peak: the claves and the cowbell answer, the bleeps open.
    w.part (3);
    beat();
    drums (w, TR909, "cp", "....x.......x...");
    drums (w, TR909, "oh", "..............x.");
    line (w, ModelD, bass);
    drums (w, Percussion, "bh", "......x.......x.........x.....x.");
    drums (w, Percussion, "bl", "..x.........x.......x........x..");
    drums (w, Percussion, "sh", "xxxxxxxxxxxxxxxx");
    stepValues (w, Percussion, "perc_sh_level", "0.1 0.16 0.24 0.16 ");
    drums (w, Percussion, "cl", ".....x.........x..........x.....");
    drums (w, Percussion, "cb", "...........x...................x");
    line (w, SH101, bleeps);
    stepValues (w, SH101, "sh101_cutoff", ramp (0.45f, 0.75f, 32) + ramp (0.75f, 0.45f, 32));

    // E - break: no kick, the bleeps and the claves in the echo, the reverse cymbal.
    w.part (4);
    drums (w, TR909, "rs", rims);
    drums (w, Percussion, "cl", ".....x.........x..........x.....");
    line (w, SH101, bleeps);
    stepValues (w, SH101, "sh101_echo", hold (0.85f, 64));
    drums (w, SoundFx, "swp", bars ({ "x...............", none, none, none }));
    drums (w, SoundFx, "rev", bars ({ none, none, none, "........x......." }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 3 }, { 3, 3 }, { 4, 1 }, { 3, 2 }, { 2, 1 }, { 1, 1 } });
}

void hardTechno (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 150); p ("swing", 0.0f); p ("key", 4); p ("scale", 6); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 1); p ("echoFeedback", 0.35f); p ("echoLevel", 0.45f);
    p ("reverbType", 1); p ("reverbSize", 0.75f); p ("reverbLevel", 0.4f);
    p ("pumpMode", 1); p ("pump", 0.4f); p ("comp", 0.5f); p ("disto", 0.15f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.55f); p ("tr909_bd_tone", 0.5f); p ("tr909_bd_drive", 0.75f); p ("tr909_bd_reverb", 0.2f);
    p ("tr909_ch_level", 0.35f); p ("tr909_ch_decay", 0.12f); p ("tr909_oh_level", 0.42f); p ("tr909_oh_decay", 0.3f);
    p ("tr909_cp_level", 0.45f); p ("tr909_cp_reverb", 0.35f); p ("tr909_cp_drive", 0.3f); p ("tr909_rd_level", 0.3f);
    p ("tr909_sd_level", 0.45f); p ("tr909_cr_level", 0.36f);

    w.show (ModelD);
    p ("modeld_wave", 0); p ("modeld_sub", 0.7f); p ("modeld_cutoff", 0.24f); p ("modeld_emphasis", 0.35f); p ("modeld_contour", 0.4f);
    p ("modeld_decay", 0.12f); p ("modeld_sustain", 0.0f); p ("modeld_overdrive", 0.6f); p ("modeld_volume", 0.75f);

    w.show (JP8000);
    p ("jp8000_chord", 0); p ("jp8000_detune", 0.35f); p ("jp8000_mix", 0.5f); p ("jp8000_cutoff", 0.55f); p ("jp8000_reso", 0.3f);
    p ("jp8000_env", 0.5f); p ("jp8000_attack", 0.0f); p ("jp8000_decay", 0.28f); p ("jp8000_sustain", 0.12f); p ("jp8000_release", 0.2f);
    p ("jp8000_drive", 0.3f); p ("jp8000_echo", 0.35f); p ("jp8000_reverb", 0.3f); p ("jp8000_volume", 0.95f);

    w.show (SoundFx);
    p ("fx_rise_decay", 0.62f); p ("fx_rise_level", 0.45f); p ("fx_imp_level", 0.6f); p ("fx_rev_decay", 0.35f); p ("fx_rev_level", 0.4f);

    // E Phrygian. A distorted kick, a sub that answers it, stabs in fifths that climb by a half step (F).
    const juce::String none = "................", kick = "X...x...x...x...";
    const juce::String rumble = ". . E1 E1 . . E1 E1 . . E1 E1 . . E1 E1 ";
    const juce::String bass = bars ({ rumble, rumble, rumble, ". . E1 E1 . . E1 E1 . . F1 F1 . . F1 F1 " });
    auto stab = [] (const juce::String& a, const juce::String& b)
    { return ". . " + a + " . . . " + a + " . . . " + a + " . . " + b + " . . "; };
    const juce::String stabs = bars ({ stab ("E4+B4", "E4+B4"), stab ("E4+B4", "F4+C5"), stab ("E4+B4", "E4+B4"), stab ("F4+C5", "G4+D5") });

    auto beat = [&] (const juce::String& kicks)
    {
        drums (w, TR909, "bd", kicks);
        drums (w, TR909, "oh", "..x...x...x...x.");
        drums (w, TR909, "ch", "xxxxxxxxxxxxxxxx");
        stepValues (w, TR909, "tr909_ch_level", "0.2 0.3 0.1 0.3 ");
        drums (w, TR909, "cp", "....x.......x...");
    };

    // A - intro: the kick alone, distorted.
    w.part (0);
    drums (w, TR909, "bd", kick);
    drums (w, TR909, "ch", "..x...x...x...x.");

    // B - groove: hats, clap and the sub.
    w.part (1);
    beat (kick);
    line (w, ModelD, bass);

    // C - the stabs come in, filtered, then open.
    w.part (2);
    beat (kick);
    line (w, ModelD, bass);
    chords (w, JP8000, stabs);
    stepValues (w, JP8000, "jp8000_cutoff", ramp (0.3f, 0.6f));

    // D - build: no kick, snare roll and riser, the stabs open up.
    w.part (3);
    drums (w, TR909, "oh", "..x...x...x...x.");
    drums (w, TR909, "sd", bars ({ "x...x...x...x...", "x.x.x.x.x.x.x.x.", "xxxxxxxxxxxxxxxx", "2222222244444444" }));
    stepValues (w, TR909, "tr909_sd_level", ramp (0.15f, 0.6f));
    chords (w, JP8000, stabs);
    stepValues (w, JP8000, "jp8000_cutoff", ramp (0.35f, 0.8f));
    drums (w, SoundFx, "rise", bars ({ none, "x...............", none, none }));

    // E - drop: impact, everything at full force, ride.
    w.part (4);
    beat (kick);
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    drums (w, TR909, "cr", "x...............................................................");
    drums (w, SoundFx, "imp", "x...............................................................");
    line (w, ModelD, bass);
    chords (w, JP8000, stabs);
    stepValues (w, JP8000, "jp8000_cutoff", hold (0.68f, 64));

    // F - end of the drop: the kick doubles before the break, reverse cymbal.
    w.part (5);
    beat (bars ({ kick, kick, kick, "x...x...x.x.xxxx" }));
    drums (w, TR909, "rd", "x.x.x.x.x.x.x.x.");
    line (w, ModelD, bass);
    chords (w, JP8000, stabs);
    stepValues (w, JP8000, "jp8000_cutoff", hold (0.68f, 64));
    drums (w, SoundFx, "rev", bars ({ none, none, none, "........x......." }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 1 }, { 4, 3 }, { 5, 1 }, { 2, 1 }, { 3, 1 }, { 4, 2 }, { 5, 1 }, { 1, 1 } });
}

void ukGarage (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 132); p ("swing", 0.55f); p ("key", 7); p ("scale", 5); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("echoLevel", 0.5f);
    p ("reverbType", 2); p ("reverbSize", 0.55f); p ("reverbLevel", 0.4f);
    p ("pumpMode", 1); p ("pump", 0.2f); p ("comp", 0.35f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.5f); p ("tr909_bd_tone", 0.45f); p ("tr909_sd_level", 0.42f); p ("tr909_sd_tone", 0.6f);
    p ("tr909_cp_level", 0.42f); p ("tr909_cp_reverb", 0.3f); p ("tr909_ch_level", 0.34f); p ("tr909_ch_decay", 0.15f);
    p ("tr909_oh_level", 0.3f); p ("tr909_rs_level", 0.3f);

    w.show (Percussion);
    p ("perc_sh_level", 0.25f); p ("perc_tb_level", 0.22f); p ("perc_cgh_level", 0.25f);

    w.show (ModelD);
    p ("modeld_wave", 1); p ("modeld_sub", 0.55f); p ("modeld_cutoff", 0.32f); p ("modeld_emphasis", 0.3f); p ("modeld_contour", 0.35f);
    p ("modeld_decay", 0.35f); p ("modeld_sustain", 0.4f); p ("modeld_glide", 0.35f); p ("modeld_overdrive", 0.25f); p ("modeld_volume", 0.85f);

    w.show (Organ);
    p ("organ_chord", 0); p ("organ_type", 1); p ("organ_percussion", 0.4f); p ("organ_leslie", 0.3f); p ("organ_click", 0.4f);
    p ("organ_reverb", 0.25f); p ("organ_volume", 0.7f);

    w.show (Choir);
    p ("choir_chord", 0); p ("choir_vowel", 1); p ("choir_attack", 0.4f); p ("choir_release", 0.55f); p ("choir_ensemble", 0.6f);
    p ("choir_reverb", 0.45f); p ("choir_echo", 0.25f); p ("choir_volume", 0.75f);

    // G Dorian: Gm9 and C9 on the organ, a bass that glides between octaves, the 2-step kick that skips the third beat.
    const juce::String none = "................";
    const juce::String kicks = bars ({ "x.........x.....", "x.......x.x.....", "x.........x.....", "x.......x.....x." });
    const juce::String gm9 = "A#3+D4+F4+A4", c9 = "A#3+D4+E4+G4";
    auto stab = [] (const juce::String& c) { return ". . . " + c + " . . " + c + " . . . . " + c + " - . . . "; };
    const juce::String organ = bars ({ stab (gm9), stab (gm9), stab (c9), stab (c9) });
    const juce::String bass = bars ({ "G1 . . . . . G2~ G1 . . A#1 . . . C2~ D2 ", "G1 . . . . . G2~ G1 . . F1 . . . G1 . ",
                                      "C2 . . . . . C3~ C2 . . A#1 . . . G1~ A#1 ", "C2 . . . . . D2~ C2 . . A#1 . . . A1~ F1 " });
    const juce::String voices = bars ({ ". . . . . . . . D4+F4+A4 - - - - - - - ", ". . . . . . . . D4+F4+A4 - - - . . . . ",
                                        ". . . . . . . . E4+G4+A#4 - - - - - - - ", ". . . . . . . . D4+G4+A#4 - - - . . . . " });

    auto beat = [&] (bool full)
    {
        drums (w, TR909, "bd", kicks);
        drums (w, TR909, "sd", "....x.......x...");
        drums (w, TR909, "ch", "..x...x...x...x.");
        drums (w, Percussion, "sh", "x.xxx.xxx.xxx.xx");
        stepValues (w, Percussion, "perc_sh_level", "0.25 0.12 0.18 0.12 ");
        if (! full) return;
        drums (w, TR909, "cp", "....x.......x...");
        drums (w, TR909, "oh", "......x.......x.");
        drums (w, TR909, "rs", ".x.....x..x.....");
    };

    // A - intro: the organ and the shaker.
    w.part (0);
    drums (w, Percussion, "sh", "x.xxx.xxx.xxx.xx");
    stepValues (w, Percussion, "perc_sh_level", "0.25 0.12 0.18 0.12 ");
    drums (w, TR909, "ch", "..x...x...x...x.");
    chords (w, Organ, organ);

    // B - the 2-step comes in with the bass.
    w.part (1);
    beat (false);
    line (w, ModelD, bass);
    chords (w, Organ, organ);

    // C - hook: the choir answers "ooh", clap and open hats.
    w.part (2);
    beat (true);
    line (w, ModelD, bass);
    chords (w, Organ, organ);
    chords (w, Choir, voices);

    // D - break: bass and choir alone, the organ in the echo.
    w.part (3);
    drums (w, Percussion, "sh", "x.xxx.xxx.xxx.xx");
    stepValues (w, Percussion, "perc_sh_level", "0.18 0.08 0.12 0.08 ");
    line (w, ModelD, bass);
    chords (w, Choir, voices);
    chords (w, Organ, bars ({ stab (gm9), rest16, stab (c9), rest16 }));
    stepValues (w, Organ, "organ_echo", hold (0.6f, 64));
    drums (w, TR909, "sd", bars ({ none, none, none, "........x.x.xxxx" }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 3 }, { 3, 1 }, { 1, 1 }, { 2, 3 }, { 0, 1 } });
}

void afroHouse (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 122); p ("swing", 0.15f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 2); p ("echoFeedback", 0.4f); p ("echoLevel", 0.5f);
    p ("reverbType", 1); p ("reverbSize", 0.65f); p ("reverbLevel", 0.4f);
    p ("pumpMode", 1); p ("pump", 0.2f); p ("comp", 0.3f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.6f); p ("tr909_bd_tone", 0.35f); p ("tr909_ch_level", 0.3f); p ("tr909_cp_level", 0.38f); p ("tr909_cp_reverb", 0.35f);
    p ("tr909_rs_level", 0.3f); p ("tr909_oh_level", 0.25f);

    w.show (Percussion);
    p ("perc_cgl_level", 0.38f); p ("perc_cgh_level", 0.34f); p ("perc_ag_level", 0.26f); p ("perc_ag_echo", 0.3f);
    p ("perc_sh_level", 0.24f); p ("perc_gu_level", 0.2f);

    w.show (WorldPerc);
    p ("world_tlk_level", 0.34f); p ("world_tlk_reverb", 0.3f); p ("world_djb_level", 0.36f); p ("world_djt_level", 0.3f); p ("world_djs_level", 0.28f);

    w.show (ModelD);
    p ("modeld_wave", 2); p ("modeld_sub", 0.7f); p ("modeld_cutoff", 0.3f); p ("modeld_emphasis", 0.15f); p ("modeld_contour", 0.3f);
    p ("modeld_decay", 0.3f); p ("modeld_sustain", 0.3f); p ("modeld_overdrive", 0.2f); p ("modeld_volume", 0.9f);

    w.show (Mallets);
    p ("mallets_chord", 0); p ("mallets_type", 4); p ("mallets_brightness", 0.55f); p ("mallets_decay", 0.5f); p ("mallets_mallet", 0.5f);
    p ("mallets_echo", 0.45f); p ("mallets_reverb", 0.3f); p ("mallets_volume", 0.85f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_attack", 0.8f); p ("strings_release", 0.8f); p ("strings_brightness", 0.35f);
    p ("strings_reverb", 0.55f); p ("strings_volume", 0.45f);

    // A minor pentatonic. The tresillo (3 + 3 + 2) in the bass, the kalimba plays a looping ostinato, drums talk.
    const juce::String none = "................", kick = "x...x...x...x...";
    const juce::String tresillo = bars ({ "A1 . . A1 . . A1 . . . C2 . . . G1 . ", "A1 . . A1 . . A1 . . . E2 . . . D2 . ",
                                          "F1 . . F1 . . F1 . . . A1 . . . G1 . ", "G1 . . G1 . . G1 . . . B1 . . . E1 . " });
    const juce::String kalimba = bars ({ "A4 . C5 E5 . A4 G4 . E4 . G4 A4 . C5 . . ", "A4 . C5 E5 . A4 G4 . E4 . D4 E4 . G4 . . ",
                                         "A4 . C5 F5 . A4 G4 . F4 . G4 A4 . C5 . . ", "B4 . D5 G5 . B4 A4 . G4 . E4 D4 . E4 . . " });
    const juce::String pads = bars ({ held ("A3+C4+E4", 16), held ("A3+C4+E4", 16), held ("F3+A3+C4", 16), held ("G3+B3+D4", 16) });

    auto percs = [&] (bool full)
    {
        drums (w, Percussion, "cgl", "x.....x...x.....x.....x.....x.x.");
        drums (w, Percussion, "cgh", "...x.x.....x.x.....x.x.....x..x.");
        drums (w, Percussion, "sh", "xxxxxxxxxxxxxxxx");
        stepValues (w, Percussion, "perc_sh_level", "0.1 0.18 0.26 0.18 ");
        if (! full) return;
        drums (w, Percussion, "ag", "x.x..x.x..x.x.x.");
        drums (w, WorldPerc, "tlk", bars ({ none, "..........x.x...", none, "..........x..x.x" }));
        drums (w, WorldPerc, "djb", "x.......x.......");
        drums (w, WorldPerc, "djs", ".......x.......x");
    };
    auto beat = [&]
    {
        drums (w, TR909, "bd", kick);
        drums (w, TR909, "cp", "....x.......x...");
        drums (w, TR909, "ch", "..x...x...x...x.");
    };

    // A - intro: congas and shaker, the kalimba alone.
    w.part (0);
    percs (false);
    chords (w, Mallets, kalimba);

    // B - the kick and the tresillo bass.
    w.part (1);
    beat();
    percs (false);
    line (w, ModelD, tresillo);
    chords (w, Mallets, kalimba);

    // C - everything: agogo, djembe and the talking drum.
    w.part (2);
    beat();
    percs (true);
    line (w, ModelD, tresillo);
    chords (w, Mallets, kalimba);

    // D - break: no kick, strings, the talking drum answers the kalimba.
    w.part (3);
    percs (false);
    drums (w, WorldPerc, "tlk", bars ({ "..........x.x...", "......x...x.x...", "..........x.x...", "..x...x...x..x.x" }));
    chords (w, Strings, pads);
    chords (w, Mallets, kalimba);
    stepValues (w, Mallets, "mallets_echo", hold (0.7f, 64));

    // E - return with the strings.
    w.part (4);
    beat();
    percs (true);
    drums (w, TR909, "oh", "..x...x...x...x.");
    line (w, ModelD, tresillo);
    chords (w, Mallets, kalimba);
    chords (w, Strings, pads);

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 3 }, { 3, 2 }, { 4, 3 }, { 1, 1 }, { 0, 1 } });
}
} // namespace techno::demos
