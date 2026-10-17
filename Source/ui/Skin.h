#pragma once

#include "../core/MachineDefs.h"
#include <juce_graphics/juce_graphics.h>
#include <array>

namespace techno
{
enum class KnobStyle { Classic, Cap, Chicken, Moog, Modern };

// Look of a faceplate: materials, printed text, step caps, LEDs and knobs.
struct Skin
{
    juce::Colour plateTop, plateBottom;      // faceplate gradient
    juce::Colour band, bandText;             // top strip with the model name
    juce::Colour text, textDim, line;        // silk-screen printing
    juce::Colour accent;                     // value arcs, highlights
    juce::Colour led;                        // LEDs
    std::array<juce::Colour, 4> caps;        // step button colour for each group of 4 steps
    juce::Colour lit;                        // colour of an active step
    juce::Colour capText;                    // text printed on an active step
    KnobStyle knob;
    juce::Colour knobCap;
    bool faders = false, wood = false, stripes = false, lightPlate = false;
    const char* brand;
    const char* model;
    bool litUsesCap = false;                 // active keys light up in their own colour (808)
};

inline const Skin& skinFor (int machineType)
{
    using C = juce::Colour;
    static const std::array<Skin, numMachines> skins {{
        // TR-909: silver-grey plate, dark band, orange accents, light step keys.
        { C (0xffd3d1cb), C (0xffb3b1ab), C (0xff38393c), C (0xfff2efe8), C (0xff1e1e20), C (0xff4a4a4c), C (0xff6c6b67),
          C (0xffff6a1a), C (0xffff3b1f), { C (0xfff1efe9), C (0xffdddbd4), C (0xfff1efe9), C (0xffdddbd4) },
          C (0xffff7a2a), C (0xff1e1e20), KnobStyle::Classic, C (0xff4b4b50), false, false, false, true,
          "ANALOG DRUM MACHINE", "909" },
        // TR-808: charcoal plate, the famous red / orange / yellow / cream step keys.
        { C (0xff2f2f2f), C (0xff1f1f1f), C (0xff131313), C (0xfff1e9d8), C (0xfff1e9d8), C (0xffb9b2a3), C (0xff6b655b),
          C (0xfff08a24), C (0xffff3b1f), { C (0xffd8432e), C (0xffee8a25), C (0xffefcf3a), C (0xffede6d6) },
          C (0xffffe9a8), C (0xff1a1a1a), KnobStyle::Cap, C (0xffede6d6), false, false, false, false,
          "VINTAGE DRUM MACHINE", "808", true },
        // Break kit: beige sampler with grey rubber pads and green LEDs.
        { C (0xffdcd8cd), C (0xffc2bdb0), C (0xff2d2d2f), C (0xfff2f0ea), C (0xff262626), C (0xff57554f), C (0xff77746c),
          C (0xff3dbe5a), C (0xff3dff6a), { C (0xff6d6d6d), C (0xff626262), C (0xff6d6d6d), C (0xff626262) },
          C (0xff8ff0a2), C (0xff1a1a1a), KnobStyle::Classic, C (0xff55555a), false, false, false, true,
          "SAMPLING DRUM COMPUTER", "BREAKS" },
        // TB-303: brushed silver with black keys and red LEDs.
        { C (0xffdcdee1), C (0xffb5b8bd), C (0xff2a2b2e), C (0xffededed), C (0xff202022), C (0xff4d4f53), C (0xff7a7c80),
          C (0xffe8322c), C (0xffff2a1f), { C (0xff2c2c2e), C (0xff2c2c2e), C (0xff2c2c2e), C (0xff2c2c2e) },
          C (0xfff5c542), C (0xff1a1a1a), KnobStyle::Chicken, C (0xffd9d9d9), false, false, false, true,
          "ACID BASS SYNTHESIZER", "303" },
        // SH-101: grey plate with blue accents and sliders.
        { C (0xff65686d), C (0xff4d5055), C (0xff27292d), C (0xffffffff), C (0xfff2f2f2), C (0xffc5c8cc), C (0xff9a9da2),
          C (0xff3e8ede), C (0xffff4136), { C (0xffd7d9dc), C (0xffc8cacd), C (0xffd7d9dc), C (0xffc8cacd) },
          C (0xff59aaff), C (0xff10131a), KnobStyle::Classic, C (0xff4b4d52), true, false, false, false,
          "MONOPHONIC SYNTHESIZER", "101" },
        // Model D: black panel, wood cheeks, amber accents, big knurled knobs.
        { C (0xff222223), C (0xff161617), C (0xff0f0f10), C (0xfff4f1ea), C (0xfff4f1ea), C (0xffb5b0a6), C (0xff7d786f),
          C (0xffe8a33c), C (0xffff5a1f), { C (0xff3c3c3e), C (0xff353537), C (0xff3c3c3e), C (0xff353537) },
          C (0xffffb84d), C (0xff1a1206), KnobStyle::Moog, C (0xff1b1b1c), false, true, false, false,
          "THREE OSCILLATOR SYNTHESIZER", "D-BASS" },
        // Juno-106: black panel, coloured stripes, sliders, blue accents.
        { C (0xff2b2c2f), C (0xff1d1e20), C (0xff111214), C (0xffffffff), C (0xffffffff), C (0xffbfc3c9), C (0xff6f737a),
          C (0xff4db6e8), C (0xffff3b1f), { C (0xffe3e5e8), C (0xffd2d4d8), C (0xffe3e5e8), C (0xffd2d4d8) },
          C (0xff6fd3ff), C (0xff0b1620), KnobStyle::Classic, C (0xff45474c), true, false, true, false,
          "POLYPHONIC SYNTHESIZER", "106" },
        // JP-8000: metallic blue-grey, cyan LEDs.
        { C (0xff4a5a77), C (0xff313d54), C (0xff1b2333), C (0xffe6eeff), C (0xffe6eeff), C (0xffa9b8d6), C (0xff7f8eab),
          C (0xff59c3ff), C (0xff59c3ff), { C (0xffbac4d8), C (0xffa9b4ca), C (0xffbac4d8), C (0xffa9b4ca) },
          C (0xff8fe0ff), C (0xff0a1626), KnobStyle::Modern, C (0xff2a3446), false, false, false, false,
          "ANALOG MODELING SYNTHESIZER", "8000" },
        // LinnDrum: dark panel, wood cheeks, red LEDs, grey rubber keys.
        { C (0xff2b2b2e), C (0xff1c1c1e), C (0xff111112), C (0xfff0ece4), C (0xfff0ece4), C (0xffb8b2a8), C (0xff6d685f),
          C (0xffe0453a), C (0xffff3b1f), { C (0xff8a8a8e), C (0xff7c7c80), C (0xff8a8a8e), C (0xff7c7c80) },
          C (0xffff6b5c), C (0xff1a1a1a), KnobStyle::Classic, C (0xff3c3c40), false, true, false, false,
          "DIGITAL DRUM MACHINE", "LINN" },
        // CR-78: cream panel, wood cheeks, brown keys, amber light.
        { C (0xffe3d6b8), C (0xffc9b994), C (0xff4a3426), C (0xfff3e7cf), C (0xff2b2118), C (0xff5a4a3a), C (0xff8a7860),
          C (0xffc8702e), C (0xffff5a1f), { C (0xff6b4c36), C (0xff5c412e), C (0xff6b4c36), C (0xff5c412e) },
          C (0xffffb04d), C (0xff1a1206), KnobStyle::Cap, C (0xffe9dcc0), false, true, false, true,
          "COMPU RHYTHM", "CR-78" },
        // Sampler: light grey, red rubber pads.
        { C (0xffcfcfcb), C (0xffb3b3ae), C (0xff2a2a2c), C (0xffeeeeee), C (0xff222222), C (0xff505050), C (0xff7d7d78),
          C (0xffd23b2f), C (0xffff3b1f), { C (0xff9a9a96), C (0xff8e8e8a), C (0xff9a9a96), C (0xff8e8e8a) },
          C (0xffff6a4a), C (0xff1a1a1a), KnobStyle::Classic, C (0xff4b4b50), false, false, false, true,
          "MIDI PRODUCTION CENTER", "MPC" },
        // Percussion: warm wood plate, tan skins, orange light.
        { C (0xff4a2f1f), C (0xff2f1d13), C (0xff1e120b), C (0xfff6e6d0), C (0xfff6e6d0), C (0xffcdb498), C (0xff8a6a50),
          C (0xffff9a3c), C (0xffff7a1f), { C (0xffc9a27a), C (0xffb88f66), C (0xffc9a27a), C (0xffb88f66) },
          C (0xffffc06b), C (0xff2a1608), KnobStyle::Cap, C (0xfff1dfc3), false, true, false, false,
          "LATIN & AFRO PERCUSSION", "PERCU" },
        // DX7: dark brown panel, teal accents, burgundy keys.
        { C (0xff3b332c), C (0xff29231e), C (0xff1a1613), C (0xffe8e2d6), C (0xffe8e2d6), C (0xffb4aa9a), C (0xff6e6458),
          C (0xff3fd0b0), C (0xff3fd0b0), { C (0xff7a2e3b), C (0xff6c2834), C (0xff7a2e3b), C (0xff6c2834) },
          C (0xff5ff0cf), C (0xff0b201a), KnobStyle::Modern, C (0xff3a322b), false, false, false, false,
          "FM DIGITAL SYNTHESIZER", "DX7" },
        // String machine: black panel, orange accents, silver keys.
        { C (0xff222224), C (0xff151516), C (0xff0d0d0e), C (0xffffffff), C (0xfff2f2f2), C (0xffbfbfc4), C (0xff6c6c72),
          C (0xffffb347), C (0xffff8a1f), { C (0xffcfd2d6), C (0xffbfc2c7), C (0xffcfd2d6), C (0xffbfc2c7) },
          C (0xffffc56b), C (0xff201406), KnobStyle::Chicken, C (0xffd9d9d9), false, false, false, false,
          "STRING ENSEMBLE", "STRINGS" },
        // Organ: wood cabinet, cream drawbars.
        { C (0xff5a3520), C (0xff3d2314), C (0xff24150c), C (0xfff4e9d0), C (0xfff4e9d0), C (0xffd2bf9e), C (0xff8d6c50),
          C (0xffe8c070), C (0xffff3b1f), { C (0xffefe3c8), C (0xffe2d4b6), C (0xffefe3c8), C (0xffe2d4b6) },
          C (0xffffd98a), C (0xff2a1a08), KnobStyle::Cap, C (0xffefe3c8), true, true, false, false,
          "TONEWHEEL ORGAN", "ORGAN" },
        // 808 bass: black with red.
        { C (0xff1b1b1d), C (0xff0f0f10), C (0xff080809), C (0xffffffff), C (0xfff2f2f2), C (0xffb0b0b6), C (0xff5c5c62),
          C (0xffff2d3a), C (0xffff2d3a), { C (0xff333336), C (0xff2c2c2f), C (0xff333336), C (0xff2c2c2f) },
          C (0xffff5562), C (0xff1a0406), KnobStyle::Modern, C (0xff2a2a2e), false, false, false, false,
          "TRAP BASS", "808 SUB" },
        // Piano: black lacquer, ivory keys, gold print.
        { C (0xff1a1a1c), C (0xff0c0c0d), C (0xff050506), C (0xffe9d8a6), C (0xffe9d8a6), C (0xffb9ab86), C (0xff5e5543),
          C (0xffe9c46a), C (0xffffd36b), { C (0xfff4efe1), C (0xffe6e0cf), C (0xfff4efe1), C (0xffe6e0cf) },
          C (0xffe9c46a), C (0xff1a1406), KnobStyle::Modern, C (0xff2a2a2c), false, false, false, false,
          "ACOUSTIC PIANO", "PIANO" },
        // Mallets: brushed aluminium bars, sky blue.
        { C (0xffc9d3dc), C (0xffa9b4be), C (0xff2b3540), C (0xffe6f4ff), C (0xff1d2630), C (0xff4a5663), C (0xff6c7a88),
          C (0xff2f9fe0), C (0xff3fc0ff), { C (0xffe9eef2), C (0xffd6dde3), C (0xffe9eef2), C (0xffd6dde3) },
          C (0xff6fd0ff), C (0xff0a1a26), KnobStyle::Classic, C (0xff3c4650), false, false, false, true,
          "MALLETS & BELLS", "MALLETS" },
        // Pluck: spruce top, rosewood band.
        { C (0xffd9b98a), C (0xffc19d6b), C (0xff4a2a18), C (0xfff3e2c8), C (0xff2a1a0e), C (0xff5c4330), C (0xff8a6a4c),
          C (0xffb5581f), C (0xffff7a1f), { C (0xff6b3f22), C (0xff5c351c), C (0xff6b3f22), C (0xff5c351c) },
          C (0xffffa04d), C (0xff1a0e04), KnobStyle::Cap, C (0xfff0dcc0), false, true, false, true,
          "PLUCKED STRINGS", "PLUCK" },
        // Choir: deep violet, lavender.
        { C (0xff2c2240), C (0xff1c152b), C (0xff120d1c), C (0xffe8dcff), C (0xffece4ff), C (0xffb7a8d6), C (0xff6a5c88),
          C (0xffb48cff), C (0xffc8a8ff), { C (0xff4a3d66), C (0xff42365c), C (0xff4a3d66), C (0xff42365c) },
          C (0xffd2b8ff), C (0xff1a1028), KnobStyle::Modern, C (0xff3a3050), false, false, false, false,
          "VOCAL ENSEMBLE", "CHOIR" },
        // FX: black with acid green.
        { C (0xff1e2120), C (0xff121413), C (0xff0a0b0b), C (0xffd8ffc0), C (0xffe8ffe0), C (0xffa8c0a0), C (0xff56645a),
          C (0xff9cff57), C (0xff9cff57), { C (0xff2f3532), C (0xff29302c), C (0xff2f3532), C (0xff29302c) },
          C (0xffb8ff7a), C (0xff0e1a06), KnobStyle::Modern, C (0xff2a302c), false, false, true, false,
          "SOUND EFFECTS", "FX" },
    }};
    return skins[(size_t) machineType];
}

// Skin of the main console (top bar and master section).
inline const Skin& consoleSkin()
{
    using C = juce::Colour;
    static const Skin skin { C (0xff2a2b30), C (0xff1c1d21), C (0xff121316), C (0xffe9e6e0), C (0xffe9e6e0), C (0xff9a9aa2),
                             C (0xff55565e), C (0xffff6a2b), C (0xffff3b1f),
                             { C (0xff3a3b41), C (0xff3a3b41), C (0xff3a3b41), C (0xff3a3b41) }, C (0xffff7a2a), C (0xff1a1a1a),
                             KnobStyle::Modern, C (0xff35363c), false, false, false, false, "", "" };
    return skin;
}
} // namespace techno
