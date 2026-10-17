#include "Help.h"
#include "Music.h"
#include <map>

namespace techno
{
namespace
{
const std::map<juce::String, std::pair<const char*, const char*>>& entries()
{
    static const std::map<juce::String, std::pair<const char*, const char*>> map {
        { "guide", { "Bien démarrer",
            "1. Appuie sur LECTURE (ou la barre Espace) : la musique démarre.\n\n"
            "2. Choisis un STYLE (Techno, House, Drum & Bass…) : le tempo, la gamme, les sons et les rythmes sont réglés pour toi.\n\n"
            "3. Clique sur SURPRISE ! pour générer de nouveaux rythmes et mélodies dans ce style, autant de fois que tu veux.\n\n"
            "4. Clique sur les cases pour ajouter ou enlever des sons. Chaque colonne est un « pas » : 16 pas = 1 mesure = 4 temps.\n\n"
            "5. Tourne les boutons pendant que ça joue : c'est comme ça qu'on fait vivre un morceau. Double-clic sur un bouton = retour à sa valeur d'origine.\n\n"
            "6. Ajoute ou retire des machines avec « + AJOUTER UNE MACHINE » et le ✕ de chaque machine.\n\n"
            "7. Appuie sur REC pour enregistrer ce que tu joues en fichier WAV.\n\n"
            "Le petit « i » à côté de chaque élément explique à quoi il sert. Survole un bouton pour un résumé rapide." } },

        { "play", { "Lecture / Stop",
            "Lance ou arrête la musique. Raccourci : barre Espace.\n\n"
            "La lecture repart toujours du début de la séquence." } },
        { "tempo", { "Tempo (BPM)",
            "Vitesse du morceau en battements par minute. Glisse horizontalement pour le changer.\n\n"
            "Repères : house 120-128, techno 125-135, trance 136-140, psytrance 140-148, hard techno 145-160, "
            "drum & bass 170-176, gabber 170-200." } },
        { "swing", { "Swing",
            "Décale légèrement un pas sur deux pour donner un groove « chaloupé » au lieu d'un rythme robotique.\n\n"
            "0 = parfaitement droit (techno, trance). Un peu = groove house. Beaucoup = UK garage, shuffle." } },
        { "style", { "Style",
            "Choisis un genre musical : le tempo, le swing, la gamme et les réglages de son sont adaptés, et de nouveaux rythmes "
            "typiques du style sont générés pour toutes les machines affichées.\n\n"
            "Clique sur le « i » du style choisi (sous la barre) pour savoir quelles machines lui vont le mieux." } },
        { "key", { "Tonalité",
            "La note de base du morceau : toutes les mélodies et accords tournent autour d'elle.\n\n"
            "Ça change seulement la hauteur générale, pas l'ambiance. La techno est souvent en A (La), F (Fa) ou G (Sol)." } },
        { "scale", { "Gamme",
            "L'ensemble des notes autorisées. Elle donne l'ambiance : sombre, joyeuse, orientale…\n\n"
            "Tant qu'une gamme est choisie, impossible de faire une fausse note : les notes se placent automatiquement dessus. "
            "« Chromatique » enlève cette aide et autorise toutes les notes." } },
        { "length", { "Longueur de la séquence",
            "Nombre de mesures de la boucle : 1 mesure = 16 pas = 4 temps.\n\n"
            "Avec 2 ou 4 mesures tu peux faire évoluer le rythme et la mélodie (variations, changements d'accords, roulements). "
            "Quand tu allonges, les mesures existantes sont recopiées pour que rien ne soit vide." } },
        { "page", { "Page affichée",
            "Quand la séquence fait plus d'une mesure, l'écran en montre une à la fois. Clique sur un numéro pour voir et modifier cette mesure.\n\n"
            "« SUIVRE » fait défiler automatiquement les pages pendant la lecture." } },
        { "follow", { "Suivre la lecture",
            "Activé : l'affichage passe automatiquement à la mesure en train d'être jouée.\n\n"
            "Désactive-le pour modifier tranquillement une mesure pendant que les autres jouent." } },
        { "surprise", { "Surprise !",
            "Génère de nouveaux rythmes, mélodies et accords dans le style choisi, pour toutes les machines affichées. "
            "Les sons et réglages ne changent pas.\n\n"
            "Clique autant de fois que tu veux jusqu'à trouver une idée qui te plaît, puis modifie-la à ta façon." } },
        { "rec", { "Enregistrer",
            "Enregistre tout ce que tu entends dans un fichier WAV (qualité studio) dans ton dossier Musique / Tech Machine.\n\n"
            "Clique une fois pour commencer (la lecture démarre), une deuxième fois pour arrêter." } },
        { "files", { "Fichiers enregistrés",
            "Ouvre le dossier où se trouve ton dernier enregistrement." } },
        { "add", { "Ajouter une machine",
            "Ouvre la liste des machines disponibles : boîtes à rythmes, basses, synthés. Choisis celles que tu veux voir et jouer.\n\n"
            "Tu peux en retirer une à tout moment avec son ✕ : ses réglages et ses notes sont gardés si tu la remets." } },
        { "recommended", { "Machines conseillées",
            "Ajoute d'un coup les machines typiques du style choisi et génère leurs parties." } },

        { "row.accent", { "Accent",
            "Les pas marqués d'un accent jouent plus fort, comme sur la vraie machine.\n\n"
            "Mets des accents sur certains temps pour donner du relief et du groove au rythme. Le bouton ACCENT des effets règle la différence de volume." } },
        { "row.notes", { "Notes",
            "Chaque case est un pas. Clique pour jouer une note sur ce pas (clique à nouveau pour l'enlever).\n\n"
            "Pour changer la hauteur : clique-glisse vers le haut ou le bas, ou utilise la molette sur la case. "
            "La note reste toujours dans la gamme choisie." } },
        { "row.chords", { "Accords",
            "Chaque case allumée joue un accord. Clique pour l'activer, puis clique-glisse vers le haut/bas (ou molette) pour changer d'accord.\n\n"
            "Le nom de l'accord est affiché dans la case. Le type d'accord se règle avec le bouton ACCORD." } },
        { "row.synthAccent", { "Accent",
            "La note de ce pas est jouée plus fort et plus brillante. Sur la 303, plusieurs accents d'affilée font monter le filtre "
            "de plus en plus : c'est le « cri » de l'acid." } },
        { "row.slide", { "Slide (glissé)",
            "La note de ce pas glisse jusqu'à la note suivante sans être rejouée, comme un glissando.\n\n"
            "Mets des slides entre deux notes de hauteur différente pour entendre l'effet. C'est la signature de la 303." } },
        { "row.tie", { "Tenir",
            "Garde l'accord enfoncé jusqu'au pas suivant au lieu de le couper. Si le pas suivant a le même accord, il continue sans être rejoué.\n\n"
            "Allume plusieurs TENIR d'affilée pour faire une nappe longue." } },

        { "drum.level", { "Niveau", "Volume de ce son. Monte le kick, baisse les charlestons s'ils sont trop présents." } },
        { "drum.tune", { "Hauteur", "Rend le son plus grave (à gauche) ou plus aigu (à droite)." } },
        { "drum.decay", { "Durée", "Durée du son. Court = sec et serré. Long = le son résonne plus longtemps." } },
        { "drum.tone", { "Timbre",
            "Change le caractère du son. Son effet dépend de l'instrument (clic du kick, grésillement de la snare, brillance des charlestons…). "
            "Clique sur le « i » du bouton pour le détail." } },
        { "drum.rowMute", { "Couper / remettre ce son",
            "Clique sur le nom pour couper ce son sans effacer ses notes. Clique à nouveau pour le remettre.\n\n"
            "Pratique pour entendre ce que fait chaque élément, ou pour faire des breaks." } },

        { "machine.mute", { "Couper la machine",
            "Coupe le son de toute la machine sans effacer ses notes. Idéal pour faire entrer et sortir les instruments pendant un morceau." } },
        { "machine.solo", { "Solo",
            "N'écoute que cette machine (les autres sont coupées). Tu peux mettre plusieurs machines en solo." } },
        { "machine.dice", { "Hasard",
            "Génère une nouvelle partie pour cette machine seulement, dans le style choisi. Les sons ne changent pas." } },
        { "machine.clear", { "Effacer",
            "Efface toutes les notes de cette machine (sur toute la séquence). Les réglages de son sont gardés." } },
        { "machine.collapse", { "Replier", "Réduit la machine à sa barre de titre pour gagner de la place. Elle continue de jouer." } },
        { "machine.remove", { "Retirer la machine",
            "Retire la machine du rack : elle s'arrête de jouer. Ses notes et réglages sont gardés si tu la remets avec « + AJOUTER UNE MACHINE »." } },

        { "strip.drive", { "Disto",
            "Saturation : pousse le son jusqu'à ce qu'il sature. Un peu = son plus chaud et présent. Beaucoup = son sale et agressif.\n\n"
            "Super sur un kick (hard techno, gabber) ou une basse acid." } },
        { "strip.crush", { "Crush (lo-fi)",
            "Réduit la qualité numérique (moins de bits, fréquence d'échantillonnage plus basse), comme les vieux samplers et jeux vidéo.\n\n"
            "Un peu = grain « vintage ». Beaucoup = son destroy, robotique." } },
        { "strip.filter", { "Filtre DJ",
            "Au centre : aucun effet. Vers la gauche : coupe les aigus (son étouffé, « sous l'eau »). Vers la droite : coupe les basses (son fin, « au téléphone »).\n\n"
            "Le geste classique du DJ pour faire monter ou retomber l'énergie." } },
        { "strip.echo", { "Envoi Écho",
            "Quantité de son envoyée dans l'écho (delay) : le son est répété en rebondissant à gauche et à droite, en rythme.\n\n"
            "Le temps et le nombre de répétitions se règlent dans le MASTER." } },
        { "strip.reverb", { "Envoi Reverb",
            "Quantité de son envoyée dans la réverbération : le son semble joué dans une grande salle.\n\n"
            "Un peu = de la profondeur. Beaucoup = son lointain et planant. La taille de la salle se règle dans le MASTER." } },
        { "strip.volume", { "Volume", "Volume général de cette machine dans le mix." } },
        { "strip.pan", { "Panoramique", "Place le son à gauche ou à droite dans la stéréo. Au centre par défaut." } },
        { "strip.accent", { "Force de l'accent",
            "Différence de volume entre les pas accentués et les autres. À zéro, les accents ne changent rien." } },

        { "master.echoTime", { "Temps de l'écho",
            "Écart entre les répétitions de l'écho, calé sur le tempo. 1/8 pointée = le classique de la dub techno et de la trance." } },
        { "master.echoFeedback", { "Répétitions",
            "Nombre de répétitions de l'écho. Bas = un ou deux rebonds. Haut = l'écho se répète longtemps (attention, il peut s'accumuler)." } },
        { "master.echoLevel", { "Niveau de l'écho", "Volume général de l'écho. Les machines y envoient leur son avec leur bouton ÉCHO." } },
        { "master.reverbSize", { "Taille de la salle", "Petite pièce à gauche, cathédrale à droite." } },
        { "master.reverbLevel", { "Niveau de la reverb", "Volume général de la réverbération. Les machines y envoient leur son avec leur bouton REVERB." } },
        { "master.pump", { "Pompe (sidechain)",
            "Baisse le volume des synthés à chaque coup de kick, puis le remonte : ça « respire » au rythme du kick.\n\n"
            "C'est l'effet de pompage typique de la house et de la techno, qui laisse aussi de la place au kick." } },
        { "master.filter", { "Filtre DJ master",
            "Le même filtre que sur chaque machine, mais sur tout le morceau. Centre = aucun effet. Gauche = étouffé, droite = fin.\n\n"
            "Fais-le glisser lentement pour une montée ou une descente façon DJ." } },
        { "master.comp", { "Compresseur",
            "Resserre le son : les parties fortes sont baissées, l'ensemble paraît plus compact, plus fort et plus « collé ».\n\n"
            "Un peu = mix plus pro. Beaucoup = son écrasé et énergique." } },
        { "master.disto", { "Disto master",
            "Sature tout le mix. Un peu = chaleur analogique. Beaucoup = son sale, hard techno et industriel." } },
        { "master.volume", { "Volume général",
            "Volume de sortie. Un limiteur protège tes oreilles et tes enceintes : le son ne dépasse jamais -1 dB." } },
        { "master.meter", { "Vumètre", "Montre le niveau de sortie. Si la barre est souvent tout en haut, baisse un peu le volume des machines." } },
    };
    return map;
}
} // namespace

HelpText help (const juce::String& key)
{
    const auto& map = entries();
    if (auto it = map.find (key); it != map.end())
        return { utf8 (it->second.first), utf8 (it->second.second) };
    return { key, {} };
}

juce::String shortHelp (const juce::String& body)
{
    const int end = body.indexOfAnyOf (".\n");
    return end > 0 ? body.substring (0, end + 1) : body;
}
} // namespace techno
