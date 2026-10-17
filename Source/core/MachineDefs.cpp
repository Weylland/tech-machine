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
            { "drive", "DRIVE", 0.3f,
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

    return defs;
}
} // namespace

const MachineDef& machineDef (int type)
{
    static const std::vector<MachineDef> defs = buildDefs();
    return defs[(size_t) type];
}
} // namespace techno
