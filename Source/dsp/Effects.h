#pragma once

#include "Filters.h"
#include <vector>

namespace techno
{
// Tempo-synced ping-pong echo with the character of a tape unit: filtered, saturating
// feedback and a slow wow on the read head.
class PingPongDelay
{
public:
    void prepare (float sampleRate)
    {
        fs = sampleRate;
        size = (int) (sampleRate * 2.5f) + 4;
        bufferL.assign ((size_t) size, 0.0f);
        bufferR.assign ((size_t) size, 0.0f);
        writePos = 0;
        lowpassL.set (3200.0f, fs);
        lowpassR.set (3200.0f, fs);
        highpassL.set (180.0f, fs);
        highpassR.set (180.0f, fs);
        smoothCoef = 1.0f - decayCoef (0.08f, fs);
    }

    void setTime (float seconds) { targetDelay = std::clamp (seconds * fs, 1.0f, (float) size - 2.0f); }

    void process (float input, float feedback, float& outL, float& outR)
    {
        currentDelay += (targetDelay - currentDelay) * smoothCoef;
        advance (wow, 0.55f / fs);
        const float wobble = 1.0f + 0.0012f * std::sin (2.0f * pi * wow);

        outL = read (bufferL, currentDelay * wobble);
        outR = read (bufferR, currentDelay * (2.0f - wobble));

        const float fbL = highpassL.highpass (lowpassL.lowpass (outR)) * feedback;
        const float fbR = highpassR.highpass (lowpassR.lowpass (outL)) * feedback;

        bufferL[(size_t) writePos] = fastTanh ((input + fbL) * 1.1f) * 0.91f;
        bufferR[(size_t) writePos] = fastTanh (fbR * 1.1f) * 0.91f;
        if (++writePos >= size) writePos = 0;
    }

private:
    float read (const std::vector<float>& buffer, float delay) const
    {
        float pos = (float) writePos - delay;
        while (pos < 0.0f) pos += (float) size;
        const int i0 = std::min ((int) pos, size - 1);   // writePos - delay + size can round up to size
        const int i1 = (i0 + 1) % size;
        const float frac = pos - (float) i0;
        return buffer[(size_t) i0] + frac * (buffer[(size_t) i1] - buffer[(size_t) i0]);
    }

    std::vector<float> bufferL, bufferR;
    int size = 0, writePos = 0;
    float fs = 44100.0f, currentDelay = 1000.0f, targetDelay = 1000.0f, smoothCoef = 0.0f, wow = 0.0f;
    OnePole lowpassL, lowpassR, highpassL, highpassR;
};

// Stereo-linked look-ahead peak limiter: the gain is held at its minimum over the look-ahead
// window then box-smoothed, so it is fully down before a peak leaves the delay line.
class PeakLimiter
{
public:
    static constexpr int lookahead = 64;

    void prepare (float sampleRate)
    {
        releaseCoef = 1.0f - decayCoef (0.12f, sampleRate);
        for (auto& v : delayL) v = 0.0f;
        for (auto& v : delayR) v = 0.0f;
        for (auto& v : targets) v = 1.0f;
        for (auto& v : smoothed) v = 1.0f;
        smoothedSum = (float) lookahead;
        gain = 1.0f;
        pos = 0;
    }

    void process (float& left, float& right)
    {
        const float peak = std::max (std::abs (left), std::abs (right));
        targets[pos] = peak > ceiling ? ceiling / peak : 1.0f;

        float held = 1.0f;
        for (auto t : targets) held = std::min (held, t);

        gain = held < gain ? held : gain + (held - gain) * releaseCoef;

        smoothedSum += gain - smoothed[pos];
        smoothed[pos] = gain;

        const int out = (pos + 1) % lookahead;
        const float g = smoothedSum / (float) lookahead;
        const float dl = delayL[out], dr = delayR[out];
        delayL[pos] = left;
        delayR[pos] = right;
        pos = out;

        left = dl * g;
        right = dr * g;
    }

private:
    static constexpr float ceiling = 0.89f;
    float delayL[lookahead] {}, delayR[lookahead] {}, targets[lookahead] {}, smoothed[lookahead] {};
    float smoothedSum = 0.0f, gain = 1.0f, releaseCoef = 0.0f;
    int pos = 0;
};

// Bit-depth and sample-rate reduction (lo-fi sampler / video game grit).
class Crusher
{
public:
    void process (float amount, float sampleRate, float& left, float& right)
    {
        if (amount <= 0.001f) return;

        const float levels = std::exp2 (15.0f - amount * 12.0f);
        counter += 44100.0f * std::exp2 (-amount * 4.0f) / sampleRate;
        if (counter >= 1.0f)
        {
            counter -= std::floor (counter);
            holdL = quantize (left, levels);
            holdR = quantize (right, levels);
        }
        left = holdL;
        right = holdR;
    }

private:
    float counter = 0, holdL = 0, holdR = 0;
};

// Bipolar DJ filter: 0.5 = bypass, below = low-pass sweep, above = high-pass sweep.
class DjFilter
{
public:
    void set (float position, float sampleRate)
    {
        bypass = std::abs (position - 0.5f) < 0.02f;
        if (bypass) return;

        lowpassMode = position < 0.5f;
        const float depth = std::abs (position - 0.5f) * 2.0f;
        const float cutoff = lowpassMode ? 20000.0f * std::exp2 (-depth * 7.0f) : 20.0f * std::exp2 (depth * 8.0f);
        filterL.set (cutoff, 0.9f, sampleRate);
        filterR.set (cutoff, 0.9f, sampleRate);
    }

    void process (float& left, float& right)
    {
        if (bypass) return;
        filterL.process (left);
        filterR.process (right);
        left = lowpassMode ? filterL.lp : filterL.hp;
        right = lowpassMode ? filterR.lp : filterR.hp;
    }

private:
    bool bypass = true, lowpassMode = true;
    Svf filterL, filterR;
};

// Mono version of the DJ filter, used on single drum sounds.
class MonoDjFilter
{
public:
    void set (float position, float sampleRate)
    {
        bypass = std::abs (position - 0.5f) < 0.02f;
        if (bypass) return;
        lowpassMode = position < 0.5f;
        const float depth = std::abs (position - 0.5f) * 2.0f;
        filter.set (lowpassMode ? 20000.0f * std::exp2 (-depth * 7.0f) : 20.0f * std::exp2 (depth * 8.0f), 0.9f, sampleRate);
    }

    float process (float x)
    {
        if (bypass) return x;
        filter.process (x);
        return lowpassMode ? filter.lp : filter.hp;
    }

private:
    bool bypass = true, lowpassMode = true;
    Svf filter;
};

// Stereo-linked feed-forward compressor; "amount" lowers the threshold and adds make-up gain.
class GlueCompressor
{
public:
    void prepare (float sampleRate)
    {
        rmsCoef = 1.0f - decayCoef (0.006f, sampleRate);
        attack = 1.0f - decayCoef (0.01f, sampleRate);
        release = 1.0f - decayCoef (0.14f, sampleRate);
        power = 0.0f;
        gainDb = 0.0f;
    }

    void setAmount (float amount)
    {
        active = amount > 0.001f;
        thresholdDb = -6.0f - amount * 20.0f;
        makeupDb = amount * 9.0f;
    }

    // RMS detection, 3:1 ratio with a 6 dB soft knee, smoothed gain.
    void process (float& left, float& right)
    {
        if (! active) return;
        power += (0.5f * (left * left + right * right) - power) * rmsCoef;
        const float levelDb = 10.0f * std::log10 (power + 1.0e-9f);
        const float over = levelDb - thresholdDb;
        float reduction = 0.0f;
        if (over > 3.0f) reduction = over * (1.0f - 1.0f / 3.0f);
        else if (over > -3.0f) reduction = (1.0f - 1.0f / 3.0f) * (over + 3.0f) * (over + 3.0f) / 12.0f;

        const float target = -reduction;
        gainDb += (target - gainDb) * (target < gainDb ? attack : release);
        const float gain = std::pow (10.0f, (gainDb + makeupDb) / 20.0f);
        left *= gain;
        right *= gain;
    }

private:
    bool active = false;
    float power = 0, gainDb = 0, rmsCoef = 0, attack = 0, release = 0, thresholdDb = -6.0f, makeupDb = 0;
};

// Sidechain-style ducking triggered by the kick.
class Pump
{
public:
    void prepare (float sampleRate) { fs = sampleRate; }

    void trigger (float beatSeconds)
    {
        env = 1.0f;
        coef = decayCoef (beatSeconds * 0.22f, fs);
    }

    float process (float depth)
    {
        const float gain = 1.0f - depth * 0.85f * env;
        env *= coef;
        return gain;
    }

private:
    float fs = 44100.0f, env = 0.0f, coef = 0.0f;
};
} // namespace techno
