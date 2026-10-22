#include "ExampleTools.h"

// Demos of the newer machines and sequencer features.
namespace techno::demos
{
void bongoLab (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 122); p ("swing", 0.1f); p ("key", 2); p ("scale", 1); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 2); p ("echoFeedback", 0.5f); p ("echoLevel", 0.6f);
    p ("reverbType", 0); p ("reverbSize", 0.5f); p ("reverbLevel", 0.4f);
    p ("pumpMode", 1); p ("pump", 0.15f); p ("comp", 0.3f);

    w.show (SynthDrums);
    p ("synthdrums_bp_echo", 0.4f); p ("synthdrums_cl_reverb", 0.3f); p ("synthdrums_cp_reverb", 0.3f);

    w.show (WestCoast);
    p ("westcoast_octave", 2); p ("westcoast_ratio", 2); p ("westcoast_gate", 0); p ("westcoast_fm", 0.45f);
    p ("westcoast_fold", 0.35f); p ("westcoast_decay", 0.22f); p ("westcoast_echo", 0.35f); p ("westcoast_volume", 0.95f);

    w.show (Sub808);
    p ("sub808_decay", 0.55f); p ("sub808_glide", 0.25f); p ("sub808_volume", 0.45f);

    // Polyrhythms in D minor: the West Coast bongos loop every 5 steps, the FM bleeps every 3, against a kick of 16.
    // Some steps only play by chance, so the loop never repeats exactly.
    const juce::String none = "................";
    const juce::String bongos = "D3 . A3 F3 . D3 . A3 F3 . D3 . A3 F3 . D3 ";
    const juce::String bongosB = "D3 . C4 A3 . F3 . A3 G3 . D3 . C4 A3 . E3 ";
    const juce::String sub = bars ({ "D1 - - - . . . . . . D1 - . . . . ", "D1 - - - . . . . F1 - - . C1 - . . ",
                                     "D1 - - - . . . . . . D1 - . . . . ", "A#0 - - - . . . . C1 - - . A0 - . . " });

    auto pulse = [&] (bool full)
    {
        drums (w, SynthDrums, "bd", "x...x...x...x...");
        drums (w, SynthDrums, "bp", "x.x");
        if (! full) return;
        drums (w, SynthDrums, "ch", "..x...x...x...x.");
        drums (w, SynthDrums, "cl", "......x.......x.");
        stepValues (w, SynthDrums, "chance", ". . 0.5 . . . 0.5 . . . 0.5 . . . 0.7 .");
    };
    auto fiveSteps = [&] (const juce::String& notes)
    {
        line (w, WestCoast, notes);
        w.track (WestCoast).length = 5;
    };

    // A - the bongos alone on their loop of 5.
    w.part (0);
    fiveSteps (bongos);

    // B - the kick and the bleeps on 3 come in: three loops that never meet on the same step.
    w.part (1);
    pulse (false);
    fiveSteps (bongos);

    // C - the sub, the hats, the claves that play one time in two.
    w.part (2);
    pulse (true);
    fiveSteps (bongos);
    line (w, Sub808, sub);

    // D - the bongos change notes and open their wavefolder, the clap.
    w.part (3);
    pulse (true);
    drums (w, SynthDrums, "cp", "....x.......x...");
    fiveSteps (bongosB);
    stepValues (w, WestCoast, "westcoast_fold", ramp (0.3f, 0.65f, 32) + ramp (0.65f, 0.3f, 32));
    line (w, Sub808, sub);

    // E - break: bongos in the echo and the sub.
    w.part (4);
    fiveSteps (bongosB);
    stepValues (w, WestCoast, "westcoast_echo", hold (0.7f, 64));
    line (w, Sub808, sub);
    drums (w, SynthDrums, "zp", bars ({ none, none, none, "............x..." }));

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 3 }, { 4, 1 }, { 3, 2 }, { 2, 1 }, { 0, 1 } });
}

void pixelQuest (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 150); p ("swing", 0.0f); p ("key", 0); p ("scale", 0); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 1); p ("echoFeedback", 0.25f); p ("echoLevel", 0.3f);
    p ("reverbType", 0); p ("reverbSize", 0.4f); p ("reverbLevel", 0.25f);
    p ("comp", 0.3f);

    w.show (TR606);
    p ("tr606_bd_level", 0.6f); p ("tr606_sd_level", 0.5f); p ("tr606_ch_level", 0.34f); p ("tr606_oh_level", 0.3f);
    p ("tr606_lt_level", 0.42f); p ("tr606_ht_level", 0.42f); p ("tr606_cy_level", 0.3f);

    w.show (Chip);
    p ("chip_octave", 3); p ("chip_chord", 0); p ("chip_wave", 1); p ("chip_arp", 1); p ("chip_decay", 0.4f); p ("chip_sustain", 0.6f);
    p ("chip_vibrato", 0.25f); p ("chip_volume", 0.6f);

    w.show (SemiMod);
    p ("semimod_model", 1); p ("semimod_cutoff", 0.55f); p ("semimod_reso", 0.4f); p ("semimod_env", 0.35f); p ("semimod_osc2", 0.7f);
    p ("semimod_decay", 0.25f); p ("semimod_sustain", 0.2f); p ("semimod_volume", 0.75f);

    // C minor, a level theme: Cm - Ab - Bb - G. The chip plays its fast arpeggios, then a melody (the same machine,
    // its chord and arpeggio switched off by automation), the Odyssey bass jumps octaves.
    const juce::String none = "................";
    const juce::String arps = bars ({ held ("C4+D#4+G4", 16), held ("C4+D#4+G#4", 16), held ("A#3+D4+F4", 16), held ("B3+D4+G4", 16) });
    const juce::String theme = bars ({ "C5 . G4 . C5 D5 D#5 - D5 . C5 . G4 - - - ", "G#4 . C5 . D#5 F5 G5 - F5 . D#5 . C5 - - - ",
                                       "A#4 . D5 . F5 G5 G#5 - G5 . F5 . D5 - - - ", "G5 - - - F5 - D#5 - D5 - B4 - G4 - - - " });
    const juce::String bass = bars ({ "C2 . C3 . C2 . C3 . C2 . C3 . D#2 . F2 . ", "G#1 . G#2 . G#1 . G#2 . G#1 . G#2 . C2 . D#2 . ",
                                      "A#1 . A#2 . A#1 . A#2 . A#1 . A#2 . D2 . F2 . ", "G1 . G2 . G1 . G2 . G1 . G2 . G1 . B1 . " });
    const juce::String boss = bars ({ "C2 C2 C3 C2 D#2 C2 C3 C2 C2 C2 C3 C2 F#2 F2 D#2 D2 ", "C2 C2 C3 C2 D#2 C2 C3 C2 C2 C2 C3 C2 G2 F#2 F2 D#2 ",
                                      "C#2 C#2 C#3 C#2 E2 C#2 C#3 C#2 C#2 C#2 C#3 C#2 G2 F#2 F2 E2 ", "D2 D2 D3 D2 F2 D2 D3 D2 G1 G1 G2 G1 B1 B1 D2 D2 " });

    auto beat = [&] (bool full)
    {
        drums (w, TR606, "bd", "x.....x.x.......");
        drums (w, TR606, "sd", "....x.......x...");
        drums (w, TR606, "ch", "x.x.x.x.x.x.x.xx");
        if (full) drums (w, TR606, "oh", "..............x.");
    };
    auto melody = [&]
    {
        chords (w, Chip, theme);
        stepValues (w, Chip, "chip_arp", hold (0.0f, 64));
        stepValues (w, Chip, "chip_wave", hold (0.0f, 64));
    };

    // A - title screen: the arpeggios alone.
    w.part (0);
    chords (w, Chip, arps);

    // B - the level starts: drums and the Odyssey.
    w.part (1);
    beat (false);
    line (w, SemiMod, bass);
    chords (w, Chip, arps);

    // C - the theme.
    w.part (2);
    beat (true);
    line (w, SemiMod, bass);
    melody();

    // D - boss fight: the bass in sixteenths, chromatic, the toms.
    w.part (3);
    beat (true);
    drums (w, TR606, "lt", bars ({ none, none, none, "........x.x.x.x." }));
    drums (w, TR606, "cy", "x...............................................................");
    line (w, SemiMod, boss);
    stepValues (w, SemiMod, "semimod_cutoff", ramp (0.45f, 0.75f));
    chords (w, Chip, bars ({ held ("C4+D#4+G4", 32), held ("C#4+E4+G#4", 16), held ("D4+F4+A#4", 8) + held ("B3+D4+G4", 8) }));
    stepValues (w, Chip, "chip_arp", hold (2.0f, 64));

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 3 }, { 1, 1 }, { 2, 3 }, { 3, 4 }, { 2, 2 }, { 0, 1 } });
}

void robotDisco (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 118); p ("swing", 0.05f); p ("key", 4); p ("scale", 0); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.35f); p ("echoLevel", 0.45f);
    p ("reverbType", 2); p ("reverbSize", 0.5f); p ("reverbLevel", 0.4f);
    p ("pumpMode", 0); p ("pump", 0.3f); p ("comp", 0.35f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.5f); p ("tr909_cp_level", 0.42f); p ("tr909_cp_reverb", 0.3f); p ("tr909_oh_level", 0.34f);
    p ("tr909_ch_level", 0.32f); p ("tr909_cr_level", 0.3f);

    w.show (EBass);
    p ("ebass_type", 0); p ("ebass_tone", 0.6f); p ("ebass_decay", 0.35f); p ("ebass_muting", 0.5f); p ("ebass_volume", 0.85f);

    w.show (Vocoder);
    p ("vocoder_chord", 0); p ("vocoder_words", 3); p ("vocoder_speed", 0.4f); p ("vocoder_phaser", 0.5f); p ("vocoder_reverb", 0.3f);
    p ("vocoder_volume", 0.75f);

    // E minor, French touch: Em - C - D - B. The whole band behind a filter that opens, the vocoder sings the chords
    // through a phaser, the bass in octaves.
    const juce::String none = "................", kick = "x...x...x...x...";
    const juce::String bass = bars ({ "E2 E3 . E2 E3 . E2 E3 . E2 E3 . G2 G3 A2 A3 ", "C2 C3 . C2 C3 . C2 C3 . C2 C3 . E2 E3 G2 G3 ",
                                      "D2 D3 . D2 D3 . D2 D3 . D2 D3 . F#2 F#3 A2 A3 ", "B1 B2 . B1 B2 . B1 B2 . B1 B2 . D#2 D#3 F#2 F#3 " });
    const juce::String voices = bars ({ held ("E3+G3+B3", 12) + ". . . . ", held ("E3+G3+C4", 12) + ". . . . ",
                                        held ("F#3+A3+D4", 12) + ". . . . ", held ("D#3+F#3+B3", 12) + ". . . . " });

    auto beat = [&]
    {
        drums (w, TR909, "bd", kick);
        drums (w, TR909, "cp", "....x.......x...");
        drums (w, TR909, "oh", "..x...x...x...x.");
        drums (w, TR909, "ch", "x.xxx.xxx.xxx.xx");
    };
    auto filtered = [&] (float from, float to)
    {
        stepValues (w, TR909, "tr909_filter", ramp (from, to));
        stepValues (w, EBass, "ebass_filter", ramp (from, to));
    };

    // A - intro: drums and bass muffled, the filter opens.
    w.part (0);
    beat();
    line (w, EBass, bass);
    filtered (0.18f, 0.42f);

    // B - wide open.
    w.part (1);
    beat();
    line (w, EBass, bass);

    // C - the vocoder sings.
    w.part (2);
    beat();
    drums (w, TR909, "cr", "x...............................................................");
    line (w, EBass, bass);
    chords (w, Vocoder, voices);

    // D - break: the vocoder alone, its phaser spinning, the bass comes back filtered.
    w.part (3);
    chords (w, Vocoder, voices);
    stepValues (w, Vocoder, "vocoder_phaser", hold (0.9f, 64));
    line (w, EBass, bass);
    stepValues (w, EBass, "ebass_filter", ramp (0.15f, 0.45f));
    drums (w, TR909, "sd", bars ({ none, none, none, "x.x.x.x.xxxxxxxx" }));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 3 }, { 3, 1 }, { 2, 3 }, { 0, 1 } });
}

void tearsInRain (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 72); p ("swing", 0.0f); p ("key", 7); p ("scale", 3); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 3); p ("echoFeedback", 0.45f); p ("echoLevel", 0.4f);
    p ("reverbType", 1); p ("reverbSize", 0.9f); p ("reverbLevel", 0.7f);

    w.show (CS80);
    p ("cs80_chord", 0); p ("cs80_type", 0); p ("cs80_brilliance", 0.45f); p ("cs80_attack", 0.45f); p ("cs80_release", 0.6f);
    p ("cs80_reverb", 0.5f); p ("cs80_volume", 0.7f);

    w.show (Wavetable);
    p ("wavetable_table", 2); p ("wavetable_chord", 0); p ("wavetable_attack", 0.7f); p ("wavetable_release", 0.7f);
    p ("wavetable_motion", 0.35f); p ("wavetable_reverb", 0.6f); p ("wavetable_volume", 0.6f);

    w.show (Sub808);
    p ("sub808_decay", 0.9f); p ("sub808_glide", 0.2f); p ("sub808_punch", 0.1f); p ("sub808_volume", 0.6f);

    // G minor, film music: Gm - Eb - Cm - D, the CS-80 brass swell in a huge hall, the vowel pad moves,
    // then the CS-80 plays the melody alone, bending into the notes.
    const juce::String brass = bars ({ held ("G3+A#3+D4", 16), held ("G3+A#3+D#4", 16), held ("G3+C4+D#4", 16), held ("F#3+A3+D4", 16) });
    const juce::String vowels = bars ({ held ("G2+D3", 32), held ("C3+G3", 16), held ("D3+A3", 16) });
    const juce::String sub = bars ({ held ("G1", 16), held ("D#1", 16), held ("C1", 16), held ("D1", 16) });
    const juce::String melody = bars ({ "D5 - - - - - - - A#4 - - - C5 - D5 - ", "D#5 - - - - - - - D5 - C5 - A#4 - - - ",
                                        "C5 - - - - - - - D#5 - - - G5 - - - ", "F#5 - - - - - - - - - - - - - - - " });

    // A - the vowel pad alone.
    w.part (0);
    chords (w, Wavetable, vowels);

    // B - the brass swell, the sub underneath.
    w.part (1);
    chords (w, Wavetable, vowels);
    chords (w, CS80, brass);
    line (w, Sub808, sub);

    // C - the melody on the expressive lead of the CS-80.
    w.part (2);
    chords (w, Wavetable, vowels);
    chords (w, CS80, melody);
    stepValues (w, CS80, "cs80_type", hold (4.0f, 64));
    stepValues (w, CS80, "cs80_bend", hold (0.5f, 64));
    line (w, Sub808, sub);

    // D - the vowels open, the brass alone.
    w.part (3);
    chords (w, CS80, brass);
    stepValues (w, CS80, "cs80_brilliance", ramp (0.35f, 0.7f));
    chords (w, Wavetable, vowels);
    stepValues (w, Wavetable, "wavetable_position", ramp (0.2f, 0.8f));

    w.song ({ { 0, 1 }, { 1, 2 }, { 2, 2 }, { 3, 1 }, { 2, 1 }, { 1, 1 } });
}

void euclidGarden (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 126); p ("swing", 0.05f); p ("key", 9); p ("scale", 1); p ("bars", 3);
    p ("echoType", 1); p ("echoTime", 2); p ("echoFeedback", 0.55f); p ("echoLevel", 0.6f);
    p ("reverbType", 0); p ("reverbLevel", 0.4f);
    p ("pumpMode", 1); p ("pump", 0.15f); p ("comp", 0.3f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.5f); p ("tr909_cp_level", 0.42f); p ("tr909_cp_reverb", 0.3f);
    w.show (TR808);
    p ("tr808_rs_level", 0.4f); p ("tr808_rs_echo", 0.4f); p ("tr808_cb_level", 0.25f); p ("tr808_cb_echo", 0.4f);
    w.show (TR606);
    p ("tr606_ch_level", 0.55f);
    w.show (Juno106);
    p ("juno106_chord", 4); p ("juno106_echo", 0.4f); p ("juno106_cutoff", 0.6f); p ("juno106_decay", 0.4f); p ("juno106_sustain", 0.15f);
    p ("juno106_volume", 1.0f);

    // Tracks of different lengths and speeds: the rimshot loops every 7 steps, the hats play in triplets and by chance,
    // the first clap only every second round, the chords arrive late (delay) and not every time (chance).
    const juce::String none = "................";
    auto kick = [&] { drums (w, TR909, "bd", "x...x...x...x..."); };
    auto clap = [&]
    {
        drums (w, TR909, "cp", "....x.......x...");
        stepValues (w, TR909, "condition", ". . . . 6 . . . . . . . . . . .");   // the first clap: every second round
    };
    auto rims = [&]
    {
        drums (w, TR808, "rs", "x.x.x");
        w.track (TR808).length = 7;
    };
    auto hats = [&]
    {
        drums (w, TR606, "ch", "x.x.x.x.x.x.x.x.");
        w.track (TR606).speed = 3;   // triplets
        stepValues (w, TR606, "chance", "1 0.6 1 0.6 1 0.6 1 0.6");
        stepValues (w, TR606, "velocity", "0.9 0.5 0.7 0.5");
    };
    auto chordsLate = [&] (const juce::String& notes)
    {
        line (w, Juno106, notes);
        stepValues (w, Juno106, "chance", ". . . 0.8 . . . . . . 0.5 . . . . .");
        stepValues (w, Juno106, "delay", ". . . 0.3 . . . . . . 0.3 . . . . .");
    };
    const juce::String stabs = bars ({ ". . . A3 . . . . . . A3 . . . . . ", ". . . A3 . . . . . . A3 . . . . . ",
                                       ". . . F3 . . . . . . F3 . . . . . ", ". . . G3 . . . . . . G3 . . . . . " });

    // A - kick and the triplet hats.
    w.part (0);
    kick();
    hats();

    // B - the rimshot on 7 against the 16 of the kick.
    w.part (1);
    kick();
    hats();
    rims();

    // C - the clap that skips a round, the late chords.
    w.part (2);
    kick();
    hats();
    rims();
    clap();
    chordsLate (stabs);

    // D - break: chords and rimshots in the echo, the cowbell loops on 5.
    w.part (3);
    rims();
    drums (w, TR808, "cb", "x.x.x");
    chordsLate (stabs);
    stepValues (w, Juno106, "juno106_echo", hold (0.75f, 64));

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 4 }, { 3, 2 }, { 2, 4 }, { 1, 1 } });
}

void arpHorizon (ExampleWriter& w)
{
    P p { w };
    p ("bpm", 138); p ("swing", 0.0f); p ("key", 5); p ("scale", 1); p ("bars", 3);
    p ("echoType", 0); p ("echoTime", 2); p ("echoFeedback", 0.4f); p ("echoLevel", 0.5f);
    p ("reverbType", 1); p ("reverbSize", 0.75f); p ("reverbLevel", 0.5f);
    p ("pumpMode", 1); p ("pump", 0.6f); p ("comp", 0.35f);

    w.show (TR909);
    p ("tr909_bd_decay", 0.45f); p ("tr909_bd_tone", 0.6f); p ("tr909_oh_level", 0.38f); p ("tr909_cp_level", 0.42f);
    p ("tr909_cr_level", 0.36f); p ("tr909_sd_level", 0.42f);

    w.show (Juno106);
    p ("juno106_chord", 2); p ("juno106_arp", 3); p ("juno106_arprate", 1); p ("juno106_arpoctaves", 1);
    p ("juno106_echo", 0.35f); p ("juno106_release", 0.2f); p ("juno106_volume", 0.8f);

    w.show (JP8000);
    p ("jp8000_chord", 2); p ("jp8000_attack", 0.4f); p ("jp8000_release", 0.6f); p ("jp8000_reverb", 0.4f); p ("jp8000_volume", 0.6f);

    w.show (TB303);
    p ("tb303_cutoff", 0.3f); p ("tb303_reso", 0.5f); p ("tb303_env", 0.4f); p ("tb303_decay", 0.3f); p ("tb303_volume", 0.65f);

    w.show (SoundFx);
    p ("fx_rise_decay", 0.75f); p ("fx_rise_level", 0.42f); p ("fx_imp_level", 0.5f);

    // F minor: Fm - Db - Ab - Eb. The Juno arpeggiator runs over two octaves, the real sidechain makes the synths breathe
    // with the kick, the 303 on the off-beats.
    const juce::String none = "................", kick = "x...x...x...x...";
    const juce::String roots = bars ({ held ("F3", 16), held ("C#3", 16), held ("G#3", 16), held ("D#3", 16) });
    const juce::String bass = bars ({ ". F2 F2 . . F2 F2 . . F2 F2 . . F2 F2 . ", ". C#2 C#2 . . C#2 C#2 . . C#2 C#2 . . C#2 C#2 . ",
                                      ". G#2 G#2 . . G#2 G#2 . . G#2 G#2 . . G#2 G#2 . ", ". D#2 D#2 . . D#2 D#2 . . D#2 D#2 . . D#2 D#2 . " });

    auto beat = [&]
    {
        drums (w, TR909, "bd", kick);
        drums (w, TR909, "oh", "..x...x...x...x.");
        drums (w, TR909, "cp", "....x.......x...");
    };

    // A - the arpeggio alone, its filter opening.
    w.part (0);
    line (w, Juno106, roots);
    stepValues (w, Juno106, "juno106_cutoff", ramp (0.2f, 0.5f));

    // B - the kick: now the sidechain pumps the arpeggio.
    w.part (1);
    beat();
    line (w, Juno106, roots);
    line (w, TB303, bass);

    // C - the pads breathe with it.
    w.part (2);
    beat();
    line (w, Juno106, roots);
    line (w, TB303, bass);
    line (w, JP8000, roots);

    // D - break: no kick, the arpeggio goes up to three octaves, the riser.
    w.part (3);
    line (w, Juno106, roots);
    stepValues (w, Juno106, "juno106_arpoctaves", hold (2.0f, 64));
    line (w, JP8000, roots);
    drums (w, TR909, "sd", bars ({ none, none, "x...x...x...x...", "x.x.x.x.xxxx2244" }));
    stepValues (w, TR909, "tr909_sd_level", hold (0.2f, 32) + ramp (0.2f, 0.55f, 32));
    drums (w, SoundFx, "rise", bars ({ none, none, "x...............", none }));

    // E - drop: impact, crash, the 303 opens.
    w.part (4);
    beat();
    drums (w, TR909, "cr", "x...............................................................");
    drums (w, SoundFx, "imp", "x...............................................................");
    line (w, Juno106, roots);
    line (w, TB303, bass);
    stepValues (w, TB303, "tb303_cutoff", ramp (0.3f, 0.5f, 32) + ramp (0.5f, 0.3f, 32));
    line (w, JP8000, roots);

    w.song ({ { 0, 2 }, { 1, 2 }, { 2, 3 }, { 3, 2 }, { 4, 4 }, { 2, 1 }, { 1, 1 } });
}
} // namespace techno::demos
