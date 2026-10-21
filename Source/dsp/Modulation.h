#pragma once

#include "Reverb.h"

// Modulation effects of a machine's strip, their sweep locked to the tempo.
namespace techno
{
// Six all-pass stages swept between 200 Hz and 2.5 kHz, once per bar, left and right a quarter turn apart.
class Phaser
{
public:
    void prepare (float sampleRate)
    {
        fs = sampleRate;
        for (auto& c : channels) c = {};
    }

    void process (float amount, float barHz, float& left, float& right)
    {
        advance (phase, barHz / fs);
        left = channels[0].process (left, amount, sweep (phase), fs);
        right = channels[1].process (right, amount, sweep (phase + 0.25f), fs);
    }

private:
    static float sweep (float p)
    {
        const float lfo = 0.5f + 0.5f * std::sin (2.0f * pi * p);
        return 200.0f * std::pow (12.5f, lfo);
    }

    struct Channel
    {
        float state[6] {}, feedback = 0.0f;

        float process (float x, float amount, float hz, float fs)
        {
            const float t = std::tan (pi * std::min (hz, fs * 0.45f) / fs);
            const float a = (t - 1.0f) / (t + 1.0f);
            float y = x + feedback * 0.55f;
            for (auto& z : state)
            {
                const float out = a * y + z;
                z = y - a * out;
                y = out;
            }
            feedback = y;
            return x + amount * 0.5f * (y - x);
        }
    };

    float fs = 44100.0f, phase = 0.0f;
    Channel channels[2];
};

// A short delay (0.3 to 4.3 ms) swept once every two bars, fed back on itself: the "jet plane" sound.
class Flanger
{
public:
    void prepare (float sampleRate)
    {
        fs = sampleRate;
        lineL.allocate ((int) (fs * 0.01f));
        lineR.allocate ((int) (fs * 0.01f));
        feedL = feedR = 0.0f;
    }

    void process (float amount, float barHz, float& left, float& right)
    {
        advance (phase, barHz * 0.5f / fs);
        auto delayAt = [this] (float p) { return fs * (0.0003f + 0.004f * (0.5f + 0.5f * std::sin (2.0f * pi * p))); };
        const float dl = lineL.read (delayAt (phase)), dr = lineR.read (delayAt (phase + 0.1f));
        const float feedback = 0.6f * amount;
        lineL.write (left + feedL * feedback);
        lineR.write (right + feedR * feedback);
        feedL = dl;
        feedR = dr;
        const float norm = 1.0f / (1.0f + 0.7f * amount);
        left = (left + 0.9f * amount * dl) * norm;
        right = (right + 0.9f * amount * dr) * norm;
    }

private:
    float fs = 44100.0f, phase = 0.0f, feedL = 0.0f, feedR = 0.0f;
    DelayLine lineL, lineR;
};
} // namespace techno
