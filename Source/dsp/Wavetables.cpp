#include "Wavetables.h"

#include <array>
#include <mutex>

namespace techno
{
namespace
{
constexpr int maxHarmonics = 64;
constexpr int mipHarmonics[WavetableSet::mips] { 64, 16, 5 };

float lerp (float a, float b, float t) { return a + (b - a) * t; }

// Harmonic amplitudes (index 1..64) of one frame, t = position in the table from 0 to 1.
void spectrum (int table, float t, std::array<float, maxHarmonics + 1>& a)
{
    a.fill (0.0f);
    switch (table)
    {
        case 0:   // PPG: a resonant bump that climbs through the harmonics
        {
            const float centre = 1.0f + t * 30.0f, width = 2.0f + t * 4.0f;
            for (int k = 1; k <= maxHarmonics; ++k)
            {
                const float d = ((float) k - centre) / width;
                a[(size_t) k] = std::pow (1.0f / (float) k, 0.8f) * (0.3f + 2.0f * std::exp (-0.5f * d * d)) * (k % 2 == 0 ? -1.0f : 1.0f);
            }
            break;
        }
        case 1:   // Microwave: pulse that gets narrower
        {
            const float w = 0.5f - t * 0.46f;
            for (int k = 1; k <= maxHarmonics; ++k) a[(size_t) k] = 2.0f / ((float) k * pi) * std::sin ((float) k * pi * w);
            break;
        }
        case 2:   // Vowels a, e, i, o, u
        {
            static constexpr float vowels[5][3] { { 800, 1250, 2600 }, { 450, 1900, 2550 }, { 300, 2300, 3000 }, { 500, 850, 2500 }, { 320, 750, 2300 } };
            const float v = t * 4.0f;
            const int i0 = std::min ((int) v, 3);
            const float frac = v - (float) i0;
            float f[3];
            for (int i = 0; i < 3; ++i) f[i] = lerp (vowels[i0][i], vowels[i0 + 1][i], frac);
            static constexpr float bandwidth[3] { 90.0f, 130.0f, 170.0f }, gains[3] { 1.0f, 0.6f, 0.3f };
            for (int k = 1; k <= maxHarmonics; ++k)
            {
                const float hz = (float) k * 130.0f;
                float sum = 0.04f / (float) k;
                for (int i = 0; i < 3; ++i)
                {
                    const float d = (hz - f[i]) / bandwidth[i];
                    sum += gains[i] / (1.0f + d * d);
                }
                a[(size_t) k] = sum;
            }
            break;
        }
        case 3:   // Bells and metal: sparse high harmonics
        {
            static constexpr int partials[] { 1, 3, 5, 9, 13, 17, 23, 29, 37, 43, 53, 61 };
            int n = 0;
            for (int k : partials)
            {
                const float emphasis = std::exp (-std::abs ((float) n / 11.0f - t) * 3.0f);
                a[(size_t) k] = (0.25f + emphasis) / std::sqrt ((float) k);
                ++n;
            }
            break;
        }
        case 4:   // Vector synthesis: sine -> saw -> square -> digital
        {
            std::array<float, maxHarmonics + 1> corners[4] {};
            for (auto& c : corners) c.fill (0.0f);
            corners[0][1] = 1.0f;
            for (int k = 1; k <= maxHarmonics; ++k)
            {
                corners[1][(size_t) k] = 1.0f / (float) k;
                corners[2][(size_t) k] = k % 2 == 1 ? 1.0f / (float) k : 0.0f;
                if ((k & (k - 1)) == 0) corners[3][(size_t) k] = 1.0f / (1.0f + std::log2 ((float) k));
            }
            const float v = t * 3.0f;
            const int i0 = std::min ((int) v, 2);
            const float frac = v - (float) i0;
            for (int k = 1; k <= maxHarmonics; ++k) a[(size_t) k] = lerp (corners[i0][(size_t) k], corners[i0 + 1][(size_t) k], frac);
            break;
        }
        case 5:   // Wave sequence: 8 distinct waves
        {
            const int wave = std::min ((int) (t * (float) sequenceWaves), sequenceWaves - 1);
            for (int k = 1; k <= maxHarmonics; ++k)
            {
                const float inv = 1.0f / (float) k;
                switch (wave)
                {
                    case 0: a[(size_t) k] = k == 1 ? 1.0f : 0.0f; break;
                    case 1: a[(size_t) k] = inv; break;
                    case 2: a[(size_t) k] = k % 2 == 1 ? inv : 0.0f; break;
                    case 3: a[(size_t) k] = (k == 1 || k == 2 || k == 3 || k == 4 || k == 6 || k == 8) ? 0.8f / std::sqrt ((float) k) : 0.0f; break;
                    case 4: a[(size_t) k] = (k % 2 == 1 && k > 1) ? inv * 2.0f : (k == 1 ? 0.3f : 0.0f); break;
                    case 5: a[(size_t) k] = inv * (1.0f + 3.0f * std::exp (-0.5f * std::pow (((float) k - 6.0f) / 2.0f, 2.0f))); break;
                    case 6: a[(size_t) k] = 2.0f / ((float) k * pi) * std::sin ((float) k * pi * 0.25f); break;
                    default: a[(size_t) k] = (k == 1 || k == 5 || k == 11 || k == 19 || k == 29) ? 1.0f / std::sqrt ((float) k) : 0.0f; break;
                }
            }
            break;
        }
        case 6:   // Analog: saw -> square
            for (int k = 1; k <= maxHarmonics; ++k)
                a[(size_t) k] = lerp (1.0f / (float) k, k % 2 == 1 ? 1.0f / (float) k : 0.0f, t);
            break;
        default:  // Digital organ: drawbar settings one after another
        {
            static constexpr float bars[4][9] {
                { 1, 1, 1, 0, 0, 0, 0, 0, 0 }, { 1, 0, 1, 0, 1, 0, 0, 1, 0 }, { 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0.5f, 1, 0, 0, 0, 1, 0, 0, 1 } };
            static constexpr int harmonic[9] { 1, 2, 3, 4, 6, 8, 10, 12, 16 };
            const float v = t * 3.0f;
            const int i0 = std::min ((int) v, 2);
            const float frac = v - (float) i0;
            for (int b = 0; b < 9; ++b) a[(size_t) harmonic[b]] += lerp (bars[i0][b], bars[i0 + 1][b], frac) * 0.6f;
            break;
        }
    }
}

WavetableSet build (int table)
{
    WavetableSet set;
    constexpr int n = WavetableSet::size;
    set.data.assign ((size_t) WavetableSet::frames * WavetableSet::mips * n, 0.0f);
    std::vector<float> sines ((size_t) n);
    for (int i = 0; i < n; ++i) sines[(size_t) i] = std::sin (2.0f * pi * (float) i / (float) n);

    std::array<float, maxHarmonics + 1> a {};
    for (int f = 0; f < WavetableSet::frames; ++f)
    {
        spectrum (table, (float) f / (float) (WavetableSet::frames - 1), a);
        float peak = 0.0f;
        for (int m = 0; m < WavetableSet::mips; ++m)
        {
            auto* out = set.wave (f, m);
            for (int k = 1; k <= mipHarmonics[m]; ++k)
            {
                const float amp = a[(size_t) k];
                if (amp == 0.0f) continue;
                for (int i = 0; i < n; ++i) out[i] += amp * sines[(size_t) ((k * i) % n)];
            }
            if (m == 0)
                for (int i = 0; i < n; ++i) peak = std::max (peak, std::abs (out[i]));
        }
        // Every frame at the same peak level, so moving through the table does not change the volume.
        const float scale = peak > 0.0f ? 1.0f / peak : 1.0f;
        for (int m = 0; m < WavetableSet::mips; ++m)
        {
            auto* out = set.wave (f, m);
            for (int i = 0; i < n; ++i) out[i] *= scale;
        }
    }
    return set;
}
} // namespace

const WavetableSet& wavetable (int index)
{
    static std::array<WavetableSet, numWavetables> sets;
    static std::once_flag once;
    std::call_once (once, [] { for (int i = 0; i < numWavetables; ++i) sets[(size_t) i] = build (i); });
    return sets[(size_t) juce::jlimit (0, numWavetables - 1, index)];
}
} // namespace techno
