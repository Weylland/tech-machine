#include "Styles.h"
#include "Music.h"

namespace techno
{
namespace
{
enum ScaleIndex
{
    Chromatic, Minor, Major, HarmonicMinor, MelodicMinor, Dorian, Phrygian, PhrygianDominant, Lydian,
    Mixolydian, Locrian, DoubleHarmonic, HungarianMinor, PentaMinor, PentaMajor, Blues, WholeTone,
    Diminished, InSen, Hirajoshi, Egyptian
};

std::vector<StyleDef> buildStyles()
{
    std::vector<StyleDef> list;

    list.push_back ({ "Techno",
        "La techno « classique » : kick sur chaque temps, clap sur 2 et 4, charleston ouvert entre les kicks, "
        "une basse qui roule et quelques bleeps. Hypnotique et efficace.\n\n"
        "Tempo 125-135. Machines conseillées : 909, 303, 101.",
        130.0f, 0.05f, Minor, { 0 },
        {
            { Role::Kick, { "X...x...x...x...", "X...x...x...x..o" } },
            { Role::Clap, { "....x.......x...", "....x.......x..r" } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x.", "xoxoxoxoxoxoxoxo" } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Rim, { "...r..r....r.r..", "......x.......r." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............x.xx", "..........x.xxxx" } } },
        { { "xoxxoxxoxxoxxoxx" }, { { 0, 6 }, { 7, 3 }, { 3, 1 }, { 5, 1 }, { 1, 1 } }, 0.3f, 0.25f, 0.15f, 16, false },
        { { "..x...x...x...x.", "x.x...x.x.x...x.", "..xx..x...xx..x." }, { { 0, 8 }, { 7, 2 }, { 4, 1 }, { 2, 1 } }, 0.2f, 0.1f, 0.1f, 8, true },
        { { "x..x..x...x..x..", "x.x..x.x..x..x.." }, { { 0, 4 }, { 2, 3 }, { 4, 3 }, { 7, 2 }, { 6, 1 } }, 0.2f, 0.1f, 0.2f, 8, true },
        { { "...x.......x....", "..x.....x.......", "x..x..x........." }, false },
        true, false,
        { TR909, TB303, SH101 },
        {} });

    list.push_back ({ "Detroit",
        "La techno de Detroit (années 80-90) : mélodique et futuriste, avec des accords de synthé (stabs) chaleureux, "
        "des basses funky et des charlestons en doubles croches.\n\n"
        "Tempo 120-130. Machines conseillées : 909, 106, 101.",
        125.0f, 0.15f, Dorian, { 0, 3 },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::ClosedHat, { "x.xxx.xxx.xxx.xx", "xoxxxoxxxoxxxoxx" } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Cowbell, { "......r.......r." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............xxxx" } } },
        { { "xxoxxoxxxoxxoxxo" }, { { 0, 6 }, { 7, 3 }, { 4, 1 } }, 0.25f, 0.2f, 0.15f, 16, true },
        { { "x..x..x...x..x..", "x.....x.x.....x." }, { { 0, 6 }, { 7, 2 }, { 4, 2 }, { 6, 1 } }, 0.15f, 0.05f, 0.15f, 16, true },
        { { "x.xx.x.xx.x.x.x.", "x..x..x.x..x..x." }, { { 0, 3 }, { 2, 3 }, { 4, 3 }, { 6, 2 }, { 7, 2 }, { 9, 1 } }, 0.2f, 0.1f, 0.1f, 8, true },
        { { "x..x..x...x..x..", "..x..x....x..x..", "x.....x.....x..." }, false },
        true, false,
        { TR909, Juno106, SH101 },
        { { "juno106_cutoff", 0.5f }, { "juno106_env", 0.5f }, { "juno106_decay", 0.3f }, { "juno106_sustain", 0.2f },
          { "juno106_release", 0.3f }, { "juno106_echo", 0.45f } } });

    list.push_back ({ "Berlin",
        "La techno sombre et puissante des clubs berlinois (Berghain) : kick lourd, charlestons serrés, rimshots syncopés, "
        "basse grave qui roule et beaucoup d'espace (reverb, écho).\n\n"
        "Tempo 128-138. Machines conseillées : 909, D-Bass, 106.",
        133.0f, 0.0f, Phrygian, { 0 },
        {
            { Role::Kick, { "X...x...x...x...", "X...x...x...x.x." } },
            { Role::Clap, { "....r.......x..." } },
            { Role::ClosedHat, { "xoxxxoxxxoxxxoxx" } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Rim, { "..r..r.r...r..r.", "...x..x....x..x." } },
            { Role::Ride, { "..o...o...o...o." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Rim, { "........x.x.x.xx" } } },
        { { "xxoxxoxxxoxxoxxo" }, { { 0, 8 }, { 1, 2 }, { 7, 2 } }, 0.3f, 0.2f, 0.1f, 16, false },
        { { "x..x..x.x..x..x.", ".xx.xx.xx.xx.xx." }, { { 0, 10 }, { 1, 1 }, { 7, 1 } }, 0.2f, 0.05f, 0.05f, 16, false },
        { { "x...............", "x.......x......." }, { { 0, 3 }, { 1, 2 }, { 3, 1 }, { 7, 2 } }, 0.1f, 0.1f, 0.2f, 16, false },
        { { "..........x.....", "......x........." }, false },
        true, false,
        { TR909, ModelD, Juno106 },
        { { "tr909_bd_decay", 0.65f }, { "tr909_reverb", 0.25f }, { "juno106_echo", 0.7f }, { "juno106_reverb", 0.5f },
          { "echoFeedback", 0.6f }, { "reverbSize", 0.8f }, { "pump", 0.4f }, { "comp", 0.35f } } });

    list.push_back ({ "Minimal",
        "Techno minimale : peu d'éléments, beaucoup de groove. Des petites percussions (rim, claves, shaker) qui "
        "rebondissent, une basse discrète et des sons qui évoluent lentement.\n\n"
        "Tempo 122-128, swing marqué. Machines conseillées : 909 ou 808, 101, 106.",
        125.0f, 0.3f, InSen, { 0 },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::Rim, { "..r..x...r..x...", "...x..r...x..r.." } },
            { Role::ClosedHat, { "..o...o...o...o." } },
            { Role::OpenHat, { "..x.......x....." } },
            { Role::Shaker, { "oxoxoxoxoxoxoxox" } },
            { Role::Clap, { "....r.......x..." } },
            { Role::Claves, { "...r.......r...." } },
        },
        {},
        { { "x.ox.ox.x.ox.oxo" }, { { 0, 6 }, { 7, 2 }, { 3, 1 } }, 0.25f, 0.2f, 0.1f, 16, false },
        { { "x..o..x....o.x..", "x......x..x....." }, { { 0, 6 }, { 7, 2 }, { 4, 1 }, { 3, 1 } }, 0.15f, 0.1f, 0.1f, 16, false },
        { { "...x.......x....", "..x.....x..x...." }, { { 0, 3 }, { 2, 2 }, { 4, 2 }, { 7, 2 } }, 0.1f, 0.1f, 0.2f, 16, false },
        { { "...x............", "..........x....." }, false },
        true, false,
        { TR909, SH101, Juno106 },
        { { "echoTime", 2.0f }, { "echoFeedback", 0.55f }, { "juno106_echo", 0.6f }, { "sh101_echo", 0.4f } } });

    list.push_back ({ "Acid",
        "L'acid house et l'acid techno : tout tourne autour de la 303, sa ligne de basse qui « miaule », ses accents et ses glissés. "
        "Le reste est simple : kick, clap, charlestons.\n\n"
        "Astuce : pendant la lecture, tourne lentement FILTRE et ENV MOD de la 303.\n\n"
        "Tempo 125-135. Machines conseillées : 909, 303.",
        128.0f, 0.05f, Minor, { 0 },
        {
            { Role::Kick, { "X...x...x...x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::ClosedHat, { "xoxoxoxoxoxoxoxo" } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............x.xx" } } },
        { { "xxoxxoxxxoxxoxxo", "xoxxxxoxxoxxxoxx", "x.xxx.xxx.xx.xxx" }, { { 0, 7 }, { 7, 3 }, { 3, 1 }, { 5, 1 }, { 6, 1 }, { 1, 1 } },
          0.35f, 0.3f, 0.2f, 16, false },
        { { "..x...x...x...x." }, { { 0, 8 }, { 7, 2 } }, 0.1f, 0.0f, 0.1f, 16, false },
        { { "x..x..x...x..x.." }, { { 0, 3 }, { 2, 2 }, { 4, 2 }, { 7, 2 } }, 0.2f, 0.2f, 0.2f, 16, false },
        { { "..........x....." }, false },
        true, false,
        { TR909, TB303 },
        { { "tb303_cutoff", 0.25f }, { "tb303_reso", 0.85f }, { "tb303_env", 0.7f }, { "tb303_decay", 0.45f },
          { "tb303_accent", 0.75f }, { "tb303_drive", 0.3f }, { "tb303_echo", 0.25f } } });

    list.push_back ({ "Hard Techno",
        "Techno rapide et brutale : kick saturé et énorme (« rumble »), charlestons agressifs, stabs de synthé puissants. "
        "Beaucoup de disto et de compression.\n\n"
        "Tempo 145-160. Machines conseillées : 909, D-Bass, 8000.",
        150.0f, 0.0f, Phrygian, { 0 },
        {
            { Role::Kick, { "X...X...X...X..." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::ClosedHat, { "x.xxx.xxx.xxx.xx" } },
            { Role::Clap, { "....x.......x..." } },
            { Role::Rim, { "..r....r..r....r" } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "........x.x.xxxx" } } },
        { { "xxoxxoxxxoxxoxxo" }, { { 0, 8 }, { 1, 2 }, { 7, 2 } }, 0.35f, 0.2f, 0.1f, 16, false },
        { { "..x...x...x...x.", "..xx..xx..xx..xx" }, { { 0, 10 }, { 1, 1 } }, 0.1f, 0.0f, 0.05f, 16, false },
        { { "x.......x.......", "x..x..x.........", "x.x...x.x...x..." }, { { 0, 5 }, { 1, 1 }, { 3, 1 }, { 7, 2 } }, 0.0f, 0.0f, 0.1f, 16, false },
        { { "x.......x......." }, false },
        false, true,
        { TR909, ModelD, JP8000 },
        { { "tr909_bd_decay", 0.7f }, { "tr909_bd_tone", 0.8f }, { "tr909_drive", 0.55f }, { "tr909_reverb", 0.2f },
          { "disto", 0.35f }, { "pump", 0.55f }, { "comp", 0.45f }, { "jp8000_chord", 10.0f }, { "modeld_drive", 0.6f } } });

    list.push_back ({ "Dub Techno",
        "Techno profonde et planante : des accords qui se répètent dans un écho infini, beaucoup de reverb, un kick doux, "
        "une basse ronde. Comme dans un brouillard.\n\n"
        "Astuce : joue avec le bouton ÉCHO de la 106 et les RÉPÉTITIONS du master.\n\n"
        "Tempo 115-125. Machines conseillées : 909, 106, D-Bass.",
        120.0f, 0.1f, Minor, { 0 },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::ClosedHat, { "..o...o...o...o." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Rim, { "......r.......r." } },
            { Role::Shaker, { "o.o.o.o.o.o.o.o." } },
        },
        {},
        { { "x..x..x...x..x.." }, { { 0, 6 }, { 7, 2 } }, 0.2f, 0.2f, 0.1f, 16, false },
        { { "x.....x...x.....", "x.......x.x....." }, { { 0, 8 }, { 7, 1 }, { 4, 1 } }, 0.1f, 0.05f, 0.05f, 16, true },
        { { "...x......x....." }, { { 0, 3 }, { 2, 2 }, { 4, 2 } }, 0.0f, 0.1f, 0.2f, 16, true },
        { { "..x.....x.....x.", "...x.......x....", "..x.......x....." }, false },
        true, false,
        { TR909, Juno106, ModelD },
        { { "juno106_cutoff", 0.4f }, { "juno106_reso", 0.3f }, { "juno106_env", 0.3f }, { "juno106_decay", 0.25f },
          { "juno106_sustain", 0.1f }, { "juno106_echo", 0.9f }, { "juno106_reverb", 0.6f }, { "echoTime", 2.0f },
          { "echoFeedback", 0.7f }, { "echoLevel", 0.8f }, { "reverbSize", 0.85f }, { "reverbLevel", 0.7f },
          { "modeld_cutoff", 0.25f }, { "modeld_wave", 2.0f } } });

    list.push_back ({ "House",
        "La house de Chicago et New York : groove chaloupé, clap sur 2 et 4, charleston ouvert sur les contretemps, "
        "accords de piano/synthé et basse funky. Chaleureuse et dansante.\n\n"
        "Tempo 118-128, avec du swing. Machines conseillées : 909, 106, D-Bass.",
        124.0f, 0.25f, Dorian, { 0, 3 },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x.", "xxxxxxxxxxxxxxxx" } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Shaker, { "xoxxxoxxxoxxxoxx" } },
            { Role::Cowbell, { "r.....r.....r..." } },
            { Role::Rim, { "...r......r.r..." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............x.xx" } } },
        { { "x.xx.xx.x.xx.xx." }, { { 0, 6 }, { 7, 3 }, { 4, 1 } }, 0.2f, 0.15f, 0.2f, 16, true },
        { { "x..x..x.x..x..x.", "..x...x...x...x.", "x.....x.x.....x." }, { { 0, 6 }, { 7, 3 }, { 4, 1 } }, 0.15f, 0.05f, 0.2f, 16, true },
        { { "x.x..x.x..x.x..." }, { { 0, 3 }, { 2, 3 }, { 4, 3 }, { 7, 2 } }, 0.2f, 0.1f, 0.1f, 16, true },
        { { "x..x..x...x..x..", "..x..x....x..x..", "x.x...x.x...x..." }, false },
        true, false,
        { TR909, Juno106, ModelD },
        { { "juno106_decay", 0.3f }, { "juno106_sustain", 0.3f }, { "modeld_wave", 1.0f }, { "modeld_cutoff", 0.3f },
          { "modeld_contour", 0.4f }, { "comp", 0.35f }, { "pump", 0.35f } } });

    list.push_back ({ "Trance",
        "La trance : euphorique et épique. Grosses nappes de supersaw, arpèges rapides, basse sur les contretemps, "
        "progression d'accords émouvante et beaucoup de reverb.\n\n"
        "Tempo 136-140. Machines conseillées : 909, 8000, 101, D-Bass. Mets la séquence sur 4 mesures pour entendre la grille d'accords.",
        138.0f, 0.0f, Minor, { 0, 5, 3, 4 },
        {
            { Role::Kick, { "X...x...x...x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::ClosedHat, { "xoxoxoxoxoxoxoxo" } },
            { Role::Ride, { "..o...o...o...o." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "........xxxxxxxx" } } },
        { { "xxoxxoxxxoxxoxxo" }, { { 0, 6 }, { 7, 3 } }, 0.25f, 0.2f, 0.1f, 16, true },
        { { "..x...x...x...x.", "..xx..xx..xx..xx", ".xxx.xxx.xxx.xxx" }, { { 0, 10 }, { 7, 2 } }, 0.0f, 0.0f, 0.15f, 16, true },
        { { "xxxxxxxxxxxxxxxx", "x.xxx.xxx.xxx.xx" }, { { 0, 3 }, { 2, 3 }, { 4, 3 }, { 7, 2 }, { 9, 1 } }, 0.2f, 0.0f, 0.1f, 8, true },
        { {}, true },
        true, false,
        { TR909, JP8000, SH101, ModelD },
        { { "jp8000_chord", 1.0f }, { "jp8000_attack", 0.3f }, { "jp8000_release", 0.6f }, { "jp8000_sustain", 0.8f },
          { "jp8000_reverb", 0.5f }, { "jp8000_echo", 0.3f }, { "sh101_saw", 0.8f }, { "sh101_square", 0.4f },
          { "sh101_decay", 0.2f }, { "sh101_sustain", 0.0f }, { "sh101_echo", 0.4f }, { "echoTime", 2.0f },
          { "pump", 0.5f }, { "reverbSize", 0.7f } } });

    list.push_back ({ "Psytrance",
        "La psytrance (goa) : très hypnotique, une basse qui roule sur les 3 doubles croches entre chaque kick (« kick-basse-basse-basse »), "
        "des leads psychédéliques à la gamme orientale et des effets qui tournent.\n\n"
        "Tempo 140-148. Machines conseillées : 909, 101 (la basse), 8000 (le lead), 303.",
        145.0f, 0.0f, PhrygianDominant, { 0 },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::ClosedHat, { "x.xxx.xxx.xxx.xx" } },
            { Role::Clap, { "....r.......x..." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............xxxx" } } },
        { { "xxoxxoxxxoxxoxxo" }, { { 0, 6 }, { 1, 2 }, { 4, 2 }, { 7, 2 } }, 0.3f, 0.25f, 0.15f, 16, false },
        { { ".xxx.xxx.xxx.xxx" }, { { 0, 20 }, { 1, 1 }, { 7, 1 } }, 0.0f, 0.0f, 0.05f, 16, false },
        { { "x.xx.x.xx.x.x.xx", "xxx.xx.xxx.xx.x.", "x..x..x..x..x.x." }, { { 0, 4 }, { 1, 3 }, { 4, 3 }, { 5, 2 }, { 7, 2 } },
          0.2f, 0.0f, 0.2f, 16, false },
        { { "x..............." }, true },
        false, true,
        { TR909, SH101, JP8000, TB303 },
        { { "sh101_octave", 1.0f }, { "sh101_saw", 1.0f }, { "sh101_sub", 0.3f }, { "sh101_cutoff", 0.35f },
          { "sh101_reso", 0.45f }, { "sh101_env", 0.55f }, { "sh101_decay", 0.12f }, { "sh101_sustain", 0.0f },
          { "sh101_release", 0.05f }, { "jp8000_chord", 0.0f }, { "jp8000_detune", 0.3f }, { "jp8000_attack", 0.0f },
          { "jp8000_decay", 0.3f }, { "jp8000_sustain", 0.2f }, { "jp8000_release", 0.2f }, { "jp8000_echo", 0.45f },
          { "echoTime", 2.0f }, { "echoFeedback", 0.5f }, { "pump", 0.2f } } });

    list.push_back ({ "Drum & Bass",
        "La drum & bass : breakbeat rapide (kick et snare qui rebondissent), énorme basse « Reese » qui gronde, "
        "nappes atmosphériques. Le rythme est rapide mais la basse avance à moitié vitesse.\n\n"
        "Tempo 170-176. Machines conseillées : Break Kit, D-Bass, 106.",
        174.0f, 0.0f, Minor, { 0, 5 },
        {
            { Role::Kick, { "x.........x.....", "x.........x..x..", "x.x.......x....." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Ghost, { "......r..r....r.", ".r....r..r.r...." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x.", "xoxoxoxoxoxoxoxo" } },
            { Role::OpenHat, { "..o.......o....." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "..........x.x.xx" } } },
        { { "x.x.x.xx..x.x.x." }, { { 0, 6 }, { 7, 2 }, { 5, 1 } }, 0.25f, 0.25f, 0.1f, 16, true },
        { { "x-------x---x---", "x-----x---------", "x---x---x-x-x---" }, { { 0, 6 }, { 5, 1 }, { 3, 1 }, { 4, 1 } }, 0.0f, 0.0f, 0.05f, 16, true },
        { { "x.......x.x....." }, { { 0, 3 }, { 2, 2 }, { 4, 2 }, { 7, 2 } }, 0.1f, 0.1f, 0.2f, 16, true },
        { {}, true },
        true, false,
        { BreakKit, ModelD, Juno106 },
        { { "modeld_wave", 0.0f }, { "modeld_detune", 0.75f }, { "modeld_sub", 0.6f }, { "modeld_cutoff", 0.32f },
          { "modeld_emphasis", 0.2f }, { "modeld_contour", 0.2f }, { "modeld_sustain", 0.9f }, { "modeld_glide", 0.3f },
          { "modeld_drive", 0.5f }, { "juno106_attack", 0.5f }, { "juno106_release", 0.6f }, { "juno106_sustain", 0.8f },
          { "juno106_reverb", 0.5f }, { "juno106_hpf", 2.0f }, { "breaks_crush", 0.25f } } });

    list.push_back ({ "Jungle",
        "La jungle (UK, début des années 90) : breakbeats découpés et rejoués très vite (le célèbre « Amen break »), "
        "sub-basse profonde héritée du reggae, ambiance sombre ou planante.\n\n"
        "Tempo 160-170. Machines conseillées : Break Kit, D-Bass (sub), 106.",
        165.0f, 0.05f, Minor, { 0 },
        {
            { Role::Kick, { "x.x.......xx....", "x.........x....x" } },
            { Role::Snare, { "....x..o.o..x..o", "....x.......x..x" } },
            { Role::Ghost, { "..r..r....r..r.r", ".r..r.r..r....r." } },
            { Role::ClosedHat, { "xoxoxoxoxoxoxoxo" } },
            { Role::Ride, { "o.o.o.o.o.o.o.o." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "........x.xxx.xx" } } },
        { { "x.x.x.xx..x.x.x." }, { { 0, 6 }, { 7, 2 } }, 0.25f, 0.2f, 0.1f, 16, false },
        { { "x.......x..x....", "x-------x.....x." }, { { 0, 8 }, { 5, 1 }, { 3, 1 } }, 0.0f, 0.0f, 0.0f, 16, false },
        { { "x.......x......." }, { { 0, 3 }, { 2, 2 }, { 4, 2 } }, 0.0f, 0.1f, 0.2f, 16, false },
        { {}, true },
        true, false,
        { BreakKit, ModelD, Juno106 },
        { { "modeld_wave", 2.0f }, { "modeld_cutoff", 0.3f }, { "modeld_sub", 0.8f }, { "modeld_detune", 0.05f },
          { "modeld_sustain", 0.9f }, { "modeld_glide", 0.2f }, { "breaks_crush", 0.35f }, { "breaks_bd_tune", 0.55f },
          { "breaks_sd_tune", 0.6f }, { "juno106_reverb", 0.6f }, { "juno106_attack", 0.5f }, { "juno106_release", 0.6f },
          { "reverbLevel", 0.6f } } });

    list.push_back ({ "Breakbeat",
        "Le breakbeat (big beat, nu skool breaks) : rythme cassé façon hip-hop accéléré, basse funky et grasse, "
        "riffs de synthé. Énergique et groovy.\n\n"
        "Tempo 125-140. Machines conseillées : Break Kit, 101, 106.",
        132.0f, 0.1f, Blues, { 0, 3 },
        {
            { Role::Kick, { "x.....x...x.....", "x.....x.x.......", "x..x......x....." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Ghost, { ".......r..r....r" } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x." } },
            { Role::OpenHat, { "......o.......o." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "..........x.x.xx" } } },
        { { "x.xx.x.xx.x.x.x." }, { { 0, 6 }, { 7, 2 }, { 3, 1 }, { 5, 1 } }, 0.3f, 0.25f, 0.2f, 16, true },
        { { "x..x..x...x.x...", "x.x...x..x..x..." }, { { 0, 6 }, { 7, 2 }, { 3, 1 }, { 5, 1 } }, 0.2f, 0.15f, 0.2f, 16, true },
        { { "x..x..x...x..x.." }, { { 0, 3 }, { 2, 3 }, { 4, 2 }, { 7, 2 } }, 0.2f, 0.1f, 0.2f, 16, true },
        { { "x.....x...x.....", "..x.....x...x..." }, false },
        false, false,
        { BreakKit, SH101, Juno106 },
        { { "sh101_octave", 1.0f }, { "sh101_cutoff", 0.4f }, { "sh101_reso", 0.5f }, { "sh101_env", 0.6f }, { "breaks_crush", 0.2f } } });

    list.push_back ({ "Électro",
        "L'électro (Detroit, Miami, Kraftwerk) : rythme syncopé à la 808, cowbell, basses robotiques et mélodies "
        "de synthé froides et futuristes.\n\n"
        "Tempo 120-135. Machines conseillées : 808, 101, 8000.",
        128.0f, 0.05f, Phrygian, { 0 },
        {
            { Role::Kick, { "x.....x...x..x..", "x......xx.....x.", "x.....x.x.....x." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x.", "xxxxxxxxxxxxxxxx" } },
            { Role::Cowbell, { "..o.......o.....", "..x..x....x....." } },
            { Role::Claves, { "...r..r....r...." } },
            { Role::Cymbal, { "x..............." }, true },
        },
        { { Role::HighTom, { "............x.xx" } } },
        { { "x.xx.x.xx.x.x.x." }, { { 0, 6 }, { 7, 2 }, { 1, 1 } }, 0.3f, 0.2f, 0.2f, 16, false },
        { { "x..x..x.x.....x.", "x.xx..x...xx..x." }, { { 0, 6 }, { 7, 2 }, { 3, 1 }, { 1, 1 } }, 0.25f, 0.1f, 0.2f, 16, false },
        { { "x...x...x.x.x...", "x..x..x..x..x..." }, { { 0, 3 }, { 2, 2 }, { 4, 2 }, { 7, 2 } }, 0.1f, 0.0f, 0.2f, 16, false },
        { { "x..............." }, false },
        false, true,
        { TR808, SH101, JP8000 },
        { { "tr808_bd_decay", 0.6f }, { "sh101_octave", 1.0f }, { "sh101_saw", 0.3f }, { "sh101_square", 0.8f },
          { "sh101_pwm", 0.5f }, { "sh101_cutoff", 0.4f }, { "sh101_env", 0.5f }, { "jp8000_chord", 0.0f },
          { "jp8000_detune", 0.15f }, { "jp8000_mix", 0.3f }, { "jp8000_decay", 0.25f }, { "jp8000_sustain", 0.3f } } });

    list.push_back ({ "UK Garage",
        "Le UK garage (2-step) : rythme très chaloupé où le kick saute certains temps, rimshots syncopés, "
        "basse ronde et accords soul. Le swing est très marqué.\n\n"
        "Tempo 128-135. Machines conseillées : 909, D-Bass, 106.",
        132.0f, 0.55f, Dorian, { 0, 3 },
        {
            { Role::Kick, { "x......x..x.....", "x.........x..x..", "x.....x...x....." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Rim, { "..x..x.....x..x.", ".x....x...x..x.." } },
            { Role::ClosedHat, { "xoxoxoxoxoxoxoxo" } },
            { Role::OpenHat, { "..r...x...r...x." } },
            { Role::Shaker, { "oxoxoxoxoxoxoxox" } },
        },
        {},
        { { "x.xx.x.xx.x.x.x." }, { { 0, 6 }, { 7, 2 }, { 4, 1 } }, 0.2f, 0.2f, 0.2f, 16, true },
        { { "x..x.....x..x...", "x.....x.x..x...." }, { { 0, 6 }, { 7, 2 }, { 4, 1 }, { 6, 1 } }, 0.2f, 0.15f, 0.2f, 16, true },
        { { "x..x..x...x..x.." }, { { 0, 3 }, { 2, 3 }, { 4, 2 }, { 7, 2 } }, 0.2f, 0.1f, 0.2f, 16, true },
        { { "..x..x....x..x..", ".x..x..x.x..x..." }, false },
        true, false,
        { TR909, ModelD, Juno106 },
        { { "modeld_wave", 1.0f }, { "modeld_cutoff", 0.3f }, { "modeld_glide", 0.25f }, { "juno106_decay", 0.25f },
          { "juno106_sustain", 0.15f }, { "juno106_echo", 0.3f } } });

    list.push_back ({ "Gabber",
        "Le gabber / hardcore (Rotterdam, années 90) : extrêmement rapide, kick tellement saturé qu'il devient une note, "
        "leads stridents. Brutal et euphorique.\n\n"
        "Tempo 170-200. Machines conseillées : 909, 8000.",
        185.0f, 0.0f, Minor, { 0 },
        {
            { Role::Kick, { "X...X...X...X..." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Clap, { "....r.......r..." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Kick, { "........xxxxxxxx" } } },
        { { "xxoxxoxxxoxxoxxo" }, { { 0, 6 }, { 7, 3 } }, 0.3f, 0.2f, 0.1f, 16, false },
        { { "..x...x...x...x." }, { { 0, 10 } }, 0.0f, 0.0f, 0.0f, 16, false },
        { { "x.......x...x...", "x.x.x.x.x.x.x.x." }, { { 0, 5 }, { 7, 2 }, { 3, 1 }, { 1, 1 } }, 0.2f, 0.0f, 0.1f, 16, false },
        { { "x..............." }, false },
        false, true,
        { TR909, JP8000 },
        { { "tr909_bd_decay", 0.55f }, { "tr909_bd_tone", 1.0f }, { "tr909_bd_tune", 0.6f }, { "tr909_drive", 0.9f },
          { "disto", 0.5f }, { "comp", 0.5f }, { "jp8000_chord", 10.0f }, { "jp8000_detune", 0.6f }, { "jp8000_drive", 0.4f } } });

    return list;
}

bool hit (char c, juce::Random& r)
{
    switch (c)
    {
        case 'X': case 'x': case '-': return true;
        case 'o': return r.nextFloat() < 0.5f;
        case 'r': return r.nextFloat() < 0.2f;
        default: return false;
    }
}

template <typename T>
const T& pickOne (const std::vector<T>& items, juce::Random& r)
{
    return items[(size_t) r.nextInt ((int) items.size())];
}

int pickNote (const std::vector<std::pair<int, int>>& notes, juce::Random& r)
{
    int total = 0;
    for (auto& n : notes) total += n.second;
    int x = r.nextInt (juce::jmax (1, total));
    for (auto& n : notes)
    {
        if (x < n.second) return n.first;
        x -= n.second;
    }
    return 0;
}

int rowFor (const MachineDef& def, Role role)
{
    auto find = [&def] (Role r)
    {
        for (size_t i = 0; i < def.rows.size(); ++i)
            if (def.rows[i].role == r) return (int) i;
        return -1;
    };

    if (const int row = find (role); row >= 0) return row;
    if (role == Role::Clap) return find (Role::Snare) >= 0 && find (Role::Ghost) >= 0 ? find (Role::Snare) : -1;
    if (role == Role::Cymbal) return find (Role::Crash);
    if (role == Role::Crash) return find (Role::Cymbal);
    return -1;
}

// Chord root degree for a step: one chord per bar (two per bar when the pattern is a single bar).
int progressionDegree (const StyleDef& style, int step, int steps)
{
    const auto& prog = style.progression;
    if (prog.empty()) return 0;
    const int index = steps <= pageSteps ? step / 8 : step / pageSteps;
    return prog[(size_t) (index % (int) prog.size())];
}

int clampNote (int semitone)
{
    while (semitone >= noteRange) semitone -= 12;
    return juce::jmax (0, semitone);
}

void generateDrums (Track& t, const MachineDef& def, const StyleDef& style, int steps, juce::Random& r)
{
    t.clear();
    const int bars = steps / pageSteps;

    auto apply = [&] (const DrumPart& part, int firstBar, int lastBar)
    {
        const int row = rowFor (def, part.role);
        if (row < 0) return;
        const char* pattern = pickOne (part.variants, r);
        jassert (std::strlen (pattern) == pageSteps);

        for (int bar = firstBar; bar <= lastBar; ++bar)
        {
            if (part.firstBarOnly && bar > 0) break;
            for (int i = 0; i < pageSteps; ++i)
            {
                const int s = bar * pageSteps + i;
                if (! hit (pattern[i], r)) continue;
                t.hits[(size_t) row][(size_t) s] = true;
                if (pattern[i] == 'X') t.accent[(size_t) s] = true;
            }
        }
    };

    for (auto& part : style.drums) apply (part, 0, bars - 1);
    if (bars > 1)
        for (auto& part : style.fill) apply (part, bars - 1, bars - 1);
}

void generateMelody (Track& t, const MelodyPart& part, const StyleDef& style, int steps, int scale, bool mono, juce::Random& r)
{
    t.clear();
    if (part.rhythms.empty()) return;

    const char* rhythm = pickOne (part.rhythms, r);
    jassert (std::strlen (rhythm) == pageSteps);
    const int motif = juce::jlimit (1, pageSteps, part.motif);

    bool motifOn[pageSteps] {}, motifAccent[pageSteps] {}, motifTie[pageSteps] {};
    int motifDegree[pageSteps] {};
    for (int i = 0; i < motif; ++i)
    {
        const char c = rhythm[i];
        motifOn[i] = hit (c, r);
        motifTie[i] = c == '-' && i > 0;
        motifAccent[i] = c == 'X' || (motifOn[i] && r.nextFloat() < part.accent);
        motifDegree[i] = motifTie[i] ? motifDegree[i - 1]
                                     : pickNote (part.notes, r) + (r.nextFloat() < part.octaveJump ? 7 : 0);
    }

    for (int s = 0; s < steps; ++s)
    {
        const int i = s % motif;
        bool on = motifOn[i];
        int degree = motifDegree[i];

        if (s >= pageSteps && ! motifTie[i] && r.nextFloat() < 0.07f)
        {
            on = r.nextFloat() < 0.6f;
            degree = pickNote (part.notes, r);
        }

        const int chord = part.followChords ? progressionDegree (style, s, steps) : 0;
        t.on[(size_t) s] = on;
        t.accent[(size_t) s] = on && motifAccent[i];
        t.note[(size_t) s] = clampNote (degreeToSemitone (scale, chord + degree));
    }

    for (int s = 0; s < steps; ++s)
    {
        const auto cur = (size_t) s, prev = (size_t) ((s + steps - 1) % steps), next = (size_t) ((s + 1) % steps);
        if (motifTie[s % motif] && t.on[cur] && t.on[prev])
        {
            t.slide[prev] = true;
            t.note[cur] = t.note[prev].load();
        }
        else if (mono && t.on[cur] && t.on[next] && r.nextFloat() < part.slide)
        {
            t.slide[cur] = true;
        }
    }
}

void generateChords (Track& t, const ChordPart& part, const StyleDef& style, int steps, int scale, juce::Random& r)
{
    t.clear();

    if (part.pad || part.rhythms.empty())
    {
        const int segment = steps <= pageSteps && style.progression.size() > 1 ? 8 : pageSteps;
        for (int start = 0; start < steps; start += segment)
        {
            const int note = clampNote (degreeToSemitone (scale, progressionDegree (style, start, steps)));
            for (int s = start; s < start + segment; ++s)
            {
                t.on[(size_t) s] = true;
                t.note[(size_t) s] = note;
                t.slide[(size_t) s] = s < start + segment - 1;
            }
        }
        return;
    }

    const char* rhythm = pickOne (part.rhythms, r);
    for (int s = 0; s < steps; ++s)
    {
        const char c = rhythm[s % pageSteps];
        if (! hit (c, r)) continue;
        t.on[(size_t) s] = true;
        t.accent[(size_t) s] = c == 'X';
        t.note[(size_t) s] = clampNote (degreeToSemitone (scale, progressionDegree (style, s, steps)));
        if (c == '-' && s > 0)
        {
            t.slide[(size_t) (s - 1)] = true;
            t.note[(size_t) s] = t.note[(size_t) (s - 1)].load();
        }
    }
}
} // namespace

const std::vector<StyleDef>& styles()
{
    static const std::vector<StyleDef> list = buildStyles();
    return list;
}

void generate (Track& track, const MachineDef& def, const StyleDef& style, int steps, int scale, juce::Random& r)
{
    switch (def.type)
    {
        case TR909:
        case TR808:
        case BreakKit: generateDrums (track, def, style, steps, r); break;
        case TB303:    generateMelody (track, style.acid.rhythms.empty() ? style.bass : style.acid, style, steps, scale, true, r); break;
        case SH101:    generateMelody (track, style.leadOn101 ? style.lead : style.bass, style, steps, scale, true, r); break;
        case ModelD:   generateMelody (track, style.bass, style, steps, scale, true, r); break;
        case Juno106:  generateChords (track, style.chords, style, steps, scale, r); break;
        case JP8000:
            if (style.leadOn8000) generateMelody (track, style.lead, style, steps, scale, false, r);
            else generateChords (track, style.chords, style, steps, scale, r);
            break;
        default: break;
    }
}
} // namespace techno
