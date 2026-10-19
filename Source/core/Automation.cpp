#include "Automation.h"
#include "Music.h"

namespace techno::automation
{
namespace
{
struct Decoded { enum Kind { Strip, Param, Row, Step, None } kind; int index, row; };

Decoded decode (const MachineDef& def, int target)
{
    if (target < 0) return { Decoded::None, 0, 0 };
    if (target < stripCount) return { Decoded::Strip, target, 0 };
    target -= stripCount;
    if (target < (int) def.params.size()) return { Decoded::Param, target, 0 };
    target -= (int) def.params.size();
    if (target < (int) def.rows.size() * rowCount) return { Decoded::Row, target % rowCount, target / rowCount };
    target -= (int) def.rows.size() * rowCount;
    if (target < stepSettingCount) return { Decoded::Step, target, 0 };
    return { Decoded::None, 0, 0 };
}
} // namespace

int targetCount (int machineType)
{
    const auto& def = machineDef (machineType);
    return stripCount + (int) def.params.size() + (int) def.rows.size() * rowCount + stepSettingCount;
}

juce::String paramId (int machineType, int target)
{
    const auto d = decode (machineDef (machineType), target);
    switch (d.kind)
    {
        case Decoded::Strip: return pid::machine (machineType, stripSuffixes[d.index]);
        case Decoded::Param: return pid::synth (machineType, d.index);
        case Decoded::Row:   return pid::row (machineType, d.row, rowSuffixes[d.index]);
        case Decoded::Step:  return stepSuffixes[d.index];
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
        case Decoded::Step:
        {
            static const char* const stepLabels[] { "VÉLOCITÉ (force des notes)", "CHANCE (probabilité de jouer)",
                                                    "CONDITION (à quels tours)", "RETARD (groove)" };
            return utf8 (stepLabels[d.index]);
        }
        case Decoded::None:  break;
    }
    return {};
}

int choiceCount (int machineType, int target)
{
    const auto& def = machineDef (machineType);
    const auto d = decode (def, target);
    if (d.kind == Decoded::Step) return d.index == Condition ? conditionCount : 0;
    return d.kind == Decoded::Param ? (int) def.params[(size_t) d.index].choices.size() : 0;
}

juce::String choiceName (int machineType, int target, int index)
{
    const auto& def = machineDef (machineType);
    const auto d = decode (def, target);
    if (d.kind == Decoded::Step && d.index == Condition) return utf8 (conditionNames[juce::jlimit (0, conditionCount - 1, index)]);
    if (d.kind != Decoded::Param) return {};
    const auto& choices = def.params[(size_t) d.index].choices;
    return choices.empty() ? juce::String() : utf8 (choices[(size_t) juce::jlimit (0, (int) choices.size() - 1, index)]);
}

bool conditionPlays (int condition, long long round, bool fill)
{
    switch (condition)
    {
        case 1: return fill;
        case 2: return ! fill;
        case 3: return round == 0;
        case 4: return round > 0;
        case 5: case 6: return round % 2 == condition - 5;
        case 7: case 8: case 9: return round % 3 == condition - 7;
        case 10: case 11: case 12: case 13: return round % 4 == condition - 10;
        default: return true;
    }
}
} // namespace techno::automation
