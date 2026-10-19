#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace techno
{
constexpr int maxSteps = 64;
constexpr int pageSteps = 16;
constexpr int maxDrumRows = 13;
constexpr int noteRange = 36;
constexpr int maxLockLanes = 8;      // settings of one machine that can change from step to step
constexpr float noLock = -1.0f;      // a step without a value: the knob rules

// Plain copy of a Track (for undo and copy / paste).
struct TrackData
{
    std::array<std::array<bool, maxSteps>, maxDrumRows> hits {};
    std::array<std::array<std::uint8_t, maxSteps>, maxDrumRows> rolls {};
    std::array<bool, maxSteps> on {}, accent {}, slide {};
    std::array<int, maxSteps> note {};
    std::array<std::uint64_t, maxSteps> chord {}, tie {};
    std::array<int, maxLockLanes> lockTarget {};                               // automation target, -1 = free lane
    std::array<std::array<float, maxSteps>, maxLockLanes> lockValue {};

    TrackData()
    {
        lockTarget.fill (-1);
        for (auto& lane : lockValue) lane.fill (noLock);
    }
};

// Sequencer data of one machine, shared between the UI (writes) and the audio thread (reads):
// every cell is atomic. Drum machines use hits + accent, synths use on/note/accent/slide
// (slide = glide for mono synths, hold the chord for poly synths).
// chord: notes written freely in the piano roll of a poly synth, one bit per semitone; 0 = chord built from note + chord type.
// tie: for those notes, the ones that go on sounding into the next step (each note has its own length).
// lockTarget / lockValue: automation. A lane holds one setting of the machine (see core/Automation.h) and a value
// per step; while a step has a value, it replaces the knob.
struct Track
{
    std::array<std::array<std::atomic<bool>, maxSteps>, maxDrumRows> hits;
    std::array<std::array<std::atomic<std::uint8_t>, maxSteps>, maxDrumRows> rolls;   // 0 = single hit, 2..4 = roll
    std::array<std::atomic<bool>, maxSteps> on, accent, slide;
    std::array<std::atomic<int>, maxSteps> note;
    std::array<std::atomic<std::uint64_t>, maxSteps> chord, tie;
    std::array<std::atomic<int>, maxLockLanes> lockTarget;
    std::array<std::array<std::atomic<float>, maxSteps>, maxLockLanes> lockValue;

    Track() { clear(); }

    void clear()
    {
        for (auto& row : hits)
            for (auto& cell : row) cell = false;
        for (auto& row : rolls)
            for (auto& cell : row) cell = 0;
        for (int i = 0; i < maxSteps; ++i)
        {
            on[(size_t) i] = accent[(size_t) i] = slide[(size_t) i] = false;
            note[(size_t) i] = 0;
            chord[(size_t) i] = 0;
            tie[(size_t) i] = 0;
        }
        clearLocks();
    }

    void clearLocks()
    {
        for (auto& t : lockTarget) t = -1;
        for (auto& lane : lockValue)
            for (auto& v : lane) v = noLock;
    }

    // The lane of a setting, or a free one for it (-1 when all 8 are used by other settings).
    int laneFor (int target, bool create)
    {
        for (int i = 0; i < maxLockLanes; ++i)
            if (lockTarget[(size_t) i].load() == target) return i;
        if (! create) return -1;
        for (int i = 0; i < maxLockLanes; ++i)
        {
            if (lockTarget[(size_t) i].load() >= 0) continue;
            for (auto& v : lockValue[(size_t) i]) v = noLock;
            lockTarget[(size_t) i] = target;
            return i;
        }
        return -1;
    }

    // Frees the lanes that no longer hold any value.
    void releaseEmptyLanes()
    {
        for (int i = 0; i < maxLockLanes; ++i)
        {
            bool any = false;
            for (auto& v : lockValue[(size_t) i]) any = any || v.load() >= 0.0f;
            if (! any) lockTarget[(size_t) i] = -1;
        }
    }

    TrackData snapshot() const
    {
        TrackData d;
        for (size_t r = 0; r < hits.size(); ++r)
            for (size_t s = 0; s < (size_t) maxSteps; ++s)
            {
                d.hits[r][s] = hits[r][s].load();
                d.rolls[r][s] = rolls[r][s].load();
            }
        for (size_t s = 0; s < (size_t) maxSteps; ++s)
        {
            d.on[s] = on[s].load();
            d.accent[s] = accent[s].load();
            d.slide[s] = slide[s].load();
            d.note[s] = note[s].load();
            d.chord[s] = chord[s].load();
            d.tie[s] = tie[s].load();
        }
        for (size_t l = 0; l < (size_t) maxLockLanes; ++l)
        {
            d.lockTarget[l] = lockTarget[l].load();
            for (size_t s = 0; s < (size_t) maxSteps; ++s) d.lockValue[l][s] = lockValue[l][s].load();
        }
        return d;
    }

    void restore (const TrackData& d)
    {
        for (size_t r = 0; r < hits.size(); ++r)
            for (size_t s = 0; s < (size_t) maxSteps; ++s)
            {
                hits[r][s] = d.hits[r][s];
                rolls[r][s] = d.rolls[r][s];
            }
        for (size_t s = 0; s < (size_t) maxSteps; ++s)
        {
            on[s] = d.on[s];
            accent[s] = d.accent[s];
            slide[s] = d.slide[s];
            note[s] = d.note[s];
            chord[s] = d.chord[s];
            tie[s] = d.tie[s];
        }
        restoreLocks (d);
    }

    void restoreLocks (const TrackData& d)
    {
        for (size_t l = 0; l < (size_t) maxLockLanes; ++l)
        {
            lockTarget[l] = d.lockTarget[l];
            for (size_t s = 0; s < (size_t) maxSteps; ++s) lockValue[l][s] = d.lockValue[l][s];
        }
    }

    bool isEmpty() const
    {
        for (size_t s = 0; s < (size_t) maxSteps; ++s)
        {
            if (on[s]) return false;
            for (auto& row : hits)
                if (row[s]) return false;
        }
        return true;
    }

    // Repeats the first `fromSteps` steps over the whole 64-step range.
    void tile (int fromSteps)
    {
        for (int i = fromSteps; i < maxSteps; ++i)
        {
            const auto src = (size_t) (i % fromSteps), dst = (size_t) i;
            for (auto& row : hits) row[dst] = row[src].load();
            for (auto& row : rolls) row[dst] = row[src].load();
            on[dst] = on[src].load();
            accent[dst] = accent[src].load();
            slide[dst] = slide[src].load();
            note[dst] = note[src].load();
            chord[dst] = chord[src].load();
            tie[dst] = tie[src].load();
            for (auto& lane : lockValue) lane[dst] = lane[src].load();
        }
    }
};
} // namespace techno
