#pragma once

#include "DrumVoices.h"

#include <memory>

namespace techno
{
// TR-606 kick: a short, tight tone with a quick drop in pitch and a small click.
class Kick606 : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        pitchCoef = decayCoef (0.012f, fs);
        clickCoef = decayCoef (0.002f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase = 0.0f;
        amp = pitchEnv = clickEnv = 1.0f;
        freq = 62.0f * tuneRatio (s.tune, 7.0f);
        ampCoef = decayCoef (knobTime (s.decay, 0.05f, 0.4f), fs);
        click = s.tone * 0.8f;
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        advance (phase, freq * (1.0f + 1.6f * pitchEnv) / fs);
        const float x = std::sin (2.0f * pi * phase) * amp + click * clickEnv * noise.next();
        amp *= ampCoef;
        pitchEnv *= pitchCoef;
        clickEnv *= clickCoef;
        if (amp < 0.0001f) active = false;
        return fastTanh (x * 1.6f) * gain * 0.85f;
    }

private:
    float phase = 0, freq = 62, gain = 1, amp = 0, pitchEnv = 0, clickEnv = 0, click = 0.4f, ampCoef = 0, pitchCoef = 0, clickCoef = 0;
    bool active = false;
};

// TR-606 snare: two short tones under a lot of bright noise.
class Snare606 : public DrumVoice
{
public:
    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        const float ratio = tuneRatio (s.tune, 7.0f);
        f1 = 230.0f * ratio;
        f2 = 400.0f * ratio;
        tone = toneCoef = 1.0f;
        toneCoef = decayCoef (0.05f, fs);
        noiseEnv = 1.0f;
        noiseCoef = decayCoef (knobTime (s.decay, 0.05f, 0.3f), fs);
        noiseLevel = 0.5f + s.tone;
        highpass.set (2200.0f, 0.7f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        advance (p1, f1 / fs);
        advance (p2, f2 / fs);
        highpass.process (noise.next());
        const float x = (std::sin (2.0f * pi * p1) + 0.6f * std::sin (2.0f * pi * p2)) * tone * 0.5f + highpass.hp * noiseEnv * noiseLevel;
        tone *= toneCoef;
        noiseEnv *= noiseCoef;
        if (noiseEnv < 0.0001f && tone < 0.0001f) active = false;
        return fastTanh (x * 1.4f) * gain * 0.8f;
    }

private:
    float f1 = 230, f2 = 400, p1 = 0, p2 = 0, tone = 0, toneCoef = 0, noiseEnv = 0, noiseCoef = 0, noiseLevel = 1, gain = 1;
    bool active = false;
    Svf highpass;
};

// Organ rhythm box kick: a soft, round thump without click.
class SoftKick : public DrumVoice
{
public:
    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase = 0.0f;
        amp = pitchEnv = 1.0f;
        freq = 65.0f * tuneRatio (s.tune, 7.0f);
        ampCoef = decayCoef (knobTime (s.decay, 0.08f, 0.5f), fs);
        pitchCoef = decayCoef (0.03f, fs);
        lowpass.set (200.0f + s.tone * 600.0f, 0.7f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        advance (phase, freq * (1.0f + 0.5f * pitchEnv) / fs);
        lowpass.process (std::sin (2.0f * pi * phase) * amp);
        amp *= ampCoef;
        pitchEnv *= pitchCoef;
        if (amp < 0.0001f) active = false;
        return lowpass.lp * gain * 1.6f;
    }

private:
    float phase = 0, freq = 65, gain = 1, amp = 0, pitchEnv = 0, ampCoef = 0, pitchCoef = 0;
    bool active = false;
    Svf lowpass;
};

// Organ rhythm box snare: a band of noise, like a brush.
class BrushSnare : public DrumVoice
{
public:
    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        env = 1.0f;
        coef = decayCoef (knobTime (s.decay, 0.05f, 0.3f), fs);
        band.set ((1800.0f + s.tone * 3000.0f) * tuneRatio (s.tune, 7.0f), 1.2f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        band.process (noise.next());
        const float y = band.bp * env * gain * 1.4f;
        env *= coef;
        if (env < 0.0001f) active = false;
        return y;
    }

private:
    float gain = 1, env = 0, coef = 0;
    bool active = false;
    Svf band;
};

// Cheap digital drum machines: the same sound with fewer bits and a lower sample rate.
// TONE also sets how coarse the grain is.
class Bitcrushed : public DrumVoice
{
public:
    Bitcrushed (std::unique_ptr<DrumVoice> voice, float bitDepth, int holdSamples)
        : inner (std::move (voice)), baseBits (bitDepth), hold (holdSamples) {}

    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        inner->prepare (sampleRate);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        inner->trigger (velocity, s);
        levels = std::exp2 (baseBits * (0.75f + 0.5f * s.tone)) * 0.5f;
    }

    void choke() override { inner->choke(); }

    float process() override
    {
        const float x = inner->process();
        if (++counter >= hold)
        {
            counter = 0;
            held = std::round (x * levels) / levels;
        }
        return held;
    }

private:
    std::unique_ptr<DrumVoice> inner;
    float baseBits, levels = 128.0f, held = 0;
    int hold, counter = 0;
};

// Synthesized percussion: a sine modulated in frequency (and optionally in ring) with a pitch sweep and noise.
// It runs at twice the sample rate, and the FM never pushes its partials past what that rate can hold: no folded-back noise.
struct FmPercConfig
{
    float baseHz, sweepOctaves, sweepSeconds, ratio, index, indexSeconds, ringRatio, noise, highpassHz, minDecay, maxDecay, gain;
};

class FmPerc : public DrumVoice
{
public:
    explicit FmPerc (FmPercConfig c) : cfg (c) {}

    void prepare (float sampleRate) override { DrumVoice::prepare (sampleRate * 2.0f); }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase = modPhase = ringPhase = 0.0f;
        amp = pitchEnv = indexEnv = 1.0f;
        freq = cfg.baseHz * tuneRatio (s.tune, 12.0f);
        ampCoef = decayCoef (knobTime (s.decay, cfg.minDecay, cfg.maxDecay), fs);
        pitchCoef = decayCoef (cfg.sweepSeconds, fs);
        indexCoef = decayCoef (cfg.indexSeconds, fs);
        index = cfg.index * (0.3f + 1.4f * s.tone);
        if (cfg.highpassHz > 0.0f) highpass.set (cfg.highpassHz, 0.7f, fs);
        active = true;
    }

    float process() override
    {
        if (! active && ! ringing) return 0.0f;
        const float a = step();
        const float b = step();
        const float out = decimator.process (a, b);
        ringing = active || std::abs (out) > 1.0e-6f;
        return out;
    }

private:
    float step()
    {
        if (! active) return 0.0f;
        const float f = freq * std::exp2 (cfg.sweepOctaves * pitchEnv);
        const float modHz = std::min (f * cfg.ratio, fs * 0.45f);
        advance (modPhase, modHz / fs);
        // Carson: the partials reach about (index + 1) x the modulator above the carrier; they stay below 0.45 x the rate.
        const float maxIndex = std::max (0.0f, (fs * 0.45f - f) / std::max (1.0f, modHz) - 1.0f);
        const float mod = std::sin (2.0f * pi * modPhase) * std::min (index * indexEnv, maxIndex);
        advance (phase, std::min (f, fs * 0.45f) / fs);
        float x = std::sin (2.0f * pi * phase + mod);
        if (cfg.ringRatio > 0.0f)
        {
            advance (ringPhase, std::min (f * cfg.ringRatio, fs * 0.45f) / fs);
            x *= std::sin (2.0f * pi * ringPhase);
        }
        x += cfg.noise * noise.next() * indexEnv;
        if (cfg.highpassHz > 0.0f)
        {
            highpass.process (x);
            x = highpass.hp;
        }
        const float out = fastTanh (x * amp * 1.3f) * gain * cfg.gain;
        amp *= ampCoef;
        pitchEnv *= pitchCoef;
        indexEnv *= indexCoef;
        if (amp < 0.0001f) active = false;
        return out;
    }

    FmPercConfig cfg;
    Decimator2x decimator;
    bool ringing = false;
    float phase = 0, modPhase = 0, ringPhase = 0, freq = 100, gain = 1, amp = 0, pitchEnv = 0, indexEnv = 0, index = 1;
    float ampCoef = 0, pitchCoef = 0, indexCoef = 0;
    bool active = false;
    Svf highpass;
};
} // namespace techno
