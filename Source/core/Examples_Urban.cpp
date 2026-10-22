#include "ExampleTools.h"

// Hip-hop, R&B and Latin demos.
namespace techno::demos
{
void boomBap (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 90); p ("swing", 0.35f); p ("key", 2); p ("scale", 5); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 3); p ("echoFeedback", 0.3f); p ("echoLevel", 0.4f);
    p ("reverbType", 0); p ("reverbSize", 0.45f); p ("reverbLevel", 0.3f);
    p ("comp", 0.4f);

    w.show (MPC);
    p ("mpc_bd_level", 0.65f); p ("mpc_bd_decay", 0.5f); p ("mpc_sd_level", 0.58f); p ("mpc_sd_reverb", 0.12f);
    p ("mpc_ch_level", 0.38f); p ("mpc_oh_level", 0.3f); p ("mpc_rs_level", 0.3f); p ("mpc_sh_level", 0.22f); p ("mpc_crush", 0.2f);

    w.show (DX7);
    p ("dx7_chord", 0); p ("dx7_preset", 0); p ("dx7_brightness", 0.4f); p ("dx7_decay", 0.6f); p ("dx7_release", 0.4f);
    p ("dx7_tremolo", 0.3f); p ("dx7_reverb", 0.3f); p ("dx7_volume", 0.75f);

    w.show (EBass);
    p ("ebass_type", 0); p ("ebass_tone", 0.35f); p ("ebass_decay", 0.55f); p ("ebass_muting", 0.35f); p ("ebass_volume", 0.9f);

    w.show (SoundFx);
    p ("fx_vin_decay", 1.0f); p ("fx_vin_level", 0.25f); p ("fx_scr_level", 0.42f); p ("fx_scr_tone", 0.5f);

    // D Dorian, jazz chords on the FM electric piano: Dm9, G13, Fmaj7, Em7. A lazy kick, a snare that cracks, scratches on the hook.
    const juce::String none = "................";
    const juce::String dm9 = "F3+A3+C4+E4", g13 = "F3+B3+E4", fmaj7 = "E3+A3+C4", em7 = "D3+G3+B3";
    auto keys = [] (const juce::String& c) { return c + " - - - - - . . . . " + c + " - . . . . "; };
    const juce::String ep = bars ({ keys (dm9), keys (g13), keys (fmaj7), keys (em7) });
    const juce::String kicks = bars ({ "x.......x.x.....", "x.x.......x.....", "x.......x.x.....", "x.x.......x..x.." });
    const juce::String bass = bars ({ "D2 - - - . . . . . . D2 . A1 . . . ", "G1 - - - . . . . . . G1 . B1 . . . ",
                                      "F1 - - - . . . . . . F1 . C2 . . . ", "E1 - - - . . . . . . E1 . G1 . A1 . " });
    auto beat = [&] (bool open)
    {
        drums (w, MPC, "bd", kicks);
        drums (w, MPC, "sd", "....x.......x...");
        drums (w, MPC, "ch", "x.x.x.x.x.x.x.x.");
        stepValues (w, MPC, "mpc_ch_level", "0.4 . 0.22 . 0.32 . 0.22 . ");
        if (open) drums (w, MPC, "oh", "..............x.");
    };
    auto crackle = [&] { drums (w, SoundFx, "vin", "x..............................................................."); };

    // A - intro: the crackle, the chords alone.
    w.part (0);
    crackle();
    chords (w, DX7, ep);

    // B - verse: the beat and the bass.
    w.part (1);
    crackle();
    beat (false);
    line (w, EBass, bass);
    chords (w, DX7, ep);

    // C - hook: open hat, rimshot, the DJ cuts on the end of each phrase.
    w.part (2);
    crackle();
    beat (true);
    drums (w, MPC, "rs", "...........x....");
    line (w, EBass, bass);
    chords (w, DX7, ep);
    drums (w, SoundFx, "scr", bars ({ none, "............x.x.", none, "........x...x.x." }));

    // D - break: the drums drop, bass and keys only, the snare comes back at the end.
    w.part (3);
    crackle();
    line (w, EBass, bass);
    chords (w, DX7, ep);
    drums (w, MPC, "sd", bars ({ none, none, none, "....x.......x.xx" }));
    drums (w, MPC, "sh", "..x...x...x...x.");

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 2 }, { 1, 2 }, { 3, 1 }, { 2, 2 }, { 0, 1 } });
}

void trap (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 140); p ("swing", 0.0f); p ("key", 4); p ("scale", 3); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("echoLevel", 0.45f);
    p ("reverbType", 1); p ("reverbSize", 0.7f); p ("reverbLevel", 0.4f);
    p ("comp", 0.4f);

    w.show (TR808);
    p ("tr808_bd_level", 0.55f); p ("tr808_bd_decay", 0.4f); p ("tr808_cp_level", 0.5f); p ("tr808_cp_reverb", 0.3f);
    p ("tr808_sd_level", 0.4f); p ("tr808_ch_level", 0.36f); p ("tr808_ch_decay", 0.15f); p ("tr808_oh_level", 0.3f);

    w.show (Sub808);
    p ("sub808_decay", 0.7f); p ("sub808_glide", 0.4f); p ("sub808_punch", 0.5f); p ("sub808_overdrive", 0.4f); p ("sub808_tone", 0.55f);
    p ("sub808_volume", 0.58f);

    w.show (DX7);
    p ("dx7_chord", 0); p ("dx7_preset", 2); p ("dx7_brightness", 0.55f); p ("dx7_decay", 0.5f); p ("dx7_release", 0.45f);
    p ("dx7_echo", 0.4f); p ("dx7_reverb", 0.35f); p ("dx7_volume", 0.7f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_attack", 0.6f); p ("strings_release", 0.7f); p ("strings_brightness", 0.35f);
    p ("strings_reverb", 0.5f); p ("strings_volume", 0.5f);

    // E harmonic minor: Em - C - Am - B. Half-time: the clap on the third beat, hats that roll, an 808 that slides.
    const juce::String none = "................";
    const juce::String kicks = bars ({ "x......x..x.....", "x.........x..x..", "x......x..x.....", "x.....x...x.x..." });
    const juce::String hats = bars ({ "x.x.x.x.x.x.x3x.", "x.x.x.x.x.x.4.x.", "x.x.x.x.x3x.x.x.", "x.x.x.x.x.x.4444" });
    const juce::String bass = bars ({ "E1 - - - - - - B1~ E1 - . . . . . . ", "C2 - - - - - - - . . C2 - - G1~ C2 - ",
                                      "A1 - - - - - - E2~ A1 - . . . . . . ", "B1 - - - - - - - . . B1 - D#2~ B1 - - " });
    const juce::String bell = bars ({ "E5 . . G5 . . B5 . A5 . G5 . F#5 . . . ", "E5 . . G5 . . C6 . B5 . G5 . E5 . . . ",
                                      "E5 . . G5 . . A5 . C6 . B5 . A5 . . . ", "B5 . . A5 . . G5 . F#5 . D#5 . B4 . . . " });
    const juce::String pads = bars ({ held ("E3+G3+B3", 16), held ("E3+G3+C4", 16), held ("E3+A3+C4", 16), held ("D#3+F#3+B3", 16) });

    auto beat = [&] (const juce::String& hatLine)
    {
        drums (w, TR808, "bd", kicks);
        drums (w, TR808, "cp", "........x.......");
        drums (w, TR808, "ch", hatLine);
    };

    // A - intro: the bell and the strings.
    w.part (0);
    chords (w, DX7, bell);
    chords (w, Strings, pads);

    // B - hook: everything comes in, the 808 slides.
    w.part (1);
    beat (hats);
    drums (w, TR808, "oh", "..............x.");
    line (w, Sub808, bass);
    chords (w, DX7, bell);
    chords (w, Strings, pads);

    // C - verse: the bell steps out, the hats roll more.
    w.part (2);
    beat (bars ({ "x.x.x.x.x3x.x.x.", "x.x.4.x.x.x.x3x.", "x.x.x.x.x3x.x.x.", "x.x.x3x.x.4.4444" }));
    drums (w, TR808, "sd", bars ({ none, "..............x.", none, "...........x..x." }));
    line (w, Sub808, bass);
    chords (w, Strings, pads);

    // D - break: the strings and the bell, the 808 holds, no drums.
    w.part (3);
    chords (w, Strings, pads);
    chords (w, DX7, bell);
    stepValues (w, DX7, "dx7_echo", hold (0.7f, 64));
    line (w, Sub808, bars ({ held ("E1", 16), held ("C2", 16), held ("A1", 16), held ("B1", 16) }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 2 }, { 1, 2 }, { 3, 1 }, { 2, 2 }, { 1, 2 }, { 0, 1 } });
}

void lofi (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 80); p ("swing", 0.32f); p ("key", 5); p ("scale", 0); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 3); p ("echoFeedback", 0.3f); p ("echoLevel", 0.45f);
    p ("reverbType", 0); p ("reverbSize", 0.5f); p ("reverbLevel", 0.35f);
    p ("comp", 0.35f); p ("djFilter", 0.42f);

    w.show (MPC);
    p ("mpc_bd_level", 0.6f); p ("mpc_bd_decay", 0.45f); p ("mpc_sd_level", 0.5f); p ("mpc_sd_reverb", 0.15f);
    p ("mpc_ch_level", 0.4f); p ("mpc_oh_level", 0.3f); p ("mpc_rs_level", 0.35f); p ("mpc_sh_level", 0.25f);
    p ("mpc_crush", 0.25f); p ("mpc_filter", 0.42f);

    w.show (EBass);
    p ("ebass_type", 4); p ("ebass_tone", 0.35f); p ("ebass_decay", 0.6f); p ("ebass_muting", 0.25f); p ("ebass_volume", 0.9f);

    w.show (EPiano);
    p ("epiano_chord", 0); p ("epiano_type", 0); p ("epiano_tone", 0.38f); p ("epiano_decay", 0.65f); p ("epiano_release", 0.45f);
    p ("epiano_tremolo", 0.45f); p ("epiano_bark", 0.15f); p ("epiano_reverb", 0.35f); p ("epiano_crush", 0.15f); p ("epiano_volume", 0.75f);

    w.show (Pluck);
    p ("pluck_chord", 0); p ("pluck_type", 1); p ("pluck_brightness", 0.5f); p ("pluck_decay", 0.55f); p ("pluck_release", 0.4f);
    p ("pluck_body", 0.6f); p ("pluck_echo", 0.35f); p ("pluck_reverb", 0.3f); p ("pluck_volume", 1.0f);

    w.show (Mellotron);
    p ("mellotron_chord", 0); p ("mellotron_type", 0); p ("mellotron_tone", 0.4f); p ("mellotron_attack", 0.4f);
    p ("mellotron_release", 0.5f); p ("mellotron_wow", 0.5f); p ("mellotron_reverb", 0.4f); p ("mellotron_volume", 0.55f);

    w.show (SoundFx);
    p ("fx_vin_decay", 1.0f); p ("fx_vin_level", 0.3f); p ("fx_rain_decay", 1.0f); p ("fx_rain_level", 0.25f);

    // F major, ii - V - I - vi: Gm9, C13, Fmaj9, Dm9. The bass walks into each chord by a half step.
    const juce::String none = "................";
    const juce::String gm9 = "F3+A3+A#3+D4", c13 = "E3+A3+A#3+D4", fmaj9 = "E3+G3+A3+C4", dm9 = "F3+A3+C4+E4";
    auto keys = [] (const juce::String& c) { return c + " - - - - - - . . . " + c + " - - - - . "; };
    const juce::String bass = bars ({ "G1 - - - - - . . . . D2 - - . C#2 . ", "C2 - - - - - . . . . G1 - - . E1 . ",
                                      "F1 - - - - - . . . . C2 - - . D#2 . ", "D2 - - - - - . . . . A1 - - . F#1 . " });
    const juce::String melody = bars ({ ". . . . A4 . C5 . D5 - - . C5 . A4 . ", "G4 - - - . . . . . . E4 . G4 . A4 . ",
                                        "C5 - - . A4 . G4 . E4 - - - . . . . ", ". . D4 . F4 . A4 . C5 - - - A4 - - . " });
    const juce::String kicks = bars ({ "x......x..x.....", "x.........x..x..", "x......x..x.....", "x.........x....." });
    const juce::String snares = "....x.......x...", hats = "x.x.x.x.x.x.x.x.";

    auto beat = [&] (bool shaker)
    {
        drums (w, MPC, "bd", kicks);
        drums (w, MPC, "sd", snares);
        drums (w, MPC, "ch", hats);
        stepValues (w, MPC, "mpc_ch_level", "0.42 . 0.22 . 0.32 . 0.22 . ");
        drums (w, MPC, "rs", bars ({ none, none, none, "...............x" }));
        if (shaker) drums (w, MPC, "sh", "..x...x...x...x.");
    };
    auto vinyl = [&] { drums (w, SoundFx, "vin", "x..............................................................."); };
    auto progression = [&] { chords (w, EPiano, bars ({ keys (gm9), keys (c13), keys (fmaj9), keys (dm9) })); };

    // A - intro: the rain, the crackle and the Rhodes behind a filter.
    w.part (0);
    vinyl();
    drums (w, SoundFx, "rain", "x...............................................................");
    progression();
    stepValues (w, EPiano, "epiano_filter", ramp (0.2f, 0.45f));

    // B - verse: the beat and the upright bass.
    w.part (1);
    vinyl();
    beat (false);
    line (w, EBass, bass);
    progression();

    // C - hook: the nylon guitar plays the tune.
    w.part (2);
    vinyl();
    beat (false);
    line (w, EBass, bass);
    progression();
    chords (w, Pluck, melody);
    stepValues (w, EPiano, "epiano_volume", hold (0.55f, 64));   // the Rhodes steps back under the tune

    // D - break: half the beat, flutes of the Mellotron, the bass holds.
    w.part (3);
    vinyl();
    drums (w, MPC, "ch", hats);
    stepValues (w, MPC, "mpc_ch_level", "0.3 . 0.16 . 0.24 . 0.16 . ");
    drums (w, MPC, "rs", "....x.......x...");
    line (w, EBass, bars ({ held ("G1", 16), held ("C2", 16), held ("F1", 16), held ("D2", 12) + ". . A1 . " }));
    chords (w, Mellotron, bars ({ held ("A#3+D4+F4", 16), held ("A#3+E4+G4", 16), held ("A3+C4+E4", 16), held ("A3+C4+F4", 16) }));
    chords (w, EPiano, bars ({ held (gm9, 6) + rest16.substring (0, 20), held (c13, 6) + rest16.substring (0, 20),
                               held (fmaj9, 6) + rest16.substring (0, 20), held (dm9, 6) + rest16.substring (0, 20) }));
    chords (w, Pluck, bars ({ rest16, rest16, ". . . . . . . . C5 - - . A4 . G4 . ", "F4 - - - - - - - . . . . . . . . " }));

    // E - last hook: shaker and flutes under the guitar.
    w.part (4);
    vinyl();
    beat (true);
    line (w, EBass, bass);
    progression();
    chords (w, Pluck, melody);
    stepValues (w, EPiano, "epiano_volume", hold (0.55f, 64));   // the Rhodes steps back under the tune
    chords (w, Mellotron, bars ({ held ("A#3+D4+F4", 16), held ("A#3+E4+G4", 16), held ("A3+C4+E4", 16), held ("A3+C4+F4", 16) }));
    stepValues (w, Mellotron, "mellotron_volume", hold (0.35f, 64));

    // F - outro: the beat stops, the rain comes back, one last chord.
    w.part (5);
    vinyl();
    drums (w, SoundFx, "rain", "x...............................................................");
    chords (w, EPiano, bars ({ keys (gm9), keys (c13), held (fmaj9, 16), rest16 }));
    line (w, EBass, bars ({ "G1 - - - - - - - - - - - - - - - ", "C2 - - - - - - - - - - - - - - - ", "F1 - - - - - - - - - - - - . . . ", rest16 }));
    stepValues (w, EPiano, "epiano_filter", ramp (0.45f, 0.25f));

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 2 }, { 1, 1 }, { 3, 1 }, { 4, 2 }, { 5, 1 } });
}

void drill (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 142); p ("swing", 0.05f); p ("key", 2); p ("scale", 3); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("echoLevel", 0.4f);
    p ("reverbType", 1); p ("reverbSize", 0.8f); p ("reverbLevel", 0.45f);
    p ("comp", 0.4f);

    w.show (TR808);
    p ("tr808_bd_level", 0.6f); p ("tr808_bd_decay", 0.35f); p ("tr808_sd_level", 0.62f); p ("tr808_sd_tone", 0.65f);
    p ("tr808_cp_level", 0.4f); p ("tr808_ch_level", 0.36f); p ("tr808_ch_decay", 0.12f); p ("tr808_oh_level", 0.28f);

    w.show (Sub808);
    p ("sub808_decay", 0.75f); p ("sub808_glide", 0.55f); p ("sub808_punch", 0.45f); p ("sub808_overdrive", 0.45f); p ("sub808_tone", 0.5f);
    p ("sub808_volume", 0.58f);

    w.show (Piano);
    p ("piano_chord", 0); p ("piano_type", 4); p ("piano_brightness", 0.35f); p ("piano_decay", 0.6f); p ("piano_release", 0.5f);
    p ("piano_reverb", 0.45f); p ("piano_echo", 0.25f); p ("piano_volume", 0.8f);

    w.show (Choir);
    p ("choir_chord", 0); p ("choir_vowel", 3); p ("choir_attack", 0.6f); p ("choir_release", 0.7f); p ("choir_ensemble", 0.7f);
    p ("choir_brightness", 0.35f); p ("choir_reverb", 0.6f); p ("choir_volume", 0.5f);

    // D harmonic minor: Dm - Bb - Gm - A, the C# that makes it dark. Snares on the third beat and pushed late, hats in triplets.
    const juce::String none = "................";
    const juce::String kicks = bars ({ "x.....x.........", "..x.......x.....", "x.....x.........", "..x.....x.....x." });
    const juce::String snares = bars ({ "........x.......", "........x.....x.", "........x.......", "........x..x...." });
    const juce::String hats = bars ({ "x..x3.x..x.x3..x", "x..x.x3.x..x.x3.", "x..x3.x..x.x3..x", "x..x.x3.x.3.3.33" });
    const juce::String bass = bars ({ "D2 - - - - - G1~ D2 - - . . . . . . ", ". . A#1 - - - - - - - D2~ A#1 - . . . ",
                                      "G1 - - - - - D2~ G1 - - . . . . . . ", ". . A1 - - - - - C#2~ A1 - - - . . . " });
    const juce::String melody = bars ({ "D5 . . F5 . . A5 . . . G5 . F5 . E5 . ", "D5 . . . F5 . . . A#4 . . . . . . . ",
                                        "D5 . . F5 . . G5 . . . F5 . E5 . D5 . ", "C#5 . . . E5 . . . A4 . . . . . . . " });
    const juce::String keys = bars ({ held ("D3+F3+A3", 16), held ("D3+F3+A#3", 16), held ("D3+G3+A#3", 16), held ("C#3+E3+A3", 16) });
    const juce::String monks = bars ({ held ("D3+A3", 32), held ("D3+G3", 16), held ("C#3+A3", 16) });

    auto beat = [&]
    {
        drums (w, TR808, "bd", kicks);
        drums (w, TR808, "sd", snares);
        drums (w, TR808, "ch", hats);
    };

    // A - intro: the monks and the piano.
    w.part (0);
    chords (w, Choir, monks);
    chords (w, Piano, melody);

    // B - the beat and the sliding 808.
    w.part (1);
    beat();
    line (w, Sub808, bass);
    chords (w, Piano, melody);

    // C - full: piano chords under the melody, the choir, the clap doubles the snare.
    w.part (2);
    beat();
    drums (w, TR808, "cp", snares);
    line (w, Sub808, bass);
    chords (w, Piano, melody);
    chords (w, Choir, monks);

    // D - break: the 808 alone with the choir, then a snare roll.
    w.part (3);
    chords (w, Choir, monks);
    chords (w, Piano, keys);
    stepValues (w, Piano, "piano_volume", hold (0.55f, 64));
    line (w, Sub808, bass);
    drums (w, TR808, "sd", bars ({ none, none, none, "........x.x.3333" }));

    w.song ({ { 0, 2 }, { 1, 3 }, { 2, 4 }, { 3, 2 }, { 2, 4 }, { 1, 1 }, { 0, 1 } });
}

void rnb (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 90); p ("swing", 0.2f); p ("key", 5); p ("scale", 5); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 3); p ("echoFeedback", 0.3f); p ("echoLevel", 0.4f);
    p ("reverbType", 2); p ("reverbSize", 0.55f); p ("reverbLevel", 0.4f);
    p ("comp", 0.35f);

    w.show (LinnDrum);
    p ("linn_bd_level", 0.6f); p ("linn_sd_level", 0.45f); p ("linn_sd_reverb", 0.3f); p ("linn_ss_level", 0.3f); p ("linn_cp_level", 0.38f);
    p ("linn_ch_level", 0.34f); p ("linn_oh_level", 0.26f); p ("linn_cg_level", 0.28f); p ("linn_tb_level", 0.22f);

    w.show (Sub808);
    p ("sub808_decay", 0.6f); p ("sub808_glide", 0.3f); p ("sub808_punch", 0.2f); p ("sub808_overdrive", 0.1f); p ("sub808_tone", 0.4f);
    p ("sub808_volume", 0.7f);

    w.show (EPiano);
    p ("epiano_chord", 0); p ("epiano_type", 0); p ("epiano_tone", 0.42f); p ("epiano_decay", 0.65f); p ("epiano_release", 0.45f);
    p ("epiano_tremolo", 0.4f); p ("epiano_bark", 0.2f); p ("epiano_reverb", 0.3f); p ("epiano_volume", 0.75f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_attack", 0.55f); p ("strings_release", 0.7f); p ("strings_brightness", 0.45f);
    p ("strings_reverb", 0.5f); p ("strings_volume", 0.55f);

    // F Dorian: Fm9 - Bb9 - Fm9 - Cm7 on the Rhodes. Slow groove, hats that roll at the end of the bar, strings that sing a line.
    const juce::String none = "................";
    const juce::String fm9 = "G#3+C4+D#4+G4", bb9 = "G#3+C4+D4+F4", cm7 = "G3+A#3+D#4";
    auto keys = [] (const juce::String& c) { return c + " - - - - - - . . . " + c + " - - . . . "; };
    const juce::String ep = bars ({ keys (fm9), keys (bb9), keys (fm9), keys (cm7) });
    const juce::String kicks = bars ({ "x......x..x.....", "x.........x.....", "x......x..x.....", "x.........x...x." });
    const juce::String hats = bars ({ "x.x.x.x.x.x.x.x.", "x.x.x.x.x.x.x3x.", "x.x.x.x.x.x.x.x.", "x.x.x.x.x.x.4.x." });
    const juce::String bass = bars ({ "F1 - - - - - - C2~ F1 - - . . . . . ", "A#1 - - - - - - - . . A#1 - G#1~ A#1 - . ",
                                      "F1 - - - - - - C2~ F1 - - . . . . . ", "C2 - - - - - - - . . C2 - D#2~ C2 - . " });
    const juce::String line1 = bars ({ "C5 - - - D#5 - - - F5 - - - - - - - ", "D5 - - - C5 - - - A#4 - - - - - - - ",
                                       "C5 - - - D#5 - - - G5 - - - F5 - - - ", "D#5 - - - D5 - - - C5 - - - - - - - " });

    auto beat = [&] (bool full)
    {
        drums (w, LinnDrum, "bd", kicks);
        drums (w, LinnDrum, "sd", "....x.......x...");
        drums (w, LinnDrum, "ch", hats);
        if (! full) return;
        drums (w, LinnDrum, "ss", "..x.......x.....");
        drums (w, LinnDrum, "cg", "......x.......x.");
        drums (w, LinnDrum, "tb", "....x.......x...");
    };

    // A - intro: the Rhodes alone.
    w.part (0);
    chords (w, EPiano, ep);

    // B - verse: drums and the soft 808.
    w.part (1);
    beat (false);
    line (w, Sub808, bass);
    chords (w, EPiano, ep);

    // C - chorus: the strings sing, side stick, conga and tambourine.
    w.part (2);
    beat (true);
    line (w, Sub808, bass);
    chords (w, EPiano, ep);
    chords (w, Strings, line1);

    // D - bridge: no kick, the strings hold the chords, the Rhodes answers.
    w.part (3);
    drums (w, LinnDrum, "ch", hats);
    drums (w, LinnDrum, "ss", "....x.......x...");
    chords (w, Strings, bars ({ held (fm9, 16), held (bb9, 16), held (fm9, 16), held (cm7, 16) }));
    chords (w, EPiano, bars ({ rest16, keys (bb9), rest16, keys (cm7) }));
    line (w, Sub808, bars ({ held ("F1", 16), held ("A#1", 16), held ("F1", 16), held ("C2", 16) }));

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 2 }, { 1, 1 }, { 3, 1 }, { 2, 2 }, { 0, 1 } });
}

void reggaeton (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 95); p ("swing", 0.0f); p ("key", 7); p ("scale", 1); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.3f); p ("echoLevel", 0.4f);
    p ("reverbType", 2); p ("reverbSize", 0.5f); p ("reverbLevel", 0.35f);
    p ("comp", 0.4f);

    w.show (TR808);
    p ("tr808_bd_level", 0.6f); p ("tr808_bd_decay", 0.45f); p ("tr808_sd_level", 0.45f); p ("tr808_sd_tone", 0.6f);
    p ("tr808_cp_level", 0.38f); p ("tr808_ch_level", 0.34f); p ("tr808_rs_level", 0.32f); p ("tr808_oh_level", 0.26f);

    w.show (Percussion);
    p ("perc_bh_level", 0.3f); p ("perc_bl_level", 0.32f); p ("perc_gu_level", 0.25f); p ("perc_tl_level", 0.3f); p ("perc_th_level", 0.3f);
    p ("perc_cb_level", 0.22f);

    w.show (Sub808);
    p ("sub808_decay", 0.5f); p ("sub808_glide", 0.2f); p ("sub808_punch", 0.5f); p ("sub808_overdrive", 0.3f); p ("sub808_volume", 0.75f);

    w.show (DX7);
    p ("dx7_chord", 0); p ("dx7_preset", 4); p ("dx7_brightness", 0.5f); p ("dx7_decay", 0.45f); p ("dx7_echo", 0.3f);
    p ("dx7_reverb", 0.25f); p ("dx7_volume", 0.8f);

    w.show (Strings);
    p ("strings_chord", 0); p ("strings_attack", 0.5f); p ("strings_release", 0.6f); p ("strings_reverb", 0.4f); p ("strings_volume", 0.45f);

    // G minor: Gm - Eb - Bb - F. The dembow (boom-ch-boom-chick), the 808 on the tresillo, an FM marimba ostinato.
    const juce::String none = "................", kick = "x...x...x...x...";
    const juce::String dembow = "...x..x....x..x.";
    auto tres = [] (const juce::String& n, const juce::String& m) { return n + " . . " + n + " . . " + n + " . . . " + m + " . . . . . "; };
    const juce::String bass = bars ({ tres ("G1", "G1"), tres ("D#2", "D#2"), tres ("A#1", "A#1"), tres ("F1", "A1") });
    const juce::String marimba = bars ({ "G4 . A#4 D5 . A#4 G4 . D5 . A#4 . G4 . D4 . ", "G4 . A#4 D#5 . A#4 G4 . D#5 . A#4 . G4 . D#4 . ",
                                         "F4 . A#4 D5 . A#4 F4 . D5 . A#4 . F4 . D4 . ", "F4 . A4 C5 . A4 F4 . C5 . A4 . F4 . C4 . " });
    const juce::String hook = bars ({ "D5 - - . D5 - C5 - A#4 - - . G4 - - - ", "G4 - A#4 - C5 - - . A#4 - G4 - - - . . ",
                                      "D5 - - . D5 - F5 - D5 - - . C5 - A#4 - ", "A4 - - - C5 - - - A4 - - - . . . . " });

    auto beat = [&] (bool full)
    {
        drums (w, TR808, "bd", kick);
        drums (w, TR808, "sd", dembow);
        drums (w, TR808, "ch", "x.x.x.x.x.x.x.x.");
        if (! full) return;
        drums (w, TR808, "rs", "..x...x...x...x.");
        drums (w, Percussion, "bh", "..x..x....x..x..");
        drums (w, Percussion, "bl", "x.....x.x.....x.");
        drums (w, Percussion, "gu", "x.xxx.xxx.xxx.xx");
        drums (w, Percussion, "th", bars ({ none, none, none, "..........x.x.xx" }));
    };

    // A - intro: the marimba alone, then the dembow on its own.
    w.part (0);
    chords (w, DX7, marimba);
    drums (w, TR808, "sd", bars ({ none, none, dembow, dembow }));

    // B - verse: kick, dembow, the 808.
    w.part (1);
    beat (false);
    line (w, Sub808, bass);
    chords (w, DX7, marimba);

    // C - chorus: all the percussion, the strings play the hook.
    w.part (2);
    beat (true);
    line (w, Sub808, bass);
    chords (w, DX7, marimba);
    chords (w, Strings, hook);

    // D - break: the kick drops, the hook and the marimba.
    w.part (3);
    drums (w, TR808, "sd", dembow);
    drums (w, Percussion, "gu", "x.xxx.xxx.xxx.xx");
    chords (w, DX7, marimba);
    chords (w, Strings, hook);
    line (w, Sub808, bars ({ held ("G1", 16), held ("D#2", 16), held ("A#1", 16), held ("F1", 16) }));

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 2 }, { 1, 2 }, { 3, 1 }, { 2, 3 }, { 0, 1 } });
}
} // namespace techno::demos
