#include "../PluginProcessor.h"

using namespace techno;

// Parts (A to H) and the song that plays them in order. The part being played lives in the machines' tracks;
// the others wait in partStore. Both are atomic, so the audio thread can switch at the start of a loop.

Track& TechMachineProcessor::partTrack (int part, int type)
{
    return part == activePart.load() ? machines[(size_t) type]->track : (*partStore)[(size_t) part][(size_t) type];
}

const Track& TechMachineProcessor::partTrack (int part, int type) const
{
    return part == activePart.load() ? machines[(size_t) type]->track : (*partStore)[(size_t) part][(size_t) type];
}

void TechMachineProcessor::switchPart (int part)
{
    const int old = activePart.load();
    if (part == old || part < 0 || part >= numParts) return;
    for (int t = 0; t < numMachines; ++t)
    {
        (*partStore)[(size_t) old][(size_t) t].copyFrom (machines[(size_t) t]->track);
        machines[(size_t) t]->track.copyFrom ((*partStore)[(size_t) part][(size_t) t]);
    }
    activePart = part;
    ++patternVersion;
}

void TechMachineProcessor::selectPart (int part)
{
    if (part < 0 || part >= numParts) return;
    if (playing.load() && ! songMode.load())
    {
        cuedPart = part == activePart.load() ? -1 : part;   // waits for the end of the loop
        return;
    }
    switchPart (part);
}

void TechMachineProcessor::startOfLoop()
{
    if (songMode.load() && songLength.load() > 0)
    {
        int entry = songEntry.load();
        if (entry < 0 || --repeatsLeft <= 0)
        {
            entry = entry < 0 ? 0 : (entry + 1) % songLength.load();
            repeatsLeft = juce::jmax (1, songRepeats[(size_t) entry].load());
            songEntry = entry;
            switchPart (songParts[(size_t) entry].load());
        }
        return;
    }
    if (const int cued = cuedPart.exchange (-1); cued >= 0) switchPart (cued);
}

bool TechMachineProcessor::isPartEmpty (int part) const
{
    for (int t = 0; t < numMachines; ++t)
        if (! partTrack (part, t).isEmpty()) return false;
    return true;
}

void TechMachineProcessor::copyPart (int from, int to)
{
    if (from == to) return;
    undoManager.beginNewTransaction();
    beginPartEdit (to);
    for (int t = 0; t < numMachines; ++t) partTrack (to, t).copyFrom (partTrack (from, t));
    touchPattern();
}

void TechMachineProcessor::clearPart (int part)
{
    undoManager.beginNewTransaction();
    beginPartEdit (part);
    for (int t = 0; t < numMachines; ++t) partTrack (part, t).clear();
    touchPattern();
}

std::vector<TechMachineProcessor::SongEntry> TechMachineProcessor::song() const
{
    std::vector<SongEntry> entries;
    for (int i = 0; i < songLength.load(); ++i)
        entries.push_back ({ songParts[(size_t) i].load(), songRepeats[(size_t) i].load() });
    return entries;
}

void TechMachineProcessor::setSong (const std::vector<SongEntry>& entries)
{
    const int count = juce::jmin ((int) entries.size(), maxSongEntries);
    songLength = 0;   // the audio thread never reads a half-written list
    for (int i = 0; i < count; ++i)
    {
        songParts[(size_t) i] = juce::jlimit (0, numParts - 1, entries[(size_t) i].part);
        songRepeats[(size_t) i] = juce::jlimit (1, 64, entries[(size_t) i].repeats);
    }
    songLength = count;
    if (songEntry.load() >= count) songEntry = -1;
    dirty = true;
}

void TechMachineProcessor::setSongMode (bool on)
{
    songMode = on;
    songEntry = -1;
    cuedPart = -1;
    if (on && ! playing.load() && songLength.load() > 0) switchPart (songParts[0].load());
    dirty = true;
}

double TechMachineProcessor::songSeconds() const
{
    int loops = 0;
    for (int i = 0; i < songLength.load(); ++i) loops += songRepeats[(size_t) i].load();
    return (double) loops * (double) numSteps() * 15.0 / (double) bpm->load();
}
