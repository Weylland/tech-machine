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
    virtual Track& track (int machineType) = 0;
    virtual int baseNote (int machineType) = 0;   // MIDI note of semitone 0 for this machine
};

void buildExample (int index, ExampleWriter&);
} // namespace techno
