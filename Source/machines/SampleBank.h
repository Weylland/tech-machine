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
    static constexpr int keysSlot = numPads;          // the sound of SAMPLE KEYS
    static constexpr int numSlots = numPads + 1;

    bool load (int slot, const juce::File&);
    bool chop (const juce::File& loop);   // 8 equal slices of a loop, one per pad
    void clear();
    void clearPads();
    void setRange (int slot, int start, int end, int loopStart, int loopEnd);   // in samples of the file
    const SampleData* data (int slot) const { return slots[(size_t) slot].load(); }

    juce::String name (int pad) const;
    const std::atomic<const SampleData*>* slot (int pad) const { return &slots[(size_t) pad]; }

    // A slot as written in a project file ("path|start|end|name|loopStart|loopEnd", empty when it is empty), and back.
    juce::String describe (int slot) const;
    void restore (const juce::StringArray& descriptions);

    std::atomic<int> version { 0 };

private:
    using Audio = std::shared_ptr<const juce::AudioBuffer<float>>;
    static Audio read (const juce::File&, double& sampleRate);
    void set (int slot, Audio, double rate, int start, int end, const juce::String& padName, const juce::String& path,
              int loopStart = -1, int loopEnd = -1);

    std::array<std::atomic<const SampleData*>, numSlots> slots {};
    std::vector<std::unique_ptr<SampleData>> store;   // kept until the end: the audio thread may still play an old sound
};
} // namespace techno
