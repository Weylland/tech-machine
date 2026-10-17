#include "Music.h"

namespace techno
{
const std::vector<ScaleDef>& scales()
{
    static const std::vector<ScaleDef> list {
        { "Chromatique (libre)", { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },
          "Aucune gamme : les 12 notes sont permises. Liberté totale, mais attention, tu peux jouer des fausses notes. "
          "À choisir si tu sais ce que tu fais ou si tu veux expérimenter." },
        { "Mineur", { 0, 2, 3, 5, 7, 8, 10 },
          "LA gamme de la techno : sombre, mélancolique, sérieuse. Si tu ne sais pas quoi choisir, prends celle-ci." },
        { "Majeur", { 0, 2, 4, 5, 7, 9, 11 },
          "Joyeuse, lumineuse, positive. Parfaite pour la house, la trance euphorique ou un morceau qui donne le sourire." },
        { "Mineur harmonique", { 0, 2, 3, 5, 7, 8, 11 },
          "Mineur avec une touche orientale et dramatique (la 7e note est relevée). Très utilisée en trance et en psytrance." },
        { "Mineur mélodique", { 0, 2, 3, 5, 7, 9, 11 },
          "Mineur plus doux et jazzy. Sonne élégant et un peu mystérieux." },
        { "Dorien", { 0, 2, 3, 5, 7, 9, 10 },
          "Mineur mais moins triste, avec un côté funky et groovy. Idéal pour la deep house, la Detroit et l'acid." },
        { "Phrygien", { 0, 1, 3, 5, 7, 8, 10 },
          "Très sombre et tendu : la 2e note est juste un demi-ton au-dessus de la fondamentale. "
          "Parfait pour la techno dure, l'industriel et l'acid agressif." },
        { "Phrygien dominant", { 0, 1, 4, 5, 7, 8, 10 },
          "Le son « oriental » de la psytrance et de la goa : exotique, hypnotique, mystique." },
        { "Lydien", { 0, 2, 4, 6, 7, 9, 11 },
          "Majeur rêveur et flottant. Sonne aérien et cinématographique. Super pour les nappes et l'ambient." },
        { "Mixolydien", { 0, 2, 4, 5, 7, 9, 10 },
          "Majeur avec un côté blues/rock. Groovy et chaleureux, bien pour la house et le breakbeat." },
        { "Locrien", { 0, 1, 3, 5, 6, 8, 10 },
          "La plus instable et inquiétante (la quinte est diminuée). Pour des ambiances angoissantes et industrielles." },
        { "Double harmonique (arabe)", { 0, 1, 4, 5, 7, 8, 11 },
          "Très orientale, dramatique, exotique. Idéale pour des riffs psytrance ou des mélodies qui marquent." },
        { "Hongrois mineur", { 0, 2, 3, 6, 7, 8, 11 },
          "Mineur sombre avec une couleur tzigane/orientale. Tendu et dramatique." },
        { "Pentatonique mineure", { 0, 3, 5, 7, 10 },
          "Seulement 5 notes qui sonnent toujours bien ensemble : impossible de faire une fausse note. "
          "Parfaite pour débuter, très utilisée en drum & bass et en house." },
        { "Pentatonique majeure", { 0, 2, 4, 7, 9 },
          "5 notes joyeuses et simples, toujours harmonieuses. Sonne positif et ouvert." },
        { "Blues", { 0, 3, 5, 6, 7, 10 },
          "Pentatonique mineure + la « blue note ». Groovy et un peu sale. Bien pour le breakbeat, l'électro et la house." },
        { "Par tons", { 0, 2, 4, 6, 8, 10 },
          "6 notes espacées d'un ton : son flottant et irréel, comme dans un rêve. Effets étranges et psychédéliques." },
        { "Diminuée", { 0, 2, 3, 5, 6, 8, 9, 11 },
          "Alterne ton et demi-ton : son tendu, cinématographique, inquiétant. Pour l'industriel et l'expérimental." },
        { "Japonaise (In-Sen)", { 0, 1, 5, 7, 10 },
          "Gamme japonaise à 5 notes, sombre et mystérieuse. Superbe pour la minimale et la dub techno." },
        { "Hirajoshi", { 0, 2, 3, 7, 8 },
          "Autre gamme japonaise, mélancolique et zen. Très atmosphérique." },
        { "Égyptienne (suspendue)", { 0, 2, 5, 7, 10 },
          "5 notes sans tierce : ni triste ni joyeuse, ouverte et neutre. Très efficace pour la techno hypnotique." },
    };
    return list;
}

namespace
{
int mod12 (int s) { return ((s % 12) + 12) % 12; }

const std::vector<int>& intervalsOf (int scale)
{
    return scales()[(size_t) juce::jlimit (0, (int) scales().size() - 1, scale)].intervals;
}
} // namespace

bool inScale (int scale, int semitone)
{
    const auto& iv = intervalsOf (scale);
    return std::find (iv.begin(), iv.end(), mod12 (semitone)) != iv.end();
}

int snapToScale (int scale, int semitone)
{
    for (int d = 0; d < 12; ++d)
    {
        if (inScale (scale, semitone - d)) return semitone - d;
        if (inScale (scale, semitone + d)) return semitone + d;
    }
    return semitone;
}

int moveInScale (int scale, int semitone, int delta)
{
    int s = snapToScale (scale, semitone);
    const int dir = delta > 0 ? 1 : -1;
    for (int n = 0; n < std::abs (delta); ++n)
    {
        do s += dir;
        while (! inScale (scale, s));
    }
    return s;
}

int degreeToSemitone (int scale, int degree)
{
    static const std::vector<int> minor { 0, 2, 3, 5, 7, 8, 10 };
    const auto& iv = scale == chromaticScale ? minor : intervalsOf (scale);
    const int n = (int) iv.size();
    const int octave = degree >= 0 ? degree / n : -((-degree + n - 1) / n);
    return iv[(size_t) (degree - octave * n)] + 12 * octave;
}

juce::String noteName (int midiNote, bool withOctave)
{
    return juce::MidiMessage::getMidiNoteName (midiNote, true, withOctave, 4);
}

int chordIntervals (int scale, int root, int type, int* out)
{
    auto pick = [scale, root] (std::initializer_list<int> options, int fallback)
    {
        for (int o : options)
            if (inScale (scale, root + o)) return o;
        return fallback;
    };

    auto fill = [out] (std::initializer_list<int> values)
    {
        int n = 0;
        for (int v : values) out[n++] = v;
        return n;
    };

    switch (type)
    {
        case ChordSingle: return fill ({ 0 });
        case ChordAuto:   return fill ({ 0, pick ({ 3, 4 }, 3), pick ({ 7, 6, 8 }, 7), pick ({ 10, 11, 9 }, 10) });
        case ChordMinor:  return fill ({ 0, 3, 7 });
        case ChordMajor:  return fill ({ 0, 4, 7 });
        case ChordMin7:   return fill ({ 0, 3, 7, 10 });
        case ChordMaj7:   return fill ({ 0, 4, 7, 11 });
        case ChordDom7:   return fill ({ 0, 4, 7, 10 });
        case ChordSus2:   return fill ({ 0, 2, 7 });
        case ChordSus4:   return fill ({ 0, 5, 7 });
        case ChordMin9:   return fill ({ 0, 3, 7, 10, 14 });
        case ChordPower:  return fill ({ 0, 7, 12 });
        case ChordOctave: return fill ({ 0, 12 });
        default:          return fill ({ 0 });
    }
}

juce::String chordLabel (int scale, int rootMidi, int rootSemitone, int type)
{
    if (type == ChordSingle) return noteName (rootMidi);

    int iv[5];
    const int n = chordIntervals (scale, rootSemitone, type, iv);
    auto has = [&] (int x) { return std::find (iv, iv + n, x) != iv + n; };

    juce::String name = noteName (rootMidi, false);
    if (type == ChordOctave) return name + " oct";
    if (type == ChordPower) return name + "5";
    if (type == ChordSus2) return name + "sus2";
    if (type == ChordSus4) return name + "sus4";
    if (type == ChordMin9) return name + "m9";

    const bool minor = has (3), dim = has (6) && minor, aug = has (8) && ! has (7);
    if (has (10)) return name + (dim ? "m7b5" : minor ? "m7" : "7");
    if (has (11)) return name + (minor ? "mM7" : "maj7");
    if (dim) return name + "dim";
    if (aug) return name + "aug";
    return minor ? name + "m" : name;
}
} // namespace techno
