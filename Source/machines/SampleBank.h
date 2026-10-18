#pragma once

#include "../dsp/SamplerVoice.h"

#include <array>
#include <vector>

namespace techno
{
// The sounds of the Sampler's pads. Files are loaded on the message thread; the audio thread reads the slots.
class SampleBank
{
public:
    static constexpr int numPads = 8;

    bool load (int pad, const juce::File&);
    bool chop (const juce::File& loop);   // 8 equal slices of a loop, one per pad
    void clear();

    juce::String name (int pad) const;
    const std::atomic<const SampleData*>* slot (int pad) const { return &slots[(size_t) pad]; }

    // A pad as written in a project file ("path|start|end|name", empty when the pad is empty), and back.
    juce::String describe (int pad) const;
    void restore (const juce::StringArray& descriptions);

    std::atomic<int> version { 0 };

private:
    using Audio = std::shared_ptr<const juce::AudioBuffer<float>>;
    static Audio read (const juce::File&, double& sampleRate);
    void set (int pad, Audio, double rate, int start, int end, const juce::String& padName, const juce::String& path);

    std::array<std::atomic<const SampleData*>, numPads> slots {};
    std::vector<std::unique_ptr<SampleData>> store;   // kept until the end: the audio thread may still play an old sound
};
} // namespace techno
