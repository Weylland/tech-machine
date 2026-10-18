#pragma once

#include "MachineDefs.h"
#include <vector>

namespace techno
{
// A ready-made sound for one machine, picked from its SONS menu.
// Keys: a synth parameter ("cutoff"), a drum row parameter ("bd.decay") or an effect of the machine ("@echo").
// Every parameter not listed goes back to its default value.
struct PresetValue
{
    const char* key;
    float value;   // the parameter's own value: 0..1 for a knob, the index of a choice
};

struct SoundPreset
{
    const char* name;
    std::vector<PresetValue> values;
};

const std::vector<SoundPreset>& presetsFor (int machineType);
} // namespace techno
