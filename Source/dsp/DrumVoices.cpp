#include "DrumVoices.h"

namespace techno
{
std::unique_ptr<DrumVoice> createDrumVoice (const juce::String& machine, const juce::String& row)
{
    using namespace metalFreqs;

    auto metal = [] (const float* freqs, float noiseMix, float bits, float bandHz, float q, float hpHz,
                     float minDecay, float maxDecay, float gain, float fastPart)
    {
        return std::make_unique<Metal> (MetalConfig { freqs, noiseMix, bits, bandHz, q, hpHz, minDecay, maxDecay, gain, fastPart });
    };

    if (machine == "tr909")
    {
        if (row == "bd") return std::make_unique<Kick909>();
        if (row == "sd") return std::make_unique<Snare909>();
        if (row == "lt") return std::make_unique<Tom> (100.0f, 0.6f, 0.15f);
        if (row == "mt") return std::make_unique<Tom> (140.0f, 0.6f, 0.15f);
        if (row == "ht") return std::make_unique<Tom> (195.0f, 0.6f, 0.15f);
        if (row == "rs") return std::make_unique<RimShot> (1720.0f, 497.0f);
        if (row == "cp") return std::make_unique<HandClap> (4, 0.0105f, 1150.0f);
        if (row == "ch") return metal (tr909, 0.35f, 32.0f, 9000.0f, 0.9f, 7000.0f, 0.012f, 0.09f, 4.8f, 0.0f);
        if (row == "oh") return metal (tr909, 0.35f, 32.0f, 9000.0f, 0.9f, 7000.0f, 0.08f, 0.7f, 3.9f, 0.0f);
        if (row == "cr") return metal (crash, 0.55f, 32.0f, 7000.0f, 0.6f, 3500.0f, 0.5f, 3.0f, 2.6f, 0.35f);
        if (row == "rd") return metal (ride, 0.15f, 32.0f, 6500.0f, 1.2f, 2500.0f, 0.4f, 2.5f, 3.0f, 0.25f);
    }
    else if (machine == "tr808")
    {
        if (row == "bd") return std::make_unique<Kick808>();
        if (row == "sd") return std::make_unique<Snare808>();
        if (row == "lt") return std::make_unique<Tom> (88.0f, 0.18f, 0.05f);
        if (row == "mt") return std::make_unique<Tom> (128.0f, 0.18f, 0.05f);
        if (row == "ht") return std::make_unique<Tom> (178.0f, 0.18f, 0.05f);
        if (row == "rs") return std::make_unique<RimShot> (1667.0f, 455.0f);
        if (row == "cp") return std::make_unique<HandClap> (4, 0.009f, 1000.0f);
        if (row == "cb") return std::make_unique<Cowbell808>();
        if (row == "cy") return std::make_unique<Cymbal808> (tr808);
        if (row == "ch") return metal (tr808, 0.05f, 0.0f, 7500.0f, 1.0f, 6000.0f, 0.01f, 0.06f, 7.0f, 0.0f);
        if (row == "oh") return metal (tr808, 0.05f, 0.0f, 7500.0f, 1.0f, 6000.0f, 0.08f, 0.6f, 5.2f, 0.0f);
        if (row == "cl") return std::make_unique<Claves>();
        if (row == "ma") return std::make_unique<Maracas>();
    }
    else if (machine == "breaks")
    {
        if (row == "bd") return std::make_unique<AcousticKick>();
        if (row == "sd" || row == "gs") return std::make_unique<AcousticSnare>();
        if (row == "ch") return metal (acoustic, 0.7f, 0.0f, 9000.0f, 0.6f, 6000.0f, 0.02f, 0.08f, 3.8f, 0.0f);
        if (row == "oh") return metal (acoustic, 0.7f, 0.0f, 9000.0f, 0.6f, 6000.0f, 0.12f, 0.7f, 3.2f, 0.0f);
        if (row == "rd") return metal (ride, 0.15f, 0.0f, 6500.0f, 1.2f, 2500.0f, 0.4f, 2.5f, 2.5f, 0.25f);
        if (row == "cr") return metal (crash, 0.55f, 0.0f, 7000.0f, 0.6f, 3500.0f, 0.5f, 3.0f, 2.2f, 0.35f);
    }

    jassertfalse;
    return std::make_unique<Claves>();
}
} // namespace techno
