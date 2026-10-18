#pragma once

#include "DrumVoices.h"

namespace techno
{
// Skin drum (conga, bongo, timbale, acoustic tom): a tone that bends a little after the hit,
// an inharmonic overtone and the "slap" of the hand.
class Membrane : public DrumVoice
{
public:
    Membrane (float baseHz, float bendAmount, float overtoneRatio, float overtoneLevel, float minDecaySeconds, float maxDecaySeconds)
        : base (baseHz), bend (bendAmount), ratio (overtoneRatio), overtone (overtoneLevel), minDecay (minDecaySeconds), maxDecay (maxDecaySeconds)
    {
    }

    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        pitchCoef = decayCoef (0.03f, fs);
        slapCoef = decayCoef (0.004f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase = overtonePhase = 0.0f;
        amp = overtoneAmp = pitchEnv = slapEnv = 1.0f;
        freq = base * tuneRatio (s.tune, 7.0f);
        const float len = knobTime (s.decay, minDecay, maxDecay);
        ampCoef = decayCoef (len, fs);
        overtoneCoef = decayCoef (len * 0.35f, fs);
        slap = 0.15f + s.tone * 1.1f;
        slapFilter.set (1500.0f + s.tone * 3000.0f, 1.2f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        const float f = freq * (1.0f + bend * pitchEnv);
        advance (phase, f / fs);
        advance (overtonePhase, f * ratio / fs);
        slapFilter.process (noise.next());

        const float body = std::sin (2.0f * pi * phase) * amp + overtone * std::sin (2.0f * pi * overtonePhase) * overtoneAmp;
        const float out = fastTanh ((body + slap * slapFilter.bp * slapEnv * 2.0f) * 1.2f) * gain;

        amp *= ampCoef;
        overtoneAmp *= overtoneCoef;
        pitchEnv *= pitchCoef;
        slapEnv *= slapCoef;
        if (amp < 0.0001f) active = false;
        return out;
    }

private:
    float base, bend, ratio, overtone, minDecay, maxDecay;
    float phase = 0, overtonePhase = 0, freq = 200, gain = 1, slap = 0.5f;
    float amp = 0, overtoneAmp = 0, pitchEnv = 0, slapEnv = 0;
    float ampCoef = 0, overtoneCoef = 0, pitchCoef = 0, slapCoef = 0;
    bool active = false;
    Svf slapFilter;
};

// Jingles: high metallic partials and noise, shaken twice.
class Tambourine : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        band.set (8500.0f, 0.7f, fs);
        hp.set (5000.0f, 0.7f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        env = 1.0f;
        t = 0.0f;
        secondShake = false;
        ratio = tuneRatio (s.tune, 5.0f);
        coef = decayCoef (knobTime (s.decay, 0.05f, 0.4f), fs);
        shakeGap = 0.03f + (1.0f - s.tone) * 0.03f;
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        static constexpr float freqs[6] { 3120.0f, 4410.0f, 5280.0f, 6750.0f, 7930.0f, 9100.0f };

        t += 1.0f / fs;
        if (! secondShake && t >= shakeGap)
        {
            secondShake = true;
            env = std::max (env, 0.7f);
        }

        const float source = metal.process (freqs, ratio, fs) * 0.5f + noise.next() * 0.5f;
        band.process (source);
        hp.process (band.bp);
        const float out = hp.hp * env * gain * 3.0f;
        env *= coef;
        if (secondShake && env < 0.0001f) active = false;
        return out;
    }

private:
    float gain = 1, env = 0, t = 0, ratio = 1, coef = 0, shakeGap = 0.04f;
    bool secondShake = false, active = false;
    MetalBank metal;
    Svf band, hp;
};

// Guiro: a fast train of scraping clicks.
class Guiro : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        band.set (3500.0f, 1.5f, fs);
        clickCoef = decayCoef (0.0025f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        clicks = 6 + (int) (s.decay * 10.0f);
        spacing = 0.009f / tuneRatio (s.tune, 7.0f);
        band.set (2500.0f + s.tone * 3000.0f, 1.5f, fs);
        countdown = 0.0f;
        clickEnv = 0.0f;
        index = 0;
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        countdown -= 1.0f / fs;
        if (countdown <= 0.0f && index < clicks)
        {
            const float position = (float) index / (float) clicks;
            clickEnv = 0.5f + 0.5f * std::sin (pi * position);
            countdown += spacing;
            ++index;
        }
        band.process (noise.next());
        const float out = band.bp * clickEnv * gain * 3.5f;
        clickEnv *= clickCoef;
        if (index >= clicks && clickEnv < 0.0001f) active = false;
        return out;
    }

private:
    float gain = 1, spacing = 0.01f, countdown = 0, clickEnv = 0, clickCoef = 0;
    int clicks = 8, index = 0;
    bool active = false;
    Svf band;
};

// Triangle: a few inharmonic partials ringing for a long time.
class Triangle : public DrumVoice
{
public:
    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        for (auto& p : phases) p = 0.0f;
        env = 1.0f;
        freq = 2900.0f * tuneRatio (s.tune, 5.0f);
        coef = decayCoef (knobTime (s.decay, 0.25f, 3.0f), fs);
        brightness = 0.3f + s.tone;
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        static constexpr float ratios[4] { 1.0f, 2.76f, 5.4f, 8.93f };
        static constexpr float amps[4] { 1.0f, 0.5f, 0.3f, 0.15f };
        float sum = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
            advance (phases[i], freq * ratios[i] / fs);
            sum += std::sin (2.0f * pi * phases[i]) * amps[i] * (i == 0 ? 1.0f : brightness);
        }
        const float out = sum * env * gain * 0.4f;
        env *= coef;
        if (env < 0.0001f) active = false;
        return out;
    }

private:
    float phases[4] {};
    float gain = 1, env = 0, freq = 3000, coef = 0, brightness = 0.8f;
    bool active = false;
};

// Agogo / bell: a small FM bell with a bright attack.
class Bell : public DrumVoice
{
public:
    explicit Bell (float baseHz) : base (baseHz) {}

    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        indexCoef = decayCoef (0.06f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        carrier = modulator = 0.0f;
        env = 1.0f;
        index = 2.0f + s.tone * 3.0f;
        freq = base * tuneRatio (s.tune, 7.0f);
        coef = decayCoef (knobTime (s.decay, 0.06f, 0.7f), fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        advance (modulator, freq * 1.41f / fs);
        const float mod = std::sin (2.0f * pi * modulator) * index;
        advance (carrier, freq / fs);
        const float out = std::sin (2.0f * pi * carrier + mod) * env * gain * 0.7f;
        index = 0.5f + (index - 0.5f) * indexCoef;
        env *= coef;
        if (env < 0.0001f) active = false;
        return out;
    }

private:
    float base, carrier = 0, modulator = 0, gain = 1, env = 0, index = 2, freq = 900, coef = 0, indexCoef = 0;
    bool active = false;
};

// Finger snap: a very short band of noise.
class Snap : public DrumVoice
{
public:
    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        env = 1.0f;
        coef = decayCoef (knobTime (s.decay, 0.008f, 0.06f), fs);
        band.set (1800.0f * tuneRatio (s.tune, 7.0f), 1.0f + s.tone * 2.0f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        band.process (noise.next());
        const float out = fastTanh (band.bp * env * 5.0f) * gain;
        env *= coef;
        if (env < 0.0001f) active = false;
        return out * 0.9f;
    }

private:
    float gain = 1, env = 0, coef = 0;
    bool active = false;
    Svf band;
};

// Boom bap kick: sine thump, slow pitch drop, saturated and kept in 12 bits like an MPC sample.
class SampledKick : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        pitchCoef = decayCoef (0.04f, fs);
        clickCoef = decayCoef (0.003f, fs);
        clickLp.set (3000.0f, 0.7f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase = 0.0f;
        amp = pitchEnv = clickEnv = 1.0f;
        freq = 52.0f * tuneRatio (s.tune, 7.0f);
        ampCoef = decayCoef (knobTime (s.decay, 0.12f, 0.8f), fs);
        click = 0.2f + s.tone;
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        advance (phase, freq * (1.0f + 1.2f * pitchEnv) / fs);
        clickLp.process (noise.next());
        const float x = std::sin (2.0f * pi * phase) * amp + click * clickLp.lp * clickEnv;
        const float out = quantize (fastTanh (x * 2.0f), 2048.0f) * gain;
        amp *= ampCoef;
        pitchEnv *= pitchCoef;
        clickEnv *= clickCoef;
        if (amp < 0.0001f) active = false;
        return out;
    }

private:
    float phase = 0, freq = 52, gain = 1, click = 0.6f, amp = 0, pitchEnv = 0, clickEnv = 0;
    float ampCoef = 0, pitchCoef = 0, clickCoef = 0;
    bool active = false;
    Svf clickLp;
};

// Boom bap snare: body + crack + a little room, 12 bits.
class SampledSnare : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        crack.set (2600.0f, 0.6f, fs);
        room.set (1800.0f, 0.7f, fs);
        bodyCoef = decayCoef (0.06f, fs);
        roomCoef = decayCoef (0.22f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase1 = phase2 = 0.0f;
        body = noiseEnv = roomEnv = 1.0f;
        ratio = tuneRatio (s.tune, 5.0f);
        noiseCoef = decayCoef (knobTime (s.decay, 0.06f, 0.35f), fs);
        snappy = 0.5f + s.tone;
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        advance (phase1, 185.0f * ratio / fs);
        advance (phase2, 315.0f * ratio / fs);
        const float n = noise.next();
        crack.process (n);
        room.process (n);
        const float x = (std::sin (2.0f * pi * phase1) + 0.5f * std::sin (2.0f * pi * phase2)) * body * 0.7f
                      + (crack.bp * 1.8f + crack.hp * 0.3f) * noiseEnv * snappy + room.lp * roomEnv * 0.25f;
        const float out = quantize (fastTanh (x * 1.6f), 2048.0f) * gain;
        body *= bodyCoef;
        noiseEnv *= noiseCoef;
        roomEnv *= roomCoef;
        if (roomEnv < 0.0001f && noiseEnv < 0.0001f) active = false;
        return out;
    }

private:
    float phase1 = 0, phase2 = 0, gain = 1, ratio = 1, snappy = 1, body = 0, noiseEnv = 0, roomEnv = 0;
    float bodyCoef = 0, noiseCoef = 0, roomCoef = 0;
    bool active = false;
    Svf crack, room;
};
// Simmons SDS-V: a triangle-like tone with a long pitch sweep down, mixed with filtered noise ("pew").
class SimmonsDrum : public DrumVoice
{
public:
    SimmonsDrum (float baseHz, float bendAmount, float sweepSeconds, float noiseLevel, float minDecaySeconds, float maxDecaySeconds)
        : base (baseHz), bend (bendAmount), sweep (sweepSeconds), noiseMix (noiseLevel), minDecay (minDecaySeconds), maxDecay (maxDecaySeconds)
    {
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase = 0.0f;
        amp = pitchEnv = 1.0f;
        freq = base * tuneRatio (s.tune, 7.0f);
        ampCoef = decayCoef (knobTime (s.decay, minDecay, maxDecay), fs);
        pitchCoef = decayCoef (sweep, fs);
        noiseFilter.set (800.0f + s.tone * 6000.0f, 0.8f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;
        const float f = freq * (1.0f + bend * pitchEnv);
        advance (phase, f / fs);
        const float tri = 1.0f - 4.0f * std::abs (phase - 0.5f);
        noiseFilter.process (noise.next());
        const float out = fastTanh ((tri * (1.0f - noiseMix) + noiseFilter.bp * noiseMix * 2.5f) * amp * 1.4f) * gain;
        amp *= ampCoef;
        pitchEnv *= pitchCoef;
        if (amp < 0.0001f) active = false;
        return out;
    }

private:
    float base, bend, sweep, noiseMix, minDecay, maxDecay;
    float phase = 0, freq = 100, gain = 1, amp = 0, pitchEnv = 0, ampCoef = 0, pitchCoef = 0;
    bool active = false;
    Svf noiseFilter;
};
} // namespace techno
