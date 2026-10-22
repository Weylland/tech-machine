#pragma once

#include "MachineDefs.h"
#include "Track.h"
#include <juce_core/juce_core.h>
#include <vector>

namespace techno
{
struct ExampleInfo
{
    const char* name;
    const char* genre;
    const char* description;
    int bpm;
};

const std::vector<ExampleInfo>& examples();

// What an example needs from the project it is written into.
class ExampleWriter
{
public:
    virtual ~ExampleWriter() = default;
    virtual void setParam (const juce::String& id, float value) = 0;
    virtual void show (int machineType) = 0;
    virtual Track& track (int machineType) = 0;   // in the part chosen with part()
    virtual int baseNote (int machineType) = 0;   // MIDI note of semitone 0 for this machine
    virtual void part (int index) = 0;            // the part (0 = A ... 7 = H) that track() writes to
    virtual void song (const std::vector<std::pair<int, int>>& entries) = 0;   // parts in order with their repeats, song mode on
};

void buildExample (int index, ExampleWriter&);
} // namespace techno
