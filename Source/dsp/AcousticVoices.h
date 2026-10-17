#pragma once

#include "Reverb.h"
#include "SynthVoices.h"
#include <array>

namespace techno
{
// sin (2 pi phase) for phase in [0, 1), max error 2e-5: cheap enough for dozens of partials per voice.
inline float sine01 (float phase)
{
    const float y = phase < 0.5f ? 2.0f * phase : 2.0f * phase - 2.0f;
    const float y2 = y * y;
    return y * (3.14154414f + y2 * (-5.16665433f + y2 * (2.54374556f + y2 * (-0.58339362f + y2 * 0.06477525f))));
}

// One decaying sine: a mode of a string, bar or tine.
struct Partial
{
    float phase = 0, dt = 0, amp = 0, coef = 1;

    float process()
    {
        advance (phase, dt);
        const float v = sine01 (phase) * amp;
        amp *= coef;
        return v;
    }
};

//==============================================================================
// Acoustic piano: inharmonic partials on two slightly detuned strings, hammer noise and dampers.

struct PianoType
{
    float inharmonicity, detuneCents, decay, brightness, hammer;
};

inline const PianoType& pianoType (int index)
{
    static const PianoType types[] {
        { 0.00012f, 0.7f, 1.0f, 1.0f, 1.0f },    // grand
        { 0.0005f, 1.6f, 0.7f, 0.85f, 1.2f },    // upright
        { 0.0004f, 16.0f, 0.7f, 1.0f, 1.1f },    // honky-tonk
        { 0.00018f, 1.0f, 0.9f, 1.35f, 1.3f },   // bright pop
        { 0.00015f, 0.6f, 0.55f, 0.45f, 0.5f },  // felt
    };
    return types[(size_t) juce::jlimit (0, 4, index)];
}

struct PianoParams
{
    int type = 0;
    float brightness = 0.5f, decay = 0.5f, release = 0.4f, hammer = 0.4f, width = 0.5f;
};

class PianoVoice : public PolyVoice
{
public:
    explicit PianoVoice (const PianoParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        active = false;
        hammerCoef = decayCoef (0.004f, fs);
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        const auto& t = pianoType (p.type);
        const float f = midiToHz ((float) midiNote);
        const float b = t.inharmonicity * (1.0f + (float) juce::jmax (0, midiNote - 40) / 30.0f);
        const float bright = juce::jlimit (0.0f, 1.3f, p.brightness * t.brightness + velocity * 0.15f);
        const float tilt = 2.3f - 1.4f * bright;
        const float baseDecay = juce::jlimit (0.15f, 12.0f, knobTime (p.decay, 0.6f, 5.0f) * t.decay * std::exp2 (-(float) (midiNote - 48) / 18.0f));
        const float limit = juce::jmin (0.45f * fs, 14000.0f);

        count = 0;
        for (int n = 1; n <= maxModes && count + 2 <= maxPartials; ++n)
        {
            const float fn = (float) n * f * std::sqrt (1.0f + b * (float) (n * n));
            if (fn > limit) break;
            const float strike = std::abs (std::sin (pi * (float) n * 0.12f)) + 0.08f;   // hammer at about 1/8 of the string
            const float a = strike / std::pow ((float) n, tilt);
            const float tn = baseDecay / (1.0f + 0.35f * (float) (n - 1) * (1.3f - bright));
            for (int k = 0; k < 2; ++k)
            {
                auto& part = partials[(size_t) count++];
                const float cents = (k == 0 ? -0.5f : 0.5f) * t.detuneCents;
                part.dt = fn * std::exp2 (cents / 1200.0f) / fs;
                part.amp = a * (k == 0 ? 0.55f : 0.45f);
                part.coef = decayCoef (tn * (k == 0 ? 1.0f : 0.6f), fs);
                part.phase = std::fmod ((float) n * 0.137f + (float) k * 0.31f, 1.0f);
            }
        }

        hammerEnv = t.hammer * (0.4f + velocity * 0.6f);
        hammerFilter.set (juce::jmin (f * 5.0f, 7000.0f), 0.9f, fs);
        damp = 1.0f;
        dampCoef = 1.0f;
        const float pan = juce::jlimit (-1.0f, 1.0f, (float) (midiNote - 60) / 30.0f) * p.width;
        panLeft = std::sqrt (0.5f * (1.0f - pan)) * 1.41f;
        panRight = std::sqrt (0.5f * (1.0f + pan)) * 1.41f;
        active = true;
    }

    void release() override
    {
        // Dampers: the top octaves of a real piano have none.
        if (note < 89) dampCoef = decayCoef (knobTime (p.release, 0.04f, 1.5f), fs);
    }

    bool isActive() const override { return active; }

    void process (float& left, float& right) override
    {
        float sum = 0.0f;
        for (int i = 0; i < count; ++i) sum += partials[(size_t) i].process();
        hammerFilter.process (noise.next());
        sum += hammerFilter.bp * hammerEnv * p.hammer * 0.6f;
        hammerEnv *= hammerCoef;
        damp *= dampCoef;

        const float y = sum * damp * gain * 0.75f;
        left += y * panLeft;
        right += y * panRight;
        if (damp < 0.0001f || (count > 0 && partials[0].amp < 0.00002f && partials[1].amp < 0.00002f)) active = false;
    }

private:
    static constexpr int maxModes = 16, maxPartials = 24;
    const PianoParams& p;
    std::array<Partial, maxPartials> partials {};
    int count = 0;
    float fs = 44100.0f, gain = 1, hammerEnv = 0, hammerCoef = 0, damp = 1, dampCoef = 1, panLeft = 1, panRight = 1;
    bool active = false;
    Svf hammerFilter;
    Noise noise;
};

//==============================================================================
// Mallets and bells: a few modes with the frequency ratios of each instrument, plus the mallet click.

struct MalletType
{
    int modes;
    float ratios[5], amps[5], decays[5];
    int octave;
    float click, tremolo;
};

inline const MalletType& malletType (int index)
{
    static const MalletType types[] {
        { 4, { 1.0f, 2.71f, 5.15f, 8.43f }, { 1.0f, 0.45f, 0.3f, 0.15f }, { 1.4f, 0.7f, 0.35f, 0.18f }, 24, 0.6f, 0.1f },   // glockenspiel
        { 3, { 1.0f, 3.98f, 9.9f }, { 1.0f, 0.25f, 0.08f }, { 2.0f, 0.6f, 0.2f }, 0, 0.3f, 1.0f },                         // vibraphone
        { 3, { 1.0f, 3.0f, 6.1f }, { 1.0f, 0.3f, 0.1f }, { 0.8f, 0.25f, 0.12f }, 12, 0.4f, 0.1f },                         // celesta
        { 4, { 1.0f, 2.92f, 5.9f, 9.4f }, { 1.0f, 0.4f, 0.25f, 0.1f }, { 0.9f, 0.4f, 0.2f, 0.12f }, 24, 0.5f, 0.05f },     // music box
        { 3, { 1.0f, 6.2f, 16.0f }, { 1.0f, 0.25f, 0.06f }, { 0.75f, 0.14f, 0.05f }, 0, 0.7f, 0.05f },                     // kalimba
        { 4, { 1.0f, 2.0f, 3.0f, 4.02f }, { 1.0f, 0.6f, 0.3f, 0.15f }, { 0.7f, 0.5f, 0.35f, 0.18f }, 0, 0.5f, 0.1f },      // steel drum
        { 4, { 1.0f, 2.76f, 5.4f, 8.93f }, { 0.6f, 1.0f, 0.7f, 0.4f }, { 3.5f, 2.4f, 1.5f, 0.9f }, -12, 0.4f, 0.0f },      // tubular bells
        { 3, { 1.0f, 3.99f, 9.2f }, { 1.0f, 0.15f, 0.05f }, { 0.5f, 0.14f, 0.05f }, 0, 0.6f, 0.0f },                       // marimba
    };
    return types[(size_t) juce::jlimit (0, 7, index)];
}

struct MalletParams
{
    int type = 0;
    float brightness = 0.5f, decay = 0.5f, release = 0.5f, mallet = 0.5f, vibrato = 0.3f;
    float lfo = 0.0f;
};

class MalletVoice : public PolyVoice
{
public:
    explicit MalletVoice (const MalletParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        active = false;
        clickCoef = decayCoef (0.003f, fs);
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        const auto& t = malletType (p.type);
        const float f = midiToHz ((float) (midiNote + t.octave));
        const float decayScale = knobTime (p.decay, 0.3f, 3.0f) * std::exp2 (-(float) (midiNote - 60) / 30.0f);
        const float hard = 0.3f + p.mallet * 1.4f;

        count = 0;
        for (int m = 0; m < t.modes; ++m)
        {
            const float fm = f * t.ratios[m];
            if (fm > 0.45f * fs) break;
            auto& part = partials[(size_t) count++];
            part.dt = fm / fs;
            part.amp = t.amps[m] * (m == 0 ? 1.0f : hard * (0.5f + p.brightness));
            part.coef = decayCoef (t.decays[m] * decayScale, fs);
            part.phase = 0.0f;
        }
        clickEnv = t.click * (0.3f + p.mallet) * velocity;
        clickFilter.set (juce::jmin (f * 4.0f, 9000.0f), 1.2f, fs);
        damp = 1.0f;
        dampCoef = 1.0f;
        active = true;
    }

    void release() override { dampCoef = decayCoef (knobTime (p.release, 0.05f, 3.0f), fs); }
    bool isActive() const override { return active; }

    void process (float& left, float& right) override
    {
        float sum = 0.0f;
        for (int i = 0; i < count; ++i) sum += partials[(size_t) i].process();
        clickFilter.process (noise.next());
        sum += clickFilter.bp * clickEnv;
        clickEnv *= clickCoef;
        damp *= dampCoef;

        const float trem = 1.0f - p.vibrato * malletType (p.type).tremolo * 0.5f * (0.5f + 0.5f * p.lfo);
        const float y = sum * damp * gain * trem * 0.16f;
        left += y;
        right += y;
        if (damp < 0.0001f || (count > 0 && partials[0].amp < 0.00002f)) active = false;
    }

private:
    const MalletParams& p;
    std::array<Partial, 5> partials {};
    int count = 0;
    float fs = 44100.0f, gain = 1, clickEnv = 0, clickCoef = 0, damp = 1, dampCoef = 1;
    bool active = false;
    Svf clickFilter;
    Noise noise;
};

//==============================================================================
// Plucked strings (Karplus-Strong): a noise burst circulating in a tuned delay loop with a damping filter.

struct PluckType
{
    float decay, damping, pick, body, buzz, bend;
};

inline const PluckType& pluckType (int index)
{
    static const PluckType types[] {
        { 3.0f, 0.55f, 0.75f, 0.5f, 0.0f, 0.0f },    // acoustic guitar
        { 2.4f, 0.35f, 0.45f, 0.6f, 0.0f, 0.0f },    // nylon guitar
        { 0.09f, 0.45f, 0.7f, 0.3f, 0.0f, 0.0f },    // muted funk guitar
        { 4.5f, 0.3f, 0.35f, 0.4f, 0.0f, 0.0f },     // harp
        { 1.6f, 0.65f, 0.9f, 0.3f, 0.0f, 0.015f },   // koto
        { 3.0f, 0.7f, 0.85f, 0.3f, 0.6f, 0.0f },     // sitar
        { 0.3f, 0.35f, 0.5f, 0.8f, 0.0f, 0.0f },     // pizzicato strings
    };
    return types[(size_t) juce::jlimit (0, 6, index)];
}

struct PluckParams
{
    int type = 0;
    float brightness = 0.5f, decay = 0.5f, release = 0.3f, strum = 0.3f, body = 0.5f;
    int strumIndex = 0;   // position of the next voice in the chord being strummed
};

class PluckVoice : public PolyVoice
{
public:
    explicit PluckVoice (PluckParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        loop.allocate ((int) (fs / 20.0f) + 4);
        bodyLow.set (110.0f, 2.0f, fs);
        bodyHigh.set (240.0f, 2.0f, fs);
        active = false;
    }

    void start (int midiNote, float velocity) override
    {
        note = midiNote;
        gain = velocity;
        const auto& t = pluckType (p.type);
        freq = midiToHz ((float) midiNote);
        const float damping = juce::jlimit (0.05f, 0.98f, t.damping + (p.brightness - 0.5f) * 0.6f);
        loopCoef = 0.15f + 0.84f * damping;
        // The damping filter delays the loop by about (1 - a) / a samples: take it off the line length.
        period = juce::jlimit (2.0f, (float) loop.size - 3.0f, fs / freq - (1.0f - loopCoef) / loopCoef);
        decaySeconds = t.decay * knobTime (p.decay, 0.25f, 4.0f) * std::exp2 (-(float) (midiNote - 52) / 24.0f);
        loopGain = std::exp (-6.91f / (decaySeconds * freq));
        bend = t.bend;
        pickCoef = 0.1f + 0.85f * juce::jlimit (0.0f, 1.0f, t.pick + (p.brightness - 0.5f) * 0.5f);
        excite = (int) (fs / freq);
        waitSamples = (int) ((float) p.strumIndex++ * knobTime (p.strum, 0.002f, 0.06f) * fs * (p.strum > 0.02f ? 1.0f : 0.0f));
        for (auto& v : loop.buffer) v = 0.0f;
        lp = pickLp = dcIn = dcOut = 0.0f;
        level = 1.0f;
        active = true;
    }

    void release() override { loopGain = std::min (loopGain, std::exp (-6.91f / (knobTime (p.release, 0.03f, 1.5f) * freq))); }
    bool isActive() const override { return active; }

    void process (float& left, float& right) override
    {
        if (waitSamples > 0) { --waitSamples; return; }

        const auto& t = pluckType (p.type);
        float in = 0.0f;
        if (excite > 0)
        {
            pickLp += pickCoef * (noise.next() - pickLp);
            in = pickLp;
            --excite;
        }
        const float currentPeriod = bend > 0.0f ? period * (1.0f - bend) : period;
        bend *= 0.9997f;

        lp += loopCoef * (loop.read (currentPeriod) - lp);
        float y = in + loopGain * lp;
        if (t.buzz > 0.0f) y = (1.0f - t.buzz) * y + t.buzz * 0.5f * (fastTanh (2.0f * (y + 0.1f)) - fastTanh (0.2f));
        loop.write (y);

        dcOut = y - dcIn + 0.995f * dcOut;
        dcIn = y;
        bodyLow.process (dcOut);
        bodyHigh.process (dcOut);
        const float out = (dcOut + p.body * t.body * (bodyLow.bp + bodyHigh.bp) * 1.5f) * gain * 1.1f;
        left += out;
        right += out;

        level = std::max (std::abs (y), level * 0.9995f);
        if (excite <= 0 && level < 0.00005f) active = false;
    }

private:
    PluckParams& p;
    DelayLine loop;
    float fs = 44100.0f, freq = 220, gain = 1, period = 100, loopCoef = 0.5f, loopGain = 0.99f, decaySeconds = 1;
    float bend = 0, pickCoef = 0.5f, lp = 0, pickLp = 0, dcIn = 0, dcOut = 0, level = 0;
    int excite = 0, waitSamples = 0;
    bool active = false;
    Svf bodyLow, bodyHigh;
    Noise noise;
};

//==============================================================================
// Choir: three detuned, slightly wavering saws per note through the three formants of a vowel.

struct Vowel
{
    float freqs[3], amps[3], q[3];
};

inline const Vowel& vowel (int index)
{
    static const Vowel vowels[] {
        { { 800.0f, 1150.0f, 2900.0f }, { 1.0f, 0.5f, 0.25f }, { 8.0f, 10.0f, 18.0f } },   // aah
        { { 350.0f, 650.0f, 2700.0f }, { 1.0f, 0.3f, 0.06f }, { 6.0f, 8.0f, 18.0f } },     // ooh
        { { 500.0f, 1700.0f, 2600.0f }, { 1.0f, 0.4f, 0.25f }, { 7.0f, 12.0f, 16.0f } },   // eeh
        { { 280.0f, 900.0f, 2400.0f }, { 1.0f, 0.06f, 0.03f }, { 4.0f, 8.0f, 12.0f } },    // mmh
    };
    return vowels[(size_t) juce::jlimit (0, 3, index)];
}

struct ChoirParams
{
    int vowel = 0;
    float ensemble = 0.5f, brightness = 0.5f, breath = 0.2f;
};

class ChoirVoice : public PolyVoice
{
public:
    explicit ChoirVoice (const ChoirParams& shared) : p (shared) {}

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        adsr.reset();
        for (auto& f : formants) f.reset();
        lastVowel = -1;
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
        if (p.vowel != lastVowel || ++updateCounter >= 64)
        {
            updateCounter = 0;
            lastVowel = p.vowel;
            const auto& v = vowel (p.vowel);
            for (int i = 0; i < 3; ++i) formants[(size_t) i].set (v.freqs[i], v.q[i] * (0.6f + p.brightness * 0.8f), fs);
        }

        static constexpr float rates[3] { 5.1f, 5.7f, 4.6f };
        static constexpr float offsets[3] { 0.0f, 1.0f, -1.0f };
        float source = 0.0f;
        for (int i = 0; i < 3; ++i)
        {
            advance (vibrato[i], rates[i] / fs);
            const float detune = 1.0f + offsets[i] * p.ensemble * 0.008f + sine01 (vibrato[i]) * 0.0025f;
            const float dt = freq * detune / fs;
            advance (phases[i], dt);
            source += blepSaw (phases[i], dt);
        }
        source = source * 0.33f + noise.next() * p.breath * 0.35f;

        const auto& v = vowel (p.vowel);
        float y = 0.0f;
        for (int i = 0; i < 3; ++i)
        {
            formants[(size_t) i].process (source);
            y += formants[(size_t) i].bp * v.amps[i] * (i == 0 ? 1.0f : 0.5f + p.brightness);
        }
        y *= adsr.next() * gain * 0.25f;
        left += y;
        right += y;
    }

private:
    const ChoirParams& p;
    float fs = 44100.0f, freq = 220, gain = 1;
    float phases[3] { 0.0f, 0.33f, 0.66f }, vibrato[3] { 0.0f, 0.4f, 0.7f };
    int lastVowel = -1, updateCounter = 0;
    std::array<Svf, 3> formants;
    Adsr adsr;
    Noise noise;
};
} // namespace techno
