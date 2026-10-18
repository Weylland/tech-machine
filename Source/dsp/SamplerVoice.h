#pragma once

#include "DrumVoices.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <memory>

namespace techno
{
// A sound loaded on a sampler pad (or a slice of a loop): mono audio, its sample rate and the part to play.
struct SampleData
{
    std::shared_ptr<const juce::AudioBuffer<float>> audio;
    double sampleRate = 44100.0;
    int start = 0, end = 0;
    juce::String name, path;
};

// Plays the sample of its pad. TUNE = pitch (+/- 12 semitones), DECAY = how much of the sound plays, TONE = low-pass filter.
class SampleVoice : public DrumVoice
{
public:
    void setSlot (const std::atomic<const SampleData*>* s) { slot = s; }

    void trigger (float velocity, const DrumSettings& s) override
    {
        data = slot != nullptr ? slot->load() : nullptr;
        if (data == nullptr || data->audio == nullptr || data->end <= data->start) { data = nullptr; return; }
        gain = velocity;
        position = (double) data->start;
        rate = data->sampleRate / fs * (double) tuneRatio (s.tune, 12.0f);
        const int length = data->end - data->start;
        stop = (double) data->start + (double) length * (double) juce::jlimit (0.03f, 1.0f, s.decay * s.decay * 1.03f);
        fade = 1.0f;
        fadeStep = 1.0f / juce::jmax (1.0f, 0.004f * fs);
        open = s.tone > 0.97f;
        if (! open) filter.set (60.0f * std::exp2 (s.tone * 8.5f), 0.7f, fs);
    }

    void choke() override { stop = juce::jmin (stop, position + 0.01 * fs * rate); }

    float process() override
    {
        if (data == nullptr) return 0.0f;
        if (position >= stop)
        {
            fade -= fadeStep;   // a few milliseconds of fade so a cut sample never clicks
            if (fade <= 0.0f) { data = nullptr; return 0.0f; }
        }
        const auto& audio = *data->audio;
        const int i = (int) position;
        if (i + 1 >= audio.getNumSamples() || i + 1 >= data->end + 1) { data = nullptr; return 0.0f; }
        const float frac = (float) (position - (double) i);
        const float* x = audio.getReadPointer (0);
        float y = x[i] + frac * (x[i + 1] - x[i]);
        position += rate;
        if (! open)
        {
            filter.process (y);
            y = filter.lp;
        }
        return y * gain * fade;
    }

private:
    const std::atomic<const SampleData*>* slot = nullptr;
    const SampleData* data = nullptr;
    double position = 0, rate = 1, stop = 0;
    float gain = 1, fade = 1, fadeStep = 0.01f;
    bool open = true;
    Svf filter;
};
} // namespace techno
