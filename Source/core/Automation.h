#pragma once

#include "MachineDefs.h"

// The settings of a machine that can change from step to step (automation), numbered in a fixed order:
//   the effects of its strip, then its own settings (synth parameters, sampler character), then for a drum
//   machine the settings of each sound. A Track lane stores one of these numbers ("target").
namespace techno::automation
{
inline constexpr const char* stripSuffixes[] { "drive", "crush", "filter", "echo", "reverb", "volume", "pan" };
inline constexpr int stripCount = 7;
inline constexpr const char* rowSuffixes[] { "level", "tune", "decay", "tone", "drive", "filter", "pan", "echo", "reverb" };
inline constexpr int rowCount = 9;

enum Strip { Drive, Crush, Filter, Echo, Reverb, Volume, Pan };
enum RowSetting { Level, Tune, Decay, Tone, RowDrive, RowFilter, RowPan, RowEcho, RowReverb };

inline constexpr float normalVelocity = 0.75f;   // the velocity of a step without a value

// Settings of the step itself, after all the knobs: they are not knobs, a step without a value plays normally.
enum StepSetting { Velocity, Chance, Condition, Delay, stepSettingCount };
inline constexpr const char* stepSuffixes[stepSettingCount] { "velocity", "chance", "condition", "delay" };
inline constexpr float stepNormals[stepSettingCount] { normalVelocity, 1.0f, 0.0f, 0.0f };

// Conditions: on which rounds of the loop a step plays.
inline constexpr int conditionCount = 14;
inline constexpr const char* conditionNames[conditionCount] {
    "Toujours", "Pendant un FILL", "Hors FILL", "Premier tour seulement", "Sauf le premier tour",
    "Tour 1 sur 2", "Tour 2 sur 2", "Tour 1 sur 3", "Tour 2 sur 3", "Tour 3 sur 3",
    "Tour 1 sur 4", "Tour 2 sur 4", "Tour 3 sur 4", "Tour 4 sur 4" };
inline constexpr int fillCondition = 1;
bool conditionPlays (int condition, long long round, bool fill);   // round 0 = the first time

inline int synthTarget (int param) { return stripCount + param; }
inline int stepTarget (const MachineDef& def, int setting) { return stripCount + (int) def.params.size() + (int) def.rows.size() * rowCount + setting; }
inline int velocityTarget (const MachineDef& def) { return stepTarget (def, Velocity); }
inline int stepSettingOf (int machineType, int target)   // -1 for a knob
{
    const int s = target - stepTarget (machineDef (machineType), 0);
    return s >= 0 && s < stepSettingCount ? s : -1;
}
inline bool isStepSetting (int machineType, int target) { return stepSettingOf (machineType, target) >= 0; }
inline int rowTarget (const MachineDef& def, int row, int setting) { return stripCount + (int) def.params.size() + row * rowCount + setting; }
int targetCount (int machineType);

juce::String paramId (int machineType, int target);   // "velocity", "chance"... for the settings of the step, which are not knobs
int targetFor (int machineType, const juce::String& paramId);   // -1 when the parameter cannot be automated
juce::String label (int machineType, int target);               // shown in the interface (already translated)
int choiceCount (int machineType, int target);                  // 0 for a knob (values 0..1), else the number of choices
juce::String choiceName (int machineType, int target, int index);   // already translated
} // namespace techno::automation
