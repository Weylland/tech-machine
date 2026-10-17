#pragma once

#include <algorithm>
#include <cmath>

namespace techno
{
constexpr float pi = 3.14159265358979f;

inline float fastTanh (float x)
{
    if (x < -3.0f) return -1.0f;
    if (x > 3.0f) return 1.0f;
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

inline float midiToHz (float note)
{
    return 440.0f * std::exp2 ((note - 69.0f) / 12.0f);
}

inline float decayCoef (float seconds, float sampleRate)
{
    return std::exp (-1.0f / (std::max (seconds, 0.0001f) * sampleRate));
}

inline float polyBlep (float t, float dt)
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

struct Noise
{
    unsigned int state = 22222u;

    float next()
    {
        state = state * 196314165u + 907633515u;
        return (float) (int) state * (1.0f / 2147483648.0f);
    }
};

// Cytomic state-variable filter (trapezoidal integration), stable under fast modulation.
struct Svf
{
    float a1 = 0, a2 = 0, a3 = 0, k = 1;
    float ic1 = 0, ic2 = 0;
    float lp = 0, bp = 0, hp = 0;

    void set (float cutoff, float q, float sampleRate)
    {
        cutoff = std::clamp (cutoff, 10.0f, sampleRate * 0.48f);
        const float g = std::tan (pi * cutoff / sampleRate);
        k = 1.0f / q;
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    void process (float v0)
    {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        lp = v2;
        bp = v1;
        hp = v0 - k * v1 - v2;
    }

    void reset() { ic1 = ic2 = lp = bp = hp = 0; }
};

// Zero-delay-feedback 4-pole ladder with a saturating feedback path.
struct Ladder
{
    float s[4] {};
    float G = 0, g = 0, k = 0;

    void set (float cutoff, float resonance, float sampleRate)
    {
        cutoff = std::clamp (cutoff, 20.0f, sampleRate * 0.45f);
        g = std::tan (pi * cutoff / sampleRate);
        G = g / (1.0f + g);
        k = resonance * 3.85f;
    }

    float process (float x)
    {
        const float inv = 1.0f / (1.0f + g);
        const float S1 = s[0] * inv, S2 = s[1] * inv, S3 = s[2] * inv, S4 = s[3] * inv;
        const float G2 = G * G, G3 = G2 * G, G4 = G3 * G;
        const float sigma = G3 * S1 + G2 * S2 + G * S3 + S4;
        const float y4 = (G4 * x + sigma) / (1.0f + k * G4);

        float u = fastTanh (x * (1.0f + 0.35f * k) - k * y4);

        for (auto& state : s)
        {
            const float v = (u - state) * G;
            const float y = v + state;
            state = y + v;
            u = y;
        }
        return u;
    }

    void reset() { s[0] = s[1] = s[2] = s[3] = 0; }
};

struct OnePole
{
    float z = 0, a = 0;

    void set (float cutoff, float sampleRate) { a = 1.0f - std::exp (-2.0f * pi * std::min (cutoff, sampleRate * 0.45f) / sampleRate); }
    float lowpass (float x) { return z += a * (x - z); }
    float highpass (float x) { return x - lowpass (x); }
};

// Maps a 0..1 knob to a time between minSeconds and maxSeconds on an exponential curve.
inline float knobTime (float x, float minSeconds, float maxSeconds)
{
    return minSeconds * std::pow (maxSeconds / minSeconds, x);
}

// Maps a 0..1 knob to a pitch ratio, 0.5 = original pitch.
inline float tuneRatio (float x, float semitonesRange = 7.0f)
{
    return std::exp2 ((x - 0.5f) * 2.0f * semitonesRange / 12.0f);
}

inline float quantize (float x, float levels)
{
    return std::round (x * levels) / levels;
}

inline float blepSaw (float phase, float dt)
{
    return 2.0f * phase - 1.0f - polyBlep (phase, dt);
}

inline float blepPulse (float phase, float dt, float width)
{
    float t2 = phase - width;
    if (t2 < 0.0f) t2 += 1.0f;
    return (phase < width ? 1.0f : -1.0f) + polyBlep (phase, dt) - polyBlep (t2, dt);
}

inline void advance (float& phase, float dt)
{
    phase += dt;
    if (phase >= 1.0f) phase -= 1.0f;
}

// Analog-style ADSR: the attack aims above 1 so it reaches the top in a finite, slightly curved time.
struct Adsr
{
    enum Stage { Idle, Attack, Decay, Sustain, Release };

    Stage stage = Idle;
    float value = 0, sustain = 1;
    float attackCoef = 0, decayCoef_ = 0, releaseCoef = 0;

    void set (float attackS, float decayS, float sustainLevel, float releaseS, float fs)
    {
        attackCoef = 1.0f - std::exp (-1.0f / (std::max (attackS, 0.0005f) * fs));
        decayCoef_ = decayCoef (decayS, fs);
        releaseCoef = decayCoef (releaseS, fs);
        sustain = sustainLevel;
    }

    void gateOn() { stage = Attack; }
    void gateOff() { if (stage != Idle) stage = Release; }
    void reset() { stage = Idle; value = 0; }
    bool isActive() const { return stage != Idle; }

    float next()
    {
        switch (stage)
        {
            case Attack:
                value += (1.3f - value) * attackCoef;
                if (value >= 1.0f) { value = 1.0f; stage = Decay; }
                break;
            case Decay:
                value = sustain + (value - sustain) * decayCoef_;
                if (std::abs (value - sustain) < 0.0005f) stage = Sustain;
                break;
            case Sustain:
                value = sustain;
                break;
            case Release:
                value *= releaseCoef;
                if (value < 0.00005f) { value = 0; stage = Idle; }
                break;
            case Idle:
                break;
        }
        return value;
    }
};

// Six square oscillators at inharmonic ratios: the metallic source of the analog hats and cymbals.
struct MetalBank
{
    float phases[6] { 0.0f, 0.21f, 0.47f, 0.13f, 0.71f, 0.33f };

    float process (const float* freqs, float ratio, float sampleRate)
    {
        float sum = 0.0f;
        for (int i = 0; i < 6; ++i)
        {
            advance (phases[i], freqs[i] * ratio / sampleRate);
            sum += phases[i] < 0.5f ? 1.0f : -1.0f;
        }
        return sum * (1.0f / 6.0f);
    }
};
} // namespace techno
