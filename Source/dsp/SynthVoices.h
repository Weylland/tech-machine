#pragma once

#include "../core/MachineDefs.h"
#include "Filters.h"
#include <vector>

namespace techno
{
class MonoVoice
{
public:
    virtual ~MonoVoice() = default;
    virtual void prepare (float sampleRate) = 0;
    virtual void setParams (const float* values) = 0;    // ordered as in the machine definition
    virtual void noteOn (int midiNote, bool accent, bool glide) = 0;
    virtual void gateOff() = 0;
    virtual float process() = 0;
    virtual void reset() = 0;
};

// Note/glide/gate bookkeeping shared by the mono synths.
class MonoBase : public MonoVoice
{
public:
    void noteOn (int midiNote, bool accented, bool glide) override
    {
        targetNote = (float) midiNote;
        accent = accented;
        if (glide && gate)
        {
            gliding = true;
            onLegato();
            return;
        }
        gliding = false;
        currentNote = targetNote;
        gate = true;
        onTrigger();
    }

    void gateOff() override
    {
        gate = false;
        onRelease();
    }

protected:
    virtual void onTrigger() = 0;
    virtual void onLegato() {}
    virtual void onRelease() {}

    void updateGlide (float coef)
    {
        if (gliding) currentNote += (targetNote - currentNote) * coef;
    }

    float fs = 44100.0f;
    float currentNote = 45.0f, targetNote = 45.0f;
    bool gate = false, gliding = false, accent = false;
};

//==============================================================================
// TB-303: one oscillator, resonant 4-pole filter, decaying filter envelope,
// accent "sweep" circuit (accents pile up) and constant-time slide.
class Acid303 : public MonoBase
{
public:
    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        glideCoef = 1.0f - decayCoef (0.022f, fs);
        accentCoef = decayCoef (0.18f, fs);
        accentSmoothCoef = 1.0f - decayCoef (0.035f, fs);
        attackCoef = 1.0f - decayCoef (0.0015f, fs);
        releaseCoef = 1.0f - decayCoef (0.006f, fs);
        sustainCoef = decayCoef (1.6f, fs);
        dcBlock.set (25.0f, fs);
        reset();
    }

    void setParams (const float* v) override
    {
        square = v[p303::Wave] > 0.5f;
        cutoff = v[p303::Cutoff];
        resonance = v[p303::Reso];
        envMod = v[p303::EnvMod];
        decay = v[p303::Decay];
        accentAmount = v[p303::Accent];
    }

    void reset() override
    {
        ladder.reset();
        amp = filterEnv = accentEnv = accentSmooth = 0.0f;
        gate = gliding = false;
    }

    float process() override
    {
        updateGlide (glideCoef);
        const float dt = midiToHz (currentNote) / fs;
        advance (phase, dt);

        float osc;
        if (square)
            osc = 0.8f * blepPulse (phase, dt, 0.5f);
        else
            osc = blepSaw (phase, dt);

        filterEnv *= decayCoef (accentOn ? 0.2f : 0.18f + decay * 1.8f, fs);
        accentEnv *= accentCoef;
        accentSmooth += (accentEnv - accentSmooth) * accentSmoothCoef;

        const float octaves = envMod * filterEnv * 4.5f + accentAmount * accentSmooth * 2.2f;
        ladder.set (38.0f * std::exp2 (cutoff * 6.8f + octaves), resonance, fs);
        float y = ladder.process (osc * 0.55f);

        if (gate)
        {
            if (! attackDone)
            {
                amp += (1.0f - amp) * attackCoef;
                if (amp > 0.98f) attackDone = true;
            }
            else
            {
                amp *= sustainCoef;
            }
        }
        else
        {
            amp -= amp * releaseCoef;
        }

        y *= amp * (0.75f + 0.6f * (accentOn ? accentAmount : 0.0f) * accentEnv);
        return dcBlock.highpass (y) * 1.45f;
    }

private:
    void onTrigger() override
    {
        attackDone = false;
        filterEnv = 1.0f;
        accentOn = accent;
        if (accent) accentEnv = 1.0f;
    }

    float phase = 0, cutoff = 0.3f, resonance = 0.7f, envMod = 0.5f, decay = 0.4f, accentAmount = 0.6f;
    float filterEnv = 0, accentEnv = 0, accentSmooth = 0, amp = 0;
    float glideCoef = 0, accentCoef = 0, accentSmoothCoef = 0, attackCoef = 0, releaseCoef = 0, sustainCoef = 0;
    bool square = false, accentOn = false, attackDone = false;
    Ladder ladder;
    OnePole dcBlock;
};

//==============================================================================
// SH-101: saw + pulse (PWM) + sub square + noise into a self-oscillating 24 dB filter, ADSR.
class Mono101 : public MonoBase
{
public:
    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        dcBlock.set (20.0f, fs);
        reset();
    }

    void setParams (const float* v) override
    {
        saw = v[p101::Saw];
        square = v[p101::Square];
        sub = v[p101::Sub];
        noiseLevel = v[p101::Noise];
        pwm = v[p101::Pwm];
        cutoff = v[p101::Cutoff];
        reso = v[p101::Reso];
        envAmount = v[p101::Env];
        adsr.set (knobTime (v[p101::Attack], 0.001f, 2.0f), knobTime (v[p101::Decay], 0.005f, 3.0f),
                  v[p101::Sustain], knobTime (v[p101::Release], 0.005f, 3.0f), fs);
        glideCoef = 1.0f - decayCoef (knobTime (v[p101::Glide], 0.005f, 0.6f), fs);
    }

    void reset() override
    {
        ladder.reset();
        adsr.reset();
        gate = gliding = false;
    }

    float process() override
    {
        updateGlide (glideCoef);
        const float dt = midiToHz (currentNote) / fs;
        const float previous = phase;
        advance (phase, dt);
        if (phase < previous) subHigh = ! subHigh;

        advance (lfo, 0.9f / fs);
        const float tri = 1.0f - 4.0f * std::abs (lfo - 0.5f);
        const float width = 0.5f - 0.42f * pwm * (0.5f + 0.5f * tri);

        const float osc = saw * blepSaw (phase, dt) + square * blepPulse (phase, dt, width)
                        + sub * (subHigh ? 1.0f : -1.0f) + noiseLevel * noise.next();

        const float env = adsr.next();
        const float octaves = cutoff * 10.0f + envAmount * env * 5.0f + (currentNote - 48.0f) / 24.0f + (accent ? 0.6f : 0.0f);
        ladder.set (std::min (20.0f * std::exp2 (octaves), 18000.0f), reso * 1.04f, fs);

        const float y = ladder.process (osc * 0.4f) * env * (accent ? 1.25f : 1.0f);
        return dcBlock.highpass (y) * 1.8f;
    }

private:
    void onTrigger() override { adsr.gateOn(); }
    void onRelease() override { adsr.gateOff(); }

    float phase = 0, lfo = 0, saw = 0.8f, square = 0, sub = 0.5f, noiseLevel = 0, pwm = 0;
    float cutoff = 0.5f, reso = 0.3f, envAmount = 0.5f, glideCoef = 0.1f;
    bool subHigh = true;
    Adsr adsr;
    Ladder ladder;
    OnePole dcBlock;
    Noise noise;
};

//==============================================================================
// Model D: three oscillators (the third an octave down), overdriven ladder filter,
// filter contour and amp contour.
class ModelDVoice : public MonoBase
{
public:
    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        dcBlock.set (18.0f, fs);
        reset();
    }

    void setParams (const float* v) override
    {
        wave = (int) v[pMD::Wave];
        detuneCents = v[pMD::Detune] * v[pMD::Detune] * 45.0f;
        sub = v[pMD::Sub];
        cutoff = v[pMD::Cutoff];
        emphasis = v[pMD::Emphasis];
        contour = v[pMD::Contour];
        drive = v[pMD::Drive];
        const float a = knobTime (v[pMD::Attack], 0.001f, 2.0f), d = knobTime (v[pMD::Decay], 0.01f, 3.0f), s = v[pMD::Sustain];
        filterEnv.set (a, d, s, d * 0.5f, fs);
        ampEnv.set (a, d, 0.4f + 0.6f * s, 0.02f + d * 0.15f, fs);
        glideCoef = 1.0f - decayCoef (knobTime (v[pMD::Glide], 0.005f, 0.6f), fs);
    }

    void reset() override
    {
        ladder.reset();
        filterEnv.reset();
        ampEnv.reset();
        gate = gliding = false;
    }

    float process() override
    {
        updateGlide (glideCoef);
        const float f = midiToHz (currentNote);
        const float detune = std::exp2 (detuneCents / 1200.0f);
        const float dt1 = f / fs, dt2 = f * detune / fs, dt3 = 0.5f * f / (detune * detune) / fs;
        advance (phases[0], dt1);
        advance (phases[1], dt2);
        advance (phases[2], dt3);

        const float mix = (oscillator (phases[0], dt1) + oscillator (phases[1], dt2) + sub * oscillator (phases[2], dt3))
                        / (2.0f + sub);

        const float fe = filterEnv.next();
        const float octaves = cutoff * 9.5f + contour * fe * 4.5f + (currentNote - 36.0f) / 36.0f + (accent ? 0.5f : 0.0f);
        ladder.set (std::min (20.0f * std::exp2 (octaves), 18000.0f), emphasis, fs);

        const float y = ladder.process (mix * (0.5f + drive * 2.5f));
        return dcBlock.highpass (y) * ampEnv.next() * (accent ? 1.2f : 1.0f) * (2.7f - 0.9f * drive);
    }

private:
    float oscillator (float phase, float dt) const
    {
        if (wave == 1) return blepPulse (phase, dt, 0.5f);
        if (wave == 2) return 1.0f - 4.0f * std::abs (phase - 0.5f);
        return blepSaw (phase, dt);
    }

    void onTrigger() override
    {
        filterEnv.gateOn();
        ampEnv.gateOn();
    }

    void onRelease() override
    {
        filterEnv.gateOff();
        ampEnv.gateOff();
    }

    float phases[3] { 0.0f, 0.33f, 0.66f };
    float detuneCents = 5, sub = 0.4f, cutoff = 0.35f, emphasis = 0.25f, contour = 0.5f, drive = 0.3f, glideCoef = 0.1f;
    int wave = 0;
    Adsr filterEnv, ampEnv;
    Ladder ladder;
    OnePole dcBlock;
};

//==============================================================================
// Poly voices

class PolyVoice
{
public:
    virtual ~PolyVoice() = default;
    virtual void prepare (float sampleRate) = 0;
    virtual void start (int midiNote, float velocity) = 0;
    virtual void release() = 0;
    virtual bool isActive() const = 0;
    virtual void process (float& left, float& right) = 0;

    int note = -1;
    unsigned int age = 0;
};

struct JunoParams
{
    float saw = 0.7f, pulse = 0.4f, sub = 0.3f, noise = 0, pwm = 0.3f, cutoff = 0.5f, reso = 0.2f, env = 0.3f;
    int hpf = 1;
    float lfo = 0;    // shared LFO value, -1..1
};

// Juno-106 voice: DCO saw + pulse (PWM) + sub + noise -> HPF -> 24 dB VCF -> VCA (ADSR).
class JunoVoice : public PolyVoice
{
public:
    explicit JunoVoice (const JunoParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        bassShelf.set (100.0f, fs);
        ladder.reset();
        adsr.reset();
    }

    void setEnvelope (float a, float d, float s, float r) { adsr.set (a, d, s, r, fs); }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        adsr.gateOn();
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        const float dt = freq / fs;
        const float previous = phase;
        advance (phase, dt);
        if (phase < previous) subHigh = ! subHigh;

        const float width = 0.5f - 0.4f * p.pwm * (0.5f + 0.5f * p.lfo);
        float x = p.saw * blepSaw (phase, dt) + p.pulse * blepPulse (phase, dt, width)
                + p.sub * (subHigh ? 1.0f : -1.0f) + p.noise * noise.next();

        if (p.hpf == 0) x += 0.6f * bassShelf.lowpass (x);
        else if (p.hpf >= 2)
        {
            hpf.set (p.hpf == 2 ? 240.0f : 720.0f, fs);
            x = hpf.highpass (x);
        }

        const float e = adsr.next();
        const float octaves = p.cutoff * 10.0f + p.env * e * 4.5f + (freq > 0 ? std::log2 (freq / 261.6f) * 0.5f : 0.0f);
        ladder.set (std::min (20.0f * std::exp2 (octaves), 18000.0f), p.reso, fs);

        const float y = ladder.process (x * 0.35f) * e * gain;
        left += y;
        right += y;
    }

private:
    const JunoParams& p;
    float fs = 44100.0f, phase = 0, freq = 220, gain = 1;
    bool subHigh = true;
    Adsr adsr;
    Ladder ladder;
    OnePole bassShelf, hpf;
    Noise noise;
};

// Juno BBD chorus: a short modulated delay read with opposite modulation on each side.
class BbdChorus
{
public:
    void prepare (float sampleRate)
    {
        fs = sampleRate;
        size = (int) (fs * 0.02f) + 4;
        buffer.assign ((size_t) size, 0.0f);
        pos = 0;
        wetLp.set (9000.0f, fs);
    }

    void process (float input, int mode, float& left, float& right)
    {
        buffer[(size_t) pos] = input;

        if (mode == 0)
        {
            left = right = input;
        }
        else
        {
            static constexpr float rates[4] { 0.0f, 0.513f, 0.863f, 9.75f };
            static constexpr float depths[4] { 0.0f, 0.0018f, 0.0021f, 0.00035f };
            advance (lfo, rates[mode] / fs);
            const float tri = 1.0f - 4.0f * std::abs (lfo - 0.5f);
            const float centre = 0.0035f * fs, depth = depths[mode] * fs;

            const float wet1 = wetLp.lowpass (read (centre + depth * tri));
            const float wet2 = read (centre - depth * tri);
            left = 0.7f * (input + wet1);
            right = 0.7f * (input + wet2);
        }

        if (++pos >= size) pos = 0;
    }

private:
    float read (float delaySamples) const
    {
        float p = (float) pos - delaySamples;
        while (p < 0.0f) p += (float) size;
        const int i0 = (int) p;
        const int i1 = (i0 + 1) % size;
        const float frac = p - (float) i0;
        return buffer[(size_t) i0] + frac * (buffer[(size_t) i1] - buffer[(size_t) i0]);
    }

    std::vector<float> buffer;
    int size = 1, pos = 0;
    float fs = 44100.0f, lfo = 0;
    OnePole wetLp;
};

struct SuperParams
{
    float detune = 0.4f, mix = 0.6f, cutoff = 0.6f, reso = 0.15f, env = 0.3f;
};

// JP-8000 supersaw voice: 7 free-running detuned saws (Adam Szabo's measured detune and mix
// curves), a high-pass tracking the fundamental, then a 24 dB filter, spread in stereo.
class SuperVoice : public PolyVoice
{
public:
    explicit SuperVoice (const SuperParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        ladderL.reset();
        ladderR.reset();
        adsr.reset();
    }

    void setEnvelope (float a, float d, float s, float r) { adsr.set (a, d, s, r, fs); }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        for (auto& ph : phases) ph = random.nextFloat();
        highpassL.set (freq, fs);
        highpassR.set (freq, fs);
        adsr.gateOn();
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        static constexpr float offsets[7] { -0.11002313f, -0.06288439f, -0.01952356f, 0.0f, 0.01991221f, 0.06216538f, 0.10745242f };
        static constexpr float panLeft[7] { 1.0f, 0.2f, 0.8f, 0.5f, 0.3f, 0.9f, 0.1f };

        const float amount = detuneCurve (p.detune);
        const float centreGain = -0.55366f * p.mix + 0.99785f;
        const float sideGain = -0.73764f * p.mix * p.mix + 1.2841f * p.mix + 0.044372f;

        float l = 0.0f, r = 0.0f;
        for (int i = 0; i < 7; ++i)
        {
            const float dt = freq * (1.0f + amount * offsets[i]) / fs;
            advance (phases[i], dt);
            const float s = blepSaw (phases[i], dt) * (i == 3 ? centreGain : sideGain);
            l += s * panLeft[i];
            r += s * (1.0f - panLeft[i]);
        }

        l = highpassL.highpass (l);
        r = highpassR.highpass (r);

        const float e = adsr.next();
        const float octaves = p.cutoff * 10.0f + p.env * e * 4.5f + std::log2 (freq / 261.6f) * 0.5f;
        const float cutoffHz = std::min (20.0f * std::exp2 (octaves), 18000.0f);
        ladderL.set (cutoffHz, p.reso, fs);
        ladderR.set (cutoffHz, p.reso, fs);

        left += ladderL.process (l * 0.3f) * e * gain;
        right += ladderR.process (r * 0.3f) * e * gain;
    }

private:
    static float detuneCurve (float x)
    {
        return ((((((((((10028.7312891634f * x - 50818.8652045924f) * x + 111363.4808729368f) * x - 138150.6761080548f) * x
               + 106649.6679158292f) * x - 53046.9642751875f) * x + 17019.9518580080f) * x - 3425.0836591318f) * x
               + 404.2703938388f) * x - 24.1878824391f) * x + 0.6717417634f) * x + 0.0030115596f;
    }

    const SuperParams& p;
    float fs = 44100.0f, freq = 220, gain = 1;
    float phases[7] {};
    Adsr adsr;
    Ladder ladderL, ladderR;
    OnePole highpassL, highpassR;
    juce::Random random;
};
} // namespace techno
