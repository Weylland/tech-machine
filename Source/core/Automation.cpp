#include "Automation.h"
#include "Music.h"

namespace techno::automation
{
namespace
{
struct Decoded { enum Kind { Strip, Param, Row, Velocity, None } kind; int index, row; };

Decoded decode (const MachineDef& def, int target)
{
    if (target < 0) return { Decoded::None, 0, 0 };
    if (target < stripCount) return { Decoded::Strip, target, 0 };
    target -= stripCount;
    if (target < (int) def.params.size()) return { Decoded::Param, target, 0 };
    target -= (int) def.params.size();
    if (target < (int) def.rows.size() * rowCount) return { Decoded::Row, target % rowCount, target / rowCount };
    if (target == (int) def.rows.size() * rowCount) return { Decoded::Velocity, 0, 0 };
    return { Decoded::None, 0, 0 };
}
} // namespace

int targetCount (int machineType)
{
    const auto& def = machineDef (machineType);
    return stripCount + (int) def.params.size() + (int) def.rows.size() * rowCount + 1;
}

juce::String paramId (int machineType, int target)
{
    const auto d = decode (machineDef (machineType), target);
    switch (d.kind)
    {
        case Decoded::Strip: return pid::machine (machineType, stripSuffixes[d.index]);
        case Decoded::Param: return pid::synth (machineType, d.index);
        case Decoded::Row:   return pid::row (machineType, d.row, rowSuffixes[d.index]);
        case Decoded::Velocity: return "velocity";
        case Decoded::None:  break;
    }
    return {};
}

int targetFor (int machineType, const juce::String& id)
{
    for (int t = 0; t < targetCount (machineType); ++t)
        if (paramId (machineType, t) == id) return t;
    return -1;
}

juce::String label (int machineType, int target)
{
    static const char* const stripLabels[] { "DISTO", "CRUSH", "FILTRE", "ÉCHO", "REVERB", "VOLUME", "PAN" };
    static const char* const rowLabels[] { "NIVEAU", "HAUTEUR", "DURÉE", "TIMBRE", "DISTO", "FILTRE", "PAN", "ÉCHO", "REVERB" };
    const auto& def = machineDef (machineType);
    const auto d = decode (def, target);
    switch (d.kind)
    {
        case Decoded::Strip: return utf8 (stripLabels[d.index]);
        case Decoded::Param: return utf8 (def.params[(size_t) d.index].label);
        case Decoded::Row:   return utf8 (def.rows[(size_t) d.row].label) + juce::String (juce::CharPointer_UTF8 (" \xc2\xb7 ")) + utf8 (rowLabels[d.index]);
        case Decoded::Velocity: return utf8 ("VÉLOCITÉ (force des notes)");
        case Decoded::None:  break;
    }
    return {};
}

int choiceCount (int machineType, int target)
{
    const auto& def = machineDef (machineType);
    const auto d = decode (def, target);
    return d.kind == Decoded::Param ? (int) def.params[(size_t) d.index].choices.size() : 0;
}
} // namespace techno::automation
