#include "Factories.h"
#include "SynthMachines.h"

namespace techno
{
std::unique_ptr<Machine> createMachine (int type)
{
    if (machineDef (type).kind == MachineKind::Drum) return createDrumMachine (type);

    switch (type)
    {
        case TB303:  return std::make_unique<MonoMachine> (type, std::make_unique<Acid303>());
        case SH101:  return std::make_unique<MonoMachine> (type, std::make_unique<Mono101>());
        case ModelD: return std::make_unique<MonoMachine> (type, std::make_unique<ModelDVoice>());
        case EBass:  return std::make_unique<MonoMachine> (type, std::make_unique<ElectricBassVoice>(), 0.85f);
        case Wobble: return std::make_unique<MonoMachine> (type, std::make_unique<WobbleVoice>(), 0.9f);
        case Sub808: return std::make_unique<MonoMachine> (type, std::make_unique<Sub808Voice>(), 14.0f);
        default:     return createInstrumentMachine (type);
    }
}
} // namespace techno
