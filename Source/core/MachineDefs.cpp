#include "MachineDefs.h"
#include "Music.h"

namespace techno
{
namespace
{
const std::vector<const char*> octaves { "-2", "-1", "0", "+1", "+2" };

std::vector<const char*> chordChoices()
{
    return { std::begin (chordTypeNames), std::end (chordTypeNames) };
}

const char* const octaveHelp =
    "Décale toute la machine d'une ou deux octaves vers le grave (-) ou l'aigu (+).\n\n"
    "Comment l'utiliser : mets -1 pour une basse plus profonde, +1 pour un lead qui ressort au-dessus du reste.";

const char* const cutoffHelp =
    "Le filtre coupe les fréquences aiguës. Tourné à gauche, le son est sourd et étouffé ; à droite, il est brillant et ouvert.\n\n"
    "C'est LE bouton le plus expressif d'un synthé : tourne-le doucement pendant que ça joue pour faire monter la tension.";

const char* const resoHelp =
    "La résonance accentue les fréquences juste autour du point de coupure du filtre. Elle donne ce côté « miaulant » ou sifflant.\n\n"
    "Peu = son rond. Beaucoup = son nasillard et acide. Combine-la avec le filtre pour les effets « wouah ».";

const char* const envHelp =
    "Quantité d'enveloppe envoyée au filtre : à chaque note, le filtre s'ouvre puis se referme.\n\n"
    "Monte-le pour un son qui « claque » au début de chaque note (effet « wah »). À zéro, le filtre reste fixe.";

const char* const attackHelp =
    "Temps que met le son à monter au début de la note.\n\n"
    "Court = attaque nette (basses, stabs). Long = le son apparaît doucement (nappes, ambiances).";

const char* const decayHelp =
    "Temps que met le son à redescendre après l'attaque, jusqu'au niveau de maintien (Sustain).\n\n"
    "Court = notes sèches et percussives. Long = notes qui durent.";

const char* const sustainHelp =
    "Niveau auquel le son reste tant que la note est tenue.\n\n"
    "À zéro : chaque note est une impulsion courte. Au maximum : le son reste à fond tant que la note dure.";

const char* const releaseHelp =
    "Temps que met le son à s'éteindre une fois la note relâchée.\n\n"
    "Court = coupure nette. Long = le son traîne et se fond dans le suivant (idéal pour les nappes).";

const char* const chordHelp =
    "Type d'accord joué à chaque pas.\n\n"
    "« Auto (gamme) » construit automatiquement un accord qui sonne juste avec la gamme choisie : le plus simple. "
    "« Note seule » joue une seule note (pour un lead). Les autres forcent un type précis : Mineur = sombre, "
    "Majeur = joyeux, 7e et 9e = plus riche et jazzy, Sus = suspendu et ouvert, Quinte = puissant.";

DrumRowDef row (const char* id, const char* label, Role role, float level, float decay, float tone,
                const char* help, const char* toneHelp)
{
    return { id, label, role, level, 0.5f, decay, tone, help, toneHelp };
}

std::vector<MachineDef> buildDefs()
{
    std::vector<MachineDef> defs;

    defs.push_back ({ TR909, "tr909", "909 DRUMS", "Boîte à rythmes · inspirée de la Roland TR-909 (1983)", MachineKind::Drum,
        0xffff6a2b, 0,
        "LA boîte à rythmes de la techno et de la house. Son kick percutant, ses charlestons métalliques et son clap "
        "ont défini le son des clubs depuis Detroit et Chicago jusqu'à Berlin.\n\n"
        "Ici, tout est recréé par synthèse comme dans la machine d'origine (circuits analogiques pour le kick, la snare, "
        "les toms, le rim et le clap ; grain 6 bits pour les charlestons et cymbales, comme les samples de l'époque).\n\n"
        "Pour commencer : clique les cases 1, 5, 9 et 13 de la rangée KICK, ajoute un CLAP sur 5 et 13, un OPEN HAT sur 3, 7, 11, 15. "
        "Tu as un beat techno !",
        {
            row ("bd", "KICK", Role::Kick, 0.9f, 0.45f, 0.5f,
                 "La grosse caisse, le cœur de la techno. Le kick 909 est court, sec et percutant avec un « clic » d'attaque.\n\n"
                 "Classique : un kick sur chaque temps (cases 1, 5, 9, 13) = le « four on the floor ».",
                 "Clic d'attaque : plus il est haut, plus le kick est percutant et ressort dans le mix."),
            row ("sd", "SNARE", Role::Snare, 0.6f, 0.4f, 0.55f,
                 "La caisse claire : un mélange de tonalité et de bruit (le « timbre »).\n\n"
                 "Sur les temps 2 et 4 (cases 5 et 13) ou en roulements pour faire monter la tension.",
                 "Snappy : quantité de bruit (le grésillement du timbre). Haut = snare claquante, bas = plus ronde."),
            row ("lt", "LOW TOM", Role::LowTom, 0.55f, 0.45f, 0.3f,
                 "Tom grave, avec la chute de hauteur caractéristique de la 909.\n\n"
                 "Parfait pour les grooves tribaux et les roulements de fin de mesure.",
                 "Quantité de bruit dans le tom : plus haut = plus d'attaque et de souffle."),
            row ("mt", "MID TOM", Role::MidTom, 0.55f, 0.45f, 0.3f,
                 "Tom médium. Combine-le avec les deux autres pour des descentes de toms.",
                 "Quantité de bruit dans le tom : plus haut = plus d'attaque et de souffle."),
            row ("ht", "HI TOM", Role::HighTom, 0.55f, 0.4f, 0.3f,
                 "Tom aigu. Pratique aussi comme petite percussion syncopée.",
                 "Quantité de bruit dans le tom : plus haut = plus d'attaque et de souffle."),
            row ("rs", "RIM", Role::Rim, 0.5f, 0.3f, 0.5f,
                 "Le rimshot : un « tic » sec et boisé. Très utilisé pour les syncopes en techno minimale et en Berlin.",
                 "Brillance : plus haut = tic plus aigu et fin."),
            row ("cp", "CLAP", Role::Clap, 0.6f, 0.4f, 0.5f,
                 "Le fameux clap 909 : plusieurs claquements très rapprochés suivis d'une petite réverbération.\n\n"
                 "Sur les temps 2 et 4 (cases 5 et 13) : le classique absolu.",
                 "Couleur du clap : plus haut = plus aigu et plus serré."),
            row ("ch", "HAT", Role::ClosedHat, 0.5f, 0.35f, 0.5f,
                 "Charleston fermé : un « tss » court et métallique. Il donne la vitesse et l'énergie.\n\n"
                 "En doubles croches (toutes les cases) ou une case sur deux. Il coupe l'open hat quand il joue (comme un vrai charleston).",
                 "Brillance : plus haut = plus aigu et cristallin."),
            row ("oh", "OPEN HAT", Role::OpenHat, 0.45f, 0.4f, 0.5f,
                 "Charleston ouvert : un « tsss » plus long. C'est lui qui fait le groove techno quand il est entre les kicks.\n\n"
                 "Classique : cases 3, 7, 11, 15 (les contretemps).",
                 "Brillance : plus haut = plus aigu et cristallin."),
            row ("cr", "CRASH", Role::Crash, 0.45f, 0.6f, 0.5f,
                 "Cymbale crash : un grand « pschhh » pour marquer le début d'une nouvelle partie.\n\n"
                 "Mets-en une seule sur la première case d'une séquence de 4 mesures.",
                 "Brillance : plus haut = plus aigu et agressif."),
            row ("rd", "RIDE", Role::Ride, 0.4f, 0.55f, 0.5f,
                 "Cymbale ride : un « ding » métallique qui résonne. Elle porte les montées et les passages intenses.\n\n"
                 "Sur les contretemps ou en croches régulières.",
                 "Brillance : plus haut = plus aigu et cristallin."),
        },
        {} });

    defs.push_back ({ TR808, "tr808", "808 DRUMS", "Boîte à rythmes · inspirée de la Roland TR-808 (1980)", MachineKind::Drum,
        0xffff4d6d, 0,
        "La boîte à rythmes la plus célèbre du monde. Son kick grave et long (le « boom »), sa cowbell et ses claves "
        "ont fondé l'électro, le hip-hop, la Miami bass et beaucoup de techno de Detroit.\n\n"
        "Recréée ici par synthèse analogique comme l'originale : chaque son est un circuit (oscillateur résonant, bruit, "
        "6 oscillateurs carrés pour les cymbales).\n\n"
        "Astuce : allonge la DURÉE du kick pour un boom de basse profond, idéal en électro.",
        {
            row ("bd", "KICK", Role::Kick, 0.9f, 0.5f, 0.4f,
                 "Le kick 808 : un « boom » rond et grave qui peut durer longtemps. Avec une durée longue, il remplace même la basse.",
                 "Clic d'attaque : plus haut = début plus net et audible sur de petites enceintes."),
            row ("sd", "SNARE", Role::Snare, 0.6f, 0.35f, 0.5f,
                 "La caisse claire 808 : plus légère et électronique que la 909. Typique de l'électro.",
                 "Snappy : quantité de bruit. Haut = claquante, bas = plus tonale."),
            row ("lt", "LOW TOM", Role::LowTom, 0.55f, 0.5f, 0.2f,
                 "Tom grave 808, très rond. Aussi utilisé comme conga grave.",
                 "Quantité de souffle dans le tom."),
            row ("mt", "MID TOM", Role::MidTom, 0.55f, 0.5f, 0.2f,
                 "Tom médium 808.",
                 "Quantité de souffle dans le tom."),
            row ("ht", "HI TOM", Role::HighTom, 0.55f, 0.45f, 0.2f,
                 "Tom aigu 808. Parfait pour des petites mélodies percussives.",
                 "Quantité de souffle dans le tom."),
            row ("rs", "RIM", Role::Rim, 0.5f, 0.3f, 0.5f,
                 "Rimshot 808 : un tic court et sec.",
                 "Brillance du tic."),
            row ("cp", "CLAP", Role::Clap, 0.6f, 0.4f, 0.5f,
                 "Le clap 808 : plusieurs claquements + une queue de réverbération. Plus doux que celui de la 909.",
                 "Couleur du clap : plus haut = plus aigu."),
            row ("cb", "COWBELL", Role::Cowbell, 0.45f, 0.35f, 0.5f,
                 "La cloche à vache 808, légendaire en électro et en Detroit techno. Deux oscillateurs carrés filtrés.\n\n"
                 "Mets-en peu, sur des contretemps, pour un groove funky.",
                 "Couleur : plus haut = plus brillante."),
            row ("cy", "CYMBAL", Role::Cymbal, 0.4f, 0.55f, 0.5f,
                 "La cymbale 808 : un souffle métallique long et doux. Pour marquer un début de partie.",
                 "Équilibre grave/aigu de la cymbale."),
            row ("ch", "HAT", Role::ClosedHat, 0.5f, 0.3f, 0.5f,
                 "Charleston fermé 808 : fin et métallique. Il coupe l'open hat quand il joue.",
                 "Brillance : plus haut = plus aigu."),
            row ("oh", "OPEN HAT", Role::OpenHat, 0.45f, 0.4f, 0.5f,
                 "Charleston ouvert 808. Classique sur les contretemps (cases 3, 7, 11, 15).",
                 "Brillance : plus haut = plus aigu."),
            row ("cl", "CLAVES", Role::Claves, 0.45f, 0.3f, 0.5f,
                 "Les claves : un « toc » boisé et aigu. Parfait pour des rythmes latins et électro syncopés.",
                 "Brillance du toc."),
            row ("ma", "MARACAS", Role::Shaker, 0.4f, 0.3f, 0.5f,
                 "Les maracas : un petit « tchk » de bruit. Remplit le groove en doubles croches discrètes.",
                 "Brillance : plus haut = plus fin et aigu."),
        },
        {} });

    defs.push_back ({ BreakKit, "breaks", "BREAK KIT", "Batterie « breakbeat » · son de batterie samplée façon SP-1200", MachineKind::Drum,
        0xff7bd88f, 0,
        "Une batterie acoustique synthétisée avec le grain des samplers des années 90 (E-mu SP-1200, Akai MPC). "
        "C'est le son des breakbeats découpés de la jungle, de la drum & bass et du breakbeat.\n\n"
        "Le bouton CRUSH (dans les effets) est déjà monté pour donner ce côté « samplé » 12 bits. "
        "Pour un vrai son jungle, choisis le style Jungle ou Drum & Bass : les rythmes typiques sont générés automatiquement.",
        {
            row ("bd", "KICK", Role::Kick, 0.85f, 0.4f, 0.5f,
                 "Grosse caisse acoustique : ronde, avec le claquement de la batte.",
                 "Claquement de la batte : plus haut = plus d'attaque."),
            row ("sd", "SNARE", Role::Snare, 0.7f, 0.45f, 0.55f,
                 "Caisse claire acoustique, grosse et claquante. En drum & bass : cases 5 et 13.",
                 "Snappy : quantité de timbre (grésillement)."),
            row ("gs", "GHOST", Role::Ghost, 0.35f, 0.25f, 0.5f,
                 "Ghost notes : des petits coups de caisse claire très légers entre les gros. Ils donnent le « rebond » des breaks.\n\n"
                 "Saupoudre-en quelques-uns entre les snares.",
                 "Snappy : quantité de timbre (grésillement)."),
            row ("ch", "HAT", Role::ClosedHat, 0.45f, 0.35f, 0.5f,
                 "Charleston fermé acoustique. Coupe l'open hat quand il joue.",
                 "Brillance : plus haut = plus aigu."),
            row ("oh", "OPEN HAT", Role::OpenHat, 0.4f, 0.4f, 0.5f,
                 "Charleston ouvert acoustique.",
                 "Brillance : plus haut = plus aigu."),
            row ("rd", "RIDE", Role::Ride, 0.35f, 0.5f, 0.5f,
                 "Cymbale ride : typique des breaks jungle, en croches régulières.",
                 "Brillance : plus haut = plus aigu."),
            row ("cr", "CRASH", Role::Crash, 0.4f, 0.6f, 0.5f,
                 "Cymbale crash pour marquer le début d'une partie.",
                 "Brillance : plus haut = plus aigu."),
        },
        {} });

    defs.push_back ({ TB303, "tb303", "303 ACID", "Basse acid · inspirée de la Roland TB-303 (1981)", MachineKind::Mono,
        0xfff5d442, 0,
        "La machine qui a inventé l'acid house. Un seul oscillateur, un filtre résonant qui « miaule », un accent et un slide (glissé entre deux notes). "
        "Ratée à sa sortie, elle est devenue une légende en 1987 quand les DJ de Chicago l'ont détournée.\n\n"
        "Le secret de l'acid : fais jouer une boucle, puis tourne doucement FILTRE et RÉSO pendant que ça tourne. "
        "Ajoute des ACCENTS (notes qui claquent) et des SLIDES (glissés).\n\n"
        "Rangées : NOTES (clique pour activer, glisse vers le haut/bas pour changer la note), ACCENT, SLIDE.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "wave", "ONDE", 0.0f,
              "Forme d'onde de l'oscillateur. SAW (dent de scie) = son brillant et agressif, le classique acid. "
              "SQUARE (carré) = son plus creux et rond.", { "SAW", "SQUARE" } },
            { "cutoff", "FILTRE", 0.3f, cutoffHelp, {} },
            { "reso", "RÉSO", 0.72f, resoHelp, {} },
            { "env", "ENV MOD", 0.55f,
              "Profondeur de l'enveloppe sur le filtre : à chaque note, le filtre s'ouvre puis se referme. "
              "C'est ce qui fait le « wow-wow » de l'acid. Monte-le avec la résonance pour un son très acide.", {} },
            { "decay", "DECAY", 0.35f,
              "Durée de l'ouverture du filtre à chaque note. Court = notes sèches et pincées. Long = notes qui « miaulent » plus longtemps.", {} },
            { "accent", "ACCENT", 0.6f,
              "Force des notes accentuées (rangée ACCENT) : elles sont plus fortes et plus brillantes, avec le fameux "
              "effet qui s'accumule quand plusieurs accents se suivent.", {} },
        } });

    defs.push_back ({ SH101, "sh101", "101 MONO", "Synthé monophonique · inspiré du Roland SH-101 (1982)", MachineKind::Mono,
        0xff4fa3ff, 12,
        "Le petit synthé monophonique préféré de la techno, de l'acid et de la psytrance. Un oscillateur avec dent de scie, "
        "carré (avec PWM), sub-oscillateur et bruit mélangeables, un filtre 24 dB qui peut siffler, et une enveloppe ADSR.\n\n"
        "Il fait des basses rondes, des leads perçants, des arpèges et des bleeps.\n\n"
        "Rangées : NOTES, ACCENT (note plus forte et brillante), SLIDE (glissé vers la note suivante).",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "saw", "SAW", 0.8f, "Volume de l'onde en dent de scie : brillante et riche. La base de la plupart des sons.", {} },
            { "square", "SQUARE", 0.0f, "Volume de l'onde carrée : plus creuse, façon jeu vidéo. Avec la PWM elle devient vivante et chorussée.", {} },
            { "sub", "SUB", 0.5f, "Volume du sub-oscillateur, une octave en dessous : ajoute du poids et du grave. Indispensable pour les basses.", {} },
            { "noise", "BRUIT", 0.0f, "Volume du bruit blanc. Un peu = souffle et texture ; beaucoup = effets, percussions, vent.", {} },
            { "pwm", "PWM", 0.0f, "Modulation de largeur d'impulsion : fait bouger l'onde carrée en continu pour un son épais et mouvant. Agit seulement si SQUARE est monté.", {} },
            { "cutoff", "FILTRE", 0.45f, cutoffHelp, {} },
            { "reso", "RÉSO", 0.3f, resoHelp, {} },
            { "env", "ENV", 0.5f, envHelp, {} },
            { "attack", "ATTACK", 0.0f, attackHelp, {} },
            { "decay", "DECAY", 0.35f, decayHelp, {} },
            { "sustain", "SUSTAIN", 0.2f, sustainHelp, {} },
            { "release", "RELEASE", 0.15f, releaseHelp, {} },
            { "glide", "GLIDE", 0.2f, "Durée du glissé entre deux notes reliées par un SLIDE. Court = glissé rapide, long = glissé lent et élastique.", {} },
        } });

    defs.push_back ({ ModelD, "modeld", "D-BASS", "Grosse basse · inspirée du Minimoog Model D (1970)", MachineKind::Mono,
        0xffd9a066, 0,
        "Le synthé des basses énormes. Trois oscillateurs, le célèbre filtre « ladder » 24 dB au son chaud et rond, "
        "et une saturation quand on le pousse.\n\n"
        "Parfait pour les sub-basses de techno, les basses rondes de house, et la basse « Reese » de la drum & bass "
        "(monte DÉSACCORD pour que les oscillateurs se frottent et créent ce grondement qui bouge).\n\n"
        "Rangées : NOTES, ACCENT, SLIDE (glissé).",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "wave", "ONDE", 0.0f,
              "Forme d'onde des oscillateurs. SAW = riche et brillante (Reese, basses agressives). SQUARE = creuse et épaisse. "
              "TRIANGLE = douce et ronde, presque une sub-basse pure.", { "SAW", "SQUARE", "TRIANGLE" } },
            { "detune", "DÉSACCORD", 0.15f,
              "Écart de justesse entre les oscillateurs. Un peu = son épais et vivant. Beaucoup = le grondement « Reese » de la drum & bass.", {} },
            { "sub", "SUB", 0.4f, "Volume du 3e oscillateur, une octave plus bas : ajoute du poids et de la profondeur.", {} },
            { "cutoff", "FILTRE", 0.35f, cutoffHelp, {} },
            { "emphasis", "EMPHASIS", 0.25f, resoHelp, {} },
            { "contour", "CONTOUR", 0.5f, envHelp, {} },
            { "attack", "ATTACK", 0.0f, attackHelp, {} },
            { "decay", "DECAY", 0.4f, decayHelp, {} },
            { "sustain", "SUSTAIN", 0.5f, sustainHelp, {} },
            { "glide", "GLIDE", 0.1f, "Durée du glissé entre deux notes reliées par un SLIDE.", {} },
            { "overdrive", "DRIVE", 0.3f,
              "Pousse le signal dans le filtre pour le faire saturer, comme quand on monte les volumes du mixeur du Minimoog. "
              "Plus = basse plus grasse et agressive.", {} },
        } });

    defs.push_back ({ Juno106, "juno106", "106 POLY", "Synthé polyphonique · inspiré du Roland Juno-106 (1984)", MachineKind::Poly,
        0xff5ac8fa, 24,
        "LE synthé des accords et des nappes : son chaud, épais, avec son fameux chorus stéréo qui l'élargit. "
        "Utilisé partout : Detroit techno, house, dub techno, deep house.\n\n"
        "Chaque pas joue un ACCORD complet (choisis le type avec ACCORD). Glisse une case vers le haut/bas pour changer l'accord.\n\n"
        "Rangées : ACCORDS et TENIR (garde l'accord enfoncé jusqu'au pas suivant, pour des nappes longues).",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "saw", "SAW", 0.7f, "Volume de la dent de scie : brillante, la base des nappes et des stabs.", {} },
            { "pulse", "PULSE", 0.4f, "Volume de l'onde carrée : plus creuse. Avec PWM, elle donne ce son mouvant typique du Juno.", {} },
            { "sub", "SUB", 0.3f, "Sub-oscillateur une octave en dessous : ajoute du corps aux accords.", {} },
            { "noise", "BRUIT", 0.0f, "Bruit blanc : un peu de souffle dans le son.", {} },
            { "pwm", "PWM", 0.3f, "Modulation lente de la largeur de l'onde carrée : rend le son vivant et épais.", {} },
            { "hpf", "HPF", 1.0f,
              "Le filtre passe-haut du Juno (4 positions). 0 = basses renforcées, 1 = neutre, 2 et 3 = coupe de plus en plus les basses "
              "pour des accords légers qui ne gênent pas la ligne de basse.", { "0", "1", "2", "3" } },
            { "cutoff", "FILTRE", 0.55f, cutoffHelp, {} },
            { "reso", "RÉSO", 0.2f, resoHelp, {} },
            { "env", "ENV", 0.35f, envHelp, {} },
            { "attack", "ATTACK", 0.0f, attackHelp, {} },
            { "decay", "DECAY", 0.4f, decayHelp, {} },
            { "sustain", "SUSTAIN", 0.5f, sustainHelp, {} },
            { "release", "RELEASE", 0.35f, releaseHelp, {} },
            { "chorus", "CHORUS", 1.0f,
              "Le chorus stéréo légendaire du Juno : double le son avec un léger décalage qui bouge, pour l'élargir et le rendre soyeux. "
              "I = doux, II = plus intense, I+II = vibrato rapide.", { "OFF", "I", "II", "I+II" } },
        } });

    defs.push_back ({ JP8000, "jp8000", "8000 SUPERSAW", "Synthé trance · inspiré du Roland JP-8000 (1996)", MachineKind::Poly,
        0xffff7ad9, 24,
        "Le synthé qui a créé le son de la trance avec son « Supersaw » : 7 dents de scie légèrement désaccordées qui sonnent "
        "comme un mur de son énorme et brillant.\n\n"
        "En accords (ACCORD = Auto) pour les nappes et les grosses montées trance ; en note seule pour des leads perçants "
        "(psytrance, hard techno, gabber).\n\n"
        "Rangées : NOTES/ACCORDS et TENIR (garde le son jusqu'au pas suivant).",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "detune", "DÉSACCORD", 0.4f,
              "Écart entre les 7 dents de scie. Peu = son fin et propre. Beaucoup = mur de son épais et large, typique de la trance.", {} },
            { "mix", "MIX", 0.6f,
              "Volume des 6 dents de scie désaccordées par rapport à la centrale. Plus = plus large et plus « supersaw ».", {} },
            { "cutoff", "FILTRE", 0.6f, cutoffHelp, {} },
            { "reso", "RÉSO", 0.15f, resoHelp, {} },
            { "env", "ENV", 0.3f, envHelp, {} },
            { "attack", "ATTACK", 0.05f, attackHelp, {} },
            { "decay", "DECAY", 0.5f, decayHelp, {} },
            { "sustain", "SUSTAIN", 0.7f, sustainHelp, {} },
            { "release", "RELEASE", 0.4f, releaseHelp, {} },
        } });

    defs.push_back ({ LinnDrum, "linn", "LINN DRUMS", "Boîte à rythmes · inspirée de la LinnDrum (1982)", MachineKind::Drum,
        0xffe0453a, 0,
        "La boîte à rythmes à samples des années 80 : de vrais sons de batterie enregistrés en 8 bits. C'est le son de la pop, "
        "de la funk, du R&B et du hip-hop de l'époque (Prince, Michael Jackson, Herbie Hancock).\n\n"
        "Snare claquante, sidestick, tambourin, congas, toms ronds : parfaite pour des grooves funky et de la synthwave.",
        {
            row ("bd", "KICK", Role::Kick, 0.85f, 0.4f, 0.55f, "Grosse caisse 80s : courte, ronde et claquante.", "Claquement de la batte."),
            row ("sd", "SNARE", Role::Snare, 0.7f, 0.45f, 0.6f, "La snare 80s, grosse et brillante. Sur 2 et 4.", "Snappy : quantité de timbre."),
            row ("ss", "SIDESTICK", Role::Rim, 0.5f, 0.35f, 0.5f, "Le « clic » de la baguette posée sur le bord de la caisse claire. Typique des ballades et du R&B.", "Brillance du clic."),
            row ("cp", "CLAP", Role::Clap, 0.6f, 0.35f, 0.55f, "Clap 80s serré, souvent doublé avec la snare.", "Couleur du clap."),
            row ("ch", "HAT", Role::ClosedHat, 0.45f, 0.35f, 0.5f, "Charleston fermé, grain 8 bits.", "Brillance."),
            row ("oh", "OPEN HAT", Role::OpenHat, 0.4f, 0.4f, 0.5f, "Charleston ouvert, grain 8 bits.", "Brillance."),
            row ("lt", "TOM LOW", Role::LowTom, 0.55f, 0.45f, 0.4f, "Tom grave, rond : parfait pour les roulements de fin.", "Attaque de la peau."),
            row ("ht", "TOM HIGH", Role::HighTom, 0.55f, 0.4f, 0.4f, "Tom aigu.", "Attaque de la peau."),
            row ("cb", "COWBELL", Role::Cowbell, 0.4f, 0.3f, 0.6f, "Cowbell funky.", "Couleur de la cloche."),
            row ("tb", "TAMBOURINE", Role::Tambourine, 0.4f, 0.4f, 0.5f, "Tambourin : sur 2 et 4 ou en doubles croches, il donne de l'énergie à la pop et à la soul.", "Écart entre les deux secousses."),
            row ("cg", "CONGA", Role::LowConga, 0.5f, 0.4f, 0.5f, "Conga : ajoute un groove latin.", "Claque de la main."),
            row ("cr", "CRASH", Role::Crash, 0.4f, 0.6f, 0.5f, "Cymbale crash 8 bits.", "Brillance."),
            row ("rd", "RIDE", Role::Ride, 0.35f, 0.55f, 0.5f, "Cymbale ride 8 bits.", "Brillance."),
        },
        {} });

    defs.push_back ({ CR78, "cr78", "CR-78", "Boîte à rythmes vintage · inspirée de la Roland CR-78 (1978)", MachineKind::Drum,
        0xffc8702e, 0,
        "Une des premières boîtes à rythmes programmables : des sons analogiques doux et feutrés, avec plein de percussions "
        "(guiro, bongos, maracas, claves). Son très reconnaissable chez Phil Collins, Blondie ou Hall & Oates.\n\n"
        "Idéale pour des rythmes rétro, de la pop douce, du lo-fi ou des grooves latins discrets.",
        {
            row ("bd", "KICK", Role::Kick, 0.85f, 0.3f, 0.3f, "Kick doux et court, très feutré.", "Clic d'attaque."),
            row ("sd", "SNARE", Role::Snare, 0.6f, 0.3f, 0.4f, "Snare légère et analogique.", "Snappy."),
            row ("rs", "RIM", Role::Rim, 0.5f, 0.3f, 0.5f, "Rimshot sec.", "Brillance."),
            row ("ch", "HAT", Role::ClosedHat, 0.45f, 0.3f, 0.5f, "Charleston fin et métallique.", "Brillance."),
            row ("cy", "CYMBAL", Role::Cymbal, 0.4f, 0.5f, 0.5f, "Cymbale douce.", "Grave/aigu."),
            row ("cl", "CLAVES", Role::Claves, 0.45f, 0.3f, 0.5f, "Claves : petit « toc » boisé.", "Brillance."),
            row ("ma", "MARACAS", Role::Shaker, 0.4f, 0.3f, 0.5f, "Maracas.", "Brillance."),
            row ("cb", "COWBELL", Role::Cowbell, 0.4f, 0.3f, 0.7f, "Cowbell aiguë.", "Couleur."),
            row ("gu", "GUIRO", Role::Guiro, 0.45f, 0.4f, 0.5f, "Guiro : le « rrrip » râpé des musiques latines.", "Couleur du grattement."),
            row ("bl", "BONGO LOW", Role::LowBongo, 0.5f, 0.35f, 0.4f, "Bongo grave.", "Claque de la main."),
            row ("bh", "BONGO HIGH", Role::HighBongo, 0.5f, 0.3f, 0.4f, "Bongo aigu.", "Claque de la main."),
            row ("cg", "CONGA", Role::LowConga, 0.5f, 0.4f, 0.4f, "Conga.", "Claque de la main."),
            row ("tb", "TAMBOURINE", Role::Tambourine, 0.4f, 0.35f, 0.5f, "Tambourin.", "Écart entre les secousses."),
        },
        {} });

    defs.push_back ({ MPC, "mpc", "MPC KIT", "Kit hip-hop · son des samplers MPC / SP-1200", MachineKind::Drum,
        0xffd23b2f, 0,
        "Le kit du hip-hop boom bap : kick épais, snare qui claque, finger snap, charlestons poussiéreux, le tout avec le grain "
        "12 bits des samplers qui ont fait le rap des années 90 (Pete Rock, DJ Premier, J Dilla).\n\n"
        "Mets un peu de swing (50-60 %) : c'est le secret du groove MPC.",
        {
            row ("bd", "KICK", Role::Kick, 0.9f, 0.45f, 0.5f, "Kick boom bap : rond, épais et saturé.", "Claquement d'attaque."),
            row ("sd", "SNARE", Role::Snare, 0.75f, 0.45f, 0.55f, "Snare qui claque avec un peu de pièce autour.", "Snappy : quantité de timbre."),
            row ("sn", "SNAP", Role::Snap, 0.55f, 0.3f, 0.5f, "Claquement de doigts : très utilisé en trap, R&B et hip-hop, seul ou avec la snare.", "Couleur du claquement."),
            row ("rs", "RIM", Role::Rim, 0.45f, 0.3f, 0.5f, "Rimshot.", "Brillance."),
            row ("ch", "HAT", Role::ClosedHat, 0.45f, 0.35f, 0.45f, "Charleston fermé poussiéreux.", "Brillance."),
            row ("oh", "OPEN HAT", Role::OpenHat, 0.4f, 0.4f, 0.45f, "Charleston ouvert.", "Brillance."),
            row ("sh", "SHAKER", Role::Shaker, 0.35f, 0.3f, 0.5f, "Shaker : remplit le groove en doubles croches.", "Brillance."),
            row ("pc", "PERC", Role::Perc, 0.45f, 0.5f, 0.5f, "Percussion boisée (wood block) pour les relances.", "Claque."),
            row ("cr", "CRASH", Role::Crash, 0.35f, 0.6f, 0.45f, "Cymbale crash.", "Brillance."),
        },
        {} });

    defs.push_back ({ Percussion, "perc", "PERCUSSIONS", "Percussions latines et africaines", MachineKind::Drum,
        0xffff9a3c, 0,
        "Congas, bongos, timbales, agogo, guiro, shaker, tambourin, triangle et claves : tout ce qu'il faut pour de l'afro house, "
        "du reggaeton, de la salsa, de la house latine ou pour enrichir n'importe quel groove.\n\n"
        "Astuce : ajoute 2 ou 3 percussions discrètes par-dessus une batterie simple, c'est ce qui donne vie au rythme.",
        {
            row ("cgl", "CONGA LOW", Role::LowConga, 0.55f, 0.45f, 0.4f, "Conga grave (tumba).", "Claque de la main."),
            row ("cgh", "CONGA HIGH", Role::HighConga, 0.55f, 0.4f, 0.5f, "Conga aiguë (quinto).", "Claque de la main."),
            row ("bl", "BONGO LOW", Role::LowBongo, 0.5f, 0.35f, 0.5f, "Bongo grave (hembra).", "Claque de la main."),
            row ("bh", "BONGO HIGH", Role::HighBongo, 0.5f, 0.3f, 0.5f, "Bongo aigu (macho).", "Claque de la main."),
            row ("tl", "TIMBALE LOW", Role::LowTimbale, 0.5f, 0.45f, 0.6f, "Timbale grave : sonne métallique, typique de la salsa.", "Claque."),
            row ("th", "TIMBALE HIGH", Role::HighTimbale, 0.5f, 0.4f, 0.6f, "Timbale aiguë.", "Claque."),
            row ("cb", "COWBELL", Role::Cowbell, 0.4f, 0.35f, 0.5f, "Cloche (cencerro).", "Couleur."),
            row ("ag", "AGOGO", Role::Agogo, 0.4f, 0.35f, 0.5f, "Agogo : cloche africaine/brésilienne.", "Brillance de la cloche."),
            row ("gu", "GUIRO", Role::Guiro, 0.45f, 0.4f, 0.5f, "Guiro râpé.", "Couleur du grattement."),
            row ("sh", "SHAKER", Role::Shaker, 0.4f, 0.3f, 0.5f, "Shaker en doubles croches.", "Brillance."),
            row ("tb", "TAMBOURINE", Role::Tambourine, 0.4f, 0.35f, 0.5f, "Tambourin.", "Écart entre les secousses."),
            row ("tr", "TRIANGLE", Role::Triangle, 0.35f, 0.4f, 0.5f, "Triangle : un « ting » cristallin qui résonne.", "Brillance."),
            row ("cl", "CLAVES", Role::Claves, 0.45f, 0.3f, 0.5f, "Claves : jouent la « clave », le rythme qui guide la musique latine.", "Brillance."),
        },
        {} });

    defs.push_back ({ DX7, "dx7", "DX7 FM", "Synthé FM · inspiré du Yamaha DX7 (1983)", MachineKind::Poly,
        0xff3fd0b0, 24,
        "Le synthé le plus vendu des années 80, basé sur la synthèse FM : des sons brillants et précis. Son piano électrique "
        "est partout dans la pop, le R&B, le lo-fi et la house ; ses basses FM et ses cloches sont devenues légendaires.\n\n"
        "Choisis un SON (piano électrique, basse FM, cloche, cuivres, marimba, nappe), puis règle la BRILLANCE et la DURÉE.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "preset", "SON", 0.0f,
              "Le type de son FM. Piano électrique = doux et chaleureux (lo-fi, R&B, house). Basse FM = basse claquante des années 80. "
              "Cloche = son cristallin (trap, ambient). Cuivres = attaque cuivrée. Marimba = percussion boisée. Nappe = son long et doux.",
              { "Piano électrique", "Basse FM", "Cloche", "Cuivres", "Marimba", "Nappe FM", "Basse « Lately »", "Basse solide",
                "Basse slap", "Piano électrique brillant", "Rhodes doux", "Clavecin", "Clavinet", "Orgue FM", "Orgue d'église",
                "Flûte", "Harmonica", "Cordes FM", "Glockenspiel", "Vibraphone", "Xylophone", "Koto", "Guitare FM", "Sitar FM",
                "Lead synthé", "Chœur FM", "Timbales d'orchestre", "Carillon", "Nappe cristal", "Sub basse FM" } },
            { "brightness", "BRILLANCE", 0.5f, "Quantité de modulation FM : à gauche le son est doux et rond, à droite il devient brillant, métallique et agressif.", {} },
            { "decay", "DURÉE", 0.5f, "Durée naturelle du son : court = notes sèches et percussives, long = notes qui résonnent.", {} },
            { "release", "RELEASE", 0.3f, releaseHelp, {} },
            { "detune", "DÉSACCORD", 0.2f, "Désaccorde légèrement deux couches du son pour l'épaissir (effet chorus).", {} },
            { "tremolo", "TRÉMOLO", 0.2f, "Fait onduler le volume, comme sur un vrai piano électrique.", {} },
        } });

    defs.push_back ({ Strings, "strings", "STRINGS", "String machine · inspirée de l'ARP Solina (1974)", MachineKind::Poly,
        0xffffb347, 24,
        "La « string machine » : un orgue à cordes électronique passé dans un chorus d'ensemble qui le rend large et soyeux. "
        "Le son des nappes disco, de la synthwave, de la dub, de l'ambient et de Jean-Michel Jarre.\n\n"
        "Joue des accords tenus (rangée TENIR) avec une attaque et un release longs.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "violin", "VIOLONS", 0.8f, "Volume des registres aigus (violons) : brillant et léger.", {} },
            { "cello", "VIOLONCELLES", 0.5f, "Volume du registre grave (violoncelles) une octave en dessous : donne du corps.", {} },
            { "attack", "ATTACK", 0.45f, attackHelp, {} },
            { "release", "RELEASE", 0.5f, releaseHelp, {} },
            { "ensemble", "ENSEMBLE", 0.7f, "Le chorus d'ensemble qui fait sonner l'instrument comme une section de cordes. À fond = son très large et ondulant.", {} },
            { "brightness", "BRILLANCE", 0.55f, "Filtre : sombre à gauche, brillant à droite.", {} },
        } });

    defs.push_back ({ Organ, "organ", "ORGAN", "Orgue · inspiré de l'orgue Hammond B3 et de l'orgue house du Korg M1", MachineKind::Poly,
        0xffe8c070, 24,
        "Deux orgues en un. TIRETTES : l'orgue à roues phoniques (Hammond) du gospel, de la soul, du funk et du reggae, avec sa cabine "
        "rotative (Leslie). ORGUE HOUSE : le son percussif du Korg M1, emblème de la house des années 90.\n\n"
        "Les faders sont les « tirettes » : ils dosent les harmoniques graves, médiums et aiguës.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "type", "TYPE", 0.0f, "Tirettes = orgue Hammond (soul, gospel, reggae, funk). Orgue house = orgue percussif du Korg M1 (house des années 90).",
              { "Tirettes", "Orgue house" } },
            { "bass", "GRAVES", 0.8f, "Tirettes graves (16' et 8') : le fondement du son.", {} },
            { "middle", "MÉDIUMS", 0.5f, "Tirettes médiums (4' et 2 2/3') : donnent de la présence.", {} },
            { "high", "AIGUS", 0.3f, "Tirettes aiguës (2', 1 3/5', 1') : son brillant et perçant.", {} },
            { "percussion", "PERCUSSION", 0.3f, "Petite note harmonique qui claque au début de chaque note : le « pop » typique de l'orgue jazz et house.", {} },
            { "leslie", "LESLIE", 0.4f, "La cabine à haut-parleur tournant : ondulation lente (milieu) ou rapide (à fond). À zéro : coupée.", {} },
            { "overdrive", "DRIVE", 0.2f, "Saturation de l'ampli : orgue rock et sale quand on pousse.", {} },
            { "click", "CLIC", 0.3f, "Le petit clic de contact des touches, typique des vrais orgues.", {} },
        } });

    defs.push_back ({ Sub808, "sub808", "808 SUB", "Basse 808 · la basse glissée du trap et de la drill", MachineKind::Mono,
        0xffff2d3a, 0,
        "La basse 808 : un kick 808 très long joué comme une note de basse. C'est LA basse du trap, de la drill, du R&B moderne "
        "et du reggaeton : profonde, avec ses fameux glissés entre les notes.\n\n"
        "Place souvent les notes au même endroit que le kick, et mets un SLIDE pour faire glisser vers la note suivante.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "decay", "DURÉE", 0.6f, "Durée de chaque note : court = basse rebondissante, long = grosse note qui tient.", {} },
            { "glide", "GLIDE", 0.35f, "Durée du glissé vers la note suivante quand SLIDE est activé. C'est la signature des basses trap.", {} },
            { "punch", "PUNCH", 0.4f, "Attaque du début de note (une petite chute de hauteur, comme un kick) : plus de punch = la basse claque mieux.", {} },
            { "overdrive", "DISTO", 0.3f, "Saturation : rend la basse audible même sur les petits haut-parleurs (téléphone, laptop).", {} },
            { "tone", "TIMBRE", 0.6f, "Filtre après la saturation : sombre à gauche, plus mordant à droite.", {} },
        } });

    defs.push_back ({ Piano, "piano", "PIANO", "Piano acoustique · à queue, droit, bastringue, pop, feutré", MachineKind::Poly,
        0xfff2e6c8, 24,
        "Le piano acoustique, recréé par synthèse (cordes, marteaux, étouffoirs). Indispensable pour la house « piano », "
        "la pop, les ballades, le lo-fi, le gospel et le R&B.\n\n"
        "Choisis un TYPE : piano à queue (riche), piano droit (plus sec), bastringue (désaccordé, saloon), pop brillant "
        "(qui perce dans le mix) ou feutré (très doux, idéal en lo-fi). Allume TENIR pour laisser résonner les accords.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "type", "TYPE", 0.0f,
              "Le modèle de piano. À queue = son riche et ample. Droit = plus sec et intime. Bastringue = cordes désaccordées, "
              "son de saloon. Pop brillant = attaque claire qui ressort dans le mix (house, pop). Feutré = très doux et chaud (lo-fi, piano de nuit).",
              { "Piano à queue", "Piano droit", "Piano bastringue", "Piano pop brillant", "Piano feutré", "Piano électrique à queue (CP-70)" } },
            { "brightness", "BRILLANCE", 0.5f, "Sombre et doux à gauche, brillant et percutant à droite (comme si on jouait plus fort).", {} },
            { "decay", "TENUE", 0.5f, "Combien de temps les cordes résonnent quand la note est tenue.", {} },
            { "release", "ÉTOUFFOIR", 0.4f, "Vitesse à laquelle le son s'arrête quand on relâche la note : court = notes détachées, long = comme avec la pédale.", {} },
            { "hammer", "MARTEAUX", 0.4f, "Le petit bruit des marteaux qui frappent les cordes : plus = attaque plus réaliste et percussive.", {} },
            { "width", "STÉRÉO", 0.5f, "Écarte les graves à gauche et les aigus à droite, comme quand on est assis au piano.", {} },
        } });

    defs.push_back ({ Mallets, "mallets", "MALLETS", "Cloches et percussions mélodiques · glockenspiel, vibraphone, kalimba…", MachineKind::Poly,
        0xff7fd6ff, 24,
        "Tout ce qui se joue avec une mailloche ou qui sonne comme une cloche : glockenspiel, vibraphone, célesta, boîte à musique, "
        "kalimba, steel drum, cloches tubulaires et marimba.\n\n"
        "Parfait pour les mélodies de trap et de drill (cloches, boîte à musique), le lo-fi (vibraphone, kalimba), l'afro et les "
        "musiques des îles (marimba, steel drum) ou les ambiances féeriques (célesta).",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 0.0f, chordHelp, chordChoices() },
            { "type", "TYPE", 0.0f,
              "L'instrument. Glockenspiel = petites lames de métal très aiguës. Vibraphone = lames métalliques avec un vibrato doux (jazz, lo-fi). "
              "Célesta = clochettes féeriques. Boîte à musique = mélodie de berceuse (trap). Kalimba = piano à pouces africain. "
              "Steel drum = son des Caraïbes. Cloches tubulaires = grandes cloches d'église. Marimba = lames de bois.",
              { "Glockenspiel", "Vibraphone", "Célesta", "Boîte à musique", "Kalimba", "Steel drum", "Cloches tubulaires", "Marimba",
                "Xylophone", "Crotales", "Gamelan", "Handpan", "Cloches d'église" } },
            { "brightness", "BRILLANCE", 0.5f, "Quantité d'harmoniques aiguës : sombre à gauche, scintillant à droite.", {} },
            { "decay", "DURÉE", 0.5f, "Combien de temps la lame ou la cloche résonne.", {} },
            { "release", "RELEASE", 0.5f, releaseHelp, {} },
            { "mallet", "MAILLOCHE", 0.5f, "Dureté de la mailloche : douce = son rond et feutré, dure = attaque claquante et brillante.", {} },
            { "vibrato", "VIBRATO", 0.3f, "Fait onduler le son (le moteur du vibraphone). Surtout audible sur le vibraphone.", {} },
        } });

    defs.push_back ({ Pluck, "pluck", "PLUCK", "Cordes pincées · guitares, harpe, koto, sitar, pizzicato", MachineKind::Poly,
        0xffe0a060, 24,
        "Des cordes pincées recréées par synthèse physique (une corde qui vibre vraiment) : guitare acoustique, nylon, guitare funk "
        "étouffée, harpe, koto japonais, sitar indien et pizzicato de cordes.\n\n"
        "Les accords sont « grattés » comme sur une guitare (bouton GRATTAGE). Super pour le reggaeton et l'afro (guitare), le funk "
        "(guitare étouffée), le lo-fi (nylon) ou des couleurs du monde (koto, sitar).",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "type", "TYPE", 0.0f,
              "L'instrument. Guitare acoustique = cordes métal brillantes. Nylon = guitare classique douce. Funk = notes courtes étouffées "
              "avec la paume (« tchik »). Harpe = cordes douces qui résonnent longtemps. Koto = cithare japonaise. Sitar = son indien qui "
              "grésille. Pizzicato = cordes de violon pincées.",
              { "Guitare acoustique", "Guitare nylon", "Guitare funk (étouffée)", "Harpe", "Koto", "Sitar", "Pizzicato (cordes)",
                "Banjo", "Mandoline", "Ukulélé", "Guitare électrique", "Oud", "Guzheng" } },
            { "brightness", "BRILLANCE", 0.5f, "Où et comment la corde est pincée : sombre (avec le pouce) à gauche, brillant (au médiator) à droite.", {} },
            { "decay", "DURÉE", 0.5f, "Combien de temps la corde vibre.", {} },
            { "release", "AMORTI", 0.3f, "Vitesse à laquelle la corde s'arrête quand on relâche la note.", {} },
            { "strum", "GRATTAGE", 0.3f, "Décalage entre les notes d'un accord, comme un coup de médiator sur les cordes. À zéro, toutes les notes partent ensemble.", {} },
            { "body", "CAISSE", 0.5f, "La résonance de la caisse en bois de l'instrument : plus = son plus rond et plus « boisé ».", {} },
        } });

    defs.push_back ({ Choir, "choir", "CHOIR", "Chœurs · voix « aah », « ooh », « eeh », « mmh »", MachineKind::Poly,
        0xffc8a8ff, 24,
        "Un chœur de voix synthétiques qui chantent des voyelles. Le son des nappes vocales de la house, de la trance, du gospel, "
        "des musiques de film et de l'ambient.\n\n"
        "Choisis une VOYELLE, joue des accords tenus (rangée TENIR) avec une attaque lente : effet garanti.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "vowel", "VOYELLE", 0.0f, "La voyelle chantée : Aah = ouvert et puissant, Ooh = rond et doux, Eeh = plus nasal et brillant, Mmh = bouche fermée, très doux.",
              { "Aah", "Ooh", "Eeh", "Mmh", "Oh", "Ih" } },
            { "attack", "ATTACK", 0.5f, attackHelp, {} },
            { "release", "RELEASE", 0.5f, releaseHelp, {} },
            { "ensemble", "ENSEMBLE", 0.5f, "Nombre apparent de chanteurs : à gauche une voix seule, à droite un grand chœur large.", {} },
            { "brightness", "BRILLANCE", 0.5f, "Présence des aigus de la voix : sombre et lointain à gauche, clair et proche à droite.", {} },
            { "breath", "SOUFFLE", 0.2f, "Ajoute le souffle de l'air dans la voix, pour un chœur plus intime et aérien.", {} },
        } });

    defs.push_back ({ SoundFx, "fx", "FX", "Effets sonores · risers, impacts, sirène, air horn, vinyle", MachineKind::Drum,
        0xff9cff57, 0,
        "Les effets qui font vivre un morceau : montées (riser), descentes, gros impacts, souffles, laser, sirène dub, air horn du "
        "dancehall, craquements de vinyle et cymbale à l'envers.\n\n"
        "Chaque case déclenche l'effet ; le bouton DURÉE règle sa longueur. Exemple : un RISER au début de la dernière mesure, "
        "un IMPACT sur le premier temps qui suit. Le VINYLE, long, donne le grain lo-fi.",
        {
            row ("rise", "RISER", Role::Perc, 0.55f, 0.5f, 0.3f, "Montée de bruit et de son qui grimpe vers l'aigu : annonce l'arrivée du refrain ou du drop.", "Bruit (à gauche) ou son (à droite)."),
            row ("down", "DOWN", Role::Perc, 0.5f, 0.45f, 0.3f, "Descente : un balayage qui retombe vers le grave, juste après un drop ou à la fin d'une partie.", "Bruit (à gauche) ou son (à droite)."),
            row ("imp", "IMPACT", Role::Crash, 0.7f, 0.5f, 0.4f, "Gros coup de cinéma : un boom grave et une queue de bruit. Sur le premier temps d'une nouvelle partie.", "Plus de boom (à gauche) ou de souffle (à droite)."),
            row ("swp", "SWEEP", Role::Perc, 0.45f, 0.45f, 0.3f, "Un souffle de bruit blanc qui s'ouvre puis se referme (« whoosh ») pour lier deux parties.", "Résonance du souffle."),
            row ("zap", "LASER", Role::Perc, 0.4f, 0.4f, 0.3f, "Le « piou » du laser : un son qui plonge de très aigu à grave. Rétro, électro, jeux vidéo.", "Son doux (à gauche) ou carré et agressif (à droite)."),
            row ("sir", "SIRÈNE", Role::Perc, 0.4f, 0.5f, 0.4f, "La sirène des sound systems dub et reggae : un son qui monte et descend en boucle.", "Vitesse de la sirène."),
            row ("horn", "AIR HORN", Role::Perc, 0.45f, 0.4f, 0.5f, "La corne de brume du dancehall et du reggaeton. Mets des roulements ×3 (clic droit) pour le fameux « pou-pou-pou ».", "Brillance du cuivre."),
            row ("vin", "VINYLE", Role::Perc, 0.4f, 0.5f, 0.4f, "Craquements et souffle d'un vieux disque. Un seul coup long au début de la séquence suffit : le grain lo-fi parfait.", "Quantité de craquements."),
            row ("rev", "REVERSE", Role::Perc, 0.45f, 0.5f, 0.5f, "Cymbale à l'envers qui monte et s'arrête net. Place-la pour qu'elle se termine juste avant un temps fort.", "Brillance."),
            row ("scr", "SCRATCH", Role::Perc, 0.45f, 0.4f, 0.4f, "Le scratch du DJ de hip-hop : un son poussé d'avant en arrière sous la main.", "Nombre d'allers-retours."),
            row ("rwd", "REWIND", Role::Perc, 0.45f, 0.45f, 0.4f, "Le « rewind » des sound systems : la bande rembobinée à toute vitesse (jungle, dub, dancehall).", "Quantité de souffle."),
            row ("gun", "GUNSHOT", Role::Perc, 0.4f, 0.4f, 0.5f, "Le coup de feu des sound systems dancehall et jungle (« pull up ! »).", "Longueur de l'écho."),
            row ("rain", "PLUIE", Role::Perc, 0.4f, 0.5f, 0.4f, "La pluie : un souffle doux et des gouttes. Un coup long pour une ambiance lo-fi ou ambient.", "Quantité de gouttes."),
        },
        {} });

    defs.push_back ({ EBass, "ebass", "BASS", "Basse électrique · aux doigts, slap, médiator, fretless, contrebasse", MachineKind::Mono,
        0xffe8a33d, 0,
        "La basse des musiciens : une vraie corde pincée recréée par synthèse physique. Aux doigts pour le funk, la soul, la pop et le "
        "reggae ; en slap pour le funk et le disco ; au médiator pour le rock ; fretless (sans frettes, elle glisse entre les notes) "
        "pour le jazz-funk ; contrebasse pour le jazz, le lo-fi et le hip-hop boom bap.\n\n"
        "Rangées : NOTES, ACCENT (note plus forte), SLIDE (glissé vers la note suivante).",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "type", "JEU", 0.0f, "La façon de jouer : aux doigts (rond), slap (claquant, avec le « pop » du pouce), médiator (attaque nette), "
              "fretless (glisse entre les notes), contrebasse (boisée et courte).", { "Aux doigts", "Slap", "Médiator", "Fretless", "Contrebasse" } },
            { "tone", "TIMBRE", 0.5f, "Le bouton de tonalité de la basse : sombre et rond à gauche, brillant et mordant à droite.", {} },
            { "decay", "DURÉE", 0.5f, "Combien de temps la corde vibre.", {} },
            { "muting", "ÉTOUFFEMENT", 0.3f, "Vitesse à laquelle la main étouffe la corde quand la note s'arrête : court = notes sèches et funky.", {} },
            { "glide", "GLISSÉ", 0.0f, "Durée du glissé entre deux notes reliées par un SLIDE (le doigt qui glisse sur la touche).", {} },
        } });

    defs.push_back ({ Wobble, "wobble", "WOBBLE", "Basse wobble et Reese · dubstep, drum & bass, riddim", MachineKind::Mono,
        0xff7cff4f, 0,
        "La basse qui « parle » du dubstep : un filtre qui s'ouvre et se ferme en rythme, calé sur le tempo (wouob-wouob). "
        "Aussi la Reese (deux scies désaccordées qui grondent) de la drum & bass, et le Growl, plus agressif.\n\n"
        "Choisis la VITESSE du wobble (1/4 = une fois par temps, 1/8 = deux fois…), puis la PROFONDEUR.",
        {},
        {
            { "octave", "OCTAVE", 1.0f, octaveHelp, octaves },
            { "wave", "SON", 0.0f, "Wobble = scie + carré (dubstep). Reese = scies désaccordées qui grondent (drum & bass). Growl = son « qui parle », agressif.",
              { "Wobble", "Reese", "Growl" } },
            { "rate", "VITESSE", 2.0f, "Vitesse du wobble, calée sur le tempo : 1/2 = lent, 1/4 = à chaque temps, 1/8 = deux fois par temps, "
              "triolets = balancé, 1/16 = très rapide.", { "1/2", "1/4", "1/8", "1/8 triolet", "1/16", "1/4 triolet" } },
            { "depth", "PROFONDEUR", 0.6f, "Quantité de wobble : à zéro le filtre reste fixe, à fond il s'ouvre et se ferme complètement.", {} },
            { "cutoff", "FILTRE", 0.3f, cutoffHelp, {} },
            { "reso", "RÉSO", 0.3f, resoHelp, {} },
            { "detune", "DÉSACCORD", 0.3f, "Écart entre les oscillateurs : plus = son plus épais qui grouille.", {} },
            { "overdrive", "DISTO", 0.4f, "Saturation : la basse devient plus grasse et plus agressive.", {} },
            { "glide", "GLIDE", 0.2f, "Durée du glissé entre deux notes reliées par un SLIDE.", {} },
        } });

    defs.push_back ({ Hoover, "hoover", "HOOVER", "Hoover rave · inspiré du « What The » du Roland Alpha Juno (1985)", MachineKind::Poly,
        0xffc0c8d8, 24,
        "Le son le plus célèbre de la rave, du hardcore et du gabber : un gros son de scies désaccordées dont la hauteur plonge puis "
        "remonte au début de chaque note (comme un aspirateur, d'où son nom).\n\n"
        "Joue des notes seules ou des accords, avec un peu de SLIDE pour des glissés.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 0.0f, chordHelp, chordChoices() },
            { "detune", "DÉSACCORD", 0.6f, "Écart entre les 5 oscillateurs : plus = son plus large et plus « hoover ».", {} },
            { "dip", "PLONGÉE", 0.5f, "De combien la hauteur plonge au début de chaque note avant de remonter : c'est la signature du hoover.", {} },
            { "cutoff", "FILTRE", 0.7f, cutoffHelp, {} },
            { "attack", "ATTACK", 0.1f, attackHelp, {} },
            { "release", "RELEASE", 0.3f, releaseHelp, {} },
            { "chorus", "CHORUS", 3.0f, "Le chorus qui élargit le son. I+II = très large, comme sur les disques de rave.", { "OFF", "I", "II", "I+II" } },
        } });

    defs.push_back ({ Poly80, "poly80", "POLY 80s", "Synthé polyphonique · inspiré des Prophet-5 et Oberheim OB-X (1978-1981)", MachineKind::Poly,
        0xffd8b25a, 24,
        "Le son des années 80 : des cuivres de synthé énormes (le riff de « Jump »), des nappes chaudes et des stabs. Deux oscillateurs "
        "par voix (scie et impulsion, le second désaccordé) et un filtre 12 dB au son crémeux.\n\n"
        "Idéal pour la synthwave, la pop 80s, l'italo disco et la French touch.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "saw", "SAW", 0.8f, "Volume des dents de scie : son brillant (cuivres, nappes).", {} },
            { "pulse", "PULSE", 0.4f, "Volume des ondes à impulsion : son plus creux.", {} },
            { "detune", "DÉSACCORD", 0.3f, "Écart entre les deux oscillateurs : épaissit le son.", {} },
            { "cutoff", "FILTRE", 0.5f, cutoffHelp, {} },
            { "reso", "RÉSO", 0.2f, resoHelp, {} },
            { "env", "ENV", 0.4f, envHelp, {} },
            { "attack", "ATTACK", 0.05f, attackHelp, {} },
            { "decay", "DECAY", 0.4f, decayHelp, {} },
            { "sustain", "SUSTAIN", 0.7f, sustainHelp, {} },
            { "release", "RELEASE", 0.3f, releaseHelp, {} },
        } });

    defs.push_back ({ M1, "m1", "M1 ROMPLER", "Sons de la house des années 90 · inspiré du Korg M1 (1988)", MachineKind::Poly,
        0xff4f9fff, 24,
        "Le synthé à sons « tout faits » qui a défini la house des années 90 : son piano (« Vogue »), son orgue house, ses nappes « Lore » "
        "et « Universe », ses basses slap et fretless.\n\n"
        "Choisis un SON puis règle la BRILLANCE et la DURÉE.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "preset", "SON", 0.0f, "Le son du M1 : piano house, orgue, nappes Lore et Universe, basses fretless et slap, cordes, cloche.",
              { "Piano M1", "Orgue M1", "Nappe « Lore »", "Nappe « Universe »", "Basse fretless", "Basse slap", "Cordes M1", "Cloche / kalimba" } },
            { "brightness", "BRILLANCE", 0.5f, "Doux à gauche, brillant à droite.", {} },
            { "decay", "DURÉE", 0.5f, "Durée naturelle du son.", {} },
            { "release", "RELEASE", 0.3f, releaseHelp, {} },
            { "detune", "DÉSACCORD", 0.2f, "Épaissit le son.", {} },
            { "tremolo", "TRÉMOLO", 0.1f, "Fait onduler le volume.", {} },
        } });

    defs.push_back ({ Edm, "edm", "EDM", "Synthé EDM · accords future bass, plucks, leads, sidechain intégré", MachineKind::Poly,
        0xffff4fd8, 24,
        "Le son des festivals : de gros accords de supersaw qui « pompent » au rythme du kick (sidechain intégré), des plucks courts, "
        "des leads perçants. Le son de la future bass, de l'EDM, de la big room et de la pop électro.\n\n"
        "Le bouton POMPE règle l'effet de pompage, calé sur les temps.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "detune", "DÉSACCORD", 0.5f, "Écart entre les scies : plus = plus large et plus « festival ».", {} },
            { "mix", "MIX", 0.6f, "Volume des scies désaccordées par rapport à la centrale.", {} },
            { "cutoff", "FILTRE", 0.6f, cutoffHelp, {} },
            { "reso", "RÉSO", 0.1f, resoHelp, {} },
            { "env", "ENV", 0.3f, envHelp, {} },
            { "attack", "ATTACK", 0.02f, attackHelp, {} },
            { "decay", "DECAY", 0.4f, decayHelp, {} },
            { "sustain", "SUSTAIN", 0.8f, sustainHelp, {} },
            { "release", "RELEASE", 0.3f, releaseHelp, {} },
            { "pump", "POMPE", 0.6f, "Le volume baisse à chaque temps puis remonte, comme si le kick « poussait » les accords : l'effet sidechain de l'EDM.", {} },
        } });

    defs.push_back ({ Talkbox, "talkbox", "TALKBOX", "Talkbox · le synthé qui parle du G-funk et de la French touch", MachineKind::Poly,
        0xffff8a3d, 24,
        "Un synthé dont le son passe par une « bouche » qui articule des voyelles : chaque note glisse d'une voyelle à l'autre "
        "(« ouaaah », « iiioh »). Le son de Roger Troutman, du G-funk de la côte ouest, de Daft Punk et de la funk électronique.\n\n"
        "Choisis la voyelle de DÉPART et d'ARRIVÉE, puis la VITESSE de la bouche.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 0.0f, chordHelp, chordChoices() },
            { "from", "DÉPART", 1.0f, "La voyelle au début de chaque note.", { "Aah", "Ooh", "Eeh", "Mmh", "Oh", "Ih" } },
            { "to", "ARRIVÉE", 0.0f, "La voyelle vers laquelle la note glisse.", { "Aah", "Ooh", "Eeh", "Mmh", "Oh", "Ih" } },
            { "speed", "VITESSE", 0.4f, "Vitesse à laquelle la bouche passe d'une voyelle à l'autre.", {} },
            { "brightness", "BRILLANCE", 0.5f, "Netteté de la voix : plus = voyelles plus marquées.", {} },
            { "release", "RELEASE", 0.2f, releaseHelp, {} },
        } });

    defs.push_back ({ Mellotron, "mellotron", "MELLOTRON", "Mellotron · les bandes magnétiques des années 60-70", MachineKind::Poly,
        0xffb98a5a, 24,
        "Un clavier où chaque touche lit une bande magnétique enregistrée : flûtes, cordes, chœurs, violoncelle, cuivres. Le son des "
        "Beatles (« Strawberry Fields »), du rock progressif, du rock psyché et de beaucoup de lo-fi.\n\n"
        "Comme le vrai, la bande dure 8 secondes : une note tenue plus longtemps s'éteint toute seule. Le bouton BANDE fait onduler la hauteur.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "type", "BANDE", 0.0f, "L'instrument enregistré sur les bandes.", { "Flûtes", "Cordes", "Chœur", "Violoncelle", "Cuivres" } },
            { "tone", "TIMBRE", 0.5f, "Sombre et vieilli à gauche, plus clair à droite.", {} },
            { "attack", "ATTACK", 0.25f, attackHelp, {} },
            { "release", "RELEASE", 0.3f, releaseHelp, {} },
            { "wow", "PLEURAGE", 0.4f, "L'ondulation de la hauteur due aux bandes usées : un peu = vintage, beaucoup = psychédélique.", {} },
            { "noise", "SOUFFLE", 0.3f, "Le souffle des bandes magnétiques.", {} },
        } });

    defs.push_back ({ EPiano, "epiano", "E.PIANO", "Pianos électriques · Rhodes, Wurlitzer, Clavinet, clavecin", MachineKind::Poly,
        0xffd94a3a, 24,
        "Les claviers électriques de la soul, du jazz, du funk et du R&B. Le Rhodes (doux et cristallin, le cœur du lo-fi et de la "
        "neo-soul), le Wurlitzer (plus nasillard), le Clavinet (le funk de Stevie Wonder) et le clavecin.\n\n"
        "Le bouton MORDANT fait « grogner » le Rhodes quand on joue fort.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "type", "TYPE", 0.0f, "Rhodes = doux, rond et cristallin. Wurlitzer = plus sale et nasillard. Clavinet = funky et percussif. "
              "Clavecin = baroque et brillant.", { "Rhodes", "Wurlitzer", "Clavinet", "Clavecin" } },
            { "tone", "TIMBRE", 0.5f, "Sombre à gauche, brillant à droite.", {} },
            { "decay", "DURÉE", 0.5f, "Combien de temps la note résonne.", {} },
            { "release", "RELEASE", 0.3f, releaseHelp, {} },
            { "tremolo", "TRÉMOLO", 0.3f, "Le trémolo stéréo des pianos électriques : le son voyage de gauche à droite.", {} },
            { "bark", "MORDANT", 0.4f, "Le « grognement » du Rhodes quand on tape fort sur les touches.", {} },
        } });

    defs.push_back ({ Brass, "brass", "BRASS", "Cuivres · trompette, trombone, saxophone, section funk", MachineKind::Poly,
        0xffffc02e, 24,
        "Les cuivres du funk, de la soul, du disco, du ska, de l'afrobeat et du hip-hop : trompette, trombone, saxophone, cor, une "
        "section entière ou de courts « stabs ».\n\n"
        "Plus on joue fort (ACCENT), plus le son devient brillant, comme un vrai cuivre.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 0.0f, chordHelp, chordChoices() },
            { "type", "INSTRUMENT", 0.0f, "Le cuivre joué. Section funk = plusieurs cuivres ensemble. Stab = coup court pour ponctuer.",
              { "Trompette", "Trombone", "Saxophone", "Section funk", "Stab cuivres", "Cor" } },
            { "brightness", "BRILLANCE", 0.5f, "Comme souffler plus fort : le son devient plus brillant et éclatant.", {} },
            { "attack", "ATTACK", 0.3f, "Temps que met le cuivre à démarrer : court = notes qui claquent, long = notes qui gonflent.", {} },
            { "release", "RELEASE", 0.3f, releaseHelp, {} },
            { "vibrato", "VIBRATO", 0.4f, "Le vibrato qui arrive sur les notes tenues.", {} },
            { "growl", "GROWL", 0.2f, "Le grain rugueux du souffle (surtout sur le saxophone).", {} },
        } });

    defs.push_back ({ Winds, "winds", "WINDS", "Bois · flûte, flûte de pan, clarinette, shakuhachi, ocarina", MachineKind::Poly,
        0xff8fd17a, 24,
        "Les instruments à vent : flûte traversière, flûte de pan, clarinette, shakuhachi japonaise, ocarina et la fameuse « flûte trap ». "
        "Le souffle et le vibrato rendent le son vivant.\n\n"
        "Parfait pour les mélodies de trap et de drill, le lo-fi, l'ambient et les musiques du monde.",
        {},
        {
            { "octave", "OCTAVE", 3.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 0.0f, chordHelp, chordChoices() },
            { "type", "INSTRUMENT", 0.0f, "L'instrument à vent.", { "Flûte", "Flûte de pan", "Clarinette", "Shakuhachi", "Ocarina", "Flûte trap" } },
            { "breath", "SOUFFLE", 0.5f, "La quantité d'air qu'on entend autour de la note.", {} },
            { "vibrato", "VIBRATO", 0.4f, "Le vibrato qui arrive sur les notes tenues.", {} },
            { "attack", "ATTACK", 0.2f, attackHelp, {} },
            { "release", "RELEASE", 0.3f, releaseHelp, {} },
            { "brightness", "BRILLANCE", 0.5f, "Doux et rond à gauche, plus brillant à droite.", {} },
        } });

    defs.push_back ({ Orchestra, "orchestra", "ORCHESTRA", "Orchestre · cordes, pizzicato, orch hit, timbales, harpe", MachineKind::Poly,
        0xffb08c5a, 24,
        "L'orchestre symphonique : une section de cordes, le pizzicato, les timbales, la harpe et le célèbre « orch hit » (TCHAAA), coup "
        "d'orchestre utilisé dans le hip-hop, la pop 80s et la drill.\n\n"
        "Pour des cordes épiques : accords tenus (rangée TENIR), attaque lente et un peu de reverb.",
        {},
        {
            { "octave", "OCTAVE", 2.0f, octaveHelp, octaves },
            { "chord", "ACCORD", 1.0f, chordHelp, chordChoices() },
            { "type", "INSTRUMENT", 0.0f, "Cordes = section de violons, altos et violoncelles. Pizzicato = cordes pincées. Orch hit = coup "
              "d'orchestre. Timbales = grands tambours accordés. Harpe = cordes pincées qui résonnent.",
              { "Cordes", "Pizzicato", "Orch hit", "Timbales", "Harpe" } },
            { "attack", "ATTACK", 0.4f, attackHelp, {} },
            { "release", "RELEASE", 0.4f, releaseHelp, {} },
            { "brightness", "BRILLANCE", 0.5f, "Sombre et doux à gauche, brillant à droite.", {} },
            { "ensemble", "ENSEMBLE", 0.6f, "Taille de la section : à gauche quelques musiciens, à droite tout un orchestre.", {} },
        } });

    defs.push_back ({ AcousticKit, "kit", "DRUM KIT", "Batterie acoustique · rock, pop, jazz, soul", MachineKind::Drum,
        0xffc23b3b, 0,
        "Une vraie batterie : grosse caisse, caisse claire, ghost notes, cross-stick, charleston, trois toms, crash, ride et cloche "
        "de ride. Le son du rock, de la pop, de la soul, du jazz et de tous les groupes.\n\n"
        "Les rythmes « Incontournables » du bouton PATTERNS sonnent parfaitement dessus.",
        {
            row ("bd", "KICK", Role::Kick, 0.85f, 0.45f, 0.5f, "Grosse caisse acoustique.", "Claquement de la batte."),
            row ("sd", "SNARE", Role::Snare, 0.75f, 0.45f, 0.55f, "Caisse claire.", "Timbre (grésillement)."),
            row ("gs", "GHOST", Role::Ghost, 0.3f, 0.3f, 0.5f, "Ghost notes : petits coups très légers sur la caisse claire, entre les gros.", "Timbre."),
            row ("xs", "CROSS-STICK", Role::Rim, 0.5f, 0.3f, 0.5f, "La baguette posée sur le bord de la caisse claire : « toc ». Ballades, bossa, reggae.", "Brillance."),
            row ("ch", "HAT", Role::ClosedHat, 0.45f, 0.35f, 0.5f, "Charleston fermé.", "Brillance."),
            row ("oh", "OPEN HAT", Role::OpenHat, 0.4f, 0.45f, 0.5f, "Charleston ouvert.", "Brillance."),
            row ("ht", "TOM HIGH", Role::HighTom, 0.55f, 0.45f, 0.4f, "Tom aigu.", "Attaque."),
            row ("mt", "TOM MID", Role::MidTom, 0.55f, 0.45f, 0.4f, "Tom médium.", "Attaque."),
            row ("lt", "FLOOR TOM", Role::LowTom, 0.6f, 0.5f, 0.4f, "Tom basse : le gros tom posé au sol.", "Attaque."),
            row ("cr", "CRASH", Role::Crash, 0.4f, 0.6f, 0.5f, "Cymbale crash.", "Brillance."),
            row ("rd", "RIDE", Role::Ride, 0.35f, 0.55f, 0.5f, "Cymbale ride : le « ding » qui tient le tempo dans le jazz et le rock.", "Brillance."),
            row ("rb", "RIDE BELL", Role::Cowbell, 0.35f, 0.45f, 0.5f, "La cloche de la ride : son perçant, parfait pour les refrains.", "Brillance."),
        },
        {} });

    defs.push_back ({ TR707, "tr707", "707 / 727", "Boîte à rythmes · inspirée des Roland TR-707 et TR-727 (1985)", MachineKind::Drum,
        0xff3d7fff, 0,
        "La boîte à rythmes à samples de la house de Chicago (les premiers disques house) et de la pop 80s, avec les percussions "
        "latines de sa sœur la 727 : bongos, conga, timbale.\n\n"
        "Son 8 bits sec et punchy : parfait pour la house old school, le freestyle et l'italo disco.",
        {
            row ("bd", "KICK", Role::Kick, 0.85f, 0.4f, 0.5f, "Kick 707 : court et punchy.", "Clic."),
            row ("sd", "SNARE", Role::Snare, 0.7f, 0.4f, 0.55f, "Snare 707.", "Timbre."),
            row ("rs", "RIM", Role::Rim, 0.5f, 0.3f, 0.5f, "Rimshot.", "Brillance."),
            row ("cp", "CLAP", Role::Clap, 0.6f, 0.4f, 0.5f, "Clap 707, sec.", "Couleur."),
            row ("ch", "HAT", Role::ClosedHat, 0.45f, 0.3f, 0.5f, "Charleston fermé.", "Brillance."),
            row ("oh", "OPEN HAT", Role::OpenHat, 0.4f, 0.4f, 0.5f, "Charleston ouvert.", "Brillance."),
            row ("cb", "COWBELL", Role::Cowbell, 0.4f, 0.35f, 0.5f, "Cowbell.", "Couleur."),
            row ("tb", "TAMBOURINE", Role::Tambourine, 0.4f, 0.4f, 0.5f, "Tambourin 707 : sur les contretemps en house.", "Écart."),
            row ("ht", "TOM HIGH", Role::HighTom, 0.55f, 0.4f, 0.4f, "Tom aigu.", "Attaque."),
            row ("lt", "TOM LOW", Role::LowTom, 0.55f, 0.45f, 0.4f, "Tom grave.", "Attaque."),
            row ("bg", "BONGO", Role::HighBongo, 0.5f, 0.3f, 0.5f, "Bongo (727).", "Claque."),
            row ("cg", "CONGA", Role::LowConga, 0.5f, 0.4f, 0.5f, "Conga (727).", "Claque."),
            row ("ti", "TIMBALE", Role::HighTimbale, 0.5f, 0.4f, 0.6f, "Timbale (727).", "Claque."),
        },
        {} });

    defs.push_back ({ Simmons, "simmons", "SIMMONS", "Batterie électronique · inspirée de la Simmons SDS-V (1981)", MachineKind::Drum,
        0xffff3d6e, 0,
        "La batterie électronique hexagonale des années 80 : des toms qui font « piouuu », une caisse claire électrique, un kick "
        "électronique. Le son de la new wave, de la synthpop et des tubes des années 80.\n\n"
        "Les toms en descente (de T1 à T4) sont sa signature.",
        {
            row ("bd", "KICK", Role::Kick, 0.85f, 0.4f, 0.4f, "Kick électronique.", "Bruit d'attaque."),
            row ("sd", "SNARE", Role::Snare, 0.7f, 0.4f, 0.6f, "La snare Simmons : un ton qui plonge et beaucoup de bruit.", "Couleur du bruit."),
            row ("t1", "TOM 1", Role::HighTom, 0.6f, 0.5f, 0.3f, "Tom aigu « piou ».", "Bruit."),
            row ("t2", "TOM 2", Role::MidTom, 0.6f, 0.5f, 0.3f, "Tom médium.", "Bruit."),
            row ("t3", "TOM 3", Role::LowTom, 0.6f, 0.55f, 0.3f, "Tom grave.", "Bruit."),
            row ("t4", "TOM 4", Role::Perc, 0.6f, 0.6f, 0.3f, "Tom très grave.", "Bruit."),
            row ("cp", "CLAP", Role::Clap, 0.6f, 0.4f, 0.5f, "Clap électronique.", "Couleur."),
            row ("ch", "HAT", Role::ClosedHat, 0.45f, 0.3f, 0.5f, "Charleston.", "Brillance."),
            row ("oh", "OPEN HAT", Role::OpenHat, 0.4f, 0.4f, 0.5f, "Charleston ouvert.", "Brillance."),
            row ("cr", "CRASH", Role::Crash, 0.4f, 0.6f, 0.5f, "Cymbale.", "Brillance."),
        },
        {} });

    defs.push_back ({ WorldPerc, "world", "WORLD PERC", "Percussions du monde · tabla, darbouka, djembé, taiko, cajón", MachineKind::Drum,
        0xffe07b39, 0,
        "Les tambours du monde entier : tabla indien (avec la fameuse note qui remonte du « ge »), darbouka orientale, djembé "
        "africain, talking drum, taiko japonais, cajón péruvien et caxixi.\n\n"
        "Ajoute-les par-dessus une batterie, ou fais un rythme entier avec : afro, oriental, bollywood, flamenco…",
        {
            row ("tna", "TABLA NA", Role::HighBongo, 0.5f, 0.35f, 0.6f, "Tabla, petit tambour : le « na » qui sonne comme une cloche.", "Claque."),
            row ("tge", "TABLA GE", Role::LowBongo, 0.55f, 0.5f, 0.3f, "Tabla, grand tambour (bayan) : le « ge » grave dont la note remonte.", "Claque."),
            row ("dum", "DARBOUKA DOUM", Role::LowConga, 0.6f, 0.45f, 0.3f, "Darbouka, coup grave au centre : « doum ».", "Claque."),
            row ("tek", "DARBOUKA TEK", Role::HighConga, 0.5f, 0.25f, 0.6f, "Darbouka, coup sec sur le bord : « tek ».", "Claque."),
            row ("djb", "DJEMBÉ BASSE", Role::Kick, 0.65f, 0.45f, 0.3f, "Djembé, coup de basse au centre.", "Claque."),
            row ("djt", "DJEMBÉ TONE", Role::MidTom, 0.55f, 0.35f, 0.5f, "Djembé, son ouvert.", "Claque."),
            row ("djs", "DJEMBÉ SLAP", Role::Snare, 0.55f, 0.25f, 0.7f, "Djembé, claque sèche.", "Claque."),
            row ("tlk", "TALKING DRUM", Role::LowTimbale, 0.55f, 0.5f, 0.4f, "Le tambour qui « parle » : sa note glisse vers le haut.", "Claque."),
            row ("tai", "TAIKO", Role::LowTom, 0.7f, 0.6f, 0.3f, "Grand tambour japonais : profond et puissant.", "Attaque."),
            row ("cjb", "CAJÓN BASSE", Role::Perc, 0.6f, 0.4f, 0.3f, "Cajón, coup grave au centre de la caisse.", "Attaque."),
            row ("cjs", "CAJÓN SLAP", Role::Clap, 0.55f, 0.35f, 0.6f, "Cajón, claque en haut de la caisse (avec le timbre).", "Timbre."),
            row ("cax", "CAXIXI", Role::Shaker, 0.4f, 0.3f, 0.5f, "Petit hochet en osier africain/brésilien.", "Brillance."),
        },
        {} });

    return defs;
}
} // namespace

const MachineDef& machineDef (int type)
{
    static const std::vector<MachineDef> defs = buildDefs();
    return defs[(size_t) type];
}
} // namespace techno
