#pragma once

#include "AcousticVoices.h"
#include "../core/MachineDefs.h"

namespace techno
{
//==============================================================================
// Electric bass: a plucked string (Karplus-Strong) with the touch of each playing style.

struct BassType
{
    float decay, damping, pick, pop, tone, glide;
};

inline const BassType& bassType (int index)
{
    static const BassType types[] {
        { 2.5f, 0.45f, 0.5f, 0.0f, 0.45f, 0.0f },    // fingers
        { 1.8f, 0.75f, 0.9f, 1.0f, 0.8f, 0.0f },     // slap
        { 2.2f, 0.65f, 0.85f, 0.2f, 0.65f, 0.0f },   // pick
        { 3.0f, 0.4f, 0.4f, 0.0f, 0.4f, 0.6f },      // fretless (slides between notes)
        { 1.2f, 0.3f, 0.35f, 0.3f, 0.3f, 0.0f },     // upright double bass
    };
    return types[(size_t) juce::jlimit (0, (int) std::size (types) - 1, index)];
}

class ElectricBassVoice : public MonoBase
{
public:
    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        loop.allocate ((int) (fs / 25.0f) + 4);
        popFilter.set (2500.0f, 0.8f, fs);
        reset();
    }

    void setParams (const float* v) override
    {
        type = (int) v[pBass::Type];
        const auto& t = bassType (type);
        tone.set (300.0f * std::exp2 ((t.tone * 0.6f + v[pBass::Tone] * 0.8f) * 5.0f), 0.6f, fs);
        decayScale = knobTime (v[pBass::Decay], 0.3f, 3.0f);
        muteSeconds = knobTime (v[pBass::Muting], 0.02f, 0.6f);
        glideCoef = 1.0f - decayCoef (knobTime (juce::jmax (v[pBass::Glide], t.glide), 0.005f, 0.3f), fs);
    }

    void reset() override
    {
        for (auto& x : loop.buffer) x = 0.0f;
        lp = pickLp = dcIn = dcOut = 0.0f;
        gate = gliding = false;
        excite = 0;
        loopGain = 0.0f;
        tone.reset();
    }

    float process() override
    {
        updateGlide (glideCoef);
        const auto& t = bassType (type);
        const float freq = midiToHz (currentNote);
        const float loopCoef = 0.15f + 0.84f * t.damping;
        const float period = juce::jlimit (2.0f, (float) loop.size - 3.0f, fs / freq - (1.0f - loopCoef) / loopCoef);

        float in = 0.0f;
        if (excite > 0)
        {
            pickLp += (0.1f + 0.85f * t.pick) * (noise.next() - pickLp);
            in = pickLp;
            --excite;
        }
        lp += loopCoef * (loop.read (period) - lp);
        const float y = in + loopGain * lp;
        loop.write (y);

        dcOut = y - dcIn + 0.995f * dcOut;
        dcIn = y;
        tone.process (dcOut);
        popFilter.process (noise.next());
        const float pop = popFilter.bp * popEnv * t.pop;
        popEnv *= popCoef;
        return (tone.lp * (accent ? 1.25f : 1.0f) + pop) * 2.2f;
    }

private:
    void onTrigger() override
    {
        const auto& t = bassType (type);
        const float freq = midiToHz (currentNote);
        loopGain = std::exp (-6.91f / (t.decay * decayScale * freq));
        excite = (int) (fs / freq);
        popEnv = 1.0f;
        popCoef = decayCoef (0.004f, fs);
    }

    void onRelease() override { loopGain = std::min (loopGain, std::exp (-6.91f / (muteSeconds * midiToHz (currentNote)))); }

    DelayLine loop;
    Svf tone, popFilter;
    Noise noise;
    int type = 0, excite = 0;
    float lp = 0, pickLp = 0, dcIn = 0, dcOut = 0, loopGain = 0, popEnv = 0, popCoef = 0;
    float decayScale = 1, muteSeconds = 0.1f, glideCoef = 0.01f;
};

//==============================================================================
// Wobble / Reese bass: oscillators into a resonant filter swept by an LFO locked to the tempo.

class WobbleVoice : public MonoBase
{
public:
    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        reset();
    }

    void setParams (const float* v) override
    {
        wave = (int) v[pWob::Wave];
        rateIndex = (int) v[pWob::Rate];
        depth = v[pWob::Depth];
        cutoff = v[pWob::Cutoff];
        reso = v[pWob::Reso];
        detune = v[pWob::Detune];
        drive = 1.0f + v[pWob::Drive] * 8.0f;
        glideCoef = 1.0f - decayCoef (knobTime (v[pWob::Glide], 0.005f, 0.3f), fs);
    }

    // Called on every sequencer step: the LFO stays locked to the song position.
    void syncStep (int step, double stepSamples) override
    {
        static constexpr float stepsPerCycle[] { 8.0f, 4.0f, 2.0f, 4.0f / 3.0f, 1.0f, 16.0f / 3.0f };
        const float cycle = stepsPerCycle[(size_t) juce::jlimit (0, 5, rateIndex)];
        lfoPhase = std::fmod ((float) step / cycle, 1.0f);
        lfoInc = (float) (1.0 / (cycle * stepSamples * 2.0));   // the voice runs at twice the sample rate
    }

    void reset() override
    {
        ladder.reset();
        amp = 0.0f;
        gate = gliding = false;
    }

    float process() override
    {
        updateGlide (glideCoef);
        advance (lfoPhase, lfoInc);
        const float freq = midiToHz (currentNote);
        const float spread = 1.0f + detune * 0.012f;

        float x;
        if (wave == 1)   // reese: detuned saws rubbing together
        {
            advance (phases[0], freq * spread / fs);
            advance (phases[1], freq / spread / fs);
            advance (phases[2], freq * 0.5f / fs);
            x = blepSaw (phases[0], freq * spread / fs) + blepSaw (phases[1], freq / spread / fs) + 0.6f * std::sin (2.0f * pi * phases[2]);
        }
        else if (wave == 2)   // growl: saw with a vowel-like band sweeping with the LFO
        {
            advance (phases[0], freq / fs);
            advance (phases[1], freq * 2.0f * spread / fs);
            x = blepSaw (phases[0], freq / fs) + 0.5f * blepPulse (phases[1], freq * 2.0f * spread / fs, 0.3f);
        }
        else   // wobble: saw + square
        {
            advance (phases[0], freq / fs);
            advance (phases[1], freq * spread / fs);
            x = blepSaw (phases[0], freq / fs) + blepPulse (phases[1], freq * spread / fs, 0.5f) * 0.7f;
        }

        const float lfo = 0.5f - 0.5f * std::cos (2.0f * pi * lfoPhase);
        const float octaves = cutoff * 7.0f + depth * lfo * 5.0f;
        ladder.set (juce::jmin (40.0f * std::exp2 (octaves), 16000.0f), wave == 2 ? juce::jmax (reso, 0.6f) : reso, fs);
        const float filtered = ladder.process (x * 0.35f);

        amp += ((gate ? 1.0f : 0.0f) - amp) * (gate ? 0.01f : 0.0015f);
        return fastTanh (filtered * drive) / fastTanh (drive) * amp * 0.9f;
    }

private:
    void onTrigger() override {}

    Ladder ladder;
    float phases[3] {}, lfoPhase = 0, lfoInc = 0.00001f, amp = 0;
    int wave = 0, rateIndex = 2;
    float depth = 0.6f, cutoff = 0.3f, reso = 0.3f, detune = 0.3f, drive = 2.0f, glideCoef = 0.01f;
};

//==============================================================================
// Hoover (Alpha Juno "What The"): detuned PWM saws whose pitch dips at the start of each note.

struct HooverParams
{
    float detune = 0.6f, dip = 0.5f, cutoff = 0.7f, attack = 0.02f, release = 0.3f;
};

class HooverVoice : public PolyVoice
{
public:
    explicit HooverVoice (const HooverParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
        filter.reset();
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        bend = -p.dip * 12.0f;   // starts below the note and rises to it
        bendCoef = decayCoef (0.12f, fs);
        adsr.set (p.attack, 0.5f, 1.0f, p.release, fs);
        adsr.gateOn();
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        static constexpr float offsets[5] { -1.0f, -0.5f, 0.0f, 0.5f, 1.0f };
        bend *= bendCoef;
        advance (pwm, 0.7f / fs);
        const float f = freq * std::exp2 (bend / 12.0f);
        float l = 0.0f, r = 0.0f;
        for (int i = 0; i < 5; ++i)
        {
            const float dt = f * (1.0f + offsets[i] * p.detune * 0.02f) / fs;
            advance (phases[i], dt);
            const float width = 0.5f + 0.4f * std::sin (2.0f * pi * (pwm + (float) i * 0.2f));
            const float v = blepSaw (phases[i], dt) * 0.6f + blepPulse (phases[i], dt, width) * 0.4f;
            l += v * (i < 3 ? 1.0f : 0.5f);
            r += v * (i > 1 ? 1.0f : 0.5f);
        }
        filter.set (juce::jmin (200.0f * std::exp2 (p.cutoff * 6.5f), 16000.0f), 0.8f, fs);
        filter.process ((l + r) * 0.5f);
        const float e = adsr.next() * gain * 0.22f;
        left += (filter.lp * 0.7f + l * 0.15f) * e;
        right += (filter.lp * 0.7f + r * 0.15f) * e;
    }

private:
    const HooverParams& p;
    float fs = 44100.0f, freq = 220, gain = 1, bend = 0, bendCoef = 0, pwm = 0;
    float phases[5] { 0.0f, 0.2f, 0.4f, 0.6f, 0.8f };
    Adsr adsr;
    Svf filter;
};

//==============================================================================
// Poly 80s (Prophet-5 / OB-X): two oscillators per voice (saw + pulse), the second detuned,
// a 12 dB state-variable filter with its own envelope. Brass, pads and synthwave stabs.

struct Poly80Params
{
    float saw = 0.8f, pulse = 0.4f, detune = 0.3f, cutoff = 0.5f, reso = 0.2f, env = 0.4f;
};

class Poly80Voice : public PolyVoice
{
public:
    explicit Poly80Voice (const Poly80Params& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
        filter.reset();
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
        const float d = 1.0f + p.detune * 0.012f;
        const float dt1 = freq / fs, dt2 = freq * d / fs;
        advance (phase1, dt1);
        advance (phase2, dt2);
        const float a = p.saw * blepSaw (phase1, dt1) + p.pulse * blepPulse (phase1, dt1, 0.45f);
        const float b = p.saw * blepSaw (phase2, dt2) + p.pulse * blepPulse (phase2, dt2, 0.45f);
        const float e = adsr.next();
        const float octaves = p.cutoff * 8.5f + p.env * e * 4.0f + std::log2 (freq / 261.6f) * 0.4f;
        filter.set (juce::jmin (30.0f * std::exp2 (octaves), 17000.0f), 0.6f + p.reso * 4.0f, fs);
        filter.process ((a + b) * 0.3f);
        const float y = filter.lp * e * gain;
        left += y * 1.8f + (a - b) * 0.04f * e * gain;
        right += y * 1.8f - (a - b) * 0.04f * e * gain;
    }

private:
    const Poly80Params& p;
    float fs = 44100.0f, freq = 220, gain = 1, phase1 = 0, phase2 = 0.37f;
    Adsr adsr;
    Svf filter;
};

//==============================================================================
// Talkbox: a buzzy synth whose sound is shaped by a mouth moving from one vowel to another on each note.

struct TalkboxParams
{
    int from = 1, to = 0;
    float speed = 0.4f, brightness = 0.5f, release = 0.2f;
};

class TalkboxVoice : public PolyVoice
{
public:
    explicit TalkboxVoice (const TalkboxParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
        for (auto& f : formants) f.reset();
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        morph = 0.0f;
        morphCoef = 1.0f - decayCoef (knobTime (p.speed, 0.03f, 0.8f), fs);
        adsr.set (0.01f, 0.3f, 1.0f, knobTime (p.release, 0.02f, 1.0f), fs);
        adsr.gateOn();
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        morph += (1.0f - morph) * morphCoef;
        if ((counter++ & 31) == 0)
        {
            const auto& a = vowel (p.from);
            const auto& b = vowel (p.to);
            for (int i = 0; i < 3; ++i)
                formants[(size_t) i].set (a.freqs[i] + (b.freqs[i] - a.freqs[i]) * morph, 6.0f + p.brightness * 8.0f, fs);
        }
        const float dt = freq / fs;
        advance (phase, dt);
        const float source = blepSaw (phase, dt) * 0.7f + blepPulse (phase, dt, 0.3f) * 0.3f;
        const auto& a = vowel (p.from);
        const auto& b = vowel (p.to);
        float y = 0.0f;
        for (int i = 0; i < 3; ++i)
        {
            formants[(size_t) i].process (source);
            y += formants[(size_t) i].bp * (a.amps[i] + (b.amps[i] - a.amps[i]) * morph) * (i == 0 ? 1.0f : 0.6f + p.brightness);
        }
        y = (y + source * 0.04f) * adsr.next() * gain * 0.45f;
        left += y;
        right += y;
    }

private:
    const TalkboxParams& p;
    float fs = 44100.0f, freq = 220, gain = 1, phase = 0, morph = 0, morphCoef = 0;
    unsigned int counter = 0;
    std::array<Svf, 3> formants;
    Adsr adsr;
};

//==============================================================================
// Mellotron: tape replay keyboard. Each key plays an 8-second tape: notes fade out after that.

struct MellotronParams
{
    int type = 0;
    float tone = 0.5f, wow = 0.4f;
    float wobble = 0.0f;   // shared tape speed deviation, updated by the machine
};

class MellotronVoice : public PolyVoice
{
public:
    explicit MellotronVoice (const MellotronParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
        filter.reset();
        breath.reset();
        tapeCoef = decayCoef (0.5f, fs);
    }

    void setEnvelope (float a, float r) { adsr.set (a, 0.3f, 1.0f, r, fs); }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        played = 0;
        tapeEnd = 1.0f;
        breath.set (juce::jmin (freq * 2.0f, 9000.0f), 3.0f, fs);
        if (p.type == 2) formant.set (800.0f, 6.0f, fs);
        adsr.gateOn();
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        const float f = freq * (1.0f + p.wobble * p.wow * 0.006f);
        const float dt = f / fs;
        advance (phases[0], dt);
        advance (phases[1], dt * 1.003f);
        advance (phases[2], dt * 0.997f);

        float x;
        switch (p.type)
        {
            case 0:   // flutes
                breath.process (noise.next());
                x = sine01 (phases[0]) + 0.15f * sine01 (std::fmod (phases[0] * 2.0f, 1.0f)) + breath.bp * 0.4f;
                break;
            case 1:   // strings
                x = (blepSaw (phases[0], dt) + blepSaw (phases[1], dt) + blepSaw (phases[2], dt)) * 0.35f;
                break;
            case 2:   // choir
                formant.process (blepSaw (phases[0], dt) + blepSaw (phases[1], dt));
                x = formant.bp * 2.0f;
                break;
            case 3:   // cello
                x = blepSaw (phases[0], dt) * 0.8f + blepPulse (phases[1], dt, 0.4f) * 0.3f;
                break;
            default:  // brass
                x = blepSaw (phases[0], dt) + 0.5f * blepSaw (phases[1], dt);
                break;
        }
        filter.set (juce::jmin (500.0f * std::exp2 (p.tone * 4.5f), 12000.0f), 0.7f, fs);
        filter.process (x);

        // 8 seconds of tape, then it runs out.
        if (++played > (int) (8.0f * fs)) tapeEnd *= tapeCoef;
        const float y = filter.lp * adsr.next() * gain * tapeEnd * 0.35f;
        left += y;
        right += y;
    }

private:
    const MellotronParams& p;
    float fs = 44100.0f, freq = 220, gain = 1, tapeEnd = 1, tapeCoef = 0;
    float phases[3] { 0.0f, 0.3f, 0.6f };
    int played = 0;
    Adsr adsr;
    Svf filter, breath, formant;
    Noise noise;
};

//==============================================================================
// Electric pianos: Rhodes and Wurlitzer (tine / reed modelled with FM), Clavinet and harpsichord (plucked strings).

struct EPianoParams
{
    int type = 0;
    float tone = 0.5f, decay = 0.5f, release = 0.3f, bark = 0.4f;
};

class EPianoVoice : public PolyVoice
{
public:
    explicit EPianoVoice (const EPianoParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        loop.allocate ((int) (fs / 25.0f) + 4);
        active = false;
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        const float decay = knobTime (p.decay, 0.4f, 4.0f) * std::exp2 (-(float) (midiNote - 60) / 24.0f);
        amp = 1.0f;
        ampCoef = decayCoef (decay * (p.type == 1 ? 0.6f : 1.0f), fs);
        index = (0.4f + p.bark * 2.2f) * (0.4f + velocity * 0.8f) * (p.type == 1 ? 1.6f : 1.0f);
        indexCoef = decayCoef (p.type == 1 ? 0.4f : 0.25f, fs);
        tine = 1.0f;
        tineCoef = decayCoef (0.04f, fs);
        damp = 1.0f;
        dampCoef = 1.0f;

        if (p.type >= 2)
        {
            for (auto& v : loop.buffer) v = 0.0f;
            const float damping = p.type == 2 ? 0.75f : 0.9f;
            loopCoef = 0.15f + 0.84f * damping;
            period = juce::jlimit (2.0f, (float) loop.size - 3.0f, fs / freq - (1.0f - loopCoef) / loopCoef);
            loopGain = std::exp (-6.91f / ((p.type == 2 ? 0.9f : 1.6f) * decay * freq));
            excite = (int) (fs / freq);
            lp = pickLp = 0.0f;
        }
        active = true;
    }

    void release() override { dampCoef = decayCoef (knobTime (p.release, 0.02f, 1.0f), fs); }
    bool isActive() const override { return active; }

    void process (float& left, float& right) override
    {
        float y;
        if (p.type < 2)
        {
            advance (carrier, freq / fs);
            advance (modulator, freq / fs);
            advance (tinePhase, freq * 7.1f / fs);
            const float mod = sine01 (modulator) * index * (0.3f + p.tone);
            float x = std::sin (2.0f * pi * carrier + mod);
            if (p.type == 1) x = fastTanh (x * 1.8f + 0.25f) - fastTanh (0.25f);   // the reed's buzz
            y = x * amp + sine01 (tinePhase) * tine * 0.25f * (0.3f + p.tone);
            index *= indexCoef;
            tine *= tineCoef;
        }
        else
        {
            float in = 0.0f;
            if (excite > 0)
            {
                pickLp += 0.9f * (noise.next() - pickLp);
                in = pickLp;
                --excite;
            }
            lp += loopCoef * (loop.read (period) - lp);
            const float s = in + loopGain * lp;
            loop.write (s);
            // Pickup near the string end (clavinet) or a 4' string an octave up (harpsichord).
            y = p.type == 2 ? (s - loop.read (period * 0.12f)) * 0.8f : s + 0.5f * loop.read (period * 0.5f);
        }
        amp *= ampCoef;
        damp *= dampCoef;
        const float out = y * damp * gain * 0.4f;
        left += out;
        right += out;
        if (damp < 0.0001f || (p.type < 2 && amp < 0.0001f)) active = false;
    }

private:
    const EPianoParams& p;
    DelayLine loop;
    Noise noise;
    float fs = 44100.0f, freq = 220, gain = 1, carrier = 0, modulator = 0, tinePhase = 0;
    float amp = 1, ampCoef = 1, index = 1, indexCoef = 1, tine = 1, tineCoef = 1, damp = 1, dampCoef = 1;
    float loopCoef = 0.5f, period = 100, loopGain = 0.99f, lp = 0, pickLp = 0;
    int excite = 0;
    bool active = false;
};

//==============================================================================
// Brass: saws whose brightness follows the breath (the louder, the brighter), with a delayed vibrato.

struct BrassType
{
    float formantHz, formantQ, attack, decayTo, detune, octaveBelow;
    bool sax;
};

inline const BrassType& brassType (int index)
{
    static const BrassType types[] {
        { 1400.0f, 1.5f, 0.04f, 0.85f, 0.0f, 0.0f, false },    // trumpet
        { 650.0f, 1.2f, 0.06f, 0.85f, 0.0f, 0.0f, false },     // trombone
        { 1200.0f, 2.0f, 0.05f, 0.9f, 0.0f, 0.0f, true },      // saxophone
        { 1100.0f, 1.3f, 0.03f, 0.75f, 0.012f, 0.4f, false },  // funk section
        { 1300.0f, 1.3f, 0.01f, 0.0f, 0.01f, 0.5f, false },    // horn stab
        { 450.0f, 1.0f, 0.08f, 0.9f, 0.0f, 0.0f, false },      // French horn
    };
    return types[(size_t) juce::jlimit (0, (int) std::size (types) - 1, index)];
}

struct BrassParams
{
    int type = 0;
    float brightness = 0.5f, attack = 0.3f, release = 0.3f, vibrato = 0.4f, growl = 0.2f;
};

class BrassVoice : public PolyVoice
{
public:
    explicit BrassVoice (const BrassParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
        filter.reset();
        formant.reset();
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        const auto& t = brassType (p.type);
        const float a = t.attack * knobTime (p.attack, 0.3f, 4.0f);
        adsr.set (a, t.decayTo > 0.0f ? 0.4f : 0.18f, t.decayTo, knobTime (p.release, 0.03f, 1.0f), fs);
        adsr.gateOn();
        vibratoDelay = (int) (0.3f * fs);
        formant.set (t.formantHz, t.formantQ, fs);
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        const auto& t = brassType (p.type);
        advance (vibratoPhase, 5.4f / fs);
        const float vib = vibratoDelay > 0 ? (--vibratoDelay, 0.0f) : std::sin (2.0f * pi * vibratoPhase) * p.vibrato * 0.006f;
        const float f = freq * (1.0f + vib);
        const float dt = f / fs;
        advance (phases[0], dt);
        advance (phases[1], dt * (1.0f + t.detune));
        advance (phases[2], dt * 0.5f);
        float x = blepSaw (phases[0], dt);
        if (t.detune > 0.0f) x = 0.6f * x + 0.6f * blepSaw (phases[1], dt * (1.0f + t.detune));
        if (t.octaveBelow > 0.0f) x += t.octaveBelow * blepSaw (phases[2], dt * 0.5f);
        if (t.sax) x = 0.7f * x + 0.5f * blepPulse (phases[0], dt, 0.35f) + noise.next() * p.growl * 0.3f;

        const float e = adsr.next();
        const float growl = 1.0f - p.growl * 0.3f * (0.5f + 0.5f * std::sin (2.0f * pi * (vibratoPhase * 6.0f)));
        filter.set (juce::jmin (f * (1.2f + e * (3.0f + p.brightness * 9.0f)), 15000.0f), 0.8f, fs);
        filter.process (x);
        formant.process (filter.lp);
        const float y = (filter.lp * 0.6f + formant.bp * 0.8f) * e * gain * growl * 0.42f;
        left += y;
        right += y;
    }

private:
    const BrassParams& p;
    float fs = 44100.0f, freq = 220, gain = 1, vibratoPhase = 0;
    float phases[3] {};
    int vibratoDelay = 0;
    Adsr adsr;
    Svf filter, formant;
    Noise noise;
};

//==============================================================================
// Woodwinds: a nearly pure tone plus breath noise around it, a chiff at the attack and a delayed vibrato.

struct WindType
{
    float breath, odd, chiff, bend, vibrato;
};

inline const WindType& windType (int index)
{
    static const WindType types[] {
        { 0.25f, 0.0f, 0.4f, 0.0f, 1.0f },     // flute
        { 0.55f, 0.0f, 0.7f, 0.0f, 0.6f },     // pan flute
        { 0.08f, 1.0f, 0.2f, 0.0f, 0.4f },     // clarinet
        { 0.7f, 0.0f, 0.5f, -0.6f, 1.4f },     // shakuhachi (bends up into the note)
        { 0.1f, 0.0f, 0.2f, 0.0f, 0.5f },      // ocarina
        { 0.4f, 0.2f, 0.5f, 0.0f, 1.2f },      // trap flute
    };
    return types[(size_t) juce::jlimit (0, (int) std::size (types) - 1, index)];
}

struct WindParams
{
    int type = 0;
    float breath = 0.5f, vibrato = 0.4f, attack = 0.3f, release = 0.3f, brightness = 0.5f;
};

class WindVoice : public PolyVoice
{
public:
    explicit WindVoice (const WindParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
        breathFilter.reset();
        tone.reset();
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        const auto& t = windType (p.type);
        adsr.set (knobTime (p.attack, 0.02f, 0.6f), 0.3f, 0.9f, knobTime (p.release, 0.03f, 1.0f), fs);
        adsr.gateOn();
        chiff = t.chiff;
        chiffCoef = decayCoef (0.03f, fs);
        bend = t.bend;
        bendCoef = decayCoef (0.12f, fs);
        vibratoDelay = (int) (0.25f * fs);
        breathFilter.set (juce::jmin (freq * 2.0f, 9000.0f), 2.5f, fs);
        tone.set (juce::jmin (freq * (2.0f + p.brightness * 6.0f), 15000.0f), 0.7f, fs);
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        const auto& t = windType (p.type);
        advance (vibratoPhase, 5.0f / fs);
        const float vib = vibratoDelay > 0 ? (--vibratoDelay, 0.0f) : std::sin (2.0f * pi * vibratoPhase) * p.vibrato * t.vibrato * 0.005f;
        bend *= bendCoef;
        const float f = freq * std::exp2 (bend / 12.0f) * (1.0f + vib), dt = f / fs;
        advance (phase, dt);
        float x = sine01 (phase) * (1.0f - t.odd) + t.odd * blepPulse (phase, dt, 0.5f) * 0.6f;
        breathFilter.process (noise.next());
        x += breathFilter.bp * (t.breath * (0.4f + p.breath) + chiff * 2.0f);
        chiff *= chiffCoef;
        tone.process (x);
        const float y = tone.lp * adsr.next() * gain * 0.26f;
        left += y;
        right += y;
    }

private:
    const WindParams& p;
    float fs = 44100.0f, freq = 220, gain = 1, phase = 0, vibratoPhase = 0, chiff = 0, chiffCoef = 0, bend = 0, bendCoef = 0;
    int vibratoDelay = 0;
    Adsr adsr;
    Svf breathFilter, tone;
    Noise noise;
};

//==============================================================================
// Orchestra: string section, pizzicato, the famous "orchestra hit", timpani and harp.

struct OrchestraParams
{
    int type = 0;
    float attack = 0.4f, release = 0.4f, brightness = 0.5f, ensemble = 0.6f;
};

class OrchestraVoice : public PolyVoice
{
public:
    explicit OrchestraVoice (const OrchestraParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        loop.allocate ((int) (fs / 25.0f) + 4);
        adsr.reset();
        filter.reset();
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = midiToHz ((float) midiNote);
        sweep = 1.0f;
        sweepCoef = decayCoef (p.type == 2 ? 0.08f : 0.25f, fs);
        switch (p.type)
        {
            case 0: adsr.set (knobTime (p.attack, 0.02f, 2.0f), 0.5f, 1.0f, knobTime (p.release, 0.05f, 3.0f), fs); break;
            case 2: adsr.set (0.002f, 0.35f, 0.0f, 0.2f, fs); break;
            case 3: adsr.set (0.002f, 1.6f, 0.0f, 1.2f, fs); break;
            default: adsr.set (0.001f, 3.0f, 0.0f, knobTime (p.release, 0.05f, 2.0f), fs); break;
        }
        adsr.gateOn();
        if (p.type == 1 || p.type == 4)
        {
            for (auto& v : loop.buffer) v = 0.0f;
            loopCoef = p.type == 1 ? 0.35f : 0.4f;
            period = juce::jlimit (2.0f, (float) loop.size - 3.0f, fs / freq - (1.0f - loopCoef) / loopCoef);
            loopGain = std::exp (-6.91f / ((p.type == 1 ? 0.35f : 4.0f) * freq));
            excite = (int) (fs / freq);
            lp = pickLp = 0.0f;
        }
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        float l = 0.0f, r = 0.0f;
        const float e = adsr.next();
        if (p.type == 0 || p.type == 2)
        {
            // Section strings (or the hit): four detuned saws, each with its own slow vibrato.
            static constexpr float offsets[4] { -1.0f, -0.35f, 0.35f, 1.0f };
            for (int i = 0; i < 4; ++i)
            {
                advance (vib[i], (4.6f + (float) i * 0.37f) / fs);
                const float dt = freq * (1.0f + offsets[i] * p.ensemble * 0.009f + std::sin (2.0f * pi * vib[i]) * 0.002f) / fs;
                advance (phases[i], dt);
                const float s = blepSaw (phases[i], dt);
                l += s * (i < 2 ? 0.8f : 0.4f);
                r += s * (i < 2 ? 0.4f : 0.8f);
            }
            if (p.type == 2)
            {
                // Orchestra hit: an octave below, a burst of noise, a filter that closes fast.
                advance (phases[4], freq * 0.5f / fs);
                const float low = blepSaw (phases[4], freq * 0.5f / fs);
                l += low + noise.next() * sweep;
                r += low + noise.next() * sweep;
            }
            const float cutoff = juce::jmin (freq * (2.0f + p.brightness * 8.0f + sweep * (p.type == 2 ? 30.0f : 0.0f)), 16000.0f);
            filter.set (cutoff, 0.7f, fs);
            filterR.set (cutoff, 0.7f, fs);
            filter.process (l);
            filterR.process (r);
            l = filter.lp * 0.3f;
            r = filterR.lp * 0.3f;
        }
        else if (p.type == 3)
        {
            // Timpani: a tuned drum with a slight pitch drop and inharmonic overtones.
            const float f = freq * (1.0f + 0.04f * sweep);
            advance (phases[0], f / fs);
            advance (phases[1], f * 1.5f / fs);
            advance (phases[2], f * 1.99f / fs);
            const float x = sine01 (phases[0]) + 0.5f * sine01 (phases[1]) * sweep + 0.3f * sine01 (phases[2]) * sweep + noise.next() * 0.3f * sweep * sweep;
            l = r = x * 0.6f;
        }
        else
        {
            // Pizzicato and harp: plucked strings.
            float in = 0.0f;
            if (excite > 0)
            {
                pickLp += 0.5f * (noise.next() - pickLp);
                in = pickLp;
                --excite;
            }
            lp += loopCoef * (loop.read (period) - lp);
            const float s = in + loopGain * lp;
            loop.write (s);
            l = r = s * 0.9f;
        }
        sweep *= sweepCoef;
        left += l * e * gain;
        right += r * e * gain;
    }

private:
    const OrchestraParams& p;
    DelayLine loop;
    Noise noise;
    float fs = 44100.0f, freq = 220, gain = 1, sweep = 0, sweepCoef = 0;
    float phases[5] { 0.0f, 0.25f, 0.5f, 0.75f, 0.1f }, vib[4] { 0.0f, 0.3f, 0.6f, 0.8f };
    float loopCoef = 0.4f, period = 100, loopGain = 0.99f, lp = 0, pickLp = 0;
    int excite = 0;
    Adsr adsr;
    Svf filter, filterR;
};
} // namespace techno
