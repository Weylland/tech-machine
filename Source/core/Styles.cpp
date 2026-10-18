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

using B = BassMode;
using L = LeadMode;
using C = ChordMode;

std::vector<StyleDef> buildStyles()
{
    std::vector<StyleDef> list;

    list.push_back ({ "Techno",
        "La techno « classique » : kick sur chaque temps, clap sur 2 et 4, charleston ouvert entre les kicks, "
        "une basse qui roule et quelques bleeps hypnotiques. Tempo conseillé : 125-135.",
        130.0f, 0.05f, Minor, { { 0 }, { 0, 0, 5, 5 }, { 0, 6, 5, 6 } },
        {
            { Role::Kick, { "X...x...x...x...", "X...x...x...x..o" } },
            { Role::Clap, { "....x.......x...", "....x.......x..r" } },
            { Role::ClosedHat, { "xx.xxx.xxx.xxx.x", ".x.x.x.x.x.x.x.x" } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Rim, { "...r..r....r.r.." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............x.xx", "..........x.xxxx" } } },
        { B::Offbeat, B::Rolling, B::Groove }, { L::Motif, L::Sparse }, { C::Stabs, C::Sparse },
        { "...x.......x....", "..x.....x.......", "x..x..x........." },
        true, false });

    list.push_back ({ "Detroit",
        "La techno de Detroit : mélodique et futuriste, accords de synthé (stabs) chaleureux, basses funky, "
        "charlestons en doubles croches. Tempo conseillé : 120-130.",
        125.0f, 0.15f, Dorian, { { 0, 3 }, { 0, 5, 3, 4 }, { 0, 6, 5, 6 }, { 0, 2, 5, 6 } },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::ClosedHat, { "x.xxx.xxx.xxx.xx", "xoxxxoxxxoxxxoxx" } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Cowbell, { "......r.......r." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............xxxx" } } },
        { B::Groove }, { L::Arp, L::Motif }, { C::Stabs },
        { "x..x..x...x..x..", "..x..x....x..x..", "x.....x.....x..." },
        true, false });

    list.push_back ({ "Berlin",
        "La techno sombre et puissante des clubs berlinois : kick lourd, charlestons serrés, rimshots syncopés, "
        "basse grave qui roule, beaucoup d'espace. Tempo conseillé : 128-138.",
        133.0f, 0.0f, Phrygian, { { 0 }, { 0, 1 }, { 0, 0, 0, 6 } },
        {
            { Role::Kick, { "X...x...x...x...", "X...x...x...x.x." } },
            { Role::Clap, { "....r.......x..." } },
            { Role::ClosedHat, { "xoxxxoxxxoxxxoxx" } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Rim, { "..r..r.r...r..r." } },
            { Role::Ride, { "..o...o...o...o." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Rim, { "........x.x.x.xx" } } },
        { B::Rolling, B::Offbeat }, { L::Sparse }, { C::Sparse },
        { "..........x.....", "......x........." },
        true, false });

    list.push_back ({ "Minimal",
        "Techno minimale : peu d'éléments, beaucoup de groove. Des petites percussions qui rebondissent, "
        "une basse discrète, des sons qui évoluent lentement. Tempo conseillé : 122-128, swing marqué.",
        125.0f, 0.3f, InSen, { { 0 }, { 0, 3 } },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::Rim, { "..r..r...r..r..." } },
            { Role::ClosedHat, { "..o...o...o...o." } },
            { Role::OpenHat, { "..x.......x....." } },
            { Role::Shaker, { "oxoxoxoxoxoxoxox" } },
            { Role::Clap, { "....r.......x..." } },
            { Role::Claves, { "...r.......r...." } },
        },
        {},
        { B::Groove }, { L::Sparse }, { C::Sparse },
        { "...x............", "..........x....." },
        true, false });

    list.push_back ({ "Acid",
        "L'acid house et l'acid techno : tout tourne autour de la 303 qui « miaule ». Le reste est simple : "
        "kick, clap, charlestons. Tempo conseillé : 125-135.",
        128.0f, 0.05f, Minor, { { 0 }, { 0, 0, 0, 5 } },
        {
            { Role::Kick, { "X...x...x...x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::ClosedHat, { "xx.xxx.xxx.xxx.x" } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............x.xx" } } },
        { B::Offbeat }, { L::Motif }, { C::Sparse },
        { "..........x....." },
        true, false });

    list.push_back ({ "Hard Techno",
        "Techno rapide et brutale : kick saturé et énorme, charlestons agressifs, stabs puissants. "
        "Tempo conseillé : 145-160.",
        150.0f, 0.0f, Phrygian, { { 0 }, { 0, 1 }, { 0, 6 } },
        {
            { Role::Kick, { "X...X...X...X..." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::ClosedHat, { "x.xxx.xxx.xxx.xx" } },
            { Role::Clap, { "....x.......x..." } },
            { Role::Rim, { "..r....r..r....r" } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "........x.x.xxxx" } } },
        { B::Offbeat, B::Rolling }, { L::Riff }, { C::Stabs },
        { "x.......x.......", "x..x..x.........", "x.x...x.x...x..." },
        false, true });

    list.push_back ({ "Dub Techno",
        "Techno profonde et planante : des accords qui se répètent dans l'écho, beaucoup de reverb, un kick doux, "
        "une basse ronde. Tempo conseillé : 115-125.",
        120.0f, 0.1f, Minor, { { 0 }, { 0, 3 }, { 0, 5 } },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::ClosedHat, { "..o...o...o...o." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Rim, { "......r.......r." } },
            { Role::Shaker, { "o.o.o.o.o.o.o.o." } },
        },
        {},
        { B::Long, B::Groove }, { L::Sparse }, { C::Stabs },
        { "..x.....x.....x.", "...x.......x....", "..x.......x....." },
        true, false });

    list.push_back ({ "House",
        "La house de Chicago et New York : groove chaloupé, clap sur 2 et 4, charleston ouvert sur les contretemps, "
        "accords et basse funky. Tempo conseillé : 118-128, avec du swing.",
        124.0f, 0.25f, Dorian, { { 0, 3 }, { 0, 5, 3, 4 }, { 0, 3, 6, 2 } },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::ClosedHat, { ".x.x.x.x.x.x.x.x", "xx.xxx.xxx.xxx.x" } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Shaker, { "xoxxxoxxxoxxxoxx" } },
            { Role::Cowbell, { "r.....r.....r..." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............x.xx" } } },
        { B::Groove }, { L::Motif }, { C::Stabs },
        { "x..x..x...x..x..", "..x..x....x..x..", "x.x...x.x...x..." },
        true, false });

    list.push_back ({ "Trance",
        "La trance : euphorique et épique. Grosses nappes, arpèges rapides, basse sur les contretemps, "
        "progression d'accords émouvante. Tempo conseillé : 136-140. Mets la séquence sur 4 mesures.",
        138.0f, 0.0f, Minor, { { 0, 5, 3, 4 }, { 0, 5, 2, 6 }, { 5, 6, 0, 0 }, { 0, 2, 5, 6 } },
        {
            { Role::Kick, { "X...x...x...x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::ClosedHat, { "xx.xxx.xxx.xxx.x", "x..xx..xx..xx..x" } },
            { Role::Ride, { "..o...o...o...o." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "........xxxxxxxx" } } },
        { B::Offbeat, B::Rolling }, { L::Arp }, { C::Pad },
        {},
        true, false });

    list.push_back ({ "Psytrance",
        "La psytrance : très hypnotique, une basse qui roule entre chaque kick (« kick-basse-basse-basse »), "
        "des leads psychédéliques à la gamme orientale. Tempo conseillé : 140-148.",
        145.0f, 0.0f, PhrygianDominant, { { 0 }, { 0, 0, 1, 0 } },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::ClosedHat, { "x.xxx.xxx.xxx.xx" } },
            { Role::Clap, { "....r.......x..." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............xxxx" } } },
        { B::Psy }, { L::Riff }, { C::Pad, C::Sparse },
        { "x..............." },
        false, true });

    list.push_back ({ "Drum & Bass",
        "La drum & bass : breakbeat rapide, énorme basse qui gronde et avance à moitié vitesse, "
        "nappes atmosphériques. Tempo conseillé : 170-176.",
        174.0f, 0.0f, Minor, { { 0, 5 }, { 0, 5, 3, 6 }, { 0, 3 } },
        {
            { Role::Kick, { "x.........x.....", "x.........x..x..", "x.x.......x....." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Ghost, { "......r..r....r.", ".r....r..r.r...." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x.", "xoxoxoxoxoxoxoxo" } },
            { Role::OpenHat, { "..o.......o....." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "..........x.x.xx" } } },
        { B::Long }, { L::Sparse, L::Motif }, { C::Pad },
        {},
        true, false });

    list.push_back ({ "Jungle",
        "La jungle : breakbeats découpés et rejoués très vite, sub-basse profonde héritée du reggae, "
        "ambiance sombre ou planante. Tempo conseillé : 160-170.",
        165.0f, 0.05f, Minor, { { 0 }, { 0, 5 } },
        {
            { Role::Kick, { "x.x.......xx....", "x.........x....x" } },
            { Role::Snare, { "....x..o.o..x..o", "....x.......x..x" } },
            { Role::Ghost, { "..r..r....r..r.r", ".r..r.r..r....r." } },
            { Role::ClosedHat, { "xoxoxoxoxoxoxoxo" } },
            { Role::Ride, { "o.o.o.o.o.o.o.o." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "........x.xxx.xx" } } },
        { B::Long }, { L::Sparse }, { C::Pad },
        {},
        true, false });

    list.push_back ({ "Breakbeat",
        "Le breakbeat : rythme cassé façon hip-hop accéléré, basse funky et grasse, riffs de synthé. "
        "Tempo conseillé : 125-140.",
        132.0f, 0.1f, Blues, { { 0, 3 }, { 0, 6, 5, 6 } },
        {
            { Role::Kick, { "x.....x...x.....", "x.....x.x.......", "x..x......x....." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Ghost, { ".......r..r....r" } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x." } },
            { Role::OpenHat, { "......o.......o." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "..........x.x.xx" } } },
        { B::Groove }, { L::Motif }, { C::Stabs },
        { "x.....x...x.....", "..x.....x...x..." },
        false, false });

    list.push_back ({ "Électro",
        "L'électro (Detroit, Miami, Kraftwerk) : rythme syncopé à la 808, cowbell, basses robotiques, "
        "mélodies froides et futuristes. Tempo conseillé : 120-135.",
        128.0f, 0.05f, Phrygian, { { 0 }, { 0, 1 } },
        {
            { Role::Kick, { "x.....x...x..x..", "x......xx.....x.", "x.....x.x.....x." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x.", "xxxxxxxxxxxxxxxx" } },
            { Role::Cowbell, { "..r.......r....." } },
            { Role::Claves, { "...r..r....r...." } },
            { Role::Cymbal, { "x..............." }, true },
        },
        { { Role::HighTom, { "............x.xx" } } },
        { B::Groove }, { L::Riff, L::Arp }, { C::Sparse },
        { "x..............." },
        false, true });

    list.push_back ({ "UK Garage",
        "Le UK garage (2-step) : rythme très chaloupé où le kick saute certains temps, rimshots syncopés, "
        "basse ronde et accords soul. Tempo conseillé : 128-135, swing très marqué.",
        132.0f, 0.55f, Dorian, { { 0, 3 }, { 0, 5, 3, 4 } },
        {
            { Role::Kick, { "x......x..x.....", "x.........x..x..", "x.....x...x....." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Rim, { "..x..x.....x..x.", ".x....x...x..x.." } },
            { Role::ClosedHat, { "xoxoxoxoxoxoxoxo" } },
            { Role::OpenHat, { "..r...x...r...x." } },
            { Role::Shaker, { "oxoxoxoxoxoxoxox" } },
        },
        {},
        { B::Groove }, { L::Motif }, { C::Stabs },
        { "..x..x....x..x..", ".x..x..x.x..x..." },
        true, false });

    list.push_back ({ "Gabber",
        "Le gabber / hardcore : extrêmement rapide, kick tellement saturé qu'il devient une note, leads stridents. "
        "Tempo conseillé : 170-200.",
        185.0f, 0.0f, Minor, { { 0 }, { 0, 5, 6, 0 } },
        {
            { Role::Kick, { "X...X...X...X..." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::Clap, { "....r.......r..." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Kick, { "........xxxxxxxx" } } },
        { B::Offbeat }, { L::Riff }, { C::Stabs },
        { "x..............." },
        false, true });

    list.push_back ({ "Hip-Hop",
        "Le hip-hop boom bap des années 90 : kick lourd et syncopé, snare qui claque sur 2 et 4, charlestons swingués, "
        "boucle d'accords jazzy et basse ronde. Tempo conseillé : 85-95, swing marqué (50-60 %).",
        90.0f, 0.3f, Dorian, { { 0 }, { 0, 3 }, { 0, 0, 5, 5 } },
        {
            { Role::Kick, { "x......x..x.....", "x.......x.x.....", "x......xx.....x." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x.", "xxxxxxxxxxxxxxxx" } },
            { Role::OpenHat, { "..............o." } },
            { Role::Shaker, { "rxrxrxrxrxrxrxrx" } },
        },
        { { Role::Kick, { "x......x..x...x." } } },
        { B::Groove }, { L::Sparse }, { C::Comping },
        { "x.......x.......", "x.....x........." },
        false, false });

    list.push_back ({ "Trap",
        "Le trap d'Atlanta : rythme en demi-tempo (snare sur le 3e temps), charlestons rapides avec des roulements, "
        "énorme basse 808 qui suit le kick et glisse entre les notes, mélodie sombre. Tempo conseillé : 130-150.",
        140.0f, 0.0f, HarmonicMinor, { { 0 }, { 0, 5 }, { 0, 6, 5, 6 } },
        {
            { Role::Kick, { "x.........x.....", "x......x..x.....", "x.....x...x..x.." } },
            { Role::Snare, { "........x......." } },
            { Role::Clap, { "........x......." } },
            { Role::ClosedHat, { "x.x.x.x.x.x2x.x.", "x.x.x.x.x.x.x3x.", "x.x.x3x.x.x.x2x2", "xxxxxxxx3xxxxx4x" } },
            { Role::OpenHat, { ".......r........" } },
            { Role::Snap, { "........x......." } },
        },
        { { Role::Snare, { "........x.....xx" } } },
        { B::Sub808 }, { L::Riff, L::Sparse }, { C::Pad, C::Sparse },
        { "x..............." },
        true, true });

    list.push_back ({ "Drill",
        "La drill (Chicago, UK) : demi-tempo sombre, snares décalés, charlestons en triolets, 808 qui glisse beaucoup "
        "et mélodies tendues en mineur. Tempo conseillé : 138-145.",
        142.0f, 0.05f, HarmonicMinor, { { 0 }, { 0, 1 }, { 0, 5 } },
        {
            { Role::Kick, { "x.....x...x..x..", "x.......x.....x." } },
            { Role::Snare, { "........x....x..", "........x.x....." } },
            { Role::ClosedHat, { "x..x..x.x..x..x.", "x.x..x.x.x..x.x.", "x..x..x.x2.x..x." } },
            { Role::Rim, { "...x.....x......" } },
            { Role::Snap, { "........x....x.." } },
        },
        {},
        { B::Sub808 }, { L::Riff }, { C::Pad },
        {},
        true, true });

    list.push_back ({ "Lo-fi",
        "Le lo-fi hip-hop : tranquille et nostalgique. Batterie douce très swinguée, accords jazzy au piano électrique, "
        "basse discrète, petite mélodie. Tempo conseillé : 70-90, beaucoup de swing.",
        82.0f, 0.4f, Dorian, { { 0, 3 }, { 0, 5, 3, 4 }, { 1, 4, 0, 0 } },
        {
            { Role::Kick, { "x......x.x......", "x.......x.x....." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x." } },
            { Role::Rim, { "..........r....." } },
            { Role::Shaker, { "r.r.r.r.r.r.r.r." } },
        },
        {},
        { B::Groove }, { L::Sparse, L::Motif }, { C::Comping },
        {},
        true, false });

    list.push_back ({ "R&B",
        "Le R&B moderne : groove lent et sensuel, claquements de doigts, charlestons en doubles croches avec roulements, "
        "accords riches (7e, 9e), basse 808 ou ronde. Tempo conseillé : 65-100.",
        92.0f, 0.2f, Dorian, { { 0, 3, 5, 4 }, { 0, 5 }, { 0, 3 } },
        {
            { Role::Kick, { "x......x..x..x..", "x.......x.x....." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Snap, { "....x.......x..." } },
            { Role::ClosedHat, { "xxxxxxxxxxxxxxxx", "xxxxxxx3xxxxxxx2" } },
            { Role::Rim, { "......r.......r." } },
        },
        {},
        { B::Groove, B::Sub808 }, { L::Motif }, { C::Comping, C::Pad },
        {},
        true, false });

    list.push_back ({ "Funk",
        "Le funk : tout sur le « un ». Basse slap très syncopée, snare avec ghost notes, charlestons en doubles croches, "
        "petits accords de guitare ou d'orgue qui claquent. Tempo conseillé : 95-115.",
        105.0f, 0.1f, Mixolydian, { { 0 }, { 0, 3 } },
        {
            { Role::Kick, { "x..x..x...x..x..", "x.x...x..x....x." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Ghost, { "..r..r.r...r..r." } },
            { Role::ClosedHat, { "xxxxxxxxxxxxxxxx", "x.x.x.x.x.x.x.x." } },
            { Role::OpenHat, { "..............x." } },
            { Role::Cowbell, { "r.....r.....r..." } },
        },
        { { Role::Snare, { "..........x.x.xx" } } },
        { B::Funk }, { L::Riff }, { C::Stabs },
        { "..x...x...x.x...", ".x..x..x.x..x..." },
        false, false });

    list.push_back ({ "Disco",
        "Le disco : kick sur chaque temps, charleston ouvert sur les contretemps, basse en octaves, cordes et "
        "accords joyeux. Tempo conseillé : 115-125.",
        120.0f, 0.05f, Minor, { { 0, 3 }, { 0, 5, 3, 4 } },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::OpenHat, { "..x...x...x...x." } },
            { Role::ClosedHat, { ".x.x.x.x.x.x.x.x" } },
            { Role::Tambourine, { "xxxxxxxxxxxxxxxx" } },
        },
        { { Role::Snare, { "............xxxx" } } },
        { B::Disco }, { L::Arp, L::Motif }, { C::Stabs, C::Pad },
        { "..x...x...x...x.", "x..x..x...x..x.." },
        true, false });

    list.push_back ({ "Synthwave",
        "La synthwave : l'esthétique des années 80. Batterie à grosse snare, basse en croches qui pulse, nappes, "
        "arpèges et lead nostalgique. Tempo conseillé : 85-115.",
        100.0f, 0.0f, Minor, { { 0, 5, 2, 6 }, { 0, 6, 5, 6 } },
        {
            { Role::Kick, { "x...x...x...x...", "x.......x.x....." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::LowTom, { "............x.x." } }, { Role::HighTom, { "............x.x." } } },
        { B::Disco }, { L::Motif, L::Arp }, { C::Pad },
        {},
        true, true });

    list.push_back ({ "Pop",
        "La pop : rythme simple et efficace, grille d'accords qui fait chanter (I – V – vi – IV), mélodie accrocheuse. "
        "Tempo conseillé : 95-125.",
        110.0f, 0.0f, Major, { { 0, 4, 5, 3 }, { 5, 3, 0, 4 }, { 0, 5, 3, 4 } },
        {
            { Role::Kick, { "x.......x.x.....", "x...x...x...x..." } },
            { Role::Snare, { "....x.......x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x." } },
            { Role::Tambourine, { "....x.......x..." } },
            { Role::Crash, { "x..............." }, true },
        },
        { { Role::Snare, { "............x.xx" } } },
        { B::Groove, B::Offbeat }, { L::Motif }, { C::Pad, C::Stabs },
        { "x.x.x.x.x.x.x.x.", "x..x..x...x..x.." },
        true, false });

    list.push_back ({ "Reggaeton",
        "Le reggaeton : le rythme « dembow » (boum-tchak... boum-tchak) qui fait danser toute l'Amérique latine, "
        "basse 808 sur le tresillo, accords mineurs. Tempo conseillé : 88-100.",
        95.0f, 0.0f, Minor, { { 0, 5, 2, 6 }, { 0, 5 } },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::Snare, { "...x..x....x..x." } },
            { Role::Rim, { "...x..x....x..x." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x." } },
            { Role::HighTimbale, { "............x.xx" } },
        },
        {},
        { B::Tresillo, B::Sub808 }, { L::Motif }, { C::Stabs, C::Pad },
        { "x..x..x.x..x..x.", "..x...x...x...x." },
        true, false });

    list.push_back ({ "Afro House",
        "L'afro house : kick sur chaque temps et plein de percussions qui dialoguent (congas, shaker, cloches), "
        "basse ronde, accords hypnotiques. Tempo conseillé : 118-124.",
        122.0f, 0.15f, Minor, { { 0 }, { 0, 3 } },
        {
            { Role::Kick, { "x...x...x...x..." } },
            { Role::Clap, { "....x.......x..." } },
            { Role::ClosedHat, { "..x...x...x...x." } },
            { Role::Shaker, { "xoxxxoxxxoxxxoxx" } },
            { Role::LowConga, { "x..x..x...x..x.." } },
            { Role::HighConga, { "...x..x.x.....x." } },
            { Role::Agogo, { "r..r..r....r..r." } },
        },
        {},
        { B::Tresillo, B::Groove }, { L::Sparse }, { C::Stabs },
        { "..x.....x...x...", "...x......x....." },
        true, false });

    list.push_back ({ "Reggae",
        "Le reggae et la dub : le « one drop » (kick et rimshot ensemble sur le 3e temps), accords sur les contretemps (skank), "
        "basse ronde et chantante, beaucoup d'écho. Tempo conseillé : 70-85.",
        76.0f, 0.2f, Minor, { { 0, 3 }, { 0, 6 }, { 0 } },
        {
            { Role::Kick, { "........x......." } },
            { Role::Rim, { "........x......." } },
            { Role::ClosedHat, { "x.x.x.x.x.x.x.x.", "x..xx..xx..xx..x" } },
            { Role::OpenHat, { "..............r." } },
        },
        { { Role::Snare, { "............x.xx" } } },
        { B::Reggae }, { L::Sparse }, { C::Skank },
        {},
        false, false });

    list.push_back ({ "Ambient",
        "L'ambient : peu ou pas de rythme, de longues nappes qui évoluent, des notes éparses, beaucoup de reverb. "
        "Tempo conseillé : 60-90.",
        80.0f, 0.0f, Lydian, { { 0, 3 }, { 0, 4 }, { 0 } },
        {
            { Role::Kick, { "x..............." } },
            { Role::Shaker, { "r...r...r...r..." } },
            { Role::Triangle, { "..........r....." } },
        },
        {},
        { B::Long }, { L::Sparse }, { C::Pad },
        {},
        true, false });

    return list;
}

//==============================================================================
// Helpers

bool chance (juce::Random& r, float p) { return r.nextFloat() < p; }

template <typename T>
const T& pickOne (const std::vector<T>& items, juce::Random& r)
{
    return items[(size_t) r.nextInt ((int) items.size())];
}

int pickWeighted (std::initializer_list<std::pair<int, int>> options, juce::Random& r)
{
    int total = 0;
    for (auto& o : options) total += o.second;
    int x = r.nextInt (juce::jmax (1, total));
    for (auto& o : options)
    {
        if (x < o.second) return o.first;
        x -= o.second;
    }
    return options.begin()->first;
}

bool hit (char c, juce::Random& r)
{
    switch (c)
    {
        case 'X': case 'x': case '2': case '3': case '4': return true;
        case 'o': return r.nextFloat() < 0.5f;
        case 'r': return r.nextFloat() < 0.2f;
        default: return false;
    }
}

bool isPercussion (Role role)
{
    switch (role)
    {
        case Role::Rim: case Role::Claves: case Role::Cowbell:
        case Role::LowTom: case Role::MidTom: case Role::HighTom:
        case Role::LowConga: case Role::HighConga: case Role::LowBongo: case Role::HighBongo:
        case Role::LowTimbale: case Role::HighTimbale: case Role::Agogo: case Role::Guiro:
        case Role::Triangle: case Role::Perc:
            return true;
        default:
            return false;
    }
}

bool onlyScattered (const char* pattern)
{
    for (const char* p = pattern; *p != 0; ++p)
        if (*p == 'x' || *p == 'X' || *p == 'o') return false;
    return true;
}

// The closest sounds to use when a kit lacks one, best first.
std::vector<Role> fallbacksFor (Role role)
{
    using R = Role;
    switch (role)
    {
        case R::Snare:       return { R::Clap, R::Snap };
        case R::Clap:        return { R::Snap, R::Snare };
        case R::Snap:        return { R::Clap, R::Snare };
        case R::Rim:         return { R::Claves, R::Snap, R::Perc, R::Snare };
        case R::ClosedHat:   return { R::Shaker, R::Tambourine };
        case R::OpenHat:     return { R::ClosedHat, R::Shaker };
        case R::Ride:        return { R::Cymbal, R::Crash, R::ClosedHat };
        case R::Crash:       return { R::Cymbal, R::Ride };
        case R::Cymbal:      return { R::Crash, R::Ride };
        case R::Shaker:      return { R::Tambourine, R::ClosedHat };
        case R::Tambourine:  return { R::Shaker, R::ClosedHat };
        case R::Cowbell:     return { R::Agogo, R::Claves, R::Rim };
        case R::Agogo:       return { R::Cowbell, R::Claves };
        case R::Claves:      return { R::Rim, R::Perc, R::Cowbell };
        case R::Perc:        return { R::Claves, R::Rim, R::Cowbell };
        case R::Triangle:    return { R::Agogo, R::Cowbell, R::Claves };
        case R::Guiro:       return { R::Shaker, R::Tambourine };
        case R::LowTom:      return { R::LowConga, R::MidTom, R::HighTom };
        case R::MidTom:      return { R::LowTom, R::HighTom, R::LowConga };
        case R::HighTom:     return { R::HighConga, R::MidTom, R::LowTom };
        case R::LowConga:    return { R::LowTom, R::HighConga, R::LowBongo };
        case R::HighConga:   return { R::HighTom, R::LowConga, R::HighBongo };
        case R::LowBongo:    return { R::HighBongo, R::HighConga, R::MidTom };
        case R::HighBongo:   return { R::LowBongo, R::HighConga, R::HighTom };
        case R::LowTimbale:  return { R::HighTimbale, R::MidTom, R::LowTom };
        case R::HighTimbale: return { R::LowTimbale, R::HighTom, R::Cowbell };
        default:             return {};
    }
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
    for (const Role other : fallbacksFor (role))
        if (const int row = find (other); row >= 0) return row;
    return -1;
}

// Chord root degree for a step: one chord per bar (two per bar when the pattern is a single bar).
int chordAt (const GenerateContext& ctx, int step)
{
    const auto& prog = ctx.progression;
    if (prog.empty()) return 0;
    const int index = ctx.steps <= pageSteps ? step / 8 : step / pageSteps;
    return prog[(size_t) (index % (int) prog.size())];
}

int clampNote (int semitone)
{
    while (semitone >= noteRange) semitone -= 12;
    while (semitone < 0) semitone += 12;
    return semitone;
}

// k hits spread as evenly as possible over 16 steps, rotated.
std::array<bool, pageSteps> euclid (int k, int rotation)
{
    std::array<bool, pageSteps> mask {};
    for (int i = 0; i < pageSteps; ++i)
        mask[(size_t) ((i + rotation) % pageSteps)] = ((i * k) % pageSteps) < k;
    return mask;
}

// A melodic line under construction: degrees are absolute scale degrees (chord root included).
struct Line
{
    std::array<bool, maxSteps> on {}, accent {}, slide {};
    std::array<int, maxSteps> degree {};

    void note (int step, int deg, bool acc = false)
    {
        on[(size_t) step] = true;
        degree[(size_t) step] = deg;
        accent[(size_t) step] = acc;
    }

    // Holds the note of `from` through `to` (glide on mono synths, held chord on poly synths).
    void hold (int from, int to, int steps)
    {
        for (int s = from + 1; s <= to && s < steps; ++s)
        {
            on[(size_t) s] = true;
            degree[(size_t) s] = degree[(size_t) from];
            slide[(size_t) (s - 1)] = true;
        }
    }

    void write (Track& t, const GenerateContext& ctx) const
    {
        t.clear();
        for (int s = 0; s < ctx.steps; ++s)
        {
            const auto i = (size_t) s;
            t.on[i] = on[i];
            t.accent[i] = on[i] && accent[i];
            t.slide[i] = on[i] && slide[i];
            t.note[i] = clampNote (degreeToSemitone (ctx.scale, degree[i]));
        }
        t.tile (ctx.steps);
    }
};

//==============================================================================
// Drums

void drums (Track& t, const MachineDef& def, const GenerateContext& ctx, juce::Random& r)
{
    t.clear();
    const int bars = ctx.steps / pageSteps;

    auto apply = [&] (const DrumPart& part, int firstBar, int lastBar)
    {
        const int row = rowFor (def, part.role);
        if (row < 0) return;
        const Role rowRole = def.rows[(size_t) row].role;
        if (rowRole != part.role)
            for (auto& other : ctx.style.drums)
                if (other.role == rowRole) return;
        const char* pattern = pickOne (part.variants, r);

        // Scattered percussion becomes an evenly spread (Euclidean) rhythm that avoids the kicks.
        std::array<bool, pageSteps> mask {};
        const bool spread = isPercussion (part.role) && onlyScattered (pattern);
        if (spread)
        {
            const int k = 2 + r.nextInt (3);
            int bestRotation = 0, bestScore = -100;
            for (int rotation = 0; rotation < pageSteps; ++rotation)
            {
                const auto candidate = euclid (k, rotation);
                int score = r.nextInt (3);
                for (int i = 0; i < pageSteps; ++i)
                    if (candidate[(size_t) i]) score += (i % 4 == 0 ? -4 : 0) + (i % 2 == 1 ? 1 : 0);
                if (score > bestScore) { bestScore = score; bestRotation = rotation; }
            }
            mask = euclid (k, bestRotation);
        }

        for (int bar = firstBar; bar <= lastBar; ++bar)
        {
            if (part.firstBarOnly && bar > 0) break;
            for (int i = 0; i < pageSteps; ++i)
            {
                const int s = bar * pageSteps + i;
                const bool on = spread ? mask[(size_t) i] : hit (pattern[i], r);
                if (! on) continue;
                t.hits[(size_t) row][(size_t) s] = true;
                if (pattern[i] == 'X') t.accent[(size_t) s] = true;
                if (pattern[i] >= '2' && pattern[i] <= '4') t.rolls[(size_t) row][(size_t) s] = (std::uint8_t) (pattern[i] - '0');
            }
        }
    };

    for (auto& part : ctx.style.drums) apply (part, 0, bars - 1);
    if (bars > 1)
        for (auto& part : ctx.style.fill) apply (part, bars - 1, bars - 1);

    if (rowFor (def, Role::Kick) < 0)
    {
        int count = 0;
        for (auto& row : t.hits)
            for (int s = 0; s < pageSteps; ++s) count += row[(size_t) s] ? 1 : 0;
        if (count < 8)
        {
            static const DrumPart defaults[] {
                { Role::LowConga, { "r.r.r.r.r.r.r.r." } },
                { Role::HighConga, { ".r.r.r.r.r.r.r.r" } },
                { Role::Shaker, { "xoxxxoxxxoxxxoxx", "x.x.x.x.x.x.x.x." } },
                { Role::HighBongo, { "...r...r...r...r" } },
            };
            for (auto& part : defaults) apply (part, 0, bars - 1);
        }
    }

    // A closed hat never plays together with the open hat, nor a ghost note with the snare.
    const int ch = rowFor (def, Role::ClosedHat), oh = rowFor (def, Role::OpenHat);
    const int sd = rowFor (def, Role::Snare), gs = rowFor (def, Role::Ghost);
    for (int s = 0; s < ctx.steps; ++s)
    {
        if (ch >= 0 && oh >= 0 && ch != oh && t.hits[(size_t) oh][(size_t) s]) t.hits[(size_t) ch][(size_t) s] = false;
        if (sd >= 0 && gs >= 0 && sd != gs && t.hits[(size_t) sd][(size_t) s]) t.hits[(size_t) gs][(size_t) s] = false;
    }
    t.tile (ctx.steps);
}

//==============================================================================
// Bass lines

void bassLine (Line& line, const GenerateContext& ctx, BassMode mode, juce::Random& r)
{
    const int steps = ctx.steps;

    switch (mode)
    {
        case BassMode::Offbeat:
        {
            const int variant = r.nextInt (3);
            for (int s = 0; s < steps; ++s)
            {
                const int i = s % pageSteps;
                const bool on = i % 4 == 2 || (variant == 1 && i % 4 == 3) || (variant == 2 && i % 4 == 3 && chance (r, 0.35f));
                if (! on) continue;
                const bool octave = i % 4 == 2 && (i == 6 || i == 14) && chance (r, 0.25f);
                line.note (s, chordAt (ctx, s) + (octave ? 7 : 0), i % 4 == 2);
            }
            break;
        }

        case BassMode::Rolling:
        {
            int motif[pageSteps];
            for (int i = 0; i < pageSteps; ++i)
                motif[i] = pickWeighted ({ { 0, 8 }, { 7, 2 }, { 4, 1 } }, r);
            for (int s = 0; s < steps; ++s)
            {
                const int i = s % pageSteps;
                if (ctx.kick[(size_t) s] || i % 4 == 0 || chance (r, 0.08f)) continue;
                line.note (s, chordAt (ctx, s) + motif[i], i % 4 == 2);
            }
            break;
        }

        case BassMode::Groove:
        {
            // A one-bar motif placed on syncopated steps between the kicks, repeated on each chord.
            static constexpr int weights[pageSteps] { 3, 1, 3, 4, 1, 2, 4, 3, 1, 2, 4, 3, 1, 2, 4, 2 };
            bool onset[pageSteps] {};
            int deg[pageSteps] {};
            const int count = 4 + r.nextInt (4);
            for (int n = 0; n < count; ++n)
            {
                int total = 0;
                float w[pageSteps];
                for (int i = 0; i < pageSteps; ++i)
                {
                    w[i] = onset[i] ? 0.0f : (float) weights[i] * (ctx.kick[(size_t) i] ? 0.3f : 1.0f);
                    total += (int) (w[i] * 10.0f);
                }
                int x = r.nextInt (juce::jmax (1, total));
                for (int i = 0; i < pageSteps; ++i)
                {
                    x -= (int) (w[i] * 10.0f);
                    if (x < 0) { onset[i] = true; break; }
                }
            }
            bool first = true;
            for (int i = 0; i < pageSteps; ++i)
            {
                if (! onset[i]) continue;
                deg[i] = first ? 0 : pickWeighted ({ { 0, 5 }, { 7, 3 }, { 4, 2 }, { 6, 1 }, { 2, 1 }, { 3, 1 } }, r);
                first = false;
            }

            const int bars = steps / pageSteps;
            for (int s = 0; s < steps; ++s)
            {
                const int i = s % pageSteps;
                if (! onset[i]) continue;
                int d = deg[i];
                if (bars > 1 && s / pageSteps == bars - 1 && i >= 12) d = pickWeighted ({ { 7, 2 }, { 4, 2 }, { 6, 1 } }, r);
                line.note (s, chordAt (ctx, s) + d, i == 3 || i == 6 || i == 10 || i == 14 ? chance (r, 0.35f) : false);
            }
            for (int s = 0; s + 1 < steps; ++s)
                if (line.on[(size_t) s] && line.on[(size_t) s + 1] && line.degree[(size_t) s] != line.degree[(size_t) s + 1] && chance (r, 0.3f))
                    line.slide[(size_t) s] = true;
            break;
        }

        case BassMode::Long:
        {
            static const std::vector<std::vector<int>> rhythms { { 0 }, { 0, 8 }, { 0, 8, 12 }, { 0, 6 }, { 0, 10 } };
            const auto& starts = pickOne (rhythms, r);
            const int segment = steps <= pageSteps && ctx.progression.size() > 1 ? 8 : pageSteps;
            for (int bar = 0; bar < steps; bar += segment)
            {
                for (size_t k = 0; k < starts.size(); ++k)
                {
                    const int start = bar + starts[k] * segment / pageSteps;
                    const int end = k + 1 < starts.size() ? bar + starts[k + 1] * segment / pageSteps - 1 : bar + segment - 1;
                    const int d = k == 0 ? 0 : pickWeighted ({ { 0, 3 }, { 4, 2 }, { 7, 1 } }, r);
                    line.note (start, chordAt (ctx, start) + d);
                    line.hold (start, end, steps);
                }
            }
            break;
        }

        case BassMode::Psy:
        {
            for (int s = 0; s < steps; ++s)
            {
                const int i = s % pageSteps;
                if (i % 4 == 0) continue;
                int d = 0;
                if ((s / pageSteps) % 2 == 1 && i >= 13) d = pickWeighted ({ { 0, 2 }, { 1, 1 }, { 4, 1 }, { 7, 1 } }, r);
                line.note (s, chordAt (ctx, s) + d);
            }
            break;
        }

        case BassMode::Sub808:
        {
            // The 808 follows the kick and holds each note until the next one, sometimes gliding up or down.
            int last = -1;
            for (int s = 0; s < steps; ++s)
            {
                if (! ctx.kick[(size_t) s]) continue;
                const int i = s % pageSteps;
                const int d = i == 0 ? 0 : pickWeighted ({ { 0, 6 }, { 7, 2 }, { -2, 1 }, { 4, 1 }, { 5, 1 } }, r);
                line.note (s, chordAt (ctx, s) + d, i == 0);
                last = s;
            }
            if (last < 0)
            {
                for (int s = 0; s < steps; s += pageSteps / 2) line.note (s, chordAt (ctx, s), true);
            }
            for (int s = 0; s < steps; ++s)
            {
                if (! line.on[(size_t) s] || (s > 0 && line.slide[(size_t) (s - 1)])) continue;
                int next = s + 1;
                while (next < steps && ! line.on[(size_t) next]) ++next;
                const int end = juce::jmin (next - 1, s + 2 + r.nextInt (6));
                if (end > s) line.hold (s, end, steps);
                if (next < steps && end == next - 1 && chance (r, 0.3f)) line.slide[(size_t) end] = true;
            }
            break;
        }

        case BassMode::Disco:
        {
            // Octave jumps on every 8th note.
            const bool sixteenths = chance (r, 0.3f);
            for (int s = 0; s < steps; ++s)
            {
                const int i = s % pageSteps;
                if (i % 2 == 1 && ! (sixteenths && i % 4 == 3)) continue;
                const bool up = i % 4 != 0;
                line.note (s, chordAt (ctx, s) + (up ? 7 : 0), i % 4 == 0);
            }
            break;
        }

        case BassMode::Funk:
        {
            // A strong note on "the one", syncopated 16ths, octave pops and passing notes.
            static const std::vector<const char*> rhythms {
                "x..xx.x...x.xx.x", "x.x..x.x..xx.x..", "x..x..x.xx..x.x.", "x.xx..x..x.x..xx"
            };
            const char* rhythm = pickOne (rhythms, r);
            int deg[pageSteps];
            for (int i = 0; i < pageSteps; ++i)
                deg[i] = i == 0 ? 0 : pickWeighted ({ { 0, 5 }, { 7, 3 }, { 4, 2 }, { 6, 1 }, { 2, 1 }, { 3, 1 } }, r);
            for (int s = 0; s < steps; ++s)
            {
                const int i = s % pageSteps;
                if (rhythm[i] != 'x') continue;
                line.note (s, chordAt (ctx, s) + deg[i], i == 0 || deg[i] == 7);
            }
            break;
        }

        case BassMode::Reggae:
        {
            // A round, singing line that leaves space on the downbeat of bar 1 and walks through chord tones.
            static const std::vector<std::vector<std::pair<int, int>>> shapes {
                { { 2, 0 }, { 4, 0 }, { 6, 2 }, { 8, 4 }, { 11, 2 }, { 12, 0 } },
                { { 0, 0 }, { 3, 0 }, { 6, 4 }, { 8, 7 }, { 10, 4 }, { 14, 2 } },
                { { 2, 0 }, { 6, 0 }, { 7, 2 }, { 10, 4 }, { 12, 2 }, { 14, 0 } },
            };
            const auto& shape = pickOne (shapes, r);
            for (int bar = 0; bar < steps; bar += pageSteps)
                for (size_t k = 0; k < shape.size(); ++k)
                {
                    const int s = bar + shape[k].first;
                    const int end = k + 1 < shape.size() ? bar + shape[k + 1].first - 1 : bar + pageSteps - 1;
                    line.note (s, chordAt (ctx, s) + shape[k].second);
                    line.hold (s, juce::jmin (end, s + 1), steps);
                }
            for (int s = 0; s + 1 < steps; ++s) line.slide[(size_t) s] = false;
            break;
        }

        case BassMode::Tresillo:
        {
            // 3-3-2: the Latin / Afro rhythm under reggaeton and afro house.
            static constexpr int starts[] { 0, 3, 6, 8, 11, 14 };
            for (int bar = 0; bar < steps; bar += pageSteps)
                for (int k = 0; k < 6; ++k)
                {
                    const int s = bar + starts[k];
                    const int d = k == 0 || k == 3 ? 0 : pickWeighted ({ { 0, 5 }, { 7, 2 }, { 4, 2 } }, r);
                    line.note (s, chordAt (ctx, s) + d, k == 0 || k == 3);
                    if (k < 5) line.hold (s, s + 1, steps);
                }
            for (int s = 0; s + 1 < steps; ++s) line.slide[(size_t) s] = false;
            break;
        }
    }
}

// The 303: a dense 16-step motif with accents and glides, varied a little on each bar.
void acidLine (Line& line, const GenerateContext& ctx, juce::Random& r)
{
    const float density = 0.62f + r.nextFloat() * 0.22f;
    bool on[pageSteps] {};
    int deg[pageSteps] {};
    bool acc[pageSteps] {};
    for (int i = 0; i < pageSteps; ++i)
    {
        on[i] = i == 0 || chance (r, density);
        deg[i] = i == 0 ? 0 : pickWeighted ({ { 0, 7 }, { 7, 3 }, { 2, 1 }, { 4, 1 }, { 6, 1 }, { 1, 1 }, { 9, 1 } }, r);
        acc[i] = chance (r, 0.3f);
    }

    for (int s = 0; s < ctx.steps; ++s)
    {
        const int i = s % pageSteps;
        bool o = on[i];
        int d = deg[i];
        if (s >= pageSteps && chance (r, 0.06f))
        {
            o = chance (r, 0.7f);
            d = pickWeighted ({ { 0, 3 }, { 7, 2 }, { 4, 1 }, { 6, 1 } }, r);
        }
        if (o) line.note (s, d, acc[i]);
    }
    for (int s = 0; s + 1 < ctx.steps; ++s)
        if (line.on[(size_t) s] && line.on[(size_t) s + 1] && line.degree[(size_t) s] != line.degree[(size_t) s + 1] && chance (r, 0.3f))
            line.slide[(size_t) s] = true;
}

//==============================================================================
// Leads

// A singable melody: a one-bar rhythmic motif repeated over the phrase, chord tones on the strong beats,
// small steps in between, an arch-shaped contour and a cadence that settles on the root.
// Phrase form over four bars: A, A' (the motif moved onto the next chord), B (variation), A'' + cadence.
bool isChordTone (int degree, int root)
{
    const int rel = ((degree - root) % 7 + 7) % 7;
    return rel == 0 || rel == 2 || rel == 4;
}

int nearestChordTone (int from, int root, int direction, juce::Random& r)
{
    int best = from, bestCost = 100;
    for (int d = from - 4; d <= from + 4; ++d)
    {
        if (! isChordTone (d, root)) continue;
        int cost = std::abs (d - from) * 2;
        if (direction != 0 && (d - from) * direction < 0) cost += 3;   // prefer the direction of the contour
        cost += r.nextInt (2);
        if (cost < bestCost) { bestCost = cost; best = d; }
    }
    return best;
}

void singableMelody (Line& line, const GenerateContext& ctx, juce::Random& r)
{
    const int steps = ctx.steps, bars = juce::jmax (1, steps / pageSteps);
    const bool fast = ctx.style.bpm >= 160.0f;
    static const std::vector<const char*> slowRhythms {
        "x..x..x.x...x...", "x.x...x.x.x.....", "x..x..x...x.x...", "..x.x...x..x.x..", "x...x.x...x..x..",
        "x.xx..x.x.......", "x.x.x...x..x....", "...x..x.x..x..x.",
    };
    static const std::vector<const char*> fastRhythms {
        "x.....x...x.....", "x...x...x.x.....", "x.....x.x.......", "..x...x...x.....", "x...x.....x...x.",
    };
    const char* rhythm = pickOne (fast ? fastRhythms : slowRhythms, r);

    // Motif: pitch moves for each onset of the bar, as steps relative to the previous note.
    int onsets[pageSteps], count = 0;
    for (int i = 0; i < pageSteps; ++i) if (rhythm[i] == 'x') onsets[count++] = i;
    int moves[pageSteps] {};
    for (int k = 1; k < count; ++k) moves[k] = pickWeighted ({ { 1, 3 }, { -1, 3 }, { 2, 3 }, { -2, 3 }, { 3, 2 }, { -3, 1 }, { 4, 1 }, { -4, 1 }, { 0, 1 } }, r);

    const int low = 2, high = 11;          // range in scale degrees above the tonic
    int current = 4 + r.nextInt (3);

    for (int bar = 0; bar < bars; ++bar)
    {
        const int phrasePos = bar % 4;                 // 0 = A, 1 = A', 2 = B, 3 = cadence
        const bool lastBar = bar == bars - 1;
        const int direction = phrasePos < 2 ? 1 : -1;  // rise in the first half of the phrase, fall back after
        for (int k = 0; k < count; ++k)
        {
            const int i = onsets[k], s = bar * pageSteps + i;
            if (s >= steps) break;
            const int root = chordAt (ctx, s);
            if (lastBar && i >= 8) break;              // leave room for the final long note

            int move = moves[k];
            if (phrasePos == 2 && k >= count / 2) move = -move;                         // B: the answer goes the other way
            if (phrasePos == 2 && k == count - 1 && chance (r, 0.5f)) continue;         // and breathes
            int next = current + move;
            const bool strong = i % 4 == 0 || k == 0;
            if (strong) next = nearestChordTone (next, root, direction, r);
            if (next > high) next = nearestChordTone (high - 2, root, -1, r);
            if (next < low) next = nearestChordTone (low + 2, root, 1, r);
            current = next;
            line.note (s, current, i == 0);
        }
        // Phrase endings: a long note on a chord tone (the root at the very end).
        const bool phraseEnd = phrasePos == 3 || lastBar;
        if (phraseEnd)
        {
            const int s = bar * pageSteps + 8;
            if (s < steps)
            {
                const int root = chordAt (ctx, s);
                int target = root + 7 * ((current - root) / 7);
                if (std::abs (target + 7 - current) < std::abs (target - current)) target += 7;
                target = juce::jlimit (low, high, target);
                for (int t = s; t < bar * pageSteps + pageSteps && t < steps; ++t) line.on[(size_t) t] = false;
                line.note (s, target, true);
                line.hold (s, juce::jmin (steps - 1, s + 5), steps);
                current = target;
            }
        }
    }

    // Some notes are held until the next one (legato), the others stay short.
    for (int s = 0; s < steps - 1; ++s)
    {
        if (! line.on[(size_t) s] || line.slide[(size_t) s]) continue;
        int next = s + 1;
        while (next < steps && ! line.on[(size_t) next]) ++next;
        if (next - s >= 3 && chance (r, 0.45f)) line.hold (s, juce::jmin (next - 1, s + 3), steps);
    }
}

void leadLine (Line& line, const GenerateContext& ctx, LeadMode mode, juce::Random& r)
{
    const int steps = ctx.steps, bars = steps / pageSteps;

    switch (mode)
    {
        case LeadMode::Motif:
        {
            if (bars >= 2 || chance (r, 0.6f))
            {
                singableMelody (line, ctx, r);
                break;
            }
            // Two half-bar cells: the second answers the first and resolves on a chord tone.
            static constexpr int weights[8] { 3, 1, 3, 3, 1, 2, 3, 2 };
            bool onset[8] {};
            const int count = 3 + r.nextInt (3);
            for (int n = 0; n < count; ++n)
            {
                int i = r.nextInt (8);
                for (int tries = 0; tries < 8 && (onset[i] || r.nextInt (4) >= weights[i]); ++tries) i = r.nextInt (8);
                onset[i] = true;
            }
            onset[r.nextBool() ? 0 : 2] = true;

            int cell[8] {};
            int current = pickWeighted ({ { 0, 2 }, { 2, 2 }, { 4, 2 } }, r);
            for (int i = 0; i < 8; ++i)
            {
                if (! onset[i]) continue;
                cell[i] = current;
                current = juce::jlimit (0, 9, current + pickWeighted ({ { -2, 1 }, { -1, 3 }, { 1, 3 }, { 2, 1 }, { 3, 1 } }, r));
            }
            int answer[8];
            std::copy (std::begin (cell), std::end (cell), std::begin (answer));
            for (int i = 7; i >= 0; --i)
                if (onset[i]) { answer[i] = pickWeighted ({ { 0, 3 }, { 4, 2 }, { 7, 1 }, { 2, 1 } }, r); break; }

            for (int s = 0; s < steps; ++s)
            {
                const int i = s % pageSteps, j = i % 8;
                if (! onset[j]) continue;
                line.note (s, chordAt (ctx, s) + (i < 8 ? cell[j] : answer[j]), i == 0);
            }
            if (bars > 1)
            {
                const int last = (bars - 1) * pageSteps;
                for (int s = last + 8; s < steps; ++s) line.on[(size_t) s] = false;
                line.note (last + 8, chordAt (ctx, last + 8));
                line.hold (last + 8, last + 13, steps);
            }
            break;
        }

        case LeadMode::Arp:
        {
            static const std::vector<std::vector<int>> shapes {
                { 0, 2, 4, 7 }, { 7, 4, 2, 0 }, { 0, 2, 4, 7, 4, 2 }, { 0, 4, 2, 7, 4, 9 }, { 0, 7, 4, 2 }
            };
            const auto& shape = pickOne (shapes, r);
            const bool eighths = ctx.style.bpm < 132.0f && chance (r, 0.5f);
            int index = 0;
            for (int s = 0; s < steps; ++s)
            {
                const int i = s % pageSteps;
                if (eighths && i % 2 == 1) continue;
                const int octave = i >= 8 && chance (r, 0.08f) ? 7 : 0;
                line.note (s, chordAt (ctx, s) + shape[(size_t) (index++ % (int) shape.size())] + octave, i % 4 == 0);
            }
            break;
        }

        case LeadMode::Riff:
        {
            static const std::vector<const char*> rhythms { "x.xx.x.xx.x.x.xx", "xx.x.xx.x.xx.x.x", "x..x..x..x..x.x.", "x.x.xx.x.x.xx.x." };
            const char* rhythm = pickOne (rhythms, r);
            int notes[8];
            for (auto& n : notes) n = pickWeighted ({ { 0, 5 }, { 1, 2 }, { 4, 2 }, { 5, 1 }, { 7, 2 }, { 3, 1 } }, r);
            const bool follow = ctx.style.progressions.size() > 1 && chance (r, 0.4f);
            for (int s = 0; s < steps; ++s)
            {
                const int i = s % pageSteps;
                if (rhythm[i] != 'x') continue;
                int d = notes[i % 8];
                if (i >= 12 && (s / pageSteps) % 2 == 1) d = notes[(i + 3) % 8];
                line.note (s, (follow ? chordAt (ctx, s) : 0) + d, i % 4 == 0);
            }
            break;
        }

        case LeadMode::Sparse:
        {
            static constexpr int positions[] { 3, 6, 10, 11, 14 };
            for (int bar = 0; bar < bars; bar += 2)
            {
                const int count = 1 + r.nextInt (2);
                for (int n = 0; n < count; ++n)
                {
                    const int s = (bar + n) * pageSteps + positions[r.nextInt ((int) std::size (positions))];
                    if (s >= steps || line.on[(size_t) s]) continue;
                    line.note (s, chordAt (ctx, s) + pickWeighted ({ { 0, 2 }, { 4, 2 }, { 7, 2 }, { 2, 1 } }, r));
                    line.hold (s, juce::jmin (steps - 1, s + 1 + r.nextInt (3)), steps);
                }
            }
            break;
        }
    }
}

//==============================================================================
// Chords

void chordLine (Line& line, const GenerateContext& ctx, ChordMode mode, juce::Random& r)
{
    const int steps = ctx.steps;

    if (mode == ChordMode::Pad || (mode == ChordMode::Stabs && ctx.style.chordRhythms.empty()))
    {
        const int segment = steps <= pageSteps && ctx.progression.size() > 1 ? 8 : pageSteps;
        for (int start = 0; start < steps; start += segment)
        {
            line.note (start, chordAt (ctx, start));
            line.hold (start, start + segment - 1, steps);
        }
        return;
    }

    if (mode == ChordMode::Sparse)
    {
        static constexpr int positions[] { 3, 6, 10, 11 };
        for (int bar = 0; bar < steps / pageSteps || bar == 0; bar += 2)
        {
            const int s = bar * pageSteps + positions[r.nextInt ((int) std::size (positions))];
            if (s >= steps) break;
            line.note (s, chordAt (ctx, s));
            line.hold (s, s + 1 + r.nextInt (2), steps);
            if (steps <= pageSteps) break;
        }
        return;
    }

    if (mode == ChordMode::Skank)
    {
        // Short chords on beats 2 and 4 (reggae) or on every off-beat, sometimes doubled.
        const bool everyOffbeat = chance (r, 0.4f), doubled = chance (r, 0.35f);
        for (int s = 0; s < steps; ++s)
        {
            const int i = s % pageSteps;
            const bool on = everyOffbeat ? i % 4 == 2 : (i == 4 || i == 12 || (doubled && (i == 5 || i == 13)));
            if (on) line.note (s, chordAt (ctx, s), i == 4 || i == 12);
        }
        for (int s = 0; s < steps; ++s) line.slide[(size_t) s] = false;
        return;
    }

    if (mode == ChordMode::Comping)
    {
        // Jazzy held chords on syncopated positions, a little different on each bar.
        static const std::vector<std::vector<int>> rhythms { { 0, 7 }, { 0, 6, 10 }, { 3, 10 }, { 0, 3, 10 }, { 2, 8, 14 } };
        for (int bar = 0; bar < steps; bar += pageSteps)
        {
            const auto& starts = pickOne (rhythms, r);
            for (size_t k = 0; k < starts.size(); ++k)
            {
                const int s = bar + starts[k];
                const int next = k + 1 < starts.size() ? bar + starts[k + 1] : bar + pageSteps;
                line.note (s, chordAt (ctx, s), k == 0);
                line.hold (s, juce::jmin (next - 2, s + 2 + r.nextInt (4)), steps);
            }
        }
        return;
    }

    const char* rhythm = pickOne (ctx.style.chordRhythms, r);
    for (int s = 0; s < steps; ++s)
    {
        const int i = s % pageSteps;
        bool on = rhythm[i] == 'x' || rhythm[i] == 'X';
        if (s >= pageSteps && chance (r, 0.08f)) on = ! on && i % 2 == 0 ? true : false;
        if (on) line.note (s, chordAt (ctx, s), i == 0);
    }
}

//==============================================================================
// Transformations of an existing part

bool isEssentialDrum (Role role, int step)
{
    const int i = step % pageSteps;
    if (role == Role::Kick) return i % 4 == 0;
    if (role == Role::Snare || role == Role::Clap) return i == 4 || i == 12;
    return false;
}

void varyDrums (Track& t, const MachineDef& def, int steps, GenerateMode mode, juce::Random& r)
{
    const int rows = (int) def.rows.size();
    if (mode == GenerateMode::Vary)
    {
        for (int n = 0; n < 2 + r.nextInt (2); ++n)
        {
            const int row = r.nextInt (rows);
            if (def.rows[(size_t) row].role == Role::Kick && chance (r, 0.7f)) continue;
            const int s = r.nextInt (steps);
            if (isEssentialDrum (def.rows[(size_t) row].role, s)) continue;
            t.hits[(size_t) row][(size_t) s] = ! t.hits[(size_t) row][(size_t) s];
        }
        return;
    }

    for (int row = 0; row < rows; ++row)
    {
        const Role role = def.rows[(size_t) row].role;
        bool any = false;
        for (int s = 0; s < steps; ++s) any = any || t.hits[(size_t) row][(size_t) s];

        for (int s = 0; s < steps; ++s)
        {
            auto cell = t.hits[(size_t) row].begin() + s;
            if (mode == GenerateMode::Sparser)
            {
                if (*cell && ! isEssentialDrum (role, s) && chance (r, 0.3f)) *cell = false;
            }
            else if (any && ! *cell)
            {
                const bool hat = role == Role::ClosedHat || role == Role::Shaker;
                const float p = hat ? 0.35f : (isPercussion (role) || role == Role::Ghost) ? 0.08f : 0.0f;
                if (chance (r, p)) *cell = true;
            }
        }
    }
}

void varyMelody (Track& t, int steps, int scale, GenerateMode mode, juce::Random& r)
{
    auto at = [] (auto& array, int s) -> auto& { return array[(size_t) s]; };

    if (mode == GenerateMode::Vary)
    {
        for (int n = 0; n < 2 + r.nextInt (2); ++n)
        {
            const int s = r.nextInt (steps);
            if (! at (t.on, s) || s % pageSteps == 0) continue;
            if (chance (r, 0.7f))
                at (t.note, s) = juce::jlimit (0, noteRange - 1, moveInScale (scale, at (t.note, s).load(), r.nextBool() ? 1 : -1 - r.nextInt (2)));
            else if (s + 1 < steps && ! at (t.on, s + 1))
            {
                at (t.on, s + 1) = true;
                at (t.note, s + 1) = at (t.note, s).load();
                at (t.on, s) = false;
            }
        }
        return;
    }

    for (int s = 0; s < steps; ++s)
    {
        const int prev = (s + steps - 1) % steps;
        if (mode == GenerateMode::Sparser)
        {
            if (at (t.on, s) && s % 2 == 1 && s % pageSteps != 0 && chance (r, 0.35f))
            {
                at (t.on, s) = false;
                at (t.slide, prev) = false;
            }
        }
        else if (! at (t.on, s) && at (t.on, prev) && chance (r, 0.3f))
        {
            at (t.on, s) = true;
            at (t.note, s) = juce::jlimit (0, noteRange - 1, chance (r, 0.5f) ? at (t.note, prev).load()
                                                                          : moveInScale (scale, at (t.note, prev).load(), r.nextBool() ? 1 : -1));
        }
    }
}
} // namespace

std::vector<BandMember> defaultBand (const StyleDef& style)
{
    const juce::String name (style.name);
    struct Entry { const char* genre; std::vector<BandMember> band; };
    static const std::vector<Entry> bands {
        { "Techno", { { TR909, nullptr }, { ModelD, "Basse Minimoog" }, { Juno106, "Stab house" }, { SH101, "Bleep" } } },
        { "Detroit", { { TR909, nullptr }, { ModelD, "Basse funk" }, { Juno106, "Accord deep house" }, { Strings, nullptr } } },
        { "Berlin", { { TR909, "Rumble berlinois (kick dans la reverb)" }, { ModelD, "Sub profonde" }, { Juno106, "Accord deep house" }, { SoundFx, nullptr } } },
        { "Minimal", { { TR909, "Serré et sec" }, { Percussion, nullptr }, { SH101, "Bleep" } } },
        { "Acid", { { TR909, nullptr }, { TB303, "Acid classique" }, { Juno106, "Stab house" } } },
        { "Hard Techno", { { TR909, "Hard techno (kick saturé)" }, { ModelD, "Basse Minimoog" }, { JP8000, "Lead hardstyle" } } },
        { "Dub Techno", { { TR909, "Dub (écho sur les percussions)" }, { ModelD, "Sub profonde" }, { Juno106, "Accord deep house" } } },
        { "House", { { TR909, nullptr }, { ModelD, "Basse funk" }, { Piano, "Piano house (stabs)" }, { Choir, "Grand chœur" } } },
        { "Trance", { { TR909, nullptr }, { ModelD, "Basse Minimoog" }, { JP8000, "Supersaw trance" }, { SH101, "Lead 101" } } },
        { "Psytrance", { { TR909, nullptr }, { ModelD, "Basse pluck" }, { SH101, "Lead 101" }, { JP8000, "Nappe" } } },
        { "Drum & Bass", { { BreakKit, nullptr }, { ModelD, "Reese (drum & bass)" }, { Strings, "Nappe lente (ambient)" }, { DX7, "Rhodes doux" } } },
        { "Jungle", { { BreakKit, nullptr }, { ModelD, "Sub profonde" }, { Strings, "Cordes sombres" }, { SoundFx, nullptr } } },
        { "Breakbeat", { { BreakKit, nullptr }, { ModelD, "Basse funk" }, { SH101, "Lead 101" }, { Juno106, "Stab house" } } },
        { "Électro", { { TR808, nullptr }, { SH101, "Basse caoutchouc" }, { DX7, "Lead synthé" } } },
        { "UK Garage", { { TR909, nullptr }, { ModelD, "Basse funk" }, { Organ, "Orgue house (M1)" }, { Choir, "Chœur « ooh »" } } },
        { "Gabber", { { TR909, "Hard techno (kick saturé)" }, { JP8000, "Hoover" } } },
        { "Hip-Hop", { { MPC, nullptr }, { ModelD, "Basse funk" }, { Piano, "Piano feutré (lo-fi)" }, { Mallets, "Vibraphone" } } },
        { "Trap", { { TR808, "Trap (kick long, charlestons secs)" }, { Sub808, "808 trap" }, { Mallets, "Boîte à musique (trap)" }, { Strings, "Cordes sombres" } } },
        { "Drill", { { TR808, "Trap (kick long, charlestons secs)" }, { Sub808, "808 drill (longs glissés)" }, { Mallets, "Cloches tubulaires" }, { Choir, "Chant grégorien" } } },
        { "Lo-fi", { { CR78, "Lo-fi poussiéreux" }, { ModelD, "Sub profonde" }, { DX7, "Rhodes doux" }, { Mallets, "Vibraphone" }, { SoundFx, nullptr } } },
        { "R&B", { { LinnDrum, nullptr }, { Sub808, "808 propre" }, { DX7, "Piano électrique (E.Piano 1)" }, { Strings, nullptr } } },
        { "Funk", { { LinnDrum, nullptr }, { ModelD, "Basse funk" }, { Organ, "Jazz B3" }, { Pluck, "Guitare funk (étouffée)" } } },
        { "Disco", { { LinnDrum, nullptr }, { ModelD, "Basse funk" }, { Strings, "Disco" }, { Piano, "Piano pop brillant" } } },
        { "Synthwave", { { LinnDrum, "Grande salle (reverb sur la caisse claire)" }, { ModelD, "Basse pluck" }, { Juno106, "Nappe Juno" }, { JP8000, "Lead trance" } } },
        { "Pop", { { LinnDrum, nullptr }, { ModelD, "Basse Minimoog" }, { Piano, "Piano pop brillant" }, { SH101, "Lead 101" } } },
        { "Reggaeton", { { TR808, nullptr }, { Percussion, nullptr }, { Sub808, "808 courte (reggaeton)" }, { Pluck, "Guitare reggaeton" }, { Mallets, "Marimba" } } },
        { "Afro House", { { TR909, nullptr }, { Percussion, nullptr }, { ModelD, "Sub profonde" }, { Mallets, "Kalimba" } } },
        { "Reggae", { { LinnDrum, "Dub (écho sur les percussions)" }, { ModelD, "Sub profonde" }, { Organ, "Reggae (bubble)" }, { Pluck, "Guitare funk (étouffée)" } } },
        { "Ambient", { { Strings, "Nappe lente (ambient)" }, { Choir, "Grand chœur" }, { Mallets, "Cloches tubulaires" }, { Piano, "Ballade (avec pédale)" } } },
    };
    for (auto& e : bands)
        if (name == juce::String::fromUTF8 (e.genre)) return e.band;
    return { { TR909, nullptr }, { ModelD, nullptr }, { Juno106, nullptr } };
}

const std::vector<StyleDef>& styles()
{
    static const std::vector<StyleDef> list = buildStyles();
    return list;
}

void generate (Track& track, const MachineDef& def, const GenerateContext& ctx, GenerateMode mode, juce::Random& r)
{
    if (mode != GenerateMode::NewIdea)
    {
        if (def.kind == MachineKind::Drum) varyDrums (track, def, ctx.steps, mode, r);
        else varyMelody (track, ctx.steps, ctx.scale, mode, r);
        track.tile (ctx.steps);
        return;
    }

    if (def.kind == MachineKind::Drum)
    {
        drums (track, def, ctx, r);
        // Effects: a riser leading into the loop and, with the style's crash, an impact on the first beat.
        if (def.type == SoundFx && ctx.steps > pageSteps)
        {
            if (const int rise = rowFor (def, Role::Perc); rise >= 0)
                track.hits[(size_t) rise][(size_t) (ctx.steps - pageSteps)] = true;
            track.tile (ctx.steps);
        }
        return;
    }

    Line line;
    const auto& style = ctx.style;
    switch (def.type)
    {
        case TB303:   acidLine (line, ctx, r); break;
        case ModelD:  bassLine (line, ctx, pickOne (style.bassModes, r), r); break;
        case SH101:
            if (style.leadOn101) leadLine (line, ctx, pickOne (style.leadModes, r), r);
            else bassLine (line, ctx, pickOne (style.bassModes, r), r);
            break;
        case Juno106: chordLine (line, ctx, pickOne (style.chordModes, r), r); break;
        case JP8000:
            if (style.leadOn8000) leadLine (line, ctx, pickOne (style.leadModes, r), r);
            else chordLine (line, ctx, pickOne (style.chordModes, r), r);
            break;
        case Sub808:
        {
            const auto& modes = style.bassModes;
            const bool has808 = std::find (modes.begin(), modes.end(), BassMode::Sub808) != modes.end();
            bassLine (line, ctx, has808 ? BassMode::Sub808 : pickOne (modes, r), r);
            break;
        }
        case DX7:
        case Organ:
        case Piano:   chordLine (line, ctx, pickOne (style.chordModes, r), r); break;
        case Strings:
        case Choir:   chordLine (line, ctx, ChordMode::Pad, r); break;
        case Mallets: singableMelody (line, ctx, r); break;
        case Pluck:
        {
            const auto& modes = style.chordModes;
            if (std::find (modes.begin(), modes.end(), ChordMode::Skank) != modes.end()) chordLine (line, ctx, ChordMode::Skank, r);
            else if (std::find (modes.begin(), modes.end(), ChordMode::Comping) != modes.end()) chordLine (line, ctx, ChordMode::Comping, r);
            else leadLine (line, ctx, LeadMode::Arp, r);
            break;
        }
        default: break;
    }
    line.write (track, ctx);
}

int rowForRole (const MachineDef& def, Role role) { return rowFor (def, role); }

int chordRootAt (const GenerateContext& ctx, int step) { return chordAt (ctx, step); }

void shiftTrack (Track& t, int steps, int direction)
{
    Track copy;
    for (int s = 0; s < steps; ++s)
    {
        const auto src = (size_t) s, dst = (size_t) ((s + direction + steps) % steps);
        for (size_t row = 0; row < t.hits.size(); ++row)
        {
            copy.hits[row][dst] = t.hits[row][src].load();
            copy.rolls[row][dst] = t.rolls[row][src].load();
        }
        copy.on[dst] = t.on[src].load();
        copy.accent[dst] = t.accent[src].load();
        copy.slide[dst] = t.slide[src].load();
        copy.note[dst] = t.note[src].load();
    }
    for (int s = 0; s < steps; ++s)
    {
        const auto i = (size_t) s;
        for (size_t row = 0; row < t.hits.size(); ++row)
        {
            t.hits[row][i] = copy.hits[row][i].load();
            t.rolls[row][i] = copy.rolls[row][i].load();
        }
        t.on[i] = copy.on[i].load();
        t.accent[i] = copy.accent[i].load();
        t.slide[i] = copy.slide[i].load();
        t.note[i] = copy.note[i].load();
    }
    t.tile (steps);
}
} // namespace techno
