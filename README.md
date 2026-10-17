# Tech Machine

Une groovebox pour faire de la techno (et de la house, de la trance, de la drum & bass…) sans rien y connaître.
Huit machines légendaires recréées par synthèse, un séquenceur pas à pas, des styles prêts à l'emploi
et une explication derrière chaque bouton.

![Tech Machine](docs/screenshot.png)

## Télécharger

Va dans [Releases](../../releases/latest) et prends :

- **TechMachine-Windows-Standalone.zip** : l'application seule. Dézippe et lance `Tech Machine.exe`.
- **TechMachine-Windows-VST3.zip** : le plugin pour ton logiciel de musique (Ableton, FL Studio, Reaper, Bitwig…).
  Dézippe et copie le dossier `Tech Machine.vst3` dans `C:\Program Files\Common Files\VST3`, puis relance un scan des plugins.

Windows 10/11 64 bits. L'exe n'est pas signé : au premier lancement, Windows SmartScreen peut afficher
« Windows a protégé votre ordinateur » → « Informations complémentaires » → « Exécuter quand même ».

## Les machines

Chaque machine est entièrement synthétisée (aucun sample) en imitant les circuits des originaux.

| Machine | Inspirée de | Pour quoi faire |
|---|---|---|
| **909** | Boîte à rythmes Roland TR-909 (1983) | Le son techno / house : kick, snare, 3 toms, rim, clap, hats et cymbales au grain 6 bits |
| **808** | Boîte à rythmes Roland TR-808 (1980) | Kick « boom », cowbell, claves, maracas : électro, Detroit |
| **Break Kit** | Breakbeats samplés (E-mu SP-1200) | Batterie acoustique lo-fi pour la jungle, la drum & bass, le breakbeat |
| **303** | Roland TB-303 (1981) | Lignes de basse acid : filtre résonant, accent, slide |
| **101** | Roland SH-101 (1982) | Basses et leads : saw, square + PWM, sub, bruit, ADSR, glide |
| **D-Bass** | Minimoog Model D (1970) | Grosses basses, sub, basse « Reese » de la drum & bass |
| **106** | Roland Juno-106 (1984) | Accords et nappes avec le fameux chorus |
| **8000** | Roland JP-8000 (1996) | Supersaw de la trance, en accords ou en lead |

Les noms des constructeurs sont cités uniquement pour décrire les instruments qui ont inspiré ces machines.
Ce projet n'est ni affilié ni approuvé par Roland ou Moog.

## Ce qu'on peut faire

- **Projets** : nouveau projet vide, ouvrir, enregistrer (`.techmachine`), projets récents, écran d'accueil.
- **10 projets exemples** écrits à la main : Techno, Acid, Detroit, House, Berlin, Trance, Psytrance,
  Drum & Bass, Dub Techno, Électro.
- **Choisir ses machines** : n'affiche que ce que tu veux (une 909 seule, ou tout le studio). Une machine ajoutée arrive vide.
- **Hasard musical** dans 16 genres : la basse évite le kick et suit les accords, les mélodies répondent à un motif,
  les percussions sont réparties en rythmes euclidiens. Par machine : nouvelle idée, varier un peu, plus ou moins de notes.
- **21 gammes + mode chromatique** : impossible de faire une fausse note tant qu'une gamme est choisie.
- **Séquences de 1 à 4 mesures**, avec copier / coller, dupliquer la mesure 1, décaler d'un pas.
- **Effets par son de batterie** (disto, filtre, pan, écho, reverb : de la disto rien que sur le kick, par exemple),
  **par machine** (disto, crush, filtre DJ, écho, reverb, volume, pan) et **sur le master**
  (écho façon bande, reverb FDN, pompe sidechain, filtre DJ, compresseur, disto, limiteur).
- **Annuler / rétablir** sur tout (notes, hasard, réglages).
- **Export WAV** hors temps réel (1 à 8 boucles) et enregistrement en direct.
- **En VST3**, la lecture, le tempo et la position suivent ton logiciel de musique.
- **Un « i » partout** : clique dessus (ou clic droit sur un bouton) pour savoir à quoi sert chaque élément.

Raccourcis : **Espace** lecture / stop · **Ctrl+Z / Ctrl+Y** annuler / rétablir · **Ctrl+S** enregistrer ·
**Ctrl+O** ouvrir · **Ctrl+N** nouveau projet.

## Compiler soi-même

Prérequis : Windows, [Visual Studio 2019+ ou Build Tools](https://visualstudio.microsoft.com/fr/downloads/)
avec la charge de travail « Développement Desktop en C++ », [CMake 3.22+](https://cmake.org/download/) et Git.

```bash
git clone --recursive https://github.com/Weylland/tech-machine.git
cd tech-machine
build.bat
```

Résultats :

- `build\TechMachine_artefacts\Release\Standalone\Tech Machine.exe`
- `build\TechMachine_artefacts\Release\VST3\Tech Machine.vst3`

Stack : C++17, [JUCE 8](https://juce.com) (sous-module dans `libs/JUCE`), CMake.

## Licence

[GNU AGPL v3](LICENSE), comme la version open source de JUCE.
