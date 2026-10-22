#include "ExampleTools.h"

namespace techno
{
const std::vector<ExampleInfo>& examples()
{
    static const std::vector<ExampleInfo> list {
        { "Warehouse", "Techno", "Techno hypnotique : kick 909, basse qui roule, stabs d'accords dans l'écho.", 130 },
        { "Acid Rain", "Acid", "Une ligne de 303 qui miaule avec accents et glissés, sur un groove 909.", 128 },
        { "Motor City", "Detroit", "Accords chaleureux du 106, basse funky et ride : l'esprit de Detroit.", 125 },
        { "Deep Sunday", "House", "House profonde et chaloupée : Rhodes en Am9 / D9, basse ronde, vibraphone et congas.", 122 },
        { "Basement", "Berlin", "Techno sombre : kick qui gronde, rimshots dans l'écho, sub qui roule.", 133 },
        { "Euphoria", "Trance", "Nappes supersaw sur 4 accords, arpège du 101, basse sur les contretemps.", 138 },
        { "Goa Sunrise", "Psytrance", "Basse « kick-basse-basse-basse » et riff oriental qui arrive à la 3e mesure.", 145 },
        { "Night Bus", "Drum & Bass", "Breakbeat rapide, basse Reese qui gronde et nappes planantes.", 174 },
        { "Fog", "Dub Techno", "Un accord Cm7 qui se perd dans l'écho et la reverb, sub profonde.", 118 },
        { "Robot Funk", "Électro", "Rythme 808 syncopé, cowbell, basse robotique et lead froid.", 126 },
        { "Golden Era", "Hip-Hop", "Boom bap des années 90 : kit MPC swingué, piano électrique DX7 jazzy, basse ronde.", 90 },
        { "Midnight Trap", "Trap", "Charlestons en roulements, grosse basse 808 qui glisse, cloche FM et nappe de cordes.", 140 },
        { "Rainy Window", "Lo-fi", "Lo-fi tranquille : kit MPC feutré, Rhodes jazzy, guitare nylon, craquements de vinyle et pluie.", 80 },
        { "Studio 54", "Disco", "Disco : kick sur chaque temps, basse en octaves, cordes et orgue sur les contretemps.", 120 },
        { "Perreo", "Reggaeton", "Le dembow à la 808, percussions latines, basse 808 sur le tresillo et marimba FM.", 95 },
        { "Piano Paradise", "House", "Piano house des années 90 : stabs de piano, chœurs « aah », basse sur les contretemps, riser et impact.", 124 },
        { "Click & Cut", "Minimal", "Minimale berlinoise : rimshots dans l'écho, percussions qui rebondissent, bleeps du 101, cymbale à l'envers.", 125 },
        { "Steel Mill", "Hard Techno", "Kick saturé à 150 BPM, stabs en quintes du 8000, impact et riser.", 150 },
        { "Rainforest", "Jungle", "Amen break découpé, sub profonde, nappes et chœur, rewind de sound system.", 165 },
        { "Big Beat", "Breakbeat", "Breakbeat gras, hoover, lead du 101 et scratch.", 132 },
        { "Late Train", "UK Garage", "2-step très chaloupé, orgue house, basse qui glisse et chœur « ooh ».", 132 },
        { "Rotterdam", "Gabber", "Kick gabber à 185 BPM et hoover qui hurle.", 185 },
        { "Southside", "Drill", "Hi-hats en triolets, 808 qui glisse, piano sombre et chant grégorien.", 142 },
        { "Velvet", "R&B", "Groove lent, Rhodes en accords de 9e, basse 808 douce, charlestons en roulements.", 90 },
        { "Get On Up", "Funk", "Batterie avec ghost notes, basse slap, clavinet, guitare funk et section de cuivres.", 105 },
        { "Golden Hour", "Pop", "Grille I-V-vi-IV, batterie acoustique, piano, basse au médiator et mélodie qui chante.", 110 },
        { "Kalahari", "Afro House", "Congas, agogo, talking drum, basse sur le tresillo et kalimba.", 122 },
        { "One Drop", "Reggae", "One drop, basse ronde, skank de guitare, orgue bubble et sirène dub.", 76 },
        { "Northern Lights", "Ambient", "Nappes de cordes et de chœur, crotales, flûtes de mellotron et pluie.", 80 },
        { "Neon Drive", "Synthwave", "Années 80 : LinnDrum à grosse snare, basse qui pulse, nappes de cordes et lead nostalgique.", 100 },
        { "Bongo Lab", "Minimal", "Polyrythmies : bongos West Coast sur 5 pas et bips FM sur 3 contre un kick de 16, écho à bande.", 122 },
        { "Pixel Quest", "Électro", "Jeu vidéo : arpèges de la puce 8-bit, 606 et basse de l'Odyssey qui crie.", 150 },
        { "Robot Disco", "Disco", "French touch : le vocodeur chante les accords dans un phaser, basse en octaves, reverb plate.", 118 },
        { "Tears in Rain", "Ambient", "Cinéma des années 80 : cuivres du CS-80 dans un grand hall, nappe de voyelles en wavetable.", 72 },
        { "Euclid Garden", "Techno", "Pistes de longueurs différentes, charleys en triolets, pas au hasard et clap un tour sur deux.", 126 },
        { "Arp Horizon", "Trance", "Arpégiateur du Juno sur deux octaves, supersaw et vrai sidechain qui fait respirer les synthés.", 138 },
    };
    return list;
}

void buildExample (int index, ExampleWriter& w)
{
    switch (index)
    {
        case 0: demos::techno (w); break;
        case 1: demos::acid (w); break;
        case 2: demos::detroit (w); break;
        case 3: demos::house (w); break;
        case 4: demos::berlin (w); break;
        case 5: demos::trance (w); break;
        case 6: demos::psytrance (w); break;
        case 7: demos::drumAndBass (w); break;
        case 8: demos::dubTechno (w); break;
        case 9: demos::electro (w); break;
        case 10: demos::boomBap (w); break;
        case 11: demos::trap (w); break;
        case 12: demos::lofi (w); break;
        case 13: demos::disco (w); break;
        case 14: demos::reggaeton (w); break;
        case 15: demos::pianoHouse (w); break;
        case 16: demos::minimal (w); break;
        case 17: demos::hardTechno (w); break;
        case 18: demos::jungle (w); break;
        case 19: demos::breakbeat (w); break;
        case 20: demos::ukGarage (w); break;
        case 21: demos::gabber (w); break;
        case 22: demos::drill (w); break;
        case 23: demos::rnb (w); break;
        case 24: demos::funk (w); break;
        case 25: demos::pop (w); break;
        case 26: demos::afroHouse (w); break;
        case 27: demos::reggae (w); break;
        case 28: demos::ambient (w); break;
        case 29: demos::synthwave (w); break;
        case 30: demos::bongoLab (w); break;
        case 31: demos::pixelQuest (w); break;
        case 32: demos::robotDisco (w); break;
        case 33: demos::tearsInRain (w); break;
        case 34: demos::euclidGarden (w); break;
        case 35: demos::arpHorizon (w); break;
        default: break;
    }
}
} // namespace techno
