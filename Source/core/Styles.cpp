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
        case 'X': case 'x': return true;
        case 'o': return r.nextFloat() < 0.5f;
        case 'r': return r.nextFloat() < 0.2f;
        default: return false;
    }
}

bool isPercussion (Role role)
{
    return role == Role::Rim || role == Role::Claves || role == Role::Cowbell
        || role == Role::LowTom || role == Role::MidTom || role == Role::HighTom;
}

bool onlyScattered (const char* pattern)
{
    for (const char* p = pattern; *p != 0; ++p)
        if (*p == 'x' || *p == 'X' || *p == 'o') return false;
    return true;
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
    if (role == Role::Clap && find (Role::Ghost) >= 0) return find (Role::Snare);
    if (role == Role::Cymbal) return find (Role::Crash);
    if (role == Role::Crash) return find (Role::Cymbal);
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
            }
        }
    };

    for (auto& part : ctx.style.drums) apply (part, 0, bars - 1);
    if (bars > 1)
        for (auto& part : ctx.style.fill) apply (part, bars - 1, bars - 1);

    // A closed hat never plays together with the open hat, nor a ghost note with the snare.
    const int ch = rowFor (def, Role::ClosedHat), oh = rowFor (def, Role::OpenHat);
    const int sd = rowFor (def, Role::Snare), gs = rowFor (def, Role::Ghost);
    for (int s = 0; s < ctx.steps; ++s)
    {
        if (ch >= 0 && oh >= 0 && t.hits[(size_t) oh][(size_t) s]) t.hits[(size_t) ch][(size_t) s] = false;
        if (sd >= 0 && gs >= 0 && t.hits[(size_t) sd][(size_t) s]) t.hits[(size_t) gs][(size_t) s] = false;
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

void leadLine (Line& line, const GenerateContext& ctx, LeadMode mode, juce::Random& r)
{
    const int steps = ctx.steps, bars = steps / pageSteps;

    switch (mode)
    {
        case LeadMode::Motif:
        {
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
        default: break;
    }
    line.write (track, ctx);
}

void shiftTrack (Track& t, int steps, int direction)
{
    Track copy;
    for (int s = 0; s < steps; ++s)
    {
        const auto src = (size_t) s, dst = (size_t) ((s + direction + steps) % steps);
        for (size_t row = 0; row < t.hits.size(); ++row) copy.hits[row][dst] = t.hits[row][src].load();
        copy.on[dst] = t.on[src].load();
        copy.accent[dst] = t.accent[src].load();
        copy.slide[dst] = t.slide[src].load();
        copy.note[dst] = t.note[src].load();
    }
    for (int s = 0; s < steps; ++s)
    {
        const auto i = (size_t) s;
        for (size_t row = 0; row < t.hits.size(); ++row) t.hits[row][i] = copy.hits[row][i].load();
        t.on[i] = copy.on[i].load();
        t.accent[i] = copy.accent[i].load();
        t.slide[i] = copy.slide[i].load();
        t.note[i] = copy.note[i].load();
    }
    t.tile (steps);
}
} // namespace techno
