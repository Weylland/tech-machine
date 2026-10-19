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

inline int synthTarget (int param) { return stripCount + param; }
inline int rowTarget (const MachineDef& def, int row, int setting) { return stripCount + (int) def.params.size() + row * rowCount + setting; }
int targetCount (int machineType);

juce::String paramId (int machineType, int target);
int targetFor (int machineType, const juce::String& paramId);   // -1 when the parameter cannot be automated
juce::String label (int machineType, int target);               // shown in the interface (already translated)
int choiceCount (int machineType, int target);                  // 0 for a knob (values 0..1), else the number of choices
} // namespace techno::automation
