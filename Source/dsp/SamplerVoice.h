#pragma once

#include "../core/MachineDefs.h"
#include "DrumVoices.h"
#include "SynthVoices.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
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
    int loopStart = 0, loopEnd = 0;   // the part repeated while a note is held (SAMPLE KEYS)
    juce::String name, path;
};

// SAMPLE KEYS: one sound played on every note. Its pitch follows the note against the ROOT note; with LOOP, the
// looped part repeats (with a short crossfade, so it never clicks) while the note is held.
struct SampleKeysParams
{
    const std::atomic<const SampleData*>* slot = nullptr;
    int root = 60;
    bool loop = false;
    float attack = 0.0f, release = 0.25f, tone = 1.0f;
};

class SampleKeyVoice : public PolyVoice
{
public:
    explicit SampleKeyVoice (const SampleKeysParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
        filter.reset();
        data = nullptr;
    }

    void start (int midiNote, float velocity) override
    {
        data = p.slot != nullptr ? p.slot->load() : nullptr;
        if (data == nullptr || data->audio == nullptr || data->end <= data->start + 2) { data = nullptr; return; }
        note = midiNote;
        gain = velocity;
        position = (double) data->start;
        freq = noteFreq = (float) (std::exp2 ((double) (midiNote - p.root) / 12.0) * data->sampleRate / fs);   // read speed
        adsr.set (knobTime (p.attack, 0.002f, 2.0f), 0.1f, 1.0f, knobTime (p.release, 0.01f, 3.0f), fs);
        adsr.gateOn();
        open = p.tone > 0.97f;
        if (! open) filter.set (60.0f * std::exp2 (p.tone * 8.5f), 0.7f, fs);
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return data != nullptr && adsr.isActive(); }

    void process (float& left, float& right) override
    {
        if (data == nullptr) return;
        const double loopLength = (double) (data->loopEnd - data->loopStart);
        const bool looping = p.loop && loopLength > 64.0;
        if (looping)
        {
            while (position >= (double) data->loopEnd) position -= loopLength;
        }
        else if (position >= (double) data->end - 2.0)
        {
            data = nullptr;
            return;
        }

        float y = read (position);
        if (looping)
        {
            // The end of the loop fades into what comes just before its start.
            const double fadeLength = std::min ({ 512.0, loopLength * 0.5, (double) data->loopStart });
            const double into = position - ((double) data->loopEnd - fadeLength);
            if (fadeLength > 1.0 && into > 0.0)
            {
                const float a = (float) (into / fadeLength);
                y = y * (1.0f - a) + read (position - loopLength) * a;
            }
        }
        position += (double) freq;
        if (! open)
        {
            filter.process (y);
            y = filter.lp;
        }
        y *= adsr.next() * gain;
        left += y;
        right += y;
    }

private:
    // 4-point Hermite interpolation: clean even when the sound is played much higher than it was recorded.
    float read (double pos) const
    {
        const auto& audio = *data->audio;
        const int last = audio.getNumSamples() - 1;
        const int i = (int) pos;
        const float t = (float) (pos - (double) i);
        const float* x = audio.getReadPointer (0);
        auto at = [x, last] (int k) { return x[juce::jlimit (0, last, k)]; };
        const float xm1 = at (i - 1), x0 = at (i), x1 = at (i + 1), x2 = at (i + 2);
        const float c1 = 0.5f * (x1 - xm1), c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2, c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
        return ((c3 * t + c2) * t + c1) * t + x0;
    }

    const SampleKeysParams& p;
    const SampleData* data = nullptr;
    double position = 0.0;
    float fs = 44100.0f, gain = 1.0f;
    bool open = true;
    Adsr adsr;
    Svf filter;
};

// Plays the sample of its pad. TUNE = pitch (+/- 12 semitones), DECAY = how much of the sound plays, TONE = low-pass filter.
// The machine's CARACTÈRE gives it the grain of an old sampler, or turns it into a cloud of grains.
class SampleVoice : public DrumVoice
{
public:
    enum Mode { Original, EightBit, TwelveBit, Vintage, LoFi, Granular };

    void setSlot (const std::atomic<const SampleData*>* s) { slot = s; }

    void setMachineParams (const float* values) override
    {
        mode = juce::jlimit (0, 5, (int) values[pSampler::Mode]);
        texture = values[pSampler::Texture];
    }

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

        playing = mode;
        holdPhase = 1.0f;
        if (playing == Vintage) character.set (14000.0f - texture * 6000.0f, 0.7f, fs);
        if (playing == LoFi) character.set (6000.0f - texture * 3500.0f, 0.7f, fs);
        if (playing == Granular)
        {
            // The whole slice is spread over DECAY seconds, whatever its pitch.
            cloudLeft = (int) (knobTime (s.decay, 0.15f, 4.0f) * fs);
            head = (double) data->start;
            headRate = (double) length / (double) juce::jmax (1, cloudLeft);
            grainLength = juce::jmax (16, (int) (knobTime (texture, 0.015f, 0.25f) * fs));
            spawnCountdown = 0;
            for (auto& g : grains) g.active = false;
        }
    }

    void choke() override
    {
        stop = juce::jmin (stop, position + 0.01 * fs * rate);
        cloudLeft = juce::jmin (cloudLeft, 1);
    }

    float process() override
    {
        if (data == nullptr) return 0.0f;
        float y = playing == Granular ? grainCloud() : readPlayhead();
        if (data == nullptr) return 0.0f;
        y = colour (y);
        if (! open)
        {
            filter.process (y);
            y = filter.lp;
        }
        return y * gain * fade;
    }

private:
    float readPlayhead()
    {
        if (position >= stop)
        {
            fade -= fadeStep;   // a few milliseconds of fade so a cut sample never clicks
            if (fade <= 0.0f) { data = nullptr; return 0.0f; }
        }
        const float y = read (position);
        if (data == nullptr) return 0.0f;
        position += rate;
        return y;
    }

    float read (double pos)
    {
        const auto& audio = *data->audio;
        const int i = (int) pos;
        if (i < 0 || i + 1 >= audio.getNumSamples() || i + 1 >= data->end + 1)
        {
            if (playing != Granular) data = nullptr;
            return 0.0f;
        }
        const float frac = (float) (pos - (double) i);
        const float* x = audio.getReadPointer (0);
        return x[i] + frac * (x[i + 1] - x[i]);
    }

    // Overlapping grains, each a short window read at the pad's pitch, while a slow head moves through the sound.
    float grainCloud()
    {
        if (cloudLeft > 0)
        {
            --cloudLeft;
            head += headRate;
            if (--spawnCountdown <= 0)
            {
                spawnCountdown = grainLength / 4;
                for (auto& g : grains)
                {
                    if (g.active) continue;
                    const double jitter = (double) noise.next() * 0.3 * (double) grainLength * rate;
                    g = { juce::jlimit ((double) data->start, (double) data->end - 2.0, head + jitter), 0, grainLength, true };
                    break;
                }
            }
        }
        float sum = 0.0f;
        bool any = false;
        for (auto& g : grains)
        {
            if (! g.active) continue;
            any = true;
            const float w = 0.5f - 0.5f * std::cos (2.0f * pi * (float) g.age / (float) g.length);
            sum += read (g.position) * w;
            g.position += rate;
            if (++g.age >= g.length || g.position >= (double) data->end - 1.0) g.active = false;
        }
        if (! any && cloudLeft <= 0) data = nullptr;
        return sum * 0.5f;
    }

    // The grain of the old samplers: fewer bits, a lower sample rate, a darker and more compressed sound.
    float colour (float x)
    {
        switch (playing)
        {
            case EightBit: return crush (x, 8.0f - texture * 3.0f, 32000.0f - texture * 16000.0f);
            case TwelveBit: return fastTanh (crush (x, 12.0f - texture * 2.0f, 26040.0f) * 1.2f) / 1.1f;
            case Vintage:
                character.process (fastTanh (x * (1.0f + texture)) / (1.0f + texture * 0.5f));
                return character.lp;
            case LoFi:
            {
                character.process (fastTanh (x * 2.5f) * 0.55f);
                float crackle = 0.0f;
                if (noise.next() > 1.0f - 0.0004f * texture) crackle = noise.next() * 0.3f;
                return character.lp + crackle + noise.next() * 0.006f * texture;
            }
            default: return x;
        }
    }

    float crush (float x, float bits, float targetRate)
    {
        holdPhase += targetRate / fs;
        if (holdPhase >= 1.0f)
        {
            holdPhase -= std::floor (holdPhase);
            const float levels = std::exp2 (bits - 1.0f);
            held = std::round (x * levels) / levels;
        }
        return held;
    }

    struct Grain { double position = 0; int age = 0, length = 1; bool active = false; };

    const std::atomic<const SampleData*>* slot = nullptr;
    const SampleData* data = nullptr;
    double position = 0, rate = 1, stop = 0, head = 0, headRate = 1;
    float gain = 1, fade = 1, fadeStep = 0.01f, texture = 0.5f, holdPhase = 1, held = 0;
    int mode = Original, playing = Original, cloudLeft = 0, grainLength = 1000, spawnCountdown = 0;
    bool open = true;
    std::array<Grain, 8> grains {};
    Svf filter, character;
};
} // namespace techno
