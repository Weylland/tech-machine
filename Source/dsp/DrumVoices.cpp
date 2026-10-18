#include "DrumVoices.h"
#include "FxVoices.h"
#include "PercussionVoices.h"

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

    else if (machine == "linn")
    {
        if (row == "bd") return std::make_unique<AcousticKick>();
        if (row == "sd") return std::make_unique<AcousticSnare>();
        if (row == "ss") return std::make_unique<RimShot> (820.0f, 1950.0f);
        if (row == "cp") return std::make_unique<HandClap> (3, 0.008f, 1300.0f);
        if (row == "ch") return metal (acoustic, 0.65f, 128.0f, 9500.0f, 0.6f, 6500.0f, 0.02f, 0.07f, 3.8f, 0.0f);
        if (row == "oh") return metal (acoustic, 0.65f, 128.0f, 9500.0f, 0.6f, 6500.0f, 0.12f, 0.6f, 3.2f, 0.0f);
        if (row == "lt") return std::make_unique<Membrane> (105.0f, 0.35f, 1.5f, 0.25f, 0.12f, 0.6f);
        if (row == "ht") return std::make_unique<Membrane> (155.0f, 0.35f, 1.5f, 0.25f, 0.1f, 0.5f);
        if (row == "cb") return std::make_unique<Cowbell808>();
        if (row == "tb") return std::make_unique<Tambourine>();
        if (row == "cg") return std::make_unique<Membrane> (190.0f, 0.06f, 1.59f, 0.3f, 0.12f, 0.5f);
        if (row == "cr") return metal (crash, 0.55f, 128.0f, 7000.0f, 0.6f, 3500.0f, 0.5f, 3.0f, 2.6f, 0.35f);
        if (row == "rd") return metal (ride, 0.15f, 128.0f, 6500.0f, 1.2f, 2500.0f, 0.4f, 2.5f, 3.0f, 0.25f);
    }
    else if (machine == "cr78")
    {
        if (row == "bd") return std::make_unique<Kick808>();
        if (row == "sd") return std::make_unique<Snare808>();
        if (row == "rs") return std::make_unique<RimShot> (1400.0f, 520.0f);
        if (row == "ch") return metal (tr909, 0.15f, 0.0f, 8500.0f, 1.0f, 6500.0f, 0.008f, 0.05f, 4.5f, 0.0f);
        if (row == "cy") return std::make_unique<Cymbal808> (tr808);
        if (row == "cl") return std::make_unique<Claves>();
        if (row == "ma") return std::make_unique<Maracas>();
        if (row == "cb") return std::make_unique<Cowbell808>();
        if (row == "gu") return std::make_unique<Guiro>();
        if (row == "bl") return std::make_unique<Membrane> (330.0f, 0.05f, 1.7f, 0.2f, 0.05f, 0.3f);
        if (row == "bh") return std::make_unique<Membrane> (450.0f, 0.05f, 1.7f, 0.2f, 0.04f, 0.25f);
        if (row == "cg") return std::make_unique<Membrane> (200.0f, 0.06f, 1.59f, 0.25f, 0.1f, 0.45f);
        if (row == "tb") return std::make_unique<Tambourine>();
    }
    else if (machine == "mpc")
    {
        if (row == "bd") return std::make_unique<SampledKick>();
        if (row == "sd") return std::make_unique<SampledSnare>();
        if (row == "sn") return std::make_unique<Snap>();
        if (row == "rs") return std::make_unique<RimShot> (1700.0f, 500.0f);
        if (row == "ch") return metal (acoustic, 0.7f, 2048.0f, 9000.0f, 0.6f, 6000.0f, 0.02f, 0.08f, 3.8f, 0.0f);
        if (row == "oh") return metal (acoustic, 0.7f, 2048.0f, 9000.0f, 0.6f, 6000.0f, 0.12f, 0.7f, 3.2f, 0.0f);
        if (row == "sh") return std::make_unique<Maracas>();
        if (row == "pc") return std::make_unique<Membrane> (780.0f, 0.02f, 2.4f, 0.4f, 0.02f, 0.12f);
        if (row == "cr") return metal (crash, 0.55f, 2048.0f, 7000.0f, 0.6f, 3500.0f, 0.5f, 3.0f, 2.6f, 0.35f);
    }
    else if (machine == "kit")
    {
        if (row == "bd") return std::make_unique<AcousticKick>();
        if (row == "sd" || row == "gs") return std::make_unique<AcousticSnare>();
        if (row == "xs") return std::make_unique<RimShot> (820.0f, 1950.0f);
        if (row == "ch") return metal (acoustic, 0.6f, 0.0f, 9000.0f, 0.6f, 6000.0f, 0.02f, 0.08f, 3.8f, 0.0f);
        if (row == "oh") return metal (acoustic, 0.6f, 0.0f, 9000.0f, 0.6f, 6000.0f, 0.15f, 0.8f, 3.2f, 0.0f);
        if (row == "ht") return std::make_unique<Membrane> (180.0f, 0.25f, 1.5f, 0.25f, 0.12f, 0.6f);
        if (row == "mt") return std::make_unique<Membrane> (135.0f, 0.25f, 1.5f, 0.25f, 0.14f, 0.7f);
        if (row == "lt") return std::make_unique<Membrane> (95.0f, 0.25f, 1.5f, 0.25f, 0.18f, 0.9f);
        if (row == "cr") return metal (crash, 0.55f, 0.0f, 7000.0f, 0.6f, 3500.0f, 0.6f, 3.5f, 2.6f, 0.35f);
        if (row == "rd") return metal (ride, 0.15f, 0.0f, 6500.0f, 1.2f, 2500.0f, 0.5f, 3.0f, 3.0f, 0.25f);
        if (row == "rb") return std::make_unique<Bell> (720.0f);
    }
    else if (machine == "tr707")
    {
        if (row == "bd") return std::make_unique<SampledKick>();
        if (row == "sd") return std::make_unique<SampledSnare>();
        if (row == "rs") return std::make_unique<RimShot> (1600.0f, 520.0f);
        if (row == "cp") return std::make_unique<HandClap> (3, 0.009f, 1200.0f);
        if (row == "ch") return metal (tr909, 0.4f, 64.0f, 9000.0f, 0.9f, 7000.0f, 0.012f, 0.08f, 4.6f, 0.0f);
        if (row == "oh") return metal (tr909, 0.4f, 64.0f, 9000.0f, 0.9f, 7000.0f, 0.08f, 0.6f, 3.9f, 0.0f);
        if (row == "cb") return std::make_unique<Cowbell808>();
        if (row == "tb") return std::make_unique<Tambourine>();
        if (row == "ht") return std::make_unique<Membrane> (190.0f, 0.3f, 1.5f, 0.2f, 0.1f, 0.5f);
        if (row == "lt") return std::make_unique<Membrane> (120.0f, 0.3f, 1.5f, 0.2f, 0.12f, 0.6f);
        if (row == "bg") return std::make_unique<Membrane> (380.0f, 0.05f, 1.6f, 0.3f, 0.06f, 0.25f);
        if (row == "cg") return std::make_unique<Membrane> (200.0f, 0.06f, 1.59f, 0.3f, 0.1f, 0.45f);
        if (row == "ti") return std::make_unique<Membrane> (420.0f, 0.04f, 2.3f, 0.5f, 0.1f, 0.5f);
    }
    else if (machine == "simmons")
    {
        if (row == "bd") return std::make_unique<SimmonsDrum> (55.0f, 1.6f, 0.06f, 0.15f, 0.15f, 0.7f);
        if (row == "sd") return std::make_unique<SimmonsDrum> (210.0f, 0.9f, 0.08f, 0.6f, 0.1f, 0.5f);
        if (row == "t1") return std::make_unique<SimmonsDrum> (330.0f, 1.2f, 0.18f, 0.2f, 0.15f, 0.8f);
        if (row == "t2") return std::make_unique<SimmonsDrum> (240.0f, 1.2f, 0.2f, 0.2f, 0.18f, 0.9f);
        if (row == "t3") return std::make_unique<SimmonsDrum> (170.0f, 1.2f, 0.22f, 0.2f, 0.2f, 1.0f);
        if (row == "t4") return std::make_unique<SimmonsDrum> (115.0f, 1.2f, 0.25f, 0.2f, 0.22f, 1.1f);
        if (row == "cp") return std::make_unique<HandClap> (4, 0.008f, 1400.0f);
        if (row == "ch") return metal (tr808, 0.5f, 0.0f, 8500.0f, 1.0f, 6500.0f, 0.01f, 0.06f, 4.5f, 0.0f);
        if (row == "oh") return metal (tr808, 0.5f, 0.0f, 8500.0f, 1.0f, 6500.0f, 0.1f, 0.6f, 3.9f, 0.0f);
        if (row == "cr") return metal (crash, 0.6f, 0.0f, 7000.0f, 0.6f, 3500.0f, 0.5f, 3.0f, 2.6f, 0.35f);
    }
    else if (machine == "world")
    {
        if (row == "tna") return std::make_unique<Membrane> (430.0f, 0.02f, 2.0f, 0.7f, 0.15f, 0.7f);
        if (row == "tge") return std::make_unique<Membrane> (95.0f, -0.35f, 1.6f, 0.25f, 0.2f, 0.9f);
        if (row == "dum") return std::make_unique<Membrane> (110.0f, 0.1f, 1.5f, 0.2f, 0.15f, 0.6f);
        if (row == "tek") return std::make_unique<Membrane> (650.0f, 0.05f, 2.3f, 0.4f, 0.03f, 0.12f);
        if (row == "djb") return std::make_unique<Membrane> (80.0f, 0.25f, 1.6f, 0.2f, 0.15f, 0.5f);
        if (row == "djt") return std::make_unique<Membrane> (240.0f, 0.1f, 1.6f, 0.35f, 0.08f, 0.3f);
        if (row == "djs") return std::make_unique<Membrane> (420.0f, 0.05f, 2.1f, 0.6f, 0.03f, 0.15f);
        if (row == "tlk") return std::make_unique<Membrane> (150.0f, -0.6f, 1.5f, 0.2f, 0.2f, 0.8f);
        if (row == "tai") return std::make_unique<Membrane> (55.0f, 0.3f, 1.5f, 0.3f, 0.4f, 1.8f);
        if (row == "cjb") return std::make_unique<Membrane> (75.0f, 0.2f, 1.5f, 0.1f, 0.1f, 0.4f);
        if (row == "cjs") return std::make_unique<AcousticSnare>();
        if (row == "cax") return std::make_unique<Maracas>();
    }
    else if (machine == "fx")
    {
        if (row == "scr") return std::make_unique<Scratch>();
        if (row == "rwd") return std::make_unique<Rewind>();
        if (row == "gun") return std::make_unique<Gunshot>();
        if (row == "rain") return std::make_unique<Rain>();
        if (row == "rise") return std::make_unique<Riser>();
        if (row == "down") return std::make_unique<Downlifter>();
        if (row == "imp") return std::make_unique<Impact>();
        if (row == "swp") return std::make_unique<Sweep>();
        if (row == "zap") return std::make_unique<Zap>();
        if (row == "sir") return std::make_unique<Siren>();
        if (row == "horn") return std::make_unique<AirHorn>();
        if (row == "vin") return std::make_unique<Vinyl>();
        if (row == "rev") return std::make_unique<ReverseCymbal>();
    }
    else if (machine == "perc")
    {
        if (row == "cgl") return std::make_unique<Membrane> (165.0f, 0.06f, 1.59f, 0.3f, 0.15f, 0.6f);
        if (row == "cgh") return std::make_unique<Membrane> (225.0f, 0.06f, 1.59f, 0.3f, 0.12f, 0.5f);
        if (row == "bl") return std::make_unique<Membrane> (320.0f, 0.05f, 1.7f, 0.25f, 0.05f, 0.3f);
        if (row == "bh") return std::make_unique<Membrane> (440.0f, 0.05f, 1.7f, 0.25f, 0.04f, 0.25f);
        if (row == "tl") return std::make_unique<Membrane> (330.0f, 0.02f, 2.31f, 0.7f, 0.2f, 0.9f);
        if (row == "th") return std::make_unique<Membrane> (480.0f, 0.02f, 2.31f, 0.7f, 0.15f, 0.8f);
        if (row == "cb") return std::make_unique<Cowbell808>();
        if (row == "ag") return std::make_unique<Bell> (880.0f);
        if (row == "gu") return std::make_unique<Guiro>();
        if (row == "sh") return std::make_unique<Maracas>();
        if (row == "tb") return std::make_unique<Tambourine>();
        if (row == "tr") return std::make_unique<Triangle>();
        if (row == "cl") return std::make_unique<Claves>();
    }

    jassertfalse;
    return std::make_unique<Claves>();
}

float drumVoiceTrim (const juce::String& machine, const juce::String& row)
{
    struct Trim { const char* machine; const char* row; float gain; };
    static constexpr Trim trims[] {
        { "tr909", "sd", 1.4f }, { "tr909", "lt", 1.8f }, { "tr909", "mt", 1.8f }, { "tr909", "ht", 1.8f },
        { "tr909", "rs", 1.35f }, { "tr909", "cp", 1.5f }, { "tr909", "ch", 1.3f }, { "tr909", "oh", 1.35f },
        { "tr909", "cr", 1.35f }, { "tr909", "rd", 1.4f },
        { "tr808", "sd", 1.4f }, { "tr808", "lt", 1.8f }, { "tr808", "mt", 1.8f }, { "tr808", "ht", 1.8f },
        { "tr808", "rs", 1.35f }, { "tr808", "cp", 1.5f }, { "tr808", "cy", 1.7f }, { "tr808", "ch", 1.3f },
        { "tr808", "oh", 1.4f }, { "tr808", "cl", 1.3f },
        { "breaks", "bd", 1.4f }, { "breaks", "sd", 1.5f }, { "breaks", "oh", 1.15f }, { "breaks", "rd", 1.6f }, { "breaks", "cr", 1.6f },
        { "linn", "bd", 1.45f }, { "linn", "ch", 1.15f }, { "linn", "cr", 1.3f }, { "linn", "rd", 1.3f }, { "linn", "ss", 1.2f },
        { "cr78", "sd", 1.3f }, { "cr78", "ch", 2.4f }, { "cr78", "cy", 1.5f }, { "cr78", "gu", 0.5f },
        { "mpc", "ch", 1.1f }, { "mpc", "cr", 1.3f }, { "perc", "gu", 0.5f },
        { "fx", "zap", 2.5f }, { "fx", "gun", 1.8f }, { "fx", "rwd", 1.6f }, { "fx", "rain", 1.5f }, { "kit", "bd", 1.4f }, { "kit", "rb", 1.3f }, { "fx", "sir", 3.0f }, { "fx", "horn", 2.8f }, { "fx", "vin", 1.5f }, { "fx", "rev", 1.4f },
    };
    for (auto& t : trims)
        if (machine == t.machine && row == t.row) return t.gain;
    return 1.0f;
}
} // namespace techno
