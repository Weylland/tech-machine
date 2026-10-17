#pragma once

#include <juce_core/juce_core.h>

namespace techno
{
struct HelpText
{
    juce::String title, body;
};

// Help for generic controls (transport, rows, mixer, master). Machine, drum row, synth parameter,
// scale and style help live next to their definitions.
HelpText help (const juce::String& key);

// First sentence of a help body, used as tooltip.
juce::String shortHelp (const juce::String& body);
} // namespace techno
