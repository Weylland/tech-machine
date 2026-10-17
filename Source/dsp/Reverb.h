#pragma once

#include "Filters.h"
#include <array>
#include <vector>

namespace techno
{
struct DelayLine
{
    std::vector<float> buffer;
    int size = 1, pos = 0;

    void allocate (int samples)
    {
        size = std::max (4, samples);
        buffer.assign ((size_t) size, 0.0f);
        pos = 0;
    }

    void write (float v)
    {
        buffer[(size_t) pos] = v;
        if (++pos >= size) pos = 0;
    }

    // delay = 1 returns the last written sample.
    float read (float delay) const
    {
        float p = (float) pos - delay;
        while (p < 0.0f) p += (float) size;
        const int i0 = std::min ((int) p, size - 1);   // pos - delay + size can round up to size
        const int i1 = i0 + 1 >= size ? 0 : i0 + 1;
        const float frac = p - (float) i0;
        return buffer[(size_t) i0] + frac * (buffer[(size_t) i1] - buffer[(size_t) i0]);
    }
};

// Feedback delay network reverb: pre-delay, four input diffusers, eight modulated lines mixed
// by a Householder matrix with per-line damping. Dense and smooth, without metallic ringing.
class FdnReverb
{
public:
    void prepare (float sampleRate)
    {
        fs = sampleRate;
        for (auto& line : lines) line.allocate ((int) (fs * 0.2f));
        preDelay.allocate ((int) (fs * 0.1f));
        static constexpr float diffuserMs[4] { 4.77f, 3.59f, 12.73f, 9.31f };
        for (int i = 0; i < 4; ++i)
        {
            diffusers[(size_t) i].line.allocate ((int) (fs * diffuserMs[i] / 1000.0f) + 2);
            diffusers[(size_t) i].length = fs * diffuserMs[i] / 1000.0f;
        }
        for (auto& d : damping) d.z = 0.0f;
        setSize (lastSize < 0.0f ? 0.5f : lastSize);
    }

    void setSize (float size)
    {
        lastSize = size;
        static constexpr float baseMs[8] { 29.7f, 37.1f, 41.1f, 43.7f, 53.0f, 59.3f, 67.9f, 73.1f };
        const float scale = 0.55f + size * 0.9f;
        const float rt60 = 0.45f * std::pow (16.0f, size);
        for (size_t i = 0; i < 8; ++i)
        {
            lengths[i] = fs * baseMs[i] * scale / 1000.0f;
            gains[i] = std::pow (10.0f, -3.0f * lengths[i] / (rt60 * fs));
            damping[i].set (9500.0f - size * 5000.0f, fs);
        }
        preDelaySamples = fs * (0.008f + size * 0.025f);
    }

    void process (float input, float& outL, float& outR)
    {
        preDelay.write (input);
        float x = preDelay.read (preDelaySamples);
        for (auto& d : diffusers) x = d.process (x, 0.65f);

        advance (lfo1, 0.31f / fs);
        advance (lfo2, 0.47f / fs);
        const float mod1 = std::sin (2.0f * pi * lfo1) * fs * 0.0007f;
        const float mod2 = std::sin (2.0f * pi * lfo2) * fs * 0.0006f;

        std::array<float, 8> y;
        float sum = 0.0f;
        for (size_t i = 0; i < 8; ++i)
        {
            const float mod = i == 1 ? mod1 : i == 5 ? mod2 : 0.0f;
            y[i] = lines[i].read (lengths[i] + mod);
            sum += y[i];
        }
        const float householder = sum * 0.25f;

        static constexpr float inputSigns[8] { 1, -1, 1, -1, 1, 1, -1, -1 };
        for (size_t i = 0; i < 8; ++i)
        {
            const float feedback = damping[i].lowpass ((y[i] - householder) * gains[i]);
            lines[i].write (x * inputSigns[i] * 0.5f + feedback);
        }

        outL = (y[0] - y[1] + y[2] - y[3] + y[4] - y[5] + y[6] - y[7]) * 0.32f;
        outR = (y[0] + y[1] - y[2] - y[3] + y[4] + y[5] - y[6] - y[7]) * 0.32f;
    }

private:
    struct Diffuser
    {
        DelayLine line;
        float length = 100.0f;

        float process (float x, float g)
        {
            const float delayed = line.read (length);
            const float v = x + g * delayed;
            line.write (v);
            return delayed - g * v;
        }
    };

    float fs = 44100.0f, lastSize = -1.0f, preDelaySamples = 400.0f, lfo1 = 0.0f, lfo2 = 0.37f;
    std::array<DelayLine, 8> lines;
    std::array<float, 8> lengths {}, gains {};
    std::array<OnePole, 8> damping;
    std::array<Diffuser, 4> diffusers;
    DelayLine preDelay;
};
} // namespace techno
