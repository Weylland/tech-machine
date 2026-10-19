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
            "1. Ouvre un PROJET EXEMPLE (menu PROJET › Exemples, ou l'écran d'accueil) pour entendre ce que la machine sait faire, "
            "ou pars d'un PROJET VIDE.\n\n"
            "2. Ajoute des machines avec « + MACHINE » : une boîte à rythmes, une basse, des accords… Elles arrivent vides.\n\n"
            "3. Clique sur les cases pour placer des sons. Chaque colonne est un « pas » : 16 pas = 1 mesure = 4 temps. "
            "Sur les notes et les accords, glisse vers le haut ou le bas pour changer la hauteur. "
            "Sur la batterie, clic droit (ou molette) sur une case = roulement ×2, ×3, ×4 (les charlestons rapides du trap).\n\n"
            "4. Envie d'un rythme connu ? Le bouton PATTERNS de chaque machine propose les grands classiques : Four on the Floor, "
            "Motown, Billie Jean, Amen Break, boom bap, trap, reggaeton… et des lignes de basse, accords et arpèges. "
            "Le bouton SONS charge des sons célèbres prêts à jouer, PIANO ROLL ouvre une grille de notes.\n\n"
            "5. En panne d'idée ? Choisis un GENRE puis clique sur SURPRISE ! (tout le rack) ou sur HASARD d'une machine "
            "(Nouvelle idée, Varier un peu, Plus de notes, Moins de notes).\n\n"
            "6. Appuie sur LECTURE (barre Espace) et tourne les boutons pendant que ça joue. Double-clic sur un bouton = valeur d'origine.\n\n"
            "7. Une erreur ? Ctrl+Z annule, Ctrl+Y rétablit.\n\n"
            "8. Enregistre ton projet (Ctrl+S) et exporte-le en WAV depuis le menu PROJET.\n\n"
            "En bas à droite : AUDIO / MIDI (carte son, ASIO, clavier MIDI), PLEIN ÉCRAN (F11) et la langue (FRANÇAIS / ENGLISH). "
            "Clique sur une machine pour qu'elle reçoive ton clavier MIDI.\n\n"
            "Le petit « i » à côté de chaque élément explique à quoi il sert. Survole un bouton pour un résumé rapide." } },

        { "play", { "Lecture / Stop",
            "Lance ou arrête la musique. Raccourci : barre Espace.\n\n"
            "La lecture repart toujours du début de la séquence. En plugin (VST3), la lecture suit ton logiciel de musique." } },
        { "tempo", { "Tempo (BPM)",
            "Vitesse du morceau en battements par minute. Glisse vers le haut ou le bas sur l'écran (ou molette) pour le changer.\n\n"
            "Repères : reggae 70-85, lo-fi 70-90, hip-hop 85-95, reggaeton 90-100, funk 95-115, disco 115-125, "
            "house 120-128, techno 125-135, trance 136-140, trap 130-150, psytrance 140-148, hard techno 145-160, "
            "drum & bass 170-176, gabber 170-200. En plugin, le tempo suit ton logiciel de musique." } },
        { "swing", { "Swing",
            "Décale légèrement un pas sur deux pour donner un groove « chaloupé » au lieu d'un rythme robotique.\n\n"
            "0 = parfaitement droit (techno, trance, trap). Un peu = groove house, funk. Beaucoup = hip-hop, lo-fi, UK garage." } },
        { "genre", { "Genre du hasard",
            "Le genre dont s'inspirent SURPRISE ! et le dé de chaque machine pour écrire des rythmes, des lignes de basse, "
            "des mélodies et des accords typiques.\n\n"
            "Ça ne change ni tes sons ni tes notes déjà écrites. Sur un projet encore vide, le tempo, le swing et la gamme "
            "s'adaptent aussi au genre choisi." } },
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
            "Écrit une nouvelle idée complète pour toutes les machines du rack, dans le genre choisi : la batterie d'abord, "
            "puis une basse qui évite le kick, des accords et des mélodies qui vont ensemble.\n\n"
            "Les sons et réglages ne changent pas. Pas convaincu ? Ctrl+Z revient en arrière." } },
        { "rec", { "Enregistrer en direct",
            "Enregistre tout ce que tu entends, en temps réel, dans un fichier WAV (dossier Musique / Tech Machine). "
            "Pratique pour garder une improvisation où tu tournes les boutons.\n\n"
            "Pour exporter proprement la boucle, utilise plutôt PROJET › Exporter en WAV." } },
        { "files", { "Fichiers enregistrés",
            "Ouvre le dossier où se trouve ton dernier enregistrement." } },
        { "add", { "Ajouter une machine",
            "Ouvre la liste des machines disponibles : boîtes à rythmes, basses, synthés. Une machine ajoutée arrive vide et sans effet.\n\n"
            "Tu peux en retirer une à tout moment avec son ✕ : ses réglages et ses notes sont gardés si tu la remets." } },
        { "project", { "Projet",
            "Tout ton morceau : machines, notes, sons, effets, tempo. Depuis ce menu : nouveau projet vide, ouvrir, enregistrer, "
            "projets exemples, export WAV.\n\n"
            "Raccourcis : Ctrl+N nouveau, Ctrl+O ouvrir, Ctrl+S enregistrer. Un point « • » à côté du nom = modifications pas encore enregistrées." } },
        { "undo", { "Annuler / Rétablir",
            "Annule ta dernière action (notes, hasard, réglages) ou la refait. Raccourcis : Ctrl+Z et Ctrl+Y." } },
        { "export", { "Exporter en WAV",
            "Crée un fichier audio WAV de ta boucle, répétée 1 à 8 fois, avec la fin des échos et de la reverb. "
            "Le rendu se fait hors temps réel : la qualité est parfaite, même si ton ordinateur est lent." } },
        { "machine.diceMenu", { "Hasard",
            "Nouvelle idée : écrit une nouvelle partie pour cette machine, dans le genre choisi.\n\n"
            "Varier un peu : change quelques notes ou coups sans casser l'idée.\n\n"
            "Plus de notes / Moins de notes : rend la partie plus chargée ou plus aérée.\n\n"
            "Tout s'annule avec Ctrl+Z." } },
        { "machine.patterns", { "Patterns célèbres",
            "Une bibliothèque de rythmes et de parties connus, prêts à jouer : Four on the Floor, Motown, Billie Jean, "
            "Funky Drummer, Amen Break, boom bap, trap, reggaeton, bossa nova, one drop...\n\n"
            "Sur une boîte à rythmes : choisis un style puis un rythme. Le tempo d'origine est indiqué entre parenthèses.\n"
            "Sur un synthé : lignes de basse, accords ou arpèges. Les notes suivent automatiquement les accords et la gamme du morceau.\n\n"
            "Le pattern remplace la partie de la machine. Ctrl+Z pour revenir en arrière." } },
        { "machine.sounds", { "Sons prêts à jouer",
            "Des sons tout faits pour cette machine : les plus célèbres (piano électrique du DX7, acid de la 303, supersaw de la trance, "
            "orgue gospel, 808 du trap…) ou, sur une boîte à rythmes, des kits (hard techno, trap, lo-fi, dub…).\n\n"
            "Ça remplace les réglages du son, pas tes notes. Tu peux ensuite tourner les boutons pour l'adapter." } },
        { "parts", { "Parties",
            "Un morceau est fait de plusieurs parties : une intro, un couplet, un refrain, un break… Chaque partie (A à H) a "
            "ses propres notes et sa propre automation, pour toutes les machines ; les sons et les réglages des boutons sont communs.\n\n"
            "Clique sur une partie pour l'afficher et l'écrire. Pendant la lecture, la nouvelle partie démarre à la fin de la "
            "boucle en cours (elle clignote en attendant), pour rester en rythme. Clic droit : copier une partie dans une autre "
            "(pour partir d'une base et la varier) ou l'effacer." } },
        { "song", { "Morceau",
            "La liste des parties à jouer dans l'ordre, chacune un certain nombre de fois : par exemple A ×2, B ×4, C ×1, B ×4.\n\n"
            "MORCEAU ouvre cette liste pour la composer. Avec JOUER LE MORCEAU allumé, la lecture suit la liste au lieu de "
            "boucler sur une partie, et l'export WAV enregistre tout le morceau." } },
        { "machine.automation", { "Automation",
            "Fait changer les réglages de cette machine au fil des pas : un filtre qui s'ouvre petit à petit, un kick qui "
            "s'allonge à la fin de la mesure, un écho qui n'arrive que sur certaines notes…\n\n"
            "Choisis un réglage, puis dessine sa valeur pas par pas à la souris. Un pas sans valeur suit le bouton. "
            "Tu peux aussi enregistrer en tournant les boutons pendant la lecture (ENREG. BOUTONS, en haut). "
            "Le bouton AUTOMATION s'allume quand la machine a des réglages automatisés (8 au maximum)." } },
        { "automation.record", { "Enregistrer les boutons",
            "Allume-le, lance la lecture et tourne les boutons d'une machine : leurs mouvements s'écrivent dans les pas, "
            "comme une automation dans un logiciel de musique. Ils se rejouent ensuite à chaque tour.\n\n"
            "Pour corriger ou effacer, ouvre AUTOMATION sur la machine. Tout s'annule avec Ctrl+Z." } },
        { "machine.roll", { "Piano roll",
            "Ouvre un grand écran pour écrire les notes comme dans les logiciels de musique : les hauteurs de bas en haut, "
            "toute la séquence de gauche à droite.\n\n"
            "Clique pour poser une note (glisse vers la droite pour l'allonger), fais glisser une note pour la déplacer, tire son "
            "bord droit pour changer sa durée, clic droit ou double-clic pour l'effacer. Ctrl+glisser sélectionne plusieurs notes, "
            "les flèches les déplacent ou les transposent, Ctrl+D les duplique.\n\n"
            "Sur les synthés polyphoniques, chaque note a sa propre durée et le mode « Poser : un accord » place un accord juste "
            "en un clic. Les lignes grisées sont hors de la gamme choisie. Échap ou FERMER pour revenir au rack." } },
        { "machine.more", { "Outils",
            "Copier / coller la partie, dupliquer la 1re mesure sur toute la séquence, décaler toute la partie d'un pas "
            "vers la gauche ou la droite, ou tout effacer." } },
        { "drum.fx", { "Effets de ce son",
            "Des effets rien que pour ce son de batterie : disto, filtre, position gauche/droite, envoi dans l'écho et la reverb.\n\n"
            "Exemple : de la disto seulement sur le kick, ou de l'écho seulement sur le rimshot. "
            "Le bouton FX s'allume quand un effet est actif." } },

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
