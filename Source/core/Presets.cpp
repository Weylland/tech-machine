#include "Presets.h"
#include <deque>
#include <map>
#include <string>

namespace techno
{
namespace
{
using V = std::vector<std::pair<const char*, float>>;

// Drum kits: the same moods for every kit, written on the rows that play each role.
std::vector<SoundPreset> drumKits (const MachineDef& def)
{
    static std::deque<std::string> keys;   // owns the generated "row.param" keys
    auto key = [] (const char* row, const char* param) { keys.push_back (std::string (row) + "." + param); return keys.back().c_str(); };

    auto rowsFor = [&def] (std::initializer_list<Role> roles)
    {
        std::vector<const char*> ids;
        for (auto& row : def.rows)
            for (auto role : roles)
                if (row.role == role) ids.push_back (row.id);
        return ids;
    };
    const auto kicks = rowsFor ({ Role::Kick });
    const auto snares = rowsFor ({ Role::Snare, Role::Clap, Role::Snap });
    const auto hats = rowsFor ({ Role::ClosedHat, Role::OpenHat, Role::Shaker, Role::Ride });
    const auto echoes = rowsFor ({ Role::Rim, Role::Claves, Role::Cowbell, Role::Perc });

    std::vector<SoundPreset> list;
    list.push_back ({ "Classique (réglages d'origine)", {} });

    V hard { { "@drive", 0.25f } };
    for (auto* k : kicks) { hard.push_back ({ key (k, "drive"), 0.55f }); hard.push_back ({ key (k, "tone"), 0.7f }); hard.push_back ({ key (k, "decay"), 0.55f }); }
    for (auto* s : snares) hard.push_back ({ key (s, "drive"), 0.3f });
    if (! kicks.empty()) list.push_back ({ "Hard techno (kick saturé)", hard });

    V boom;
    for (auto* k : kicks) { boom.push_back ({ key (k, "decay"), 0.95f }); boom.push_back ({ key (k, "tune"), 0.4f }); boom.push_back ({ key (k, "level"), 0.95f }); }
    for (auto* h : hats) boom.push_back ({ key (h, "decay"), 0.2f });
    if (! kicks.empty()) list.push_back ({ "Trap (kick long, charlestons secs)", boom });

    V rumble { { "@filter", 0.46f } };
    for (auto* k : kicks) { rumble.push_back ({ key (k, "reverb"), 0.45f }); rumble.push_back ({ key (k, "decay"), 0.7f }); rumble.push_back ({ key (k, "drive"), 0.3f }); }
    if (! kicks.empty()) list.push_back ({ "Rumble berlinois (kick dans la reverb)", rumble });

    V lofi { { "@crush", 0.45f }, { "@filter", 0.42f } };
    for (auto* h : hats) lofi.push_back ({ key (h, "level"), 0.3f });
    list.push_back ({ "Lo-fi poussiéreux", lofi });

    V tight;
    for (auto& row : def.rows) tight.push_back ({ key (row.id, "decay"), 0.22f });
    list.push_back ({ "Serré et sec", tight });

    V room { { "@reverb", 0.15f } };
    for (auto* s : snares) room.push_back ({ key (s, "reverb"), 0.35f });
    list.push_back ({ "Grande salle (reverb sur la caisse claire)", room });

    V dub;
    for (auto* e : echoes) dub.push_back ({ key (e, "echo"), 0.5f });
    for (auto* s : snares) dub.push_back ({ key (s, "echo"), 0.25f });
    list.push_back ({ "Dub (écho sur les percussions)", dub });

    V wide;
    for (size_t i = 0; i < def.rows.size(); ++i)
        if (def.rows[i].role != Role::Kick && def.rows[i].role != Role::Snare)
            wide.push_back ({ key (def.rows[i].id, "pan"), i % 2 == 0 ? 0.3f : 0.7f });
    list.push_back ({ "Stéréo large", wide });
    return list;
}

std::map<int, std::vector<SoundPreset>> build()
{
    std::map<int, std::vector<SoundPreset>> m;

    m[TB303] = {
        { "Acid classique", { { "wave", 0 }, { "cutoff", 0.35f }, { "reso", 0.7f }, { "env", 0.6f }, { "decay", 0.4f }, { "accent", 0.6f } } },
        { "Squelch (Chicago 1987)", { { "cutoff", 0.25f }, { "reso", 0.85f }, { "env", 0.75f }, { "decay", 0.5f }, { "accent", 0.8f } } },
        { "Acid saturée (hard)", { { "cutoff", 0.4f }, { "reso", 0.8f }, { "env", 0.7f }, { "decay", 0.45f }, { "accent", 0.7f }, { "@drive", 0.6f } } },
        { "Basse caoutchouc", { { "wave", 1 }, { "cutoff", 0.3f }, { "reso", 0.4f }, { "env", 0.35f }, { "decay", 0.3f }, { "accent", 0.4f } } },
        { "Sub acid", { { "octave", 1 }, { "cutoff", 0.2f }, { "reso", 0.5f }, { "env", 0.3f }, { "decay", 0.3f } } },
        { "Lead acid (dans l'écho)", { { "octave", 3 }, { "cutoff", 0.5f }, { "reso", 0.75f }, { "env", 0.6f }, { "decay", 0.5f }, { "@echo", 0.35f } } },
    };

    m[SH101] = {
        { "Basse caoutchouc", { { "octave", 1 }, { "saw", 0 }, { "square", 0.8f }, { "sub", 0.7f }, { "cutoff", 0.3f }, { "reso", 0.3f }, { "env", 0.4f }, { "decay", 0.3f }, { "sustain", 0.2f }, { "release", 0.1f } } },
        { "Sub basse", { { "octave", 1 }, { "saw", 0 }, { "square", 0 }, { "sub", 1 }, { "cutoff", 0.2f }, { "reso", 0 }, { "env", 0.1f }, { "sustain", 0.9f } } },
        { "Bleep", { { "octave", 3 }, { "saw", 0 }, { "square", 0.9f }, { "sub", 0 }, { "cutoff", 0.6f }, { "reso", 0.4f }, { "env", 0.2f }, { "decay", 0.15f }, { "sustain", 0 }, { "release", 0.1f }, { "@echo", 0.4f } } },
        { "Lead 101", { { "saw", 0.9f }, { "square", 0.3f }, { "sub", 0.3f }, { "cutoff", 0.55f }, { "reso", 0.35f }, { "env", 0.4f }, { "decay", 0.4f }, { "sustain", 0.6f }, { "release", 0.2f }, { "glide", 0.2f } } },
        { "Zap laser", { { "saw", 0.8f }, { "cutoff", 0.3f }, { "reso", 0.8f }, { "env", 1 }, { "decay", 0.1f }, { "sustain", 0 } } },
        { "Arpège pluck", { { "saw", 0.8f }, { "square", 0.4f }, { "cutoff", 0.35f }, { "reso", 0.5f }, { "env", 0.6f }, { "decay", 0.2f }, { "sustain", 0 }, { "release", 0.15f }, { "@echo", 0.3f } } },
        { "Caisse claire de bruit", { { "noise", 1 }, { "saw", 0 }, { "square", 0 }, { "sub", 0 }, { "cutoff", 0.7f }, { "env", 0.3f }, { "decay", 0.12f }, { "sustain", 0 } } },
    };

    m[ModelD] = {
        { "Basse Minimoog", { { "wave", 0 }, { "detune", 0.1f }, { "sub", 0.5f }, { "cutoff", 0.3f }, { "emphasis", 0.35f }, { "contour", 0.45f }, { "decay", 0.3f }, { "sustain", 0.3f }, { "overdrive", 0.35f } } },
        { "Basse funk", { { "wave", 1 }, { "cutoff", 0.35f }, { "emphasis", 0.3f }, { "contour", 0.5f }, { "decay", 0.2f }, { "sustain", 0.2f } } },
        { "Reese (drum & bass)", { { "wave", 0 }, { "detune", 0.75f }, { "sub", 0.5f }, { "cutoff", 0.3f }, { "emphasis", 0.2f }, { "contour", 0.15f }, { "decay", 0.6f }, { "sustain", 0.9f }, { "glide", 0.3f }, { "overdrive", 0.45f } } },
        { "Sub profonde", { { "wave", 2 }, { "sub", 0.6f }, { "cutoff", 0.25f }, { "emphasis", 0 }, { "contour", 0.1f }, { "sustain", 0.9f } } },
        { "Lead Moog (G-funk)", { { "octave", 3 }, { "wave", 0 }, { "detune", 0.15f }, { "cutoff", 0.45f }, { "emphasis", 0.4f }, { "contour", 0.4f }, { "attack", 0.05f }, { "decay", 0.5f }, { "sustain", 0.7f }, { "glide", 0.45f } } },
        { "Basse pluck", { { "wave", 0 }, { "cutoff", 0.3f }, { "emphasis", 0.5f }, { "contour", 0.7f }, { "decay", 0.15f }, { "sustain", 0 } } },
    };

    m[Juno106] = {
        { "Nappe Juno", { { "saw", 0.7f }, { "pulse", 0.4f }, { "pwm", 0.4f }, { "cutoff", 0.45f }, { "reso", 0.15f }, { "env", 0.2f }, { "attack", 0.5f }, { "decay", 0.6f }, { "sustain", 0.8f }, { "release", 0.6f }, { "chorus", 2 } } },
        { "Cordes", { { "saw", 0.9f }, { "pulse", 0 }, { "cutoff", 0.55f }, { "attack", 0.4f }, { "release", 0.55f }, { "sustain", 0.9f }, { "chorus", 2 } } },
        { "Cuivres 106", { { "saw", 0.9f }, { "cutoff", 0.35f }, { "env", 0.6f }, { "attack", 0.1f }, { "decay", 0.4f }, { "sustain", 0.7f }, { "release", 0.2f }, { "chorus", 1 } } },
        { "Stab house", { { "saw", 0.8f }, { "cutoff", 0.5f }, { "reso", 0.2f }, { "env", 0.4f }, { "attack", 0 }, { "decay", 0.25f }, { "sustain", 0 }, { "release", 0.2f }, { "chorus", 1 } } },
        { "Accord deep house", { { "saw", 0.6f }, { "pulse", 0.4f }, { "cutoff", 0.38f }, { "reso", 0.3f }, { "env", 0.3f }, { "decay", 0.3f }, { "sustain", 0.1f }, { "release", 0.4f }, { "chorus", 1 }, { "@echo", 0.4f } } },
        { "Orgue 106", { { "saw", 0 }, { "pulse", 0.8f }, { "pwm", 0 }, { "sub", 0.6f }, { "cutoff", 0.6f }, { "attack", 0 }, { "sustain", 1 }, { "release", 0.1f }, { "chorus", 3 } } },
        { "Basse 106", { { "octave", 1 }, { "chord", 0 }, { "saw", 0.7f }, { "sub", 0.8f }, { "cutoff", 0.3f }, { "env", 0.4f }, { "decay", 0.3f }, { "sustain", 0.3f }, { "chorus", 0 } } },
        { "Hoover (approché)", { { "chord", 0 }, { "saw", 0.9f }, { "pulse", 0.6f }, { "pwm", 0.9f }, { "cutoff", 0.6f }, { "env", 0.3f }, { "attack", 0.05f }, { "sustain", 0.9f }, { "chorus", 3 } } },
    };

    m[JP8000] = {
        { "Supersaw trance", { { "chord", 1 }, { "detune", 0.55f }, { "mix", 0.6f }, { "cutoff", 0.6f }, { "env", 0.2f }, { "attack", 0.1f }, { "sustain", 0.8f }, { "release", 0.4f } } },
        { "Lead trance", { { "octave", 3 }, { "chord", 0 }, { "detune", 0.4f }, { "mix", 0.5f }, { "cutoff", 0.7f }, { "attack", 0 }, { "sustain", 0.8f }, { "release", 0.25f }, { "@echo", 0.4f } } },
        { "Hoover", { { "chord", 0 }, { "detune", 0.9f }, { "mix", 0.8f }, { "cutoff", 0.5f }, { "reso", 0.2f }, { "env", 0.3f }, { "attack", 0.05f }, { "sustain", 0.9f }, { "release", 0.3f } } },
        { "Pluck", { { "chord", 1 }, { "detune", 0.3f }, { "mix", 0.4f }, { "cutoff", 0.35f }, { "env", 0.6f }, { "decay", 0.2f }, { "sustain", 0 }, { "release", 0.2f } } },
        { "Lead hardstyle", { { "chord", 0 }, { "detune", 0.7f }, { "mix", 0.7f }, { "cutoff", 0.65f }, { "reso", 0.3f }, { "@drive", 0.4f } } },
        { "Nappe", { { "chord", 1 }, { "detune", 0.45f }, { "mix", 0.5f }, { "cutoff", 0.4f }, { "attack", 0.6f }, { "release", 0.7f }, { "sustain", 0.9f } } },
    };

    {
        auto& dx = m[DX7];
        static const char* const names[] {
            "Piano électrique (E.Piano 1)", "Basse FM (Bass 1)", "Cloches tubulaires (Tub Bells)", "Cuivres (Brass 1)", "Marimba", "Nappe FM",
            "Basse « Lately »", "Basse solide (Solid Bass)", "Basse slap FM", "Piano électrique brillant", "Rhodes doux", "Clavecin (Harpsichord)",
            "Clavinet", "Orgue FM (E.Organ)", "Orgue d'église", "Flûte", "Harmonica", "Cordes FM (Strings 1)", "Glockenspiel", "Vibraphone",
            "Xylophone", "Koto", "Guitare FM", "Sitar FM", "Lead synthé", "Chœur FM", "Timbales d'orchestre", "Carillon (Orch Chime)",
            "Nappe cristal (Glass)", "Sub basse FM",
        };
        for (int i = 0; i < (int) std::size (names); ++i)
        {
            V v { { "preset", (float) i } };
            const bool bass = i == 1 || (i >= 6 && i <= 8) || i == 29;
            if (bass) { v.push_back ({ "octave", 1 }); v.push_back ({ "chord", 0 }); }
            if (i == 2 || i == 18 || i == 19 || i == 20 || i == 27) v.push_back ({ "chord", 0 });
            if (i == 24 || i == 15 || i == 16) { v.push_back ({ "chord", 0 }); v.push_back ({ "octave", 3 }); }
            if (i == 19) v.push_back ({ "tremolo", 0.5f });
            if (i == 0 || i == 10) v.push_back ({ "tremolo", 0.35f });
            dx.push_back ({ names[i], v });
        }
    }

    m[Strings] = {
        { "Solina", {} },
        { "Cordes fines et brillantes", { { "violin", 1 }, { "cello", 0 }, { "brightness", 0.75f }, { "ensemble", 0.6f } } },
        { "Cordes sombres", { { "violin", 0.4f }, { "cello", 0.9f }, { "brightness", 0.3f } } },
        { "Ensemble très large", { { "ensemble", 1 }, { "@reverb", 0.3f } } },
        { "Disco", { { "attack", 0.3f }, { "release", 0.4f }, { "brightness", 0.6f } } },
        { "Nappe lente (ambient)", { { "attack", 0.85f }, { "release", 0.85f }, { "brightness", 0.4f }, { "@reverb", 0.45f } } },
    };

    m[Organ] = {
        { "Tirettes (réglage d'origine)", {} },
        { "Jazz B3", { { "bass", 0.8f }, { "middle", 0.3f }, { "high", 0.1f }, { "percussion", 0.6f }, { "leslie", 0.4f }, { "overdrive", 0.1f }, { "click", 0.4f } } },
        { "Gospel", { { "bass", 1 }, { "middle", 0.8f }, { "high", 0.6f }, { "percussion", 0.3f }, { "leslie", 0.9f }, { "overdrive", 0.2f } } },
        { "Rock saturé", { { "bass", 1 }, { "middle", 0.7f }, { "high", 0.5f }, { "percussion", 0.2f }, { "leslie", 0.8f }, { "overdrive", 0.7f } } },
        { "Reggae (bubble)", { { "bass", 0.5f }, { "middle", 0.6f }, { "high", 0.2f }, { "percussion", 0.1f }, { "leslie", 0.3f }, { "overdrive", 0 }, { "click", 0.6f } } },
        { "Orgue house (M1)", { { "type", 1 } } },
        { "Orgue d'église", { { "bass", 1 }, { "middle", 1 }, { "high", 0.8f }, { "percussion", 0 }, { "leslie", 0 }, { "overdrive", 0 }, { "click", 0 }, { "@reverb", 0.5f } } },
        { "Orgue combo années 60", { { "bass", 0.2f }, { "middle", 0.9f }, { "high", 0.9f }, { "percussion", 0 }, { "leslie", 0.2f }, { "overdrive", 0.3f } } },
    };

    m[Sub808] = {
        { "808 trap", { { "decay", 0.75f }, { "glide", 0.35f }, { "punch", 0.5f }, { "overdrive", 0.35f } } },
        { "808 drill (longs glissés)", { { "decay", 0.8f }, { "glide", 0.6f }, { "punch", 0.4f }, { "overdrive", 0.4f } } },
        { "808 propre", { { "overdrive", 0 }, { "tone", 0.4f } } },
        { "808 saturée", { { "overdrive", 0.8f }, { "tone", 0.8f } } },
        { "808 courte (reggaeton)", { { "decay", 0.4f }, { "punch", 0.6f } } },
    };

    m[Piano] = {
        { "Piano à queue", { { "type", 0 } } },
        { "Piano droit", { { "type", 1 } } },
        { "Piano bastringue (saloon)", { { "type", 2 } } },
        { "Piano pop brillant", { { "type", 3 } } },
        { "Piano feutré (lo-fi)", { { "type", 4 }, { "@crush", 0.15f } } },
        { "Piano house (stabs)", { { "type", 3 }, { "brightness", 0.7f }, { "decay", 0.4f }, { "release", 0.25f }, { "hammer", 0.5f }, { "@reverb", 0.2f } } },
        { "Ballade (avec pédale)", { { "type", 0 }, { "brightness", 0.4f }, { "decay", 0.7f }, { "release", 0.7f }, { "@reverb", 0.35f } } },
        { "Piano électrique à queue (CP-70)", { { "type", 5 } } },
        { "Piano jazz", { { "type", 0 }, { "brightness", 0.45f }, { "release", 0.5f }, { "hammer", 0.3f } } },
    };

    m[Mallets] = {
        { "Glockenspiel", { { "type", 0 } } }, { "Vibraphone", { { "type", 1 }, { "vibrato", 0.6f } } },
        { "Célesta", { { "type", 2 } } }, { "Boîte à musique (trap)", { { "type", 3 }, { "@reverb", 0.3f } } },
        { "Kalimba", { { "type", 4 } } }, { "Steel drum", { { "type", 5 } } },
        { "Cloches tubulaires", { { "type", 6 }, { "@reverb", 0.35f } } }, { "Marimba", { { "type", 7 } } },
        { "Xylophone", { { "type", 8 } } }, { "Crotales", { { "type", 9 }, { "@reverb", 0.25f } } },
        { "Gamelan", { { "type", 10 }, { "vibrato", 0.5f } } }, { "Handpan", { { "type", 11 }, { "@reverb", 0.3f } } },
        { "Cloches d'église", { { "type", 12 }, { "chord", 0 }, { "@reverb", 0.5f } } },
    };

    m[Pluck] = {
        { "Guitare acoustique", { { "type", 0 } } }, { "Guitare nylon", { { "type", 1 } } },
        { "Guitare funk (étouffée)", { { "type", 2 }, { "strum", 0.1f } } }, { "Harpe", { { "type", 3 }, { "strum", 0.6f } } },
        { "Koto", { { "type", 4 }, { "chord", 0 } } }, { "Sitar", { { "type", 5 }, { "chord", 0 } } },
        { "Pizzicato (cordes)", { { "type", 6 } } },
        { "Guitare reggaeton", { { "type", 1 }, { "strum", 0.2f }, { "decay", 0.35f } } },
        { "Banjo", { { "type", 7 } } }, { "Mandoline", { { "type", 8 } } }, { "Ukulélé", { { "type", 9 } } },
        { "Guitare électrique clean", { { "type", 10 }, { "@echo", 0.2f } } }, { "Oud", { { "type", 11 }, { "chord", 0 } } },
        { "Guzheng", { { "type", 12 }, { "chord", 0 } } },
    };

    m[Choir] = {
        { "Chœur « aah »", { { "vowel", 0 } } }, { "Chœur « ooh »", { { "vowel", 1 } } },
        { "Chœur « eeh »", { { "vowel", 2 } } }, { "Bouche fermée « mmh »", { { "vowel", 3 } } },
        { "Grand chœur", { { "vowel", 0 }, { "ensemble", 1 }, { "attack", 0.6f }, { "@reverb", 0.4f } } },
        { "Voix intime", { { "vowel", 1 }, { "ensemble", 0.1f }, { "breath", 0.5f } } },
        { "Voix d'hommes", { { "vowel", 4 }, { "octave", 1 } } },
        { "Voix de femmes", { { "vowel", 0 }, { "octave", 3 }, { "brightness", 0.65f } } },
        { "Chœur années 80 (Fairlight)", { { "vowel", 0 }, { "ensemble", 0.9f }, { "breath", 0.35f }, { "brightness", 0.7f }, { "attack", 0.3f } } },
        { "Chant grégorien", { { "vowel", 4 }, { "octave", 1 }, { "chord", 0 }, { "ensemble", 0.6f }, { "@reverb", 0.6f } } },
    };


    m[EBass] = {
        { "Basse aux doigts", { { "type", 0 } } }, { "Basse slap", { { "type", 1 } } }, { "Basse au médiator", { { "type", 2 } } },
        { "Fretless", { { "type", 3 }, { "glide", 0.5f } } }, { "Contrebasse", { { "type", 4 } } },
        { "Basse dub (ronde)", { { "type", 0 }, { "tone", 0.2f }, { "decay", 0.8f } } },
    };
    m[Wobble] = {
        { "Wobble dubstep", { { "wave", 0 }, { "rate", 2 }, { "depth", 0.7f } } },
        { "Reese (drum & bass)", { { "wave", 1 }, { "depth", 0.1f }, { "detune", 0.7f }, { "cutoff", 0.45f } } },
        { "Growl", { { "wave", 2 }, { "rate", 4 }, { "depth", 0.6f }, { "reso", 0.6f } } },
        { "Wobble lent", { { "wave", 0 }, { "rate", 1 }, { "depth", 0.8f } } },
        { "Wobble en triolets", { { "wave", 0 }, { "rate", 3 }, { "depth", 0.7f } } },
        { "Riddim", { { "wave", 0 }, { "rate", 4 }, { "depth", 0.8f }, { "overdrive", 0.7f } } },
    };
    m[Hoover] = {
        { "Hoover rave", {} },
        { "Hoover gabber", { { "dip", 0.8f }, { "cutoff", 0.9f }, { "@drive", 0.4f } } },
        { "Nappe hoover", { { "attack", 0.5f }, { "dip", 0.2f }, { "chord", 1 } } },
    };
    m[Poly80] = {
        { "Cuivres années 80 (« Jump »)", { { "saw", 1 }, { "pulse", 0 }, { "cutoff", 0.45f }, { "env", 0.6f }, { "attack", 0.08f }, { "decay", 0.5f }, { "sustain", 0.7f } } },
        { "Nappe synthwave", { { "attack", 0.6f }, { "release", 0.7f }, { "cutoff", 0.4f }, { "env", 0.2f }, { "@reverb", 0.3f } } },
        { "Stab années 80", { { "decay", 0.25f }, { "sustain", 0 }, { "env", 0.5f } } },
        { "Basse poly", { { "octave", 1 }, { "chord", 0 }, { "cutoff", 0.3f }, { "env", 0.5f }, { "decay", 0.3f }, { "sustain", 0.3f } } },
        { "Lead synchro", { { "chord", 0 }, { "octave", 3 }, { "pulse", 0.8f }, { "saw", 0.3f }, { "cutoff", 0.7f } } },
    };
    {
        static const char* const names[] { "Piano M1 (house)", "Orgue M1", "Nappe « Lore »", "Nappe « Universe »", "Basse fretless", "Basse slap", "Cordes M1", "Cloche / kalimba" };
        for (int i = 0; i < 8; ++i)
        {
            V v { { "preset", (float) i } };
            if (i == 4 || i == 5) { v.push_back ({ "octave", 1 }); v.push_back ({ "chord", 0 }); }
            if (i == 7) v.push_back ({ "chord", 0 });
            m[M1].push_back ({ names[i], v });
        }
    }
    m[Edm] = {
        { "Accords future bass", { { "detune", 0.5f }, { "mix", 0.6f }, { "cutoff", 0.55f }, { "attack", 0.05f }, { "pump", 0.7f } } },
        { "Pluck EDM", { { "cutoff", 0.35f }, { "env", 0.6f }, { "decay", 0.2f }, { "sustain", 0 }, { "pump", 0.3f } } },
        { "Lead saw", { { "chord", 0 }, { "octave", 3 }, { "detune", 0.35f }, { "cutoff", 0.75f }, { "pump", 0 } } },
        { "Nappe qui pompe", { { "attack", 0.4f }, { "release", 0.6f }, { "pump", 0.9f } } },
    };
    m[Talkbox] = {
        { "Talkbox G-funk (ouah)", { { "from", 1 }, { "to", 0 } } },
        { "Robot (French touch)", { { "from", 5 }, { "to", 4 }, { "speed", 0.2f } } },
        { "Wah", { { "from", 3 }, { "to", 0 }, { "speed", 0.5f } } },
    };
    m[Mellotron] = {
        { "Flûtes", { { "type", 0 } } }, { "Cordes", { { "type", 1 } } }, { "Chœur", { { "type", 2 } } },
        { "Violoncelle", { { "type", 3 } } }, { "Cuivres", { { "type", 4 } } },
        { "Flûtes psychédéliques", { { "type", 0 }, { "wow", 0.85f }, { "@echo", 0.3f } } },
    };
    m[EPiano] = {
        { "Rhodes", { { "type", 0 } } }, { "Rhodes brillant", { { "type", 0 }, { "bark", 0.8f }, { "tone", 0.7f } } },
        { "Wurlitzer", { { "type", 1 } } }, { "Clavinet", { { "type", 2 } } }, { "Clavecin", { { "type", 3 } } },
        { "Rhodes lo-fi", { { "type", 0 }, { "tone", 0.3f }, { "tremolo", 0.5f }, { "@crush", 0.2f } } },
    };
    m[Brass] = {
        { "Trompette", { { "type", 0 } } }, { "Trombone", { { "type", 1 }, { "octave", 1 } } }, { "Saxophone", { { "type", 2 } } },
        { "Section funk", { { "type", 3 }, { "chord", 1 } } }, { "Stab cuivres", { { "type", 4 }, { "chord", 1 } } }, { "Cor", { { "type", 5 } } },
    };
    m[Winds] = {
        { "Flûte", { { "type", 0 } } }, { "Flûte de pan", { { "type", 1 } } }, { "Clarinette", { { "type", 2 }, { "octave", 2 } } },
        { "Shakuhachi", { { "type", 3 }, { "@reverb", 0.35f } } }, { "Ocarina", { { "type", 4 } } }, { "Flûte trap", { { "type", 5 } } },
    };
    m[Orchestra] = {
        { "Cordes d'orchestre", { { "type", 0 } } }, { "Pizzicato", { { "type", 1 } } }, { "Orch hit", { { "type", 2 } } },
        { "Timbales", { { "type", 3 }, { "chord", 0 }, { "octave", 1 } } }, { "Harpe", { { "type", 4 } } },
        { "Cordes épiques", { { "type", 0 }, { "ensemble", 1 }, { "attack", 0.55f }, { "@reverb", 0.45f } } },
    };

    for (int t = 0; t < numMachines; ++t)
        if (machineDef (t).kind == MachineKind::Drum && t != SoundFx && t != Sampler)
            m[t] = drumKits (machineDef (t));

    // 808 extras.
    m[TR808].push_back ({ "Miami bass (kick infini)", { { "bd.decay", 1.0f }, { "bd.tune", 0.3f }, { "bd.level", 1.0f }, { "cb.level", 0.5f } } });
    return m;
}
} // namespace

const std::vector<SoundPreset>& presetsFor (int machineType)
{
    static const auto all = build();
    static const std::vector<SoundPreset> none;
    if (auto it = all.find (machineType); it != all.end()) return it->second;
    return none;
}
} // namespace techno
