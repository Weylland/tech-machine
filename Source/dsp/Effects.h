#pragma once

#include "Filters.h"
#include <vector>

namespace techno
{
// Tempo-synced ping-pong echo with filtered feedback (dub style).
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

        outL = read (bufferL);
        outR = read (bufferR);

        const float fbL = highpassL.highpass (lowpassL.lowpass (outR)) * feedback;
        const float fbR = highpassR.highpass (lowpassR.lowpass (outL)) * feedback;

        bufferL[(size_t) writePos] = fastTanh (input + fbL);
        bufferR[(size_t) writePos] = fbR;
        if (++writePos >= size) writePos = 0;
    }

private:
    float read (const std::vector<float>& buffer) const
    {
        float pos = (float) writePos - currentDelay;
        if (pos < 0.0f) pos += (float) size;
        const int i0 = (int) pos;
        const int i1 = (i0 + 1) % size;
        const float frac = pos - (float) i0;
        return buffer[(size_t) i0] + frac * (buffer[(size_t) i1] - buffer[(size_t) i0]);
    }

    std::vector<float> bufferL, bufferR;
    int size = 0, writePos = 0;
    float fs = 44100.0f, currentDelay = 1000.0f, targetDelay = 1000.0f, smoothCoef = 0.0f;
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

// Stereo-linked feed-forward compressor; "amount" lowers the threshold and adds make-up gain.
class GlueCompressor
{
public:
    void prepare (float sampleRate)
    {
        attack = 1.0f - decayCoef (0.008f, sampleRate);
        release = 1.0f - decayCoef (0.15f, sampleRate);
        env = 0.0f;
    }

    void setAmount (float amount)
    {
        active = amount > 0.001f;
        threshold = std::pow (10.0f, (-4.0f - amount * 22.0f) / 20.0f);
        makeup = 1.0f + amount * 1.6f;
    }

    void process (float& left, float& right)
    {
        if (! active) return;
        const float level = std::max (std::abs (left), std::abs (right));
        env += (level - env) * (level > env ? attack : release);
        const float gain = (env > threshold ? std::pow (env / threshold, -0.75f) : 1.0f) * makeup;
        left *= gain;
        right *= gain;
    }

private:
    bool active = false;
    float env = 0, attack = 0, release = 0, threshold = 1, makeup = 1;
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
