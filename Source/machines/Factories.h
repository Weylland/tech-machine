#pragma once

#include "Machines.h"

// The machine families, each built in its own file. createMachine() picks the right one.
namespace techno
{
std::unique_ptr<Machine> createDrumMachine (int type);
std::unique_ptr<Machine> createInstrumentMachine (int type);
} // namespace techno
