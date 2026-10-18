#pragma once

#include "DrumVoices.h"

namespace techno
{
// Sound effects played like drum sounds. Each one lasts a set time (the DURÉE knob) and ends on its own.
class FxVoice : public DrumVoice
{
public:
    void trigger (float velocity, const DrumSettings& s) override
    {
        gain = velocity;
        settings = s;
        t = 0;
        length = juce::jmax (1, (int) (seconds (s.decay) * fs));
        start();
    }

    float process() override
    {
        if (t >= length) return 0.0f;
        const float x = (float) t / (float) length;
        // 4 ms fades at both ends, so a cut-off effect never clicks.
        const float edge = juce::jmin (1.0f, juce::jmin ((float) t, (float) (length - t)) / (0.004f * fs));
        ++t;
        return render (x) * edge * gain;
    }

    void choke() override { length = juce::jmin (length, t + (int) (0.01f * fs)); }

protected:
    virtual float seconds (float decayKnob) const = 0;
    virtual void start() {}
    virtual float render (float x) = 0;   // x = position in the effect, 0 to 1

    // Filters swept by an effect are recomputed every 32 samples.
    bool update() { return (t & 31) == 0; }

    DrumSettings settings;
    float gain = 1;
    int t = 0, length = 1;
};

// Rising noise and tone: the build-up before a drop.
class Riser : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.5f, 8.0f); }
    void start() override { phase = 0; band.reset(); }
    float render (float x) override
    {
        if (update()) band.set (300.0f * std::pow (30.0f, x), 2.5f, fs);
        band.process (noise.next());
        const float f = 100.0f * tuneRatio (settings.tune, 12.0f) * std::pow (16.0f, x), dt = f / fs;
        advance (phase, dt);
        const float tone = blepSaw (phase, dt) * 0.35f;
        return (band.bp * 1.6f * (1.0f - settings.tone) + tone * settings.tone) * x * x;
    }
    float phase = 0;
    Svf band;
};

// The opposite: a sweep that falls and fades after a drop.
class Downlifter : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.4f, 6.0f); }
    void start() override { phase = 0; band.reset(); }
    float render (float x) override
    {
        if (update()) band.set (8000.0f * std::pow (0.03f, x), 2.0f, fs);
        band.process (noise.next());
        const float f = 1500.0f * tuneRatio (settings.tune, 12.0f) * std::pow (0.06f, x), dt = f / fs;
        advance (phase, dt);
        const float tone = blepSaw (phase, dt) * 0.35f;
        return (band.bp * 1.6f * (1.0f - settings.tone) + tone * settings.tone) * (1.0f - x) * (1.0f - x);
    }
    float phase = 0;
    Svf band;
};

// Cinematic hit: sub boom with a pitch drop plus a dark noise tail.
class Impact : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.4f, 5.0f); }
    void start() override { phase = 0; tail.reset(); }
    float render (float x) override
    {
        const float time = x * (float) length / fs;
        const float f = (38.0f + 40.0f * std::exp (-time * 12.0f)) * tuneRatio (settings.tune, 12.0f);
        advance (phase, f / fs);
        const float boom = std::sin (2.0f * pi * phase) * std::exp (-x * 4.0f);
        if (update()) tail.set (3500.0f * std::pow (0.1f, x) + 150.0f, 0.7f, fs);
        tail.process (noise.next());
        const float hit = tail.lp * std::exp (-x * 5.0f) * (time < 0.01f ? 2.0f : 1.0f);
        return boom * (1.1f - settings.tone * 0.6f) + hit * (0.3f + settings.tone * 1.2f);
    }
    float phase = 0;
    Svf tail;
};

// White noise swoosh that opens and closes.
class Sweep : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.3f, 6.0f); }
    void start() override { band.reset(); }
    float render (float x) override
    {
        const float bell = std::sin (pi * x);
        if (update()) band.set (400.0f * std::pow (20.0f, bell) * tuneRatio (settings.tune, 12.0f), 0.8f + settings.tone * 6.0f, fs);
        band.process (noise.next());
        return band.bp * bell * std::sqrt (bell) * 1.4f;
    }
    Svf band;
};

// Laser "pew": a tone diving from very high to low.
class Zap : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.05f, 0.8f); }
    void start() override { phase = 0; }
    float render (float x) override
    {
        const float f = 3000.0f * tuneRatio (settings.tune, 12.0f) * std::pow (0.02f, x), dt = f / fs;
        advance (phase, dt);
        const float wave = std::sin (2.0f * pi * phase) * (1.0f - settings.tone) + blepPulse (phase, dt, 0.5f) * settings.tone * 0.5f;
        return wave * (1.0f - x) * 0.8f;
    }
    float phase = 0;
};

// Dub siren: a square wave whose pitch sweeps up and down.
class Siren : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.3f, 6.0f); }
    void start() override { phase = lfo = 0; filter.reset(); filter.set (3500.0f, 0.7f, fs); }
    float render (float) override
    {
        advance (lfo, (0.6f + settings.tone * 7.0f) / fs);
        const float tri = 1.0f - 4.0f * std::abs (lfo - 0.5f);
        const float f = 520.0f * tuneRatio (settings.tune, 12.0f) * (1.4f + 0.6f * tri), dt = f / fs;
        advance (phase, dt);
        filter.process (blepPulse (phase, dt, 0.5f));
        return filter.lp * 0.4f;
    }
    float phase = 0, lfo = 0;
    Svf filter;
};

// Air horn of reggae and dancehall: a loud, slightly detuned brass blast.
class AirHorn : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.08f, 1.5f); }
    void start() override
    {
        for (auto& ph : phases) ph = 0;
        filter.reset();
        filter.set (1200.0f + settings.tone * 3000.0f, 1.2f, fs);
    }
    float render (float x) override
    {
        static constexpr float ratios[4] { 1.0f, 1.006f, 0.995f, 1.5f };
        static constexpr float levels[4] { 1.0f, 0.8f, 0.8f, 0.45f };
        const float bend = x > 0.85f ? 1.0f - (x - 0.85f) * 0.25f : 1.0f;
        float sum = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
            const float dt = 440.0f * tuneRatio (settings.tune, 12.0f) * ratios[i] * bend / fs;
            advance (phases[i], dt);
            sum += blepSaw (phases[i], dt) * levels[i];
        }
        filter.process (sum);
        const float attack = juce::jmin (1.0f, x * (float) length / (0.015f * fs));
        return fastTanh (filter.lp * 1.6f) * attack * 0.55f;
    }
    float phases[4] {};
    Svf filter;
};

// Vinyl noise: crackles, hiss and rumble, for lo-fi. Place one long hit per bar or per sequence.
class Vinyl : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 1.0f, 16.0f); }
    void start() override { hiss.reset(); crackle.reset(); hiss.set (3000.0f, 0.7f, fs); crackle.set (2500.0f, 1.5f, fs); }
    float render (float) override
    {
        hiss.process (noise.next());
        float impulse = 0.0f;
        if (random.nextFloat() < (15.0f + settings.tone * 160.0f) / fs) impulse = (random.nextFloat() * 2.0f - 1.0f) * 6.0f;
        crackle.process (impulse);
        rumble += 0.002f * (noise.next() - rumble);
        return hiss.hp * 0.025f + crackle.bp * 0.5f + rumble * 0.6f;
    }
    float rumble = 0;
    Svf hiss, crackle;
    juce::Random random;
};

// Reversed cymbal: grows louder and stops dead. End it one step before a downbeat.
class ReverseCymbal : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.3f, 4.0f); }
    void start() override { hp.reset(); hp.set (5500.0f * std::exp2 ((settings.tone - 0.5f) * 1.5f), 0.7f, fs); }
    float render (float x) override
    {
        static constexpr float freqs[6] { 245.0f, 368.0f, 569.0f, 812.0f, 1205.0f, 1634.0f };
        const float metal = bank.process (freqs, tuneRatio (settings.tune, 7.0f) * 2.0f, fs);
        hp.process (metal * 0.5f + noise.next() * 0.6f);
        return hp.hp * x * x * x * 1.3f;
    }
    MetalBank bank;
    Svf hp;
};
// DJ scratch: a voice-like sound pushed back and forth under the hand.
class Scratch : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.15f, 1.5f); }
    void start() override { phase = 0; band.reset(); band.set (900.0f, 2.0f, fs); }
    float render (float x) override
    {
        const float cycles = 2.0f + settings.tone * 6.0f;
        const float speed = std::sin (2.0f * pi * x * cycles);
        const float f = 180.0f * tuneRatio (settings.tune, 12.0f) * (0.2f + 1.8f * std::abs (speed)), dt = f / fs;
        advance (phase, dt);
        band.process (blepSaw (phase, dt) + noise.next() * 0.3f);
        return band.bp * std::abs (speed) * 1.8f;
    }
    float phase = 0;
    Svf band;
};

// Rewind (jungle, dub, sound systems): the tape spun back very fast.
class Rewind : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.4f, 3.0f); }
    void start() override { phase = 0; band.reset(); }
    float render (float x) override
    {
        const float f = 120.0f * tuneRatio (settings.tune, 12.0f) * std::pow (25.0f, x) * (1.0f + 0.3f * std::sin (2.0f * pi * x * 18.0f)), dt = f / fs;
        advance (phase, dt);
        if (update()) band.set (juce::jmin (f * 3.0f, 12000.0f), 1.0f, fs);
        band.process (blepSaw (phase, dt) * 0.5f + noise.next() * (0.3f + settings.tone * 0.5f));
        return band.bp * (0.3f + 0.7f * x) * 1.6f;
    }
    float phase = 0;
    Svf band;
};

// Gunshot sound effect of dancehall and jungle: a crack, a boom and a fading echo of noise.
class Gunshot : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 0.3f, 2.5f); }
    void start() override { phase = 0; tail.reset(); crack.reset(); crack.set (3000.0f, 0.8f, fs); }
    float render (float x) override
    {
        const float time = x * (float) length / fs;
        advance (phase, (60.0f * tuneRatio (settings.tune, 12.0f) + 80.0f * std::exp (-time * 30.0f)) / fs);
        const float boom = std::sin (2.0f * pi * phase) * std::exp (-time * 9.0f);
        crack.process (noise.next());
        if (update()) tail.set (2500.0f * std::exp (-time * 3.0f) + 200.0f, 0.7f, fs);
        tail.process (noise.next());
        return crack.bp * std::exp (-time * 120.0f) * 3.0f + boom * 0.9f + tail.lp * std::exp (-x * 4.0f) * (0.3f + settings.tone * 0.6f);
    }
    float phase = 0;
    Svf crack, tail;
};

// Rain: soft noise and drops, for ambient and lo-fi.
class Rain : public FxVoice
{
    float seconds (float d) const override { return knobTime (d, 1.0f, 16.0f); }
    void start() override { wash.reset(); drops.reset(); wash.set (1800.0f, 0.5f, fs); drops.set (4000.0f, 2.0f, fs); }
    float render (float) override
    {
        wash.process (noise.next());
        float drop = 0.0f;
        if (random.nextFloat() < (40.0f + settings.tone * 300.0f) / fs) drop = (random.nextFloat() * 2.0f - 1.0f) * 3.0f;
        drops.process (drop);
        return wash.lp * 0.18f + drops.bp * 0.5f;
    }
    Svf wash, drops;
    juce::Random random;
};
} // namespace techno
