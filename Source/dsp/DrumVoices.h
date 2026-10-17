#pragma once

#include "Filters.h"
#include <juce_core/juce_core.h>
#include <memory>

namespace techno
{
struct DrumSettings
{
    float tune = 0.5f, decay = 0.5f, tone = 0.5f;
};

class DrumVoice
{
public:
    virtual ~DrumVoice() = default;
    virtual void prepare (float sampleRate) { fs = sampleRate; }
    virtual void trigger (float velocity, const DrumSettings&) = 0;
    virtual float process() = 0;
    virtual void choke() {}

protected:
    float fs = 44100.0f;
    Noise noise;
};

//==============================================================================
// TR-909

// Triangle-core oscillator with a fast pitch sweep, waveshaped to a punchy near-sine, plus
// the attack click (pulse + filtered noise) set by the Attack knob of the original.
class Kick909 : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        clickHp.set (1500.0f, 0.7f, fs);
        clickCoef = decayCoef (0.0016f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase = 0.0f;
        amp = pitchEnv = clickEnv = 1.0f;
        base = 49.0f * tuneRatio (s.tune, 5.0f);
        sweep = 3.4f + s.tune * 1.6f;
        ampCoef = decayCoef (knobTime (s.decay, 0.06f, 0.75f), fs);
        pitchCoef = decayCoef (0.021f, fs);
        click = s.tone;
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        advance (phase, base * (1.0f + sweep * pitchEnv) / fs);
        const float body = std::sin (2.0f * pi * phase);

        clickHp.process (noise.next());
        const float pulse = clickEnv > 0.55f ? 1.0f : 0.0f;
        const float clickSig = (0.5f * clickHp.hp + 0.7f * pulse) * clickEnv * click;

        const float out = (fastTanh (1.7f * body * amp) * 1.15f + clickSig) * gain;

        amp *= ampCoef;
        pitchEnv *= pitchCoef;
        clickEnv *= clickCoef;
        if (amp < 0.0001f) active = false;
        return out;
    }

private:
    float phase = 0, gain = 1, amp = 0, pitchEnv = 0, clickEnv = 0, base = 50, sweep = 4, click = 0.5f;
    float ampCoef = 0, pitchCoef = 0, clickCoef = 0;
    bool active = false;
    Svf clickHp;
};

class Snare909 : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        noiseHp.set (900.0f, 0.7f, fs);
        toneCoef = decayCoef (0.045f, fs);
        pitchCoef = decayCoef (0.01f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase1 = phase2 = 0.0f;
        toneEnv = noiseEnv = pitchEnv = 1.0f;
        ratio = tuneRatio (s.tune, 5.0f);
        noiseCoef = decayCoef (knobTime (s.decay, 0.04f, 0.4f), fs);
        snappy = 0.25f + s.tone * 1.1f;
        noiseLp.set (5000.0f + s.tone * 6000.0f, 0.7f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        const float bend = 1.0f + 0.5f * pitchEnv;
        advance (phase1, 178.0f * ratio * bend / fs);
        advance (phase2, 330.0f * ratio * bend / fs);
        const float tone = (std::sin (2.0f * pi * phase1) + 0.55f * std::sin (2.0f * pi * phase2)) * toneEnv;

        noiseHp.process (noise.next());
        noiseLp.process (noiseHp.hp);

        const float out = fastTanh (0.6f * tone + snappy * noiseLp.lp * noiseEnv) * gain * 1.35f;

        toneEnv *= toneCoef;
        noiseEnv *= noiseCoef;
        pitchEnv *= pitchCoef;
        if (noiseEnv < 0.0001f && toneEnv < 0.0001f) active = false;
        return out;
    }

private:
    float phase1 = 0, phase2 = 0, gain = 1, ratio = 1, snappy = 0.8f;
    float toneEnv = 0, noiseEnv = 0, pitchEnv = 0, toneCoef = 0, noiseCoef = 0, pitchCoef = 0;
    bool active = false;
    Svf noiseHp, noiseLp;
};

// Sine tom with a strong downward pitch bend and some filtered noise (909 and 808 styles).
class Tom : public DrumVoice
{
public:
    Tom (float baseHz, float bendAmount, float noiseFloor) : fBase (baseHz), bend (bendAmount), noiseBase (noiseFloor) {}

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase = 0.0f;
        amp = pitchEnv = 1.0f;
        freq = fBase * tuneRatio (s.tune, 7.0f);
        ampCoef = decayCoef (knobTime (s.decay, 0.08f, 0.9f), fs);
        pitchCoef = decayCoef (0.055f, fs);
        noiseAmount = noiseBase + s.tone * 0.6f;
        noiseLp.set (freq * 5.0f, 0.7f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        advance (phase, freq * (1.0f + bend * pitchEnv) / fs);
        noiseLp.process (noise.next());
        const float out = (std::sin (2.0f * pi * phase) + noiseAmount * noiseLp.lp * pitchEnv) * amp * gain;

        amp *= ampCoef;
        pitchEnv *= pitchCoef;
        if (amp < 0.0001f) active = false;
        return fastTanh (out * 1.2f) * 0.7f;
    }

private:
    float fBase, bend, noiseBase;
    float phase = 0, gain = 1, amp = 0, pitchEnv = 0, freq = 100, noiseAmount = 0;
    float ampCoef = 0, pitchCoef = 0;
    bool active = false;
    Svf noiseLp;
};

// Two short resonant tones driven hard: the rimshot "tick".
class RimShot : public DrumVoice
{
public:
    RimShot (float f1, float f2) : freq1 (f1), freq2 (f2) {}

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase1 = phase2 = 0.0f;
        env = 1.0f;
        ratio = tuneRatio (s.tune, 5.0f);
        envCoef = decayCoef (knobTime (s.decay, 0.006f, 0.05f), fs);
        hp.set (200.0f + s.tone * 1200.0f, 0.7f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        advance (phase1, freq1 * ratio / fs);
        advance (phase2, freq2 * ratio / fs);
        const float raw = (std::sin (2.0f * pi * phase1) + 0.7f * std::sin (2.0f * pi * phase2)) * env;
        hp.process (fastTanh (raw * 3.0f));

        env *= envCoef;
        if (env < 0.0001f) active = false;
        return hp.hp * 0.8f * gain;
    }

private:
    float freq1, freq2, phase1 = 0, phase2 = 0, gain = 1, env = 0, envCoef = 0, ratio = 1;
    bool active = false;
    Svf hp;
};

// Band-passed noise with several quick bursts and a reverberant tail.
class HandClap : public DrumVoice
{
public:
    HandClap (int numBursts, float spacingSeconds, float centreHz) : bursts (numBursts), spacing (spacingSeconds), centre (centreHz) {}

    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        burstCoef = decayCoef (0.0045f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        t = 0.0f;
        burstEnv = 1.0f;
        tailEnv = 0.0f;
        burstsLeft = bursts - 1;
        nextBurst = spacing;
        tailCoef = decayCoef (knobTime (s.decay, 0.04f, 0.45f), fs);
        band.set (centre * tuneRatio (s.tune, 7.0f), 0.8f + s.tone * 1.8f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        t += 1.0f / fs;
        if (burstsLeft > 0 && t >= nextBurst)
        {
            burstEnv = 1.0f;
            nextBurst += spacing;
            if (--burstsLeft == 0) tailEnv = 0.85f;
        }

        band.process (noise.next());
        const float out = band.bp * (burstEnv + tailEnv) * 3.0f;

        burstEnv *= burstCoef;
        tailEnv *= tailCoef;
        if (burstsLeft == 0 && tailEnv < 0.0001f && burstEnv < 0.0001f) active = false;
        return fastTanh (out) * gain;
    }

private:
    int bursts;
    float spacing, centre;
    float gain = 1, t = 0, nextBurst = 0, burstEnv = 0, tailEnv = 0, burstCoef = 0, tailCoef = 0;
    int burstsLeft = 0;
    bool active = false;
    Svf band;
};

// Hats and cymbals: metallic square bank + noise, band-passed, optionally quantised
// (the 909 cymbals and hats were 6-bit samples played through an analog VCA).
struct MetalConfig
{
    const float* freqs;
    float noiseMix;
    float bits;            // quantisation levels, 0 = clean
    float bandHz, bandQ, highpassHz;
    float minDecay, maxDecay;
    float gain;
    float fastPart;        // share of a quick initial decay (crash/ride "attack")
};

class Metal : public DrumVoice
{
public:
    explicit Metal (MetalConfig c) : cfg (c) {}

    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        chokeCoef = decayCoef (0.004f, fs);
        fastCoef = decayCoef (0.03f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        env = 1.0f;
        fastEnv = cfg.fastPart;
        ratio = tuneRatio (s.tune, 5.0f);
        coef = decayCoef (knobTime (s.decay, cfg.minDecay, cfg.maxDecay), fs);
        const float bright = std::exp2 ((s.tone - 0.5f) * 1.4f);
        band.set (cfg.bandHz * bright, cfg.bandQ, fs);
        hp.set (cfg.highpassHz * bright, 0.7f, fs);
    }

    void choke() override { coef = chokeCoef; }

    float process() override
    {
        if (env < 0.0001f) return 0.0f;

        const float metal = cfg.freqs != nullptr ? metalBank.process (cfg.freqs, ratio, fs) : 0.0f;
        const float source = metal * (1.0f - cfg.noiseMix) + noise.next() * cfg.noiseMix;
        band.process (source);
        hp.process (band.bp);

        float sig = hp.hp * cfg.gain;
        if (cfg.bits > 0.0f) sig = quantize (std::clamp (sig, -1.0f, 1.0f), cfg.bits);

        const float out = sig * (env * (1.0f - cfg.fastPart) + fastEnv) * gain;
        env *= coef;
        fastEnv *= fastCoef;
        return out;
    }

private:
    MetalConfig cfg;
    float gain = 1, env = 0, fastEnv = 0, ratio = 1, coef = 0, chokeCoef = 0, fastCoef = 0;
    MetalBank metalBank;
    Svf band, hp;
};

//==============================================================================
// TR-808

// Bridged-T resonator: almost pure sine with a tiny pitch drop and a long, round decay.
class Kick808 : public DrumVoice
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
        freq = 49.0f * tuneRatio (s.tune, 7.0f);
        ampCoef = decayCoef (knobTime (s.decay, 0.1f, 1.8f), fs);
        clickLp.set (250.0f + s.tone * 3500.0f, 0.7f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        advance (phase, freq * (1.0f + 0.15f * pitchEnv) / fs);
        clickLp.process ((clickEnv > 0.4f ? 1.0f : 0.0f) * clickEnv);
        const float out = (std::sin (2.0f * pi * phase) * amp + 0.9f * clickLp.lp) * gain;

        amp *= ampCoef;
        pitchEnv *= pitchCoef;
        clickEnv *= clickCoef;
        if (amp < 0.0001f) active = false;
        return fastTanh (out * 1.25f);
    }

private:
    float phase = 0, gain = 1, amp = 0, pitchEnv = 0, clickEnv = 0, freq = 50;
    float ampCoef = 0, pitchCoef = 0, clickCoef = 0;
    bool active = false;
    Svf clickLp;
};

class Snare808 : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        noiseHp.set (1800.0f, 0.7f, fs);
        coef1 = decayCoef (0.05f, fs);
        coef2 = decayCoef (0.03f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase1 = phase2 = 0.0f;
        env1 = env2 = noiseEnv = 1.0f;
        ratio = tuneRatio (s.tune, 5.0f);
        noiseCoef = decayCoef (knobTime (s.decay, 0.04f, 0.3f), fs);
        snappy = s.tone * 1.3f;
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        advance (phase1, 238.0f * ratio / fs);
        advance (phase2, 476.0f * ratio / fs);
        const float tone = 0.8f * std::sin (2.0f * pi * phase1) * env1 + 0.5f * std::sin (2.0f * pi * phase2) * env2;

        noiseHp.process (noise.next());
        const float out = fastTanh (tone + snappy * noiseHp.hp * noiseEnv) * gain * 1.3f;

        env1 *= coef1;
        env2 *= coef2;
        noiseEnv *= noiseCoef;
        if (env1 < 0.0001f && noiseEnv < 0.0001f) active = false;
        return out;
    }

private:
    float phase1 = 0, phase2 = 0, gain = 1, ratio = 1, snappy = 0.6f;
    float env1 = 0, env2 = 0, noiseEnv = 0, coef1 = 0, coef2 = 0, noiseCoef = 0;
    bool active = false;
    Svf noiseHp;
};

// Two square oscillators (540 / 800 Hz) through a band-pass with a two-stage decay.
class Cowbell808 : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        fastCoef = decayCoef (0.012f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        fastEnv = slowEnv = 1.0f;
        ratio = tuneRatio (s.tune, 5.0f);
        slowCoef = decayCoef (knobTime (s.decay, 0.05f, 0.5f), fs);
        band.set (1200.0f + s.tone * 1800.0f, 1.1f, fs);
    }

    float process() override
    {
        if (slowEnv < 0.0001f) return 0.0f;

        advance (phase1, 540.0f * ratio / fs);
        advance (phase2, 800.0f * ratio / fs);
        const float sq = (phase1 < 0.5f ? 1.0f : -1.0f) + (phase2 < 0.5f ? 1.0f : -1.0f);
        band.process (sq * 0.5f);

        const float out = (band.bp * 1.6f + sq * 0.12f) * (0.6f * fastEnv + 0.4f * slowEnv) * gain;
        fastEnv *= fastCoef;
        slowEnv *= slowCoef;
        return out;
    }

private:
    float phase1 = 0, phase2 = 0.3f, gain = 1, ratio = 1, fastEnv = 0, slowEnv = 0, fastCoef = 0, slowCoef = 0;
    Svf band;
};

// Metal bank split into a low and a high band with different decays.
class Cymbal808 : public DrumVoice
{
public:
    explicit Cymbal808 (const float* metalFreqs) : freqs (metalFreqs) {}

    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        lowBand.set (3440.0f, 1.2f, fs);
        highBand.set (7100.0f, 1.2f, fs);
        hp.set (3000.0f, 0.7f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        lowEnv = highEnv = 1.0f;
        ratio = tuneRatio (s.tune, 5.0f);
        const float len = knobTime (s.decay, 0.25f, 2.2f);
        lowCoef = decayCoef (len, fs);
        highCoef = decayCoef (len * 0.5f, fs);
        tone = s.tone;
    }

    float process() override
    {
        if (lowEnv < 0.0001f) return 0.0f;

        const float source = metal.process (freqs, ratio, fs);
        lowBand.process (source);
        highBand.process (source);
        hp.process (lowBand.bp * lowEnv * (1.0f - 0.7f * tone) + highBand.bp * highEnv * (0.5f + tone));

        lowEnv *= lowCoef;
        highEnv *= highCoef;
        return hp.hp * 1.6f * gain;
    }

private:
    const float* freqs;
    float gain = 1, ratio = 1, tone = 0.5f, lowEnv = 0, highEnv = 0, lowCoef = 0, highCoef = 0;
    MetalBank metal;
    Svf lowBand, highBand, hp;
};

// Resonator "toc" (claves) — also used for short ringing percussion.
class Claves : public DrumVoice
{
public:
    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase = 0.0f;
        env = 1.0f;
        freq = 2500.0f * tuneRatio (s.tune, 7.0f);
        coef = decayCoef (knobTime (s.decay, 0.008f, 0.06f), fs);
        harmonic = s.tone * 0.5f;
    }

    float process() override
    {
        if (env < 0.0001f) return 0.0f;
        advance (phase, freq / fs);
        const float out = (std::sin (2.0f * pi * phase) + harmonic * std::sin (4.0f * pi * phase)) * env * gain;
        env *= coef;
        return out * 1.1f;
    }

private:
    float phase = 0, gain = 1, env = 0, freq = 2500, coef = 0, harmonic = 0;
};

// High-passed noise with a short attack (maracas / shaker).
class Maracas : public DrumVoice
{
public:
    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        env = 0.0f;
        attackDone = false;
        attackCoef = 1.0f - decayCoef (0.002f, fs);
        coef = decayCoef (knobTime (s.decay, 0.015f, 0.12f), fs);
        hp.set (3000.0f + s.tone * 6000.0f, 0.7f, fs);
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        if (! attackDone)
        {
            env += (1.2f - env) * attackCoef;
            if (env >= 1.0f) attackDone = true;
        }
        else
        {
            env *= coef;
            if (env < 0.0001f) active = false;
        }

        hp.process (noise.next());
        return hp.hp * env * gain * 1.3f;
    }

private:
    float gain = 1, env = 0, attackCoef = 0, coef = 0;
    bool attackDone = false, active = false;
    Svf hp;
};

//==============================================================================
// Break kit (acoustic kit with sampler character)

class AcousticKick : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        beaterLp.set (4000.0f, 0.7f, fs);
        pitchCoef = decayCoef (0.012f, fs);
        beaterCoef = decayCoef (0.004f, fs);
        modeCoef = decayCoef (0.03f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        phase = modePhase = 0.0f;
        amp = pitchEnv = beaterEnv = modeEnv = 1.0f;
        freq = 58.0f * tuneRatio (s.tune, 7.0f);
        ampCoef = decayCoef (knobTime (s.decay, 0.08f, 0.5f), fs);
        beater = 0.3f + s.tone * 1.2f;
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        advance (phase, freq * (1.0f + 1.8f * pitchEnv) / fs);
        advance (modePhase, freq * 2.3f / fs);
        beaterLp.process (noise.next());

        const float out = std::sin (2.0f * pi * phase) * amp
                        + 0.3f * std::sin (2.0f * pi * modePhase) * modeEnv
                        + beater * beaterLp.lp * beaterEnv;

        amp *= ampCoef;
        pitchEnv *= pitchCoef;
        beaterEnv *= beaterCoef;
        modeEnv *= modeCoef;
        if (amp < 0.0001f) active = false;
        return fastTanh (out * 1.3f) * gain;
    }

private:
    float phase = 0, modePhase = 0, gain = 1, freq = 58, beater = 0.8f;
    float amp = 0, pitchEnv = 0, beaterEnv = 0, modeEnv = 0, ampCoef = 0, pitchCoef = 0, beaterCoef = 0, modeCoef = 0;
    bool active = false;
    Svf beaterLp;
};

// Shell modes + wide snare-wire noise, slightly compressed like a sampled break.
class AcousticSnare : public DrumVoice
{
public:
    void prepare (float sampleRate) override
    {
        DrumVoice::prepare (sampleRate);
        wires.set (3200.0f, 0.5f, fs);
        wiresHp.set (450.0f, 0.7f, fs);
        modeCoef[0] = decayCoef (0.05f, fs);
        modeCoef[1] = decayCoef (0.04f, fs);
        modeCoef[2] = decayCoef (0.03f, fs);
    }

    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        for (int i = 0; i < 3; ++i) { phases[i] = 0.0f; modeEnv[i] = 1.0f; }
        noiseEnv = 1.0f;
        ratio = tuneRatio (s.tune, 5.0f);
        noiseCoef = decayCoef (knobTime (s.decay, 0.05f, 0.4f), fs);
        snappy = 0.4f + s.tone * 1.2f;
        active = true;
    }

    float process() override
    {
        if (! active) return 0.0f;

        static constexpr float freqs[3] { 190.0f, 300.0f, 420.0f };
        static constexpr float amps[3] { 0.7f, 0.4f, 0.25f };
        float shell = 0.0f;
        for (int i = 0; i < 3; ++i)
        {
            advance (phases[i], freqs[i] * ratio / fs);
            shell += amps[i] * std::sin (2.0f * pi * phases[i]) * modeEnv[i];
            modeEnv[i] *= modeCoef[i];
        }

        wiresHp.process (noise.next());
        wires.process (wiresHp.hp);
        const float wire = (wires.bp * 1.5f + wiresHp.hp * 0.5f) * noiseEnv * snappy;

        noiseEnv *= noiseCoef;
        if (noiseEnv < 0.0001f && modeEnv[0] < 0.0001f) active = false;
        return fastTanh ((shell + wire) * 1.5f) * gain;
    }

private:
    float phases[3] {}, modeEnv[3] {}, modeCoef[3] {};
    float gain = 1, ratio = 1, snappy = 1, noiseEnv = 0, noiseCoef = 0;
    bool active = false;
    Svf wires, wiresHp;
};

//==============================================================================

namespace metalFreqs
{
inline constexpr float tr808[6] { 205.3f, 304.4f, 369.6f, 522.7f, 540.0f, 800.0f };
inline constexpr float tr909[6] { 349.0f, 517.5f, 628.3f, 888.6f, 918.0f, 1360.0f };
inline constexpr float ride[6] { 544.0f, 806.7f, 979.4f, 1385.0f, 1431.0f, 2120.0f };
inline constexpr float crash[6] { 287.4f, 426.2f, 517.4f, 731.8f, 756.0f, 1120.0f };
inline constexpr float acoustic[6] { 435.2f, 645.3f, 783.6f, 1108.0f, 1144.8f, 1696.0f };
} // namespace metalFreqs

// Builds the voice for a row of a drum machine (identified by machine id and row id).
std::unique_ptr<DrumVoice> createDrumVoice (const juce::String& machineId, const juce::String& rowId);
} // namespace techno
