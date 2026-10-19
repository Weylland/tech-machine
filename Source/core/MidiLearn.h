#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>

namespace techno
{
// Knobs driven by the controllers (CC) of a MIDI keyboard. The assignments belong to the controller,
// not to a song: they are kept in the application settings.
class MidiLearn
{
public:
    explicit MidiLearn (juce::AudioProcessorValueTreeState&);

    void start (const juce::String& paramId);   // the next controller moved drives this parameter
    void forget (const juce::String& paramId);
    int controllerFor (const juce::String& paramId) const;   // -1 when none
    bool handleController (int cc, int value);  // audio thread; false when no knob is assigned to this controller
    void save();

    std::atomic<int> version { 0 };

private:
    void load();

    juce::AudioProcessorValueTreeState& state;
    std::atomic<juce::RangedAudioParameter*> learnTarget { nullptr };
    std::array<std::atomic<juce::RangedAudioParameter*>, 128> ccMap {};
};
} // namespace techno
