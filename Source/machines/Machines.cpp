#include "Factories.h"
#include "SynthMachines.h"
#include "../dsp/VintageVoices.h"

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
        case EBass:  return std::make_unique<MonoMachine> (type, std::make_unique<ElectricBassVoice>(), 0.85f, pBass::Type, std::vector<float> { 1, 1, 1, 1, 1.4f });
        case Wobble: return std::make_unique<MonoMachine> (type, std::make_unique<WobbleVoice>(), 0.9f);
        case Sub808: return std::make_unique<MonoMachine> (type, std::make_unique<Sub808Voice>(), 14.0f);
        case SemiMod: return std::make_unique<MonoMachine> (type, std::make_unique<SemiModVoice>());
        case Ether:  return std::make_unique<MonoMachine> (type, std::make_unique<EtherVoice>(), 0.9f, pEther::Type, std::vector<float> { 1, 1.1f, 0.64f, 0.68f, 0.23f, 1.33f });
        default:     return createInstrumentMachine (type);
    }
}
} // namespace techno
