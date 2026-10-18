#pragma once

#include "MachineDefs.h"
#include <vector>

namespace techno
{
// A ready-made sound for one machine, picked from its SONS menu.
// Keys: a synth parameter ("cutoff"), a drum row parameter ("bd.decay") or an effect of the machine ("@echo").
// Every parameter not listed goes back to its default value.
struct SoundPreset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values;
};

const std::vector<SoundPreset>& presetsFor (int machineType);
} // namespace techno
