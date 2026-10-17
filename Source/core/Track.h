#pragma once

#include <array>
#include <atomic>

namespace techno
{
constexpr int maxSteps = 64;
constexpr int pageSteps = 16;
constexpr int maxDrumRows = 13;
constexpr int noteRange = 36;

// Sequencer data of one machine, shared between the UI (writes) and the audio thread (reads):
// every cell is atomic. Drum machines use hits + accent, synths use on/note/accent/slide
// (slide = glide for mono synths, hold the chord for poly synths).
struct Track
{
    std::array<std::array<std::atomic<bool>, maxSteps>, maxDrumRows> hits;
    std::array<std::atomic<bool>, maxSteps> on, accent, slide;
    std::array<std::atomic<int>, maxSteps> note;

    Track() { clear(); }

    void clear()
    {
        for (auto& row : hits)
            for (auto& cell : row) cell = false;
        for (int i = 0; i < maxSteps; ++i)
        {
            on[(size_t) i] = accent[(size_t) i] = slide[(size_t) i] = false;
            note[(size_t) i] = 0;
        }
    }

    // Repeats the first `fromSteps` steps over the whole 64-step range.
    void tile (int fromSteps)
    {
        for (int i = fromSteps; i < maxSteps; ++i)
        {
            const auto src = (size_t) (i % fromSteps), dst = (size_t) i;
            for (auto& row : hits) row[dst] = row[src].load();
            on[dst] = on[src].load();
            accent[dst] = accent[src].load();
            slide[dst] = slide[src].load();
            note[dst] = note[src].load();
        }
    }
};
} // namespace techno
