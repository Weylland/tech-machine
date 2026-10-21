#pragma once

#include "SynthVoices.h"

#include <array>
#include <cstring>

namespace techno
{
inline float sine (float phase) { return std::sin (2.0f * pi * phase); }

// Two modulated delays per side: the wide ensemble chorus of the D-50 and the Polysix.
class StereoChorus
{
public:
    void prepare (float sampleRate)
    {
        fs = sampleRate;
        size = (int) (fs * 0.03f) + 4;
        left.assign ((size_t) size, 0.0f);
        right.assign ((size_t) size, 0.0f);
        pos = 0;
    }

    void process (float& l, float& r, float amount, float rate = 0.6f)
    {
        left[(size_t) pos] = l;
        right[(size_t) pos] = r;
        advance (lfo1, rate / fs);
        advance (lfo2, rate * 1.37f / fs);
        const float centre = 0.008f * fs, depth = 0.0025f * fs;
        const float wl = 0.5f * (read (left, centre + depth * sine (lfo1)) + read (right, centre + depth * sine (lfo2 + 0.5f)));
        const float wr = 0.5f * (read (right, centre + depth * sine (lfo1 + 0.25f)) + read (left, centre + depth * sine (lfo2 + 0.75f)));
        l = l * (1.0f - 0.4f * amount) + wl * amount * 0.8f;
        r = r * (1.0f - 0.4f * amount) + wr * amount * 0.8f;
        if (++pos >= size) pos = 0;
    }

private:
    float read (const std::vector<float>& buffer, float delaySamples) const
    {
        float p = (float) pos - delaySamples;
        while (p < 0.0f) p += (float) size;
        const int i0 = std::min ((int) p, size - 1);
        const int i1 = (i0 + 1) % size;
        const float frac = p - (float) i0;
        return buffer[(size_t) i0] + frac * (buffer[(size_t) i1] - buffer[(size_t) i0]);
    }

    std::vector<float> left, right;
    int size = 1, pos = 0;
    float fs = 44100.0f, lfo1 = 0, lfo2 = 0.3f;
};

//==============================================================================
// 8-BIT: the voices of a sound chip. Pulse waves with moving width, a stepped triangle,
// a sawtooth and a shift-register noise; fewer bits on demand. The arpeggio is played by the machine.

struct ChipParams
{
    int wave = 1;
    float pwm = 0.3f, vibrato = 0.2f, lofi = 0.3f;
};

class ChipVoice : public PolyVoice
{
public:
    explicit ChipVoice (const ChipParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
    }

    void setEnvelope (float decay, float sustain, float release) { adsr.set (0.0015f, decay, sustain, release, fs); }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = noteFreq = midiToHz ((float) midiNote);
        held = 0;
        adsr.gateOn();
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        const float e = adsr.next();
        ++held;
        advance (vibratoPhase, 5.5f / fs);
        const float onset = juce::jlimit (0.0f, 1.0f, (float) held / (0.4f * fs) - 0.4f);
        const float f = freq * (1.0f + p.vibrato * 0.012f * onset * sine (vibratoPhase));
        const float dt = f / fs;
        advance (phase, dt);

        float x = 0.0f;
        switch (p.wave)
        {
            case 0:
            case 1:
            case 2:
            {
                const float base = p.wave == 0 ? 0.5f : (p.wave == 1 ? 0.25f : 0.125f);
                advance (pwmPhase, 0.4f / fs);
                const float width = juce::jlimit (0.04f, 0.96f, base * (1.0f + 0.7f * p.pwm * sine (pwmPhase)));
                x = blepPulse (phase, dt, width);
                break;
            }
            case 3:
            {
                const float tri = 1.0f - 4.0f * std::abs (phase - 0.5f);
                x = std::round (tri * 7.5f) / 7.5f;   // 16 steps, like the triangle of the consoles
                break;
            }
            case 4: x = blepSaw (phase, dt); break;
            default:
            {
                noiseClock += dt * 16.0f;
                while (noiseClock >= 1.0f)
                {
                    noiseClock -= 1.0f;
                    const unsigned bit = ((lfsr >> 0) ^ (lfsr >> 1)) & 1u;
                    lfsr = (lfsr >> 1) | (bit << 14);
                }
                x = (lfsr & 1u) ? 0.8f : -0.8f;
                break;
            }
        }

        const float levels = std::exp2 (8.0f - p.lofi * 5.0f);
        const float y = std::round (x * e * levels) / levels * gain * 0.24f;
        left += y;
        right += y;
    }

private:
    const ChipParams& p;
    float fs = 44100.0f, gain = 1, phase = 0, pwmPhase = 0, vibratoPhase = 0, noiseClock = 0;
    unsigned lfsr = 0x4001u;
    int held = 0;
    Adsr adsr;
};

//==============================================================================
// SEMI-MOD: two oscillators, ring modulator and the character filter of five classic synths.

class SemiModVoice : public MonoBase
{
public:
    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        dcBlock.set (20.0f, fs);
        msHighpass.set (70.0f, fs);
        reset();
    }

    void setParams (const float* v) override
    {
        model = (int) v[pSemi::Model];
        wave = v[pSemi::Wave];
        osc2 = v[pSemi::Osc2];
        cutoff = v[pSemi::Cutoff];
        reso = v[pSemi::Reso];
        envAmount = v[pSemi::Env];
        ring = v[pSemi::Ring];
        drive = v[pSemi::Drive];
        adsr.set (knobTime (v[pSemi::Attack], 0.001f, 2.0f), knobTime (v[pSemi::Decay], 0.005f, 3.0f),
                  v[pSemi::Sustain], knobTime (v[pSemi::Release], 0.005f, 3.0f), fs);
        glideCoef = 1.0f - decayCoef (knobTime (v[pSemi::Glide], 0.005f, 0.6f), fs);
    }

    void reset() override
    {
        ladder.reset();
        svf.reset();
        adsr.reset();
        gate = gliding = false;
        feedback = 0.0f;
    }

    float process() override
    {
        updateGlide (glideCoef);
        if (model == 3) drift += (noise.next() * 0.002f - drift * 0.0004f);   // the VCS 3 never stays in tune
        const float f1 = midiToHz (currentNote + bendSemitones + drift * 4.0f);
        const float dt1 = f1 / fs;
        const float previous = phase1;
        advance (phase1, dt1);
        const bool wrapped = phase1 < previous;

        const float e = adsr.next();
        float ratio = 1.0035f;
        if (model == 1) ratio = 1.0f + 2.5f * envAmount * e + cutoff * 0.5f;   // Odyssey: synced oscillator swept by the envelope
        else if (model == 3) ratio = 1.006f;
        const float dt2 = f1 * ratio / fs;
        float o2;
        if (model == 1)
        {
            o2 = synced.process (wrapped ? phase1 / dt1 : -1.0f, dt2, wave);
        }
        else
        {
            advance (phase2, dt2);
            if (phase2 >= 1.0f) phase2 -= std::floor (phase2);
            o2 = (1.0f - wave) * blepSaw (phase2, dt2) + wave * blepPulse (phase2, dt2, 0.5f);
        }

        const float o1 = (1.0f - wave) * blepSaw (phase1, dt1) + wave * blepPulse (phase1, dt1, 0.5f);
        float x = (o1 * 0.6f + o2 * osc2 * 0.6f) * (1.0f - ring * 0.7f) + ring * o1 * o2 * 0.9f;
        if (model == 4) x += drive * 0.7f * fastTanh (feedback * 2.5f);   // MiniBrute: the Brute Factor feeds the output back in
        if (model == 2) x += fastTanh (svf.bp * reso * 1.2f) * 0.3f;      // MS-20: the screaming filter

        const float octaves = cutoff * 10.0f + envAmount * e * 5.0f + (currentNote - 48.0f) / 24.0f + (accent ? 0.6f : 0.0f);
        const float fc = std::min (20.0f * std::exp2 (octaves), 18000.0f);
        float y = 0.0f;
        switch (model)
        {
            case 0:
                ladder.set (fc, reso * 1.02f, fs);
                y = ladder.process (x * 0.4f * (1.0f + drive * 2.0f)) * 1.3f;
                break;
            case 1:
                svf.set (fc, 0.6f + reso * 5.0f, fs);
                svf.process (fastTanh (x * 0.5f * (1.0f + drive * 2.0f)));
                y = svf.lp * 0.8f;
                break;
            case 2:
                svf.set (fc, 0.7f + reso * 14.0f, fs);
                svf.process (msHighpass.highpass (fastTanh (x * 0.6f * (1.0f + drive * 2.5f))));
                y = fastTanh (svf.lp * (1.0f + reso)) * 0.5f;
                break;
            case 3:
                ladder.set (fc, reso * 0.9f, fs);
                y = ladder.process (fastTanh (x * (0.6f + drive * 2.0f)) * 0.5f) * 1.3f;
                break;
            default:
                svf.set (fc, 0.7f + reso * 8.0f, fs);
                svf.process (fastTanh (x * 0.5f));
                y = (svf.lp * (1.0f - 0.3f * reso) + svf.bp * 0.4f * reso) * 0.85f;
                break;
        }
        feedback = y;
        y *= e * (accent ? 1.25f : 1.0f);
        return dcBlock.highpass (y) * 2.8f;
    }

private:
    void onTrigger() override { adsr.gateOn(); }
    void onRelease() override { adsr.gateOff(); }

    // Oscillator restarted by the first one (hard sync). Every jump of its wave, its own or the restart, is smoothed by a
    // band-limited step spread over two samples: one sample of delay.
    struct SyncedOscillator
    {
        float phase = 0.3f, pending = 0.0f;

        // sinceRestart: how long ago (in samples, 0..1) the first oscillator restarted this one, -1 if it did not.
        float process (float sinceRestart, float dt, float wave)
        {
            auto valueAt = [wave] (float p) { return (1.0f - wave) * (2.0f * p - 1.0f) + wave * (p < 0.5f ? 1.0f : -1.0f); };
            float corrections = 0.0f;
            auto step = [&] (float height, float since)   // a jump of the wave, "since" samples ago (0..1)
            {
                pending += 0.5f * height * since * since;
                corrections -= 0.5f * height * (1.0f - since) * (1.0f - since);
            };

            const float before = phase;
            if (sinceRestart >= 0.0f)
            {
                const float atRestart = std::min (before + dt * (1.0f - sinceRestart), 0.999999f);
                const float from = before, to = atRestart;
                if (wave > 0.0f && from < 0.5f && to >= 0.5f) step (-2.0f * wave, (sinceRestart * dt + (to - 0.5f)) / dt);
                step (valueAt (0.0f) - valueAt (to), sinceRestart);
                phase = sinceRestart * dt;
            }
            else
            {
                phase += dt;
                if (wave > 0.0f && before < 0.5f && phase >= 0.5f) step (-2.0f * wave, (phase - 0.5f) / dt);
                if (phase >= 1.0f)
                {
                    phase -= 1.0f;
                    step (4.0f * wave - 2.0f, phase / dt);   // the saw falls by 2, the square rises by 2
                }
            }
            const float out = pending;
            pending = valueAt (phase) + corrections;
            return out;
        }
    };

    int model = 0;
    SyncedOscillator synced;
    float phase1 = 0, phase2 = 0.3f, wave = 0, osc2 = 0.5f, cutoff = 0.5f, reso = 0.35f, envAmount = 0.5f, ring = 0, drive = 0.2f;
    float glideCoef = 0.1f, drift = 0, feedback = 0;
    Adsr adsr;
    Ladder ladder;
    Svf svf;
    OnePole dcBlock, msHighpass;
    Noise noise;
};

//==============================================================================
// ÉTHER: theremin, ondes Martenot and monotron. The pitch always glides from note to note.

class EtherVoice : public MonoBase
{
public:
    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        dcBlock.set (25.0f, fs);
        reset();
    }

    void setParams (const float* v) override
    {
        type = (int) v[pEther::Type];
        glideCoef = 1.0f - decayCoef (knobTime (v[pEther::Glide], 0.01f, 0.8f), fs);
        vibrato = v[pEther::Vibrato];
        rate = 3.0f + v[pEther::Rate] * 5.0f;
        tone = v[pEther::Tone];
        adsr.set (knobTime (v[pEther::Attack], 0.005f, 1.5f), 0.5f, 1.0f, knobTime (v[pEther::Release], 0.02f, 3.0f), fs);
    }

    void noteOn (int midiNote, bool accented, bool) override
    {
        targetNote = (float) midiNote;
        accent = accented;
        if (adsr.isActive())
        {
            if (! gate) adsr.gateOn();
            gate = true;
            return;   // still sounding: it glides to the new note
        }
        currentNote = targetNote;
        gate = true;
        held = 0;
        adsr.gateOn();
    }

    void reset() override
    {
        adsr.reset();
        filter.reset();
        for (auto& r : resonators) r.reset();
        gate = false;
    }

    float process() override
    {
        currentNote += (targetNote - currentNote) * glideCoef;
        ++held;
        advance (vibratoPhase, rate / fs);
        const float onset = juce::jlimit (0.0f, 1.0f, (float) held / (0.5f * fs));
        const float f = midiToHz (currentNote + bendSemitones + vibrato * 0.45f * onset * sine (vibratoPhase));
        const float dt = f / fs;
        advance (phase, dt);

        float y = 0.0f;
        switch (type)
        {
            case 0:
            {
                // Theremin: two radio oscillators beating together give a sine with a little of the second and third harmonic.
                const float s = sine (phase);
                y = fastTanh (s * (1.0f + tone * 1.5f)) + 0.12f * (1.0f + tone) * sine (phase * 2.0f);
                break;
            }
            case 1: y = sine (phase) + tone * 0.15f * sine (phase * 2.0f); break;
            case 2:
                filter.set (f * (2.0f + tone * 6.0f), 0.7f, fs);
                filter.process (blepPulse (phase, dt, 0.5f));
                y = filter.lp * 1.3f;
                break;
            case 3:
                filter.set (f * (3.0f + tone * 10.0f), 1.2f, fs);
                filter.process (blepPulse (phase, dt, 0.15f));
                y = filter.lp * 1.6f;
                break;
            case 4:
            {
                // Palme: the speaker with sympathetic strings rings around each note.
                const float dry = sine (phase) * 0.7f + 0.3f * (1.0f - 4.0f * std::abs (phase - 0.5f));
                static constexpr float ratios[4] { 1.0f, 2.003f, 2.997f, 2.76f };
                float halo = 0.0f;
                for (int i = 0; i < 4; ++i)
                {
                    resonators[(size_t) i].set (f * ratios[i], 40.0f, fs);
                    resonators[(size_t) i].process (dry);
                    halo += resonators[(size_t) i].bp;
                }
                y = dry * 0.6f + halo * (0.1f + tone * 0.15f);
                break;
            }
            default:
            {
                // Monotron: a sawtooth into a small resonant filter that squeals.
                filter.set (f * 3.0f + 200.0f, 0.7f + tone * 12.0f, fs);
                filter.process (blepSaw (phase, dt) * 0.6f);
                y = fastTanh (filter.lp * 1.5f);
                break;
            }
        }
        return dcBlock.highpass (y * adsr.next()) * (accent ? 0.33f : 0.27f);
    }

private:
    void onTrigger() override {}
    void onRelease() override { adsr.gateOff(); }

    int type = 0, held = 0;
    float phase = 0, vibratoPhase = 0, glideCoef = 0.01f, vibrato = 0.5f, rate = 5.5f, tone = 0.5f;
    Adsr adsr;
    Svf filter;
    std::array<Svf, 4> resonators;
    OnePole dcBlock;
};

//==============================================================================
// CS-80: two layers per voice, high-pass and low-pass filters, notes that bend in and out, touch-sensitive brilliance.

struct CS80Params
{
    int type = 0;
    float brilliance = 0.5f, reso = 0.2f, touch = 0.5f, bend = 0.3f, vibrato = 0.3f, attack = 0.35f, release = 0.5f;
    bool accent = false;
};

class CS80Voice : public PolyVoice
{
public:
    explicit CS80Voice (const CS80Params& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        amp.reset();
        lowpass.reset();
        highpass.reset();
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = noteFreq = midiToHz ((float) midiNote);
        accented = p.accent;
        const auto& t = config();
        amp.set (t.attack * knobTime (p.attack, 0.2f, 5.0f), t.decay, t.sustain, knobTime (p.release, 0.05f, 4.0f), fs);
        amp.gateOn();
        filterEnv = 0.0f;
        filterAttack = 1.0f - decayCoef (t.filterAttack * knobTime (p.attack, 0.2f, 5.0f), fs);
        scoop = -p.bend * 0.6f;
        drop = 0.0f;
        held = 0;
        releasing = false;
    }

    void release() override
    {
        amp.gateOff();
        releasing = true;
    }

    bool isActive() const override { return amp.isActive(); }

    void process (float& left, float& right) override
    {
        const auto& t = config();
        ++held;
        scoop *= 0.9995f;
        if (releasing) drop -= p.bend * 2.0f / fs;   // the note falls away when it is let go
        advance (vibratoPhase, 5.8f / fs);
        const float onset = juce::jlimit (0.0f, 1.0f, (float) held / (0.6f * fs) - 0.3f);
        const float pitch = scoop + drop + p.vibrato * t.vibrato * 0.25f * onset * sine (vibratoPhase);
        const float f = freq * std::exp2 (pitch / 12.0f);

        float a = 0.0f, b = 0.0f;
        for (int i = 0; i < 3; ++i)
        {
            const float detune = 1.0f + t.detune * (float) (i - 1);
            const float dt = f * detune / fs;
            advance (phases[i], dt);
            const float osc = t.saw * blepSaw (phases[i], dt) + t.pulse * blepPulse (phases[i], dt, 0.4f);
            (i == 2 ? b : a) += osc;
            if (i == 1) b += osc * 0.5f;
        }
        if (t.ring > 0.0f)
        {
            advance (ringPhase, f * 2.76f / fs);
            advance (bellPhase, f / fs);
            const float bell = sine (bellPhase) * (1.0f - t.ring) + t.ring * sine (bellPhase) * sine (ringPhase) * 1.6f;
            a = a * 0.2f + bell;
            b = b * 0.2f + bell;
        }
        advance (subPhase, f * 0.5f / fs);
        const float sub = t.sub * sine (subPhase);

        const float e = amp.next();
        filterEnv += (1.0f - filterEnv) * filterAttack;
        const float touchBoost = accented ? p.touch * 2.0f : 0.0f;
        const float octaves = 1.0f + p.brilliance * 6.0f * t.bright + filterEnv * 1.5f + touchBoost;
        lowpass.set (juce::jmin (f * std::exp2 (octaves), 16000.0f), 0.7f + p.reso * 5.0f, fs);
        highpass.set (t.highpassHz, 0.7f, fs);

        const float mono = (a + b) * 0.5f + sub;
        highpass.process (mono);
        lowpass.process (highpass.hp);
        const float level = gain * e * (accented ? 1.0f + p.touch * 0.4f : 1.0f) * 0.42f;
        const float spread = (a - b) * 0.08f;
        left += (lowpass.lp + spread) * level;
        right += (lowpass.lp - spread) * level;
    }

private:
    struct Config { float saw, pulse, detune, sub, ring, bright, attack, filterAttack, decay, sustain, highpassHz, vibrato; };

    const Config& config() const
    {
        static constexpr Config configs[5] {
            { 1.0f, 0.0f, 0.0025f, 0.25f, 0.0f, 1.0f, 0.6f, 1.0f, 1.0f, 0.9f, 110.0f, 0.5f },    // Blade Runner brass
            { 0.4f, 0.6f, 0.004f, 0.15f, 0.0f, 0.6f, 1.2f, 1.5f, 1.5f, 0.85f, 80.0f, 0.4f },    // velvet pad
            { 1.0f, 0.0f, 0.006f, 0.0f, 0.0f, 0.8f, 0.9f, 0.8f, 1.0f, 0.9f, 260.0f, 0.8f },     // strings
            { 0.0f, 0.0f, 0.0f, 0.0f, 0.85f, 1.0f, 0.02f, 0.02f, 2.5f, 0.0f, 120.0f, 0.2f },    // ring-modulated bells
            { 0.8f, 0.4f, 0.002f, 0.1f, 0.0f, 1.2f, 0.1f, 0.3f, 0.6f, 0.8f, 100.0f, 1.0f },     // expressive lead
        };
        return configs[juce::jlimit (0, 4, p.type)];
    }

    const CS80Params& p;
    float fs = 44100.0f, gain = 1, scoop = 0, drop = 0, filterEnv = 0, filterAttack = 0.01f;
    float phases[3] { 0.0f, 0.33f, 0.71f }, subPhase = 0, ringPhase = 0, bellPhase = 0, vibratoPhase = 0;
    int held = 0;
    bool accented = false, releasing = false;
    Adsr amp;
    Svf lowpass, highpass;
};

//==============================================================================
// LA SYNTH: a short realistic attack (bell, pluck, breath, hammer) over a synth pad, as on the D-50.

struct LaParams
{
    int type = 0;
    float layer = 0.6f, brightness = 0.5f, attack = 0.2f, release = 0.4f;
};

class LaVoice : public PolyVoice
{
public:
    explicit LaVoice (const LaParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        pad.reset();
        padFilter.reset();
        breathFilter.reset();
        formant.reset();
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = noteFreq = midiToHz ((float) midiNote);
        const auto& t = config();
        bellEnv = pluckEnv = breathEnv = 1.0f;
        bellDecay = decayCoef (t.bellDecay, fs);
        pluckDecay = decayCoef (t.pluckDecay, fs);
        breathDecay = decayCoef (t.breathDecay, fs);
        pad.set (t.padAttack * knobTime (p.attack, 0.1f, 6.0f), 2.0f, t.padSustain, knobTime (p.release, 0.05f, 5.0f), fs);
        pad.gateOn();
        for (auto& ph : phases) ph = 0.0f;
        for (int i = 0; i < 6; ++i) partialEnv[i] = 1.0f;
        for (int i = 0; i < 6; ++i) partialDecay[i] = decayCoef (t.pianoDecay / (1.0f + (float) i * 0.6f), fs);
        breathFilter.set (t.breathHz, 2.0f, fs);
        formant.set (900.0f, 4.0f, fs);
    }

    void release() override { pad.gateOff(); releaseCoef = decayCoef (knobTime (p.release, 0.05f, 5.0f) * 0.5f, fs); released = true; }
    bool isActive() const override { return pad.isActive() || bellEnv > 0.0005f || pluckEnv > 0.0005f || partialEnv[0] > 0.0005f; }

    void process (float& left, float& right) override
    {
        const auto& t = config();
        const float layer = p.layer * 1.4f;

        // Attack partial
        float attack = 0.0f;
        if (t.bellLevel > 0.0f)
        {
            advance (phases[0], freq * t.bellRatio / fs);
            advance (phases[1], freq / fs);
            attack += t.bellLevel * bellEnv * sine (phases[1] + t.bellIndex * bellEnv * sine (phases[0]) * 0.25f);
        }
        if (t.pluckLevel > 0.0f)
        {
            const float dt = freq / fs;
            advance (phases[2], dt);
            pluckFilter.set (freq * (2.0f + 20.0f * pluckEnv), 0.7f, fs);
            pluckFilter.process (blepSaw (phases[2], dt));
            attack += t.pluckLevel * pluckEnv * pluckFilter.lp;
        }
        if (t.breathLevel > 0.0f)
        {
            breathFilter.process (noise.next());
            attack += t.breathLevel * breathEnv * breathFilter.bp * 2.0f;
        }
        if (t.pianoLevel > 0.0f)
        {
            float stack = 0.0f;
            for (int i = 0; i < 6; ++i)
            {
                advance (partials[i], freq * (float) (i + 1) * (1.0f + 0.0004f * (float) (i * i)) / fs);
                stack += sine (partials[i]) * partialEnv[i] / (1.0f + (float) i);
                partialEnv[i] *= released ? releaseCoef * partialDecay[i] : partialDecay[i];
            }
            attack += t.pianoLevel * stack;
        }
        bellEnv *= bellDecay;
        pluckEnv *= pluckDecay;
        breathEnv *= breathDecay;

        // Sustain partial: two detuned oscillators through a soft filter
        const float e = pad.next();
        float sustain = 0.0f;
        if (t.padLevel > 0.0f)
        {
            for (int i = 0; i < 2; ++i)
            {
                const float dt = freq * (1.0f + (i == 0 ? -t.padDetune : t.padDetune)) / fs;
                advance (phases[3 + i], dt);
                sustain += t.padPulse > 0.5f ? blepPulse (phases[3 + i], dt, 0.35f) : blepSaw (phases[3 + i], dt);
            }
            padFilter.set (juce::jmin (freq * (1.5f + p.brightness * t.padBright), 15000.0f), 0.8f, fs);
            padFilter.process (sustain * 0.5f);
            sustain = padFilter.lp;
            if (t.vocal)
            {
                formant.process (sustain);
                sustain = sustain * 0.4f + formant.bp * 1.2f;
            }
            sustain *= t.padLevel * e;
        }

        const float y = (attack * layer + sustain) * gain * 0.3f;
        left += y;
        right += y;
    }

private:
    struct Config
    {
        float bellLevel, bellRatio, bellIndex, bellDecay;
        float pluckLevel, pluckDecay;
        float breathLevel, breathHz, breathDecay;
        float pianoLevel, pianoDecay;
        float padLevel, padPulse, padDetune, padBright, padAttack, padSustain;
        bool vocal;
    };

    const Config& config() const
    {
        static constexpr Config configs[8] {
            { 0.8f, 3.5f, 2.5f, 1.2f,  0.0f, 0.1f,  0.0f, 2000.0f, 0.1f,  0.0f, 1.0f,  0.7f, 0.0f, 0.006f, 4.0f, 1.0f, 0.9f, false },  // Fantasia
            { 0.5f, 4.0f, 1.5f, 0.6f,  0.9f, 0.25f, 0.0f, 2000.0f, 0.1f,  0.0f, 1.0f,  0.25f, 0.0f, 0.004f, 3.0f, 0.6f, 0.5f, false }, // Pizzagogo
            { 0.3f, 7.0f, 3.0f, 0.15f, 0.0f, 0.1f,  0.9f, 2400.0f, 0.09f, 0.0f, 1.0f,  0.6f, 1.0f, 0.005f, 5.0f, 0.15f, 0.7f, false }, // digital dance
            { 0.9f, 4.0f, 0.6f, 0.35f, 0.0f, 0.1f,  0.15f, 3000.0f, 0.02f, 0.0f, 1.0f, 0.15f, 0.0f, 0.003f, 3.0f, 0.2f, 0.3f, false }, // staccato marimba
            { 0.5f, 2.0f, 1.0f, 0.8f,  0.0f, 0.1f,  0.4f, 5000.0f, 0.3f,  0.0f, 1.0f,  0.8f, 0.0f, 0.007f, 3.5f, 1.2f, 0.9f, true },   // glass voices
            { 0.25f, 7.0f, 1.0f, 0.5f, 0.0f, 0.1f,  0.0f, 2000.0f, 0.1f,  0.0f, 1.0f,  0.9f, 0.0f, 0.009f, 7.0f, 0.8f, 0.9f, false },  // JD pad
            { 0.0f, 1.0f, 0.0f, 0.1f,  0.7f, 0.12f, 0.2f, 1500.0f, 0.05f, 0.0f, 1.0f,  0.8f, 0.0f, 0.003f, 5.0f, 0.3f, 0.85f, false }, // LA brass
            { 0.0f, 1.0f, 0.0f, 0.1f,  0.0f, 0.1f,  0.25f, 3500.0f, 0.015f, 1.0f, 2.5f, 0.12f, 0.0f, 0.002f, 4.0f, 0.1f, 0.4f, false }, // digital piano
        };
        return configs[juce::jlimit (0, 7, p.type)];
    }

    const LaParams& p;
    float fs = 44100.0f, gain = 1;
    float phases[5] {}, partials[6] {}, partialEnv[6] {}, partialDecay[6] {};
    float bellEnv = 0, pluckEnv = 0, breathEnv = 0, bellDecay = 0, pluckDecay = 0, breathDecay = 0, releaseCoef = 0.999f;
    bool released = false;
    Adsr pad;
    Svf padFilter, pluckFilter, breathFilter, formant;
    Noise noise;
};

//==============================================================================
// WEST COAST: a sine modulated in frequency, folded on itself, through a low pass gate (vactrol).

struct WestParams
{
    int ratio = 0, gate = 0;
    float fm = 0.3f, fold = 0.4f, symmetry = 0.5f, decay = 0.4f;
};

class WestVoice : public PolyVoice
{
public:
    explicit WestVoice (const WestParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        filter.reset();
        sustainEnv.reset();
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = noteFreq = midiToHz ((float) midiNote);
        strike = 1.0f;
        const float seconds = knobTime (p.decay, 0.05f, 3.0f);
        fastDecay = decayCoef (seconds * 0.35f, fs);
        slowDecay = decayCoef (seconds, fs);
        sustainEnv.set (0.002f, 0.3f, 1.0f, seconds * 0.5f, fs);
        sustainEnv.gateOn();
    }

    void release() override { sustainEnv.gateOff(); }
    bool isActive() const override { return p.gate == 2 ? sustainEnv.isActive() : strike > 0.0003f; }

    void process (float& left, float& right) override
    {
        static constexpr float ratios[6] { 1.0f, 2.0f, 1.5f, 0.5f, 1.75f, 1.4142f };
        // The vactrol lets go quickly at first, then slowly: the natural, woody decay of the Buchla.
        strike *= strike > 0.3f ? fastDecay : slowDecay;
        const float e = sustainEnv.next();

        advance (modPhase, freq * ratios[juce::jlimit (0, 5, p.ratio)] / fs);
        const float index = p.fm * (0.4f + 1.6f * strike) * 2.5f;
        const float f = freq * (1.0f + index * sine (modPhase));
        float dt = f / fs;
        if (dt < 0.0f) dt = -dt;
        carrierPhase += dt;
        carrierPhase -= std::floor (carrierPhase);

        // Wavefolder: the further the sine is pushed, the more it folds back on itself. The fold is computed from its
        // anti-derivative between two samples (ADAA): the folds keep their sharpness without folding back into the audio.
        const float x = sine (carrierPhase) * (1.0f + p.fold * 5.0f * (0.5f + 0.5f * strike)) + (p.symmetry - 0.5f) * 1.5f;
        const float dx = x - previousX;
        const float folded = std::abs (dx) > 1.0e-4f ? (foldIntegral (x) - foldIntegral (previousX)) / dx : fold (0.5f * (x + previousX));
        previousX = x;

        float amp = strike, cutoff = 150.0f + 14000.0f * strike * strike;
        if (p.gate == 1) cutoff = 16000.0f;
        else if (p.gate == 2) { amp = e; cutoff = 400.0f + 12000.0f * strike * strike; }
        filter.set (cutoff, 0.6f, fs);
        filter.process (folded);
        const float y = filter.lp * amp * gain * 0.45f;
        left += y;
        right += y;
    }

private:
    // A triangle of the input: the shape of the fold, between -1 and 1.
    static float fold (float x)
    {
        const float t = 0.25f * x + 0.25f;
        return 4.0f * std::abs (t - std::floor (t + 0.5f)) - 1.0f;
    }

    // Its integral over x (periodic: the triangle averages zero over a fold).
    static float foldIntegral (float x)
    {
        const float t = 0.25f * x + 0.25f;
        const float u = t - std::floor (t + 0.5f);
        return 4.0f * (2.0f * u * std::abs (u) - u);
    }

    const WestParams& p;
    float fs = 44100.0f, gain = 1, strike = 0, fastDecay = 0.99f, slowDecay = 0.999f, modPhase = 0, carrierPhase = 0, previousX = 0;
    Svf filter;
    Adsr sustainEnv;
};

//==============================================================================
// VOCODER: a synth carrier shaped by a talking voice made inside the machine.

class CarrierVoice : public PolyVoice
{
public:
    explicit CarrierVoice (const float& pulseMix) : pulse (pulseMix) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
    }

    void setRelease (float seconds) { adsr.set (0.004f, 0.1f, 1.0f, seconds, fs); }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        freq = noteFreq = midiToHz ((float) midiNote);
        adsr.gateOn();
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        const float dt = freq / fs;
        advance (phase, dt);
        const float y = ((1.0f - pulse) * blepSaw (phase, dt) + pulse * blepPulse (phase, dt, 0.5f)) * adsr.next() * gain * 0.5f;
        left += y;
        right += y;
    }

private:
    const float& pulse;
    float fs = 44100.0f, gain = 1, phase = 0;
    Adsr adsr;
};

class SpeechVocoder
{
public:
    static constexpr int numBands = 16;

    void prepare (float sampleRate)
    {
        fs = sampleRate;
        for (int b = 0; b < numBands; ++b)
        {
            const float hz = 120.0f * std::pow (7500.0f / 120.0f, (float) b / (float) (numBands - 1));
            analysis[(size_t) b].set (hz, 6.0f, fs);
            analysis[(size_t) b].reset();
            synthesis[(size_t) b].reset();
            levels[(size_t) b] = 0.0f;
        }
        lastShift = -1.0f;
        attackCoef = 1.0f - decayCoef (0.004f, fs);
        releaseCoef = 1.0f - decayCoef (0.035f, fs);
        glideCoef = 1.0f - decayCoef (0.025f, fs);
        sibilantHp.set (5000.0f, 0.7f, fs);
        for (auto& f : formants) f.reset();
        slot = 1000;
    }

    void set (int wordIndex, float speedKnob, float sibilanceKnob, float shiftKnob)
    {
        words = wordIndex;
        slotSamples = fs / (4.0f + speedKnob * 20.0f);
        sibilance = sibilanceKnob;
        if (std::abs (shiftKnob - lastShift) > 0.005f)
        {
            lastShift = shiftKnob;
            const float ratio = std::exp2 ((shiftKnob - 0.5f) * 2.0f);
            for (int b = 0; b < numBands; ++b)
            {
                const float hz = 120.0f * std::pow (7500.0f / 120.0f, (float) b / (float) (numBands - 1));
                synthesis[(size_t) b].set (juce::jlimit (60.0f, fs * 0.45f, hz * ratio), 6.0f, fs);
            }
        }
    }

    // A new note: the voice starts its word again.
    void restart (juce::Random& random)
    {
        slot = 0;
        countdown = 0.0f;
        randomVowel = "aeiou"[random.nextInt (5)];
    }

    void process (float& left, float& right)
    {
        // The speaking voice: a buzz (or a hiss for the consonants) through three moving formants.
        if ((countdown -= 1.0f) <= 0.0f)
        {
            countdown += slotSamples;
            target = phoneme (currentChar());
            ++slot;
        }
        for (int i = 0; i < 3; ++i) formantHz[i] += (target.formants[i] - formantHz[i]) * glideCoef;
        voiced += (target.voiced - voiced) * glideCoef;
        loudness += (target.gain - loudness) * glideCoef;

        const float dt = 105.0f / fs;
        advance (glottis, dt);
        const float buzz = blepSaw (glottis, dt);
        const float hiss = noise.next();
        const float source = (buzz * voiced + hiss * (1.0f - voiced) * 0.6f) * loudness;
        float voice = 0.0f;
        static constexpr float q[3] { 5.0f, 8.0f, 10.0f }, g[3] { 1.0f, 0.7f, 0.4f };
        for (int i = 0; i < 3; ++i)
        {
            formants[(size_t) i].set (formantHz[i], q[i], fs);
            formants[(size_t) i].process (source);
            voice += formants[(size_t) i].bp * g[i];
        }
        voice *= 0.12f;

        float out = vocode (voice, 0.5f * (left + right));
        sibilantHp.process (voice * (1.0f - voiced));
        out = out * 2.0f + std::abs (sibilantHp.hp) * hiss * sibilance * 0.3f;
        left = right = out;
    }

    // The same with a voice from outside (the microphone): its consonants pass through as they are.
    void processExternal (float voice, float& left, float& right)
    {
        float out = vocode (voice, 0.5f * (left + right));
        sibilantHp.process (voice);
        out = out * 2.0f + sibilantHp.hp * sibilance * 0.5f;
        left = right = out;
    }

private:
    struct Phoneme { float formants[3]; float voiced, gain; };

    // Each band of the voice opens the same band of the synth.
    float vocode (float voice, float carrier)
    {
        float out = 0.0f;
        for (int b = 0; b < numBands; ++b)
        {
            auto& a = analysis[(size_t) b];
            a.process (voice);
            const float level = std::abs (a.bp);
            auto& l = levels[(size_t) b];
            l += (level - l) * (level > l ? attackCoef : releaseCoef);
            auto& s = synthesis[(size_t) b];
            s.process (carrier);
            out += s.bp * l;
        }
        return out;
    }

    char currentChar() const
    {
        static const char* const texts[] { "rroobboot   ", "tteeknnoo   ", "eelleektrroo   ", "oooo yyeeaa   ",
                                           "aaauuuiiiooo   ", "aaaaaaaaaaaaaaaa", "" };
        if (words == 5) return 'a';
        if (words >= 6) return slot < 8 ? randomVowel : ' ';
        const char* text = texts[juce::jlimit (0, 5, words)];
        const int length = (int) std::strlen (text);
        return slot < length ? text[slot] : ' ';
    }

    static Phoneme phoneme (char c)
    {
        switch (c)
        {
            case 'a': return { { 800.0f, 1250.0f, 2600.0f }, 1.0f, 1.0f };
            case 'e': return { { 450.0f, 1900.0f, 2550.0f }, 1.0f, 1.0f };
            case 'i': case 'y': return { { 300.0f, 2300.0f, 3000.0f }, 1.0f, 0.9f };
            case 'o': return { { 500.0f, 850.0f, 2500.0f }, 1.0f, 1.0f };
            case 'u': return { { 320.0f, 750.0f, 2300.0f }, 1.0f, 0.9f };
            case 'm': return { { 280.0f, 1100.0f, 2300.0f }, 1.0f, 0.4f };
            case 'n': return { { 280.0f, 1600.0f, 2600.0f }, 1.0f, 0.4f };
            case 'r': return { { 450.0f, 1300.0f, 1700.0f }, 1.0f, 0.7f };
            case 'l': return { { 380.0f, 1100.0f, 2600.0f }, 1.0f, 0.6f };
            case 'b': return { { 300.0f, 900.0f, 2200.0f }, 1.0f, 0.25f };
            case 'd': return { { 300.0f, 1700.0f, 2600.0f }, 1.0f, 0.25f };
            case 't': return { { 1800.0f, 3500.0f, 5000.0f }, 0.0f, 0.7f };
            case 'k': return { { 1500.0f, 2500.0f, 4000.0f }, 0.0f, 0.6f };
            case 's': return { { 2500.0f, 5000.0f, 7000.0f }, 0.0f, 0.7f };
            default: return { { 500.0f, 1500.0f, 2500.0f }, 1.0f, 0.0f };
        }
    }

    float fs = 44100.0f, slotSamples = 4000.0f, countdown = 0, sibilance = 0.4f, lastShift = -1.0f;
    float attackCoef = 0.1f, releaseCoef = 0.01f, glideCoef = 0.01f, glottis = 0, voiced = 1, loudness = 0;
    float formantHz[3] { 500.0f, 1500.0f, 2500.0f };
    int words = 0, slot = 1000;
    char randomVowel = 'a';
    Phoneme target { { 500.0f, 1500.0f, 2500.0f }, 1.0f, 0.0f };
    std::array<Svf, 3> formants;
    std::array<Svf, numBands> analysis, synthesis;
    std::array<float, numBands> levels {};
    Svf sibilantHp;
    Noise noise;
};
} // namespace techno
