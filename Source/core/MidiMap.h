#pragma once

#include "MachineDefs.h"

// General MIDI drum notes: what pads and keyboards send, and what the MIDI export writes for each drum sound.
namespace techno
{
int rowForGmNote (const MachineDef&, int note);   // -1 when the machine has no rows
int gmNoteForRow (const MachineDef&, int row);
} // namespace techno
