#pragma once

#include "SynthVoices.h"

namespace techno
{
// A table of single-cycle waves, built by additive synthesis. Each frame is stored at three harmonic
// limits so high notes do not alias.
struct WavetableSet
{
    static constexpr int frames = 32, size = 1024, mips = 3;
    std::vector<float> data;   // [frame][mip][sample]

    const float* wave (int frame, int mip) const { return data.data() + ((size_t) frame * mips + (size_t) mip) * size; }
    float* wave (int frame, int mip) { return data.data() + ((size_t) frame * mips + (size_t) mip) * size; }
};

constexpr int numWavetables = 8;
constexpr int sequenceWaves = 8;   // the wave-sequence table holds 8 distinct waves, 4 frames each

// Built once, the first time it is asked for (never on the audio thread: machines ask for them when created).
const WavetableSet& wavetable (int index);

struct WaveParams
{
    int table = 0;
    float position = 0.3f, sweep = 0.4f, motion = 0.2f, cutoff = 0.8f, reso = 0.1f, decaySeconds = 1.0f;
};

class WavetableVoice : public PolyVoice
{
public:
    explicit WavetableVoice (const WaveParams& shared) : p (shared)
    {
        for (int i = 0; i < numWavetables; ++i) tables[i] = &wavetable (i);
    }

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
        freq = noteFreq = midiToHz ((float) midiNote);
        sweepEnv = 1.0f;
        sweepCoef = decayCoef (p.decaySeconds * 0.6f, fs);
        sequenceClock = 0.0f;
        adsr.gateOn();
    }

    void release() override { adsr.gateOff(); }
    bool isActive() const override { return adsr.isActive(); }

    void process (float& left, float& right) override
    {
        const auto& table = *tables[juce::jlimit (0, numWavetables - 1, p.table)];
        const float dt = freq / fs;
        advance (phase, dt);
        const float maxHarmonic = 0.5f / dt;
        const int mip = maxHarmonic >= 64.0f ? 0 : (maxHarmonic >= 16.0f ? 1 : 2);

        sweepEnv *= sweepCoef;
        float x;
        if (p.table == 5)
        {
            // Wave sequence: the waves follow one another in rhythm, with a short crossfade.
            sequenceClock += (1.0f + p.motion * 15.0f) / fs;
            const float step = sequenceClock + p.position * (float) sequenceWaves;
            const int current = (int) step % sequenceWaves, next = (current + 1) % sequenceWaves;
            const float frac = step - std::floor (step);
            const float blend = juce::jlimit (0.0f, 1.0f, (frac - 0.85f) / 0.15f);
            x = read (table.wave (current * 4, mip)) * (1.0f - blend) + read (table.wave (next * 4, mip)) * blend;
        }
        else
        {
            advance (lfoPhase, (0.05f + p.motion * 2.0f) / fs);
            const float position = juce::jlimit (0.0f, 1.0f, p.position + p.sweep * sweepEnv * 0.6f + p.motion * 0.2f * sine (lfoPhase));
            const float frame = position * (float) (WavetableSet::frames - 1);
            const int f0 = juce::jmin ((int) frame, WavetableSet::frames - 2);
            const float frac = frame - (float) f0;
            x = read (table.wave (f0, mip)) * (1.0f - frac) + read (table.wave (f0 + 1, mip)) * frac;
        }

        const float e = adsr.next();
        const float octaves = p.cutoff * 9.5f + e * 1.0f + std::log2 (freq / 261.6f) * 0.3f;
        filter.set (juce::jmin (30.0f * std::exp2 (octaves), 17000.0f), 0.7f + p.reso * 6.0f, fs);
        filter.process (x);
        const float y = filter.lp * e * gain * 0.76f;
        left += y;
        right += y;
    }

private:
    static float sine (float ph) { return std::sin (2.0f * pi * ph); }

    float read (const float* wave) const
    {
        const float index = phase * (float) WavetableSet::size;
        const int i0 = (int) index & (WavetableSet::size - 1);
        const int i1 = (i0 + 1) & (WavetableSet::size - 1);
        const float frac = index - std::floor (index);
        return wave[i0] + frac * (wave[i1] - wave[i0]);
    }

    const WaveParams& p;
    const WavetableSet* tables[numWavetables] {};
    float fs = 44100.0f, gain = 1, phase = 0, lfoPhase = 0, sweepEnv = 0, sweepCoef = 0.999f, sequenceClock = 0;
    Adsr adsr;
    Svf filter;
};
} // namespace techno
