#pragma once

#include "Reverb.h"
#include "SynthVoices.h"
#include <array>

namespace techno
{
//==============================================================================
// DX7-style FM: two carrier/modulator pairs with their own modulation-index envelopes.

struct FmPreset
{
    float modRatio1, index1, indexDecay1;   // pair 1
    float carRatio2, modRatio2, index2, indexDecay2, mix2;   // pair 2
    float attack, decay, sustain;           // amplitude envelope (seconds, level)
    bool indexRises;                        // brass: the brightness grows with the attack
};

inline const FmPreset& fmPreset (int index)
{
    static const FmPreset presets[] {
        { 1.0f, 1.8f, 0.7f, 1.0f, 14.0f, 1.2f, 0.03f, 0.35f, 0.002f, 1.6f, 0.0f, false },   // electric piano
        { 1.0f, 3.5f, 0.12f, 0.5f, 1.0f, 1.0f, 0.2f, 0.5f, 0.002f, 0.5f, 0.4f, false },     // FM bass
        { 3.5f, 4.0f, 1.5f, 2.0f, 5.19f, 2.0f, 0.8f, 0.4f, 0.001f, 3.0f, 0.0f, false },     // bell
        { 1.0f, 3.0f, 0.12f, 1.0f, 2.0f, 0.6f, 0.2f, 0.3f, 0.05f, 0.6f, 0.75f, true },      // brass
        { 4.0f, 2.5f, 0.05f, 1.0f, 10.0f, 0.8f, 0.02f, 0.3f, 0.001f, 0.4f, 0.0f, false },   // marimba
        { 2.0f, 1.0f, 2.0f, 1.0f, 3.0f, 0.5f, 3.0f, 0.5f, 0.6f, 2.0f, 0.9f, true },         // FM pad
    };
    return presets[(size_t) juce::jlimit (0, 5, index)];
}

struct FmParams
{
    int preset = 0;
    float brightness = 0.5f, decay = 0.5f, release = 0.3f, detune = 0.2f, tremolo = 0.2f;
    float lfo = 0.0f;
};

class FmVoice : public PolyVoice
{
public:
    explicit FmVoice (const FmParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        const auto& preset = fmPreset (p.preset);
        const float decayScale = 0.3f * std::pow (10.0f, p.decay);
        adsr.set (preset.attack, preset.decay * decayScale, preset.sustain, knobTime (p.release, 0.01f, 3.0f), fs);
        adsr.gateOn();
        index1 = preset.indexRises ? 0.0f : 1.0f;
        index2 = preset.indexRises ? 0.0f : 1.0f;
        coef1 = decayCoef (preset.indexDecay1 * decayScale, fs);
        coef2 = decayCoef (preset.indexDecay2 * decayScale, fs);
        riseCoef = 1.0f - decayCoef (juce::jmax (0.02f, preset.attack * 1.5f), fs);
        rises = preset.indexRises;
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        const auto& preset = fmPreset (p.preset);
        const float bright = 0.25f + p.brightness * 1.75f;
        const float spread = 1.0f + p.detune * 0.004f;

        if (rises)
        {
            index1 += (1.0f - index1) * riseCoef;
            index2 += (1.0f - index2) * riseCoef;
        }
        else
        {
            index1 *= coef1;
            index2 *= coef2;
        }

        advance (m1, freq * preset.modRatio1 / fs);
        advance (c1, freq / fs);
        advance (m2, freq * preset.modRatio2 * spread / fs);
        advance (c2, freq * preset.carRatio2 * spread / fs);

        const float twoPi = 2.0f * pi;
        const float pair1 = std::sin (twoPi * c1 + std::sin (twoPi * m1) * preset.index1 * bright * (0.15f + 0.85f * index1));
        const float pair2 = std::sin (twoPi * c2 + std::sin (twoPi * m2) * preset.index2 * bright * index2) * preset.mix2;

        const float trem = 1.0f - p.tremolo * 0.35f * (0.5f + 0.5f * p.lfo);
        const float e = adsr.next() * gain * trem * 0.45f;
        left += (pair1 + pair2 * 0.6f) * e;
        right += (pair1 * 0.6f + pair2) * e;
    }

private:
    const FmParams& p;
    float fs = 44100.0f, freq = 220, gain = 1;
    float c1 = 0, m1 = 0, c2 = 0, m2 = 0, index1 = 1, index2 = 1, coef1 = 0, coef2 = 0, riseCoef = 0;
    bool rises = false;
    Adsr adsr;
};

//==============================================================================
// String machine: 8' and 4' saws (violins) plus a 16' saw (cellos), gentle filter, slow envelope.

struct StringParams
{
    float violin = 0.8f, cello = 0.5f, brightness = 0.55f;
};

class StringVoice : public PolyVoice
{
public:
    explicit StringVoice (const StringParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
        filter.reset();
    }

    void setEnvelope (float a, float r) { adsr.set (a, 0.5f, 1.0f, r, fs); }

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
        const float dt8 = freq / fs, dt4 = 2.0f * freq / fs, dt16 = 0.5f * freq / fs;
        advance (phase8, dt8);
        advance (phase4, dt4);
        advance (phase16, dt16);

        const float violins = blepSaw (phase8, dt8) * 0.6f + blepSaw (phase4, dt4) * 0.45f;
        const float x = p.violin * violins + p.cello * blepSaw (phase16, dt16) * 0.7f;

        filter.set (juce::jmin (16000.0f, (600.0f + p.brightness * 7000.0f) * (1.0f + freq / 2000.0f)), 0.6f, fs);
        filter.process (x);
        const float y = filter.lp * adsr.next() * gain * 0.3f;
        left += y;
        right += y;
    }

private:
    const StringParams& p;
    float fs = 44100.0f, freq = 220, gain = 1, phase8 = 0, phase4 = 0.31f, phase16 = 0.67f;
    Adsr adsr;
    Svf filter;
};

// Solina-style ensemble: three delay taps swept by a slow LFO 120 degrees apart, plus a fast vibrato.
class EnsembleChorus
{
public:
    void prepare (float sampleRate)
    {
        fs = sampleRate;
        line.allocate ((int) (fs * 0.03f));
    }

    void process (float input, float amount, float& left, float& right)
    {
        line.write (input);
        advance (slow, 0.62f / fs);
        advance (fast, 6.1f / fs);

        float taps[3];
        for (int i = 0; i < 3; ++i)
        {
            const float phase = slow + (float) i / 3.0f;
            const float lfo = std::sin (2.0f * pi * phase) * 0.0016f + std::sin (2.0f * pi * (fast + (float) i / 3.0f)) * 0.0002f;
            taps[i] = line.read (fs * (0.0075f + lfo));
        }
        left = input * (1.0f - amount * 0.5f) + amount * (taps[0] + 0.5f * taps[1]) * 0.8f;
        right = input * (1.0f - amount * 0.5f) + amount * (taps[2] + 0.5f * taps[1]) * 0.8f;
    }

private:
    float fs = 44100.0f, slow = 0, fast = 0;
    DelayLine line;
};

//==============================================================================
// Tonewheel / house organ: the drawbar mix is baked into a single-cycle table shared by all voices.

struct OrganParams
{
    std::array<float, 2048> table {};
    bool house = false;
    float percussion = 0.3f, click = 0.3f;
};

inline void buildOrganTable (OrganParams& p, float bass, float middle, float high, bool house)
{
    // Harmonics of the drawbars: 16', 8', 5 1/3', 4', 2 2/3', 2', 1 3/5', 1 1/3', 1'.
    static constexpr float ratios[9] { 0.5f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f };
    float levels[9] { bass * 0.8f, bass, middle * 0.5f, middle, middle * 0.7f, high, high * 0.6f, high * 0.4f, high * 0.5f };
    if (house)
    {
        // The M1 organ: strong 8', 4' and 2 2/3', little 16'.
        levels[0] *= 0.2f;
        levels[1] = 1.0f;
        levels[3] = juce::jmax (levels[3], 0.7f);
        levels[4] = juce::jmax (levels[4], 0.6f);
    }

    float peak = 0.0f;
    for (size_t i = 0; i < p.table.size(); ++i)
    {
        const float x = (float) i / (float) p.table.size();
        float v = 0.0f;
        for (int h = 0; h < 9; ++h)
            v += levels[h] * std::sin (2.0f * pi * ratios[h] * 2.0f * x);   // the table is one cycle of the 16'
        p.table[i] = v;
        peak = juce::jmax (peak, std::abs (v));
    }
    if (peak > 0.0f)
        for (auto& v : p.table) v /= peak;
    p.house = house;
}

class OrganVoice : public PolyVoice
{
public:
    explicit OrganVoice (const OrganParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
        clickFilter.set (3000.0f, 0.7f, fs);
        percCoef = decayCoef (0.22f, fs);
        clickCoef = decayCoef (0.002f, fs);
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        if (p.house) adsr.set (0.002f, 0.35f, 0.45f, 0.08f, fs);
        else adsr.set (0.003f, 0.1f, 1.0f, 0.03f, fs);
        adsr.gateOn();
        percEnv = 1.0f;
        clickEnv = 1.0f;
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        // The table holds one cycle of the 16' (half the played frequency).
        advance (phase, freq * 0.5f / fs);
        const float pos = phase * (float) p.table.size();
        const int i0 = std::min ((int) pos, (int) p.table.size() - 1);
        const int i1 = (i0 + 1) % (int) p.table.size();
        const float frac = pos - (float) i0;
        const float tone = p.table[(size_t) i0] + frac * (p.table[(size_t) i1] - p.table[(size_t) i0]);

        advance (percPhase, freq * 3.0f / fs);
        const float perc = std::sin (2.0f * pi * percPhase) * percEnv * p.percussion * (p.house ? 1.2f : 0.8f);
        clickFilter.process (noise.next());
        const float clk = clickFilter.bp * clickEnv * p.click * 1.5f;

        const float y = (tone * 0.6f + perc + clk) * adsr.next() * gain * 0.32f;
        percEnv *= percCoef;
        clickEnv *= clickCoef;
        left += y;
        right += y;
    }

private:
    const OrganParams& p;
    float fs = 44100.0f, freq = 220, gain = 1, phase = 0, percPhase = 0, percEnv = 0, clickEnv = 0, percCoef = 0, clickCoef = 0;
    Adsr adsr;
    Svf clickFilter;
    Noise noise;
};

// Rotating speaker: amplitude and pitch (doppler) modulation, opposite on each side.
class Leslie
{
public:
    void prepare (float sampleRate)
    {
        fs = sampleRate;
        line.allocate ((int) (fs * 0.01f));
        speed = 0.0f;
    }

    void process (float input, float amount, float& left, float& right)
    {
        line.write (input);
        if (amount < 0.02f)
        {
            left = right = input;
            return;
        }
        const float target = amount > 0.6f ? 6.4f : 0.8f;
        speed += (target - speed) * (1.0f / (fs * 0.8f));   // the rotor speeds up and slows down gradually
        advance (rotor, speed / fs);
        const float s = std::sin (2.0f * pi * rotor), c = std::cos (2.0f * pi * rotor);
        const float depth = juce::jmin (1.0f, amount * 1.6f);
        const float wetL = line.read (fs * (0.002f + 0.0008f * s * depth));
        const float wetR = line.read (fs * (0.002f - 0.0008f * s * depth));
        left = wetL * (1.0f - 0.35f * depth * (0.5f + 0.5f * c));
        right = wetR * (1.0f - 0.35f * depth * (0.5f - 0.5f * c));
    }

private:
    float fs = 44100.0f, rotor = 0, speed = 0;
    DelayLine line;
};

//==============================================================================
// 808 bass: a long sine with a pitch punch, glide, saturation and tone filter.
class Sub808Voice : public MonoBase
{
public:
    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        punchCoef = decayCoef (0.035f, fs);
        attackCoef = 1.0f - decayCoef (0.002f, fs);
        releaseCoef = decayCoef (0.025f, fs);
        reset();
    }

    void setParams (const float* v) override
    {
        decay = decayCoef (knobTime (v[pSub::Decay], 0.15f, 4.0f), fs);
        glideCoef = 1.0f - decayCoef (knobTime (v[pSub::Glide], 0.01f, 0.4f), fs);
        punch = v[pSub::Punch];
        drive = 1.0f + v[pSub::Drive] * 9.0f;
        tone.set (150.0f * std::exp2 (v[pSub::Tone] * 6.0f), 0.7f, fs);
    }

    void reset() override
    {
        amp = 0.0f;
        gate = gliding = false;
        tone.reset();
    }

    float process() override
    {
        updateGlide (glideCoef);
        const float f = midiToHz (currentNote) * (1.0f + punch * 1.5f * punchEnv);
        punchEnv *= punchCoef;
        advance (phase, f / fs);

        if (gate && ! attackDone)
        {
            amp += (1.0f - amp) * attackCoef;
            if (amp > 0.98f) attackDone = true;
        }
        else
        {
            amp *= gate ? decay : releaseCoef;
        }

        const float x = (std::sin (2.0f * pi * phase) + 0.12f * std::sin (4.0f * pi * phase)) * amp;
        tone.process (fastTanh (x * drive) / fastTanh (drive));
        return tone.lp * 0.55f * (accent ? 1.15f : 1.0f);
    }

private:
    void onTrigger() override
    {
        attackDone = false;
        punchEnv = 1.0f;
    }

    float phase = 0, amp = 0, punchEnv = 0, punch = 0.4f, drive = 3.0f;
    float decay = 0.9999f, glideCoef = 0.01f, punchCoef = 0, attackCoef = 0, releaseCoef = 0;
    bool attackDone = false;
    Svf tone;
};
} // namespace techno
