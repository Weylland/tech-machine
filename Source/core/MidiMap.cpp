#include "MidiMap.h"
#include "Styles.h"

namespace techno
{
namespace
{
struct Gm { int note; Role role; };

constexpr Gm gmMap[] {
    { 36, Role::Kick }, { 35, Role::Kick }, { 37, Role::Rim }, { 38, Role::Snare }, { 39, Role::Clap }, { 40, Role::Ghost },
    { 41, Role::LowTom }, { 42, Role::ClosedHat }, { 43, Role::LowTom }, { 44, Role::ClosedHat }, { 45, Role::MidTom },
    { 46, Role::OpenHat }, { 47, Role::MidTom }, { 48, Role::HighTom }, { 49, Role::Crash }, { 50, Role::HighTom },
    { 51, Role::Ride }, { 52, Role::Cymbal }, { 54, Role::Tambourine }, { 55, Role::Cymbal }, { 56, Role::Cowbell },
    { 57, Role::Crash }, { 59, Role::Ride }, { 60, Role::HighBongo }, { 61, Role::LowBongo }, { 62, Role::HighConga },
    { 63, Role::HighConga }, { 64, Role::LowConga }, { 65, Role::HighTimbale }, { 66, Role::LowTimbale }, { 67, Role::Agogo },
    { 68, Role::Agogo }, { 70, Role::Shaker }, { 73, Role::Guiro }, { 74, Role::Guiro }, { 75, Role::Claves },
    { 76, Role::Perc }, { 77, Role::Perc }, { 80, Role::Triangle }, { 81, Role::Triangle }, { 82, Role::Shaker },
    { 39, Role::Snap },
};
} // namespace

// The General MIDI note first, then the rows in order from note 36 (so every note plays something).
int rowForGmNote (const MachineDef& def, int note)
{
    const int rows = (int) def.rows.size();
    for (auto& g : gmMap)
        if (g.note == note)
            if (const int row = rowForRole (def, g.role); row >= 0) return row;
    return rows > 0 ? ((note - 36) % rows + rows) % rows : -1;
}

int gmNoteForRow (const MachineDef& def, int row)
{
    const Role role = def.rows[(size_t) row].role;
    for (auto& g : gmMap)
        if (g.role == role) return g.note;
    return 36 + row;
}
} // namespace techno
