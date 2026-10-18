#include "PatternLibrary.h"
#include "Music.h"

namespace techno
{
namespace
{
constexpr Role K = Role::Kick, S = Role::Snare, C = Role::Clap, R = Role::Rim, H = Role::ClosedHat, O = Role::OpenHat,
               Y = Role::Ride, Cb = Role::Cowbell, Cl = Role::Claves, Sh = Role::Shaker, Lt = Role::LowTom, Ht = Role::HighTom,
               Lc = Role::LowConga, Hc = Role::HighConga, Ag = Role::Agogo, Tb = Role::Tambourine, Sn = Role::Snap,
               Th = Role::HighTimbale, Lb = Role::LowBongo, Hb = Role::HighBongo, Tl = Role::LowTimbale, Tr = Role::Triangle,
               Gu = Role::Guiro;

const char* const classics = "Incontournables";
const char* const urban = "Hip-hop · Trap · R&B";
const char* const funk = "Funk · Disco · Pop · Rock";
const char* const club = "House · Techno";
const char* const breaks = "Breaks · Drum & Bass · Électro";
const char* const world = "Latin · Afro · Reggae";
const char* const percussion = "Percussions";
const char* const bassLines = "Lignes de basse";
const char* const chordParts = "Accords";
const char* const leads = "Mélodies et arpèges";

Pattern drums (const char* category, const char* name, int bpm, std::vector<DrumLine> lines)
{
    return { PatternKind::Drums, category, name, bpm, std::move (lines) };
}

Pattern melodic (PatternKind kind, const char* category, const char* name, const char* line)
{
    return { kind, category, name, 0, {}, line };
}

std::vector<Pattern> build()
{
    using P = PatternKind;
    return {
        drums (classics, "Four on the Floor", 124, { { K, "X...x...x...x..." }, { C, "....x.......x..." }, { H, "xx.xxx.xxx.xxx.x" }, { O, "..x...x...x...x." } }),
        drums (classics, "Rock (backbeat)", 110, { { K, "x.......x.x....." }, { S, "....x.......x..." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (classics, "Motown Groove", 116, { { K, "x.......x.x....." }, { S, "x...X...x...X..." }, { Tb, "....x.......x..." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (classics, "Billie Jean", 117, { { K, "x.......x......." }, { S, "....x.......x..." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (classics, "Funky Drummer", 100, { { K, "x.x...x...x..x.." }, { S, "....X..g.g..X..g" }, { H, "xxxxxxx.xxxxx.xx" }, { O, ".......x.....x.." } }),
        drums (classics, "Amen Break", 136, { { K, "x.x.......xx......xx......x....." }, { S, "....x..x.x..x..x.x..x..x.x....x." },
                                              { Y, "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x." } }),
        drums (classics, "When the Levee Breaks", 72, { { K, "x.x....x..xx...." }, { S, "....x.......x..." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (classics, "Impeach the President", 96, { { K, "x......xx.x....." }, { S, "....x.......x..." }, { H, "x.x.x.x.x.x.x..." }, { O, "..............x." } }),
        drums (classics, "Half-time", 85, { { K, "x.........x....." }, { S, "........x......." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (classics, "Ballade pop", 75, { { K, "x.......x.x....." }, { R, "....x.......x..." }, { H, "x.x.x.x.x.x.x.x." } }),

        drums (urban, "Boom Bap", 90, { { K, "x......x..x....." }, { S, "....x.......x..." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (urban, "Boom Bap syncopé", 93, { { K, "x..x......x..x.." }, { S, "....x.......x..." }, { H, "x.x.x.x.x.x.x.xx" }, { Sh, "..x...x...x...x." } }),
        drums (urban, "Trap", 140, { { K, "x......x..x....." }, { S, "........x......." }, { C, "........x......." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (urban, "Trap (roulements)", 140, { { K, "x.....x...x..x..x......x..x....." }, { S, "........x...............x......." },
                                                 { C, "........x...............x......." }, { H, "x.x.x.x.x.x3x.x.x.x.x.x2x.x.x4x2" } }),
        drums (urban, "Drill UK", 142, { { K, "x......x..x....." }, { S, "........x....x.." }, { H, "x..x..x.x..x..x." }, { R, "...x.....x......" } }),
        drums (urban, "Lo-fi", 80, { { K, "x......x.x......" }, { S, "....x.......x..." }, { H, "x.x.x.x.x.x.x.x." }, { R, "..........x....." } }),
        drums (urban, "G-Funk", 92, { { K, "x.......x.x.x..." }, { S, "....x.......x..." }, { C, "....x.......x..." }, { H, "xxxxxxxxxxxxxxxx" } }),
        drums (urban, "R&B", 95, { { K, "x......x..x..x.." }, { S, "....x.......x..." }, { Sn, "....x.......x..." }, { H, "xxxxxxx3xxxxxxx2" }, { R, "......x.......x." } }),

        drums (funk, "Disco", 120, { { K, "x...x...x...x..." }, { S, "....x.......x..." }, { O, "..x...x...x...x." }, { H, "xx.xxx.xxx.xxx.x" } }),
        drums (funk, "Funk (ghost notes)", 100, { { K, "x..x..x...x..x.." }, { S, "..g.X..g.g..X.g." }, { H, "xxxxxxxxxxxxxxxx" } }),
        drums (funk, "Hi-NRG", 130, { { K, "x...x...x...x..." }, { S, "....x.......x..." }, { C, "....x.......x..." }, { O, "..x...x...x...x." }, { H, ".x.x.x.x.x.x.x.x" } }),
        drums (funk, "Pop", 110, { { K, "x.......x.x....." }, { C, "....x.......x..." }, { H, "x.x.x.x.x.x.x.x." }, { Tb, "....x.......x..." } }),
        drums (funk, "Synthwave", 100, { { K, "x...x...x...x..." }, { S, "....X.......X..." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (funk, "Rock rapide (punk)", 170, { { K, "x...x...x...x..." }, { S, "..x...x...x...x." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (funk, "Soul (sidestick)", 92, { { K, "x.....x.x......." }, { R, "....x.......x..." }, { H, "x.xxx.xxx.xxx.xx" } }),

        drums (club, "House", 124, { { K, "x...x...x...x..." }, { C, "....x.......x..." }, { O, "..x...x...x...x." }, { Sh, "xxxxxxxxxxxxxxxx" } }),
        drums (club, "Deep House", 120, { { K, "x...x...x...x..." }, { C, "....x.......x..." }, { R, "......x....x..x." }, { H, "..x...x...x...x." }, { Sh, "x.xxx.xxx.xxx.xx" } }),
        drums (club, "Techno", 130, { { K, "X...x...x...x..." }, { C, "....x.......x..." }, { H, "xx.xxx.xxx.xxx.x" }, { O, "..x...x...x...x." }, { R, "...x..x....x.x.." } }),
        drums (club, "Berlin", 134, { { K, "X...x...x...x..." }, { H, "x.x.x.x.x.x.x.x." }, { Y, "..x...x...x...x." }, { R, "..x..x.x...x..x." }, { C, "............x..." } }),
        drums (club, "Hard Techno", 150, { { K, "X...X...X...X..." }, { O, "..x...x...x...x." }, { H, "xx.xxx.xxx.xxx.x" }, { C, "....x.......x..." } }),
        drums (club, "Minimal", 125, { { K, "x...x...x...x..." }, { R, "..x..x...x..x..." }, { H, "..x...x...x...x." }, { Sh, "x.x.x.x.x.x.x.x." }, { Cl, "...x.......x...." } }),
        drums (club, "Acid", 128, { { K, "X...x...x...x..." }, { C, "....x.......x..." }, { H, "xx.xxx.xxx.xxx.x" }, { O, "..x...x...x...x." } }),
        drums (club, "UK Garage (2-step)", 132, { { K, "x.........x..x.." }, { S, "....x.......x..." }, { R, "..x..x.....x..x." }, { H, "x.x.x.x.x.x.x.x." } }),

        drums (breaks, "Drum & Bass (two-step)", 174, { { K, "x.........x....." }, { S, "....x.......x..." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (breaks, "Jungle", 165, { { K, "x.........x.x..." }, { S, "....x..g.g..x.g." }, { Y, "x.x.x.x.x.x.x.x." } }),
        drums (breaks, "Breakbeat", 130, { { K, "x.....x...x....." }, { S, "....x.......x..." }, { H, "x.x.x...x.x.x..." }, { O, "......x.......x." } }),
        drums (breaks, "Dubstep", 140, { { K, "x.........x...x." }, { S, "........x......." }, { H, "..x...x...x...x." } }),
        drums (breaks, "Électro (Planet Rock)", 128, { { K, "x.....x...x....." }, { C, "....x.......x..." }, { Cb, "..x.......x....." }, { H, "x.x.x.x.x.x.x.x." } }),

        drums (world, "Reggaeton (dembow)", 95, { { K, "x...x...x...x..." }, { S, "...x..x....x..x." }, { H, "x.x.x.x.x.x.x.x." }, { Th, "............x.xx" } }),
        drums (world, "Bossa nova", 130, { { K, "x..xx..xx..xx..x" }, { R, "x..x..x...x..x.." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (world, "Samba", 100, { { K, "x..xx..xx..xx..x" }, { Lt, "....x.......x..." }, { Sh, "Xxxxxxxxxxxxxxxx" }, { Ag, "x.x..x.x..x.x..." } }),
        drums (world, "Afro House", 122, { { K, "x...x...x...x..." }, { C, "....x.......x..." }, { Lc, "x..x..x...x..x.." }, { Hc, "...x..x.x.....x." }, { Sh, "x.xxx.xxx.xxx.xx" } }),
        drums (world, "Afrobeat", 110, { { K, "x.....x.x.....x." }, { S, "..g.x..g.g..x.g." }, { H, "x.x.x.x.x.x.x.x." }, { Cb, "x.x.xx.x.x.xx.x." } }),
        drums (world, "Reggae (one drop)", 75, { { K, "........x......." }, { R, "........x......." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (world, "Reggae (rockers)", 78, { { K, "x.......x......." }, { R, "....x.......x..." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (world, "Reggae (steppers)", 80, { { K, "x...x...x...x..." }, { R, "........x......." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (world, "Dancehall", 100, { { K, "x..x....x..x...." }, { S, "....x..x....x..." }, { H, "x.x.x.x.x.x.x.x." } }),
        drums (world, "Son clave 3-2 (salsa)", 95, { { Cl, "x..x..x...x.x..." }, { K, "......x.......x." }, { Lc, "......x.......x." }, { Hc, "....x.x.....x.x." }, { Cb, "x.x.x.x.x.x.x.x." } }),
        drums (world, "Tresillo", 100, { { K, "x..x..x.x..x..x." }, { Cl, "x..x..x........." }, { Sh, "x.x.x.x.x.x.x.x." } }),

        drums (percussion, "Tumbao (salsa)", 95, { { Hc, "..x...x...x...x." }, { Lc, "......xx......xx" }, { Cb, "x.x.x.x.x.x.x.x." }, { Cl, "x..x..x...x.x..." } }),
        drums (percussion, "Martillo (bongos)", 95, { { Hb, "x.x.x.x.x.x.x.x." }, { Lb, "......x.......x." }, { Gu, "x...x.x.x...x.x." } }),
        drums (percussion, "Batucada (samba)", 100, { { Lc, "....x.......x..." }, { Hc, "..x...x...x...x." }, { Ag, "x.x..x.x..x.x..." }, { Sh, "Xxxxxxxxxxxxxxxx" }, { Tb, ".x.x.x.x.x.x.x.x" } }),
        drums (percussion, "Afro House", 122, { { Lc, "x..x..x...x..x.." }, { Hc, "...x..x.x.....x." }, { Sh, "x.xxx.xxx.xxx.xx" }, { Ag, "x..x..x....x..x." } }),
        drums (percussion, "Reggaeton", 95, { { Th, "............x.xx" }, { Hb, "...x..x....x..x." }, { Sh, "x.x.x.x.x.x.x.x." }, { Lc, "x.......x......." } }),
        drums (percussion, "Shaker et tambourin", 120, { { Sh, "XxxxXxxxXxxxXxxx" }, { Tb, "....x.......x..." } }),
        drums (percussion, "Bossa nova", 130, { { Sh, "xxxxxxxxxxxxxxxx" }, { Cl, "x..x..x...x..x.." }, { Tr, "..x...x...x...x." } }),
        drums (percussion, "Cascara (timbales)", 100, { { Th, "x.xx.x.xx.x.x.xx" }, { Tl, "......x.......x." }, { Cb, "x...x...x...x..." } }),
        drums (percussion, "Congas house", 124, { { Hc, "..x...x...x..xx." }, { Lc, "x.....x.x......." }, { Sh, ".x.x.x.x.x.x.x.x" } }),

        melodic (P::Bass, bassLines, "Racine en croches", "0.0.0.0.0.0.0.0."),
        melodic (P::Bass, bassLines, "Contretemps (techno, trance)", "..0...0...0...0."),
        melodic (P::Bass, bassLines, "Basse qui roule (psytrance)", ".000.000.000.000"),
        melodic (P::Bass, bassLines, "Octaves disco", "0.7.0.7.0.7.0.7."),
        melodic (P::Bass, bassLines, "Funk slap", "0..07.0..0..7.4."),
        melodic (P::Bass, bassLines, "Acid 303", "0.07.0-30.07.0-5"),
        melodic (P::Bass, bassLines, "Walking (jazz)", "0...2...4...5..."),
        melodic (P::Bass, bassLines, "808 trap (tenue et glissé)", "0-----0---7-----"),
        melodic (P::Bass, bassLines, "Boom bap", "0-....0-..4-...."),
        melodic (P::Bass, bassLines, "Reggae (one drop)", "..0.0...4..2.0.."),
        melodic (P::Bass, bassLines, "Tresillo (reggaeton, afro)", "0..0..0.0..0..0."),
        melodic (P::Bass, bassLines, "Dub (longues notes)", "0-------4---2---"),
        melodic (P::Bass, bassLines, "Synthwave (pulsation)", "0000000000000077"),
        melodic (P::Bass, bassLines, "House (groove)", "0..0..7...0..4.."),
        melodic (P::Bass, bassLines, "UK Garage", "0..-..7.0...4.2."),

        melodic (P::Chords, chordParts, "Nappe tenue", "0---------------"),
        melodic (P::Chords, chordParts, "Deux accords par mesure", "0-------0-------"),
        melodic (P::Chords, chordParts, "Stabs house (piano)", "0..0..0...0..0.."),
        melodic (P::Chords, chordParts, "Contretemps", "..0...0...0...0."),
        melodic (P::Chords, chordParts, "Skank reggae", "....0.......0..."),
        melodic (P::Chords, chordParts, "Stab dub techno", "...0.......0...."),
        melodic (P::Chords, chordParts, "Charleston", "0..0..0........."),
        melodic (P::Chords, chordParts, "Croches pop", "0.0.0.0.0.0.0.0."),
        melodic (P::Chords, chordParts, "Comping jazz", "0--.....0-.0--.."),
        melodic (P::Chords, chordParts, "Synthwave", "0-------0---0---"),
        melodic (P::Chords, chordParts, "Stab Detroit", "0-.0-.0-..0-.0-."),

        melodic (P::Lead, leads, "Arpège montant", "0247024702470247"),
        melodic (P::Lead, leads, "Arpège descendant", "7420742074207420"),
        melodic (P::Lead, leads, "Arpège aller-retour", "0247420247420247"),
        melodic (P::Lead, leads, "Arpège en croches", "0.2.4.7.0.2.4.7."),
        melodic (P::Lead, leads, "Trance (gate)", "0.07.0.70.07.0.7"),
        melodic (P::Lead, leads, "Bleeps techno", "...7......4....."),
        melodic (P::Lead, leads, "Mélodie pop", "4-4-5-4-2---0---"),
        melodic (P::Lead, leads, "Hook trap", "7..6..4...4.2..."),
        melodic (P::Lead, leads, "Riff électro", "0.0.3.0.5.4.3.2."),
        melodic (P::Lead, leads, "Notes tenues", "4-------2-------"),
    };
}

int degreeOf (char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'c') return -(c - 'a' + 1);
    return 0;
}

int clampNote (int semitone)
{
    while (semitone >= noteRange) semitone -= 12;
    while (semitone < 0) semitone += 12;
    return semitone;
}
} // namespace

const std::vector<Pattern>& patterns()
{
    static const std::vector<Pattern> list = build();
    return list;
}

std::vector<const char*> patternCategories (PatternKind kind)
{
    std::vector<const char*> list;
    for (auto& p : patterns())
        if (p.kind == kind && std::find (list.begin(), list.end(), p.category) == list.end())
            list.push_back (p.category);
    return list;
}

std::vector<PatternKind> patternKindsFor (const MachineDef& def)
{
    switch (def.kind)
    {
        case MachineKind::Drum: return def.type == SoundFx ? std::vector<PatternKind> {} : std::vector<PatternKind> { PatternKind::Drums };
        case MachineKind::Mono: return { PatternKind::Bass, PatternKind::Lead };
        case MachineKind::Poly: return { PatternKind::Chords, PatternKind::Lead };
    }
    return {};
}

float patternCoverage (const MachineDef& def, const Pattern& pattern)
{
    if (pattern.kind != PatternKind::Drums || pattern.drums.empty()) return 1.0f;
    int found = 0;
    for (auto& line : pattern.drums) found += rowForRole (def, line.role) >= 0 ? 1 : 0;
    return (float) found / (float) pattern.drums.size();
}

void applyPattern (Track& t, const MachineDef& def, const GenerateContext& ctx, const Pattern& pattern)
{
    t.clear();

    if (pattern.kind == PatternKind::Drums)
    {
        const int ghost = rowForRole (def, Role::Ghost);
        for (auto& line : pattern.drums)
        {
            const int row = rowForRole (def, line.role);
            const int length = (int) std::strlen (line.steps);
            for (int s = 0; s < ctx.steps; ++s)
            {
                const char c = line.steps[s % length];
                if (c == '.') continue;
                const int target = c == 'g' ? ghost : row;
                if (target < 0) continue;
                t.hits[(size_t) target][(size_t) s] = true;
                if (c == 'X') t.accent[(size_t) s] = true;
                if (c >= '2' && c <= '4') t.rolls[(size_t) target][(size_t) s] = (std::uint8_t) (c - '0');
            }
        }

        const int ch = rowForRole (def, Role::ClosedHat), oh = rowForRole (def, Role::OpenHat);
        if (ch >= 0 && oh >= 0 && ch != oh)
            for (int s = 0; s < ctx.steps; ++s)
                if (t.hits[(size_t) oh][(size_t) s]) t.hits[(size_t) ch][(size_t) s] = false;
        t.tile (ctx.steps);
        return;
    }

    const int length = (int) std::strlen (pattern.line);
    int previous = -1;
    for (int s = 0; s < ctx.steps; ++s)
    {
        const char c = pattern.line[s % length];
        const auto i = (size_t) s;
        if (c == '.') { previous = -1; continue; }
        if (c == '-')
        {
            if (previous < 0) continue;
            t.on[i] = true;
            t.note[i] = previous;
            t.slide[(size_t) (s - 1)] = true;
            continue;
        }
        const int note = clampNote (degreeToSemitone (ctx.scale, chordRootAt (ctx, s) + degreeOf (c)));
        t.on[i] = true;
        t.note[i] = note;
        t.accent[i] = s % pageSteps == 0;
        previous = note;
    }
    t.tile (ctx.steps);
}
} // namespace techno
