#include "MidiLearn.h"
#include "Language.h"

namespace techno
{
MidiLearn::MidiLearn (juce::AudioProcessorValueTreeState& s) : state (s)
{
    load();
}

void MidiLearn::start (const juce::String& paramId) { learnTarget = state.getParameter (paramId); }

void MidiLearn::forget (const juce::String& paramId)
{
    auto* p = state.getParameter (paramId);
    for (auto& c : ccMap)
        if (c.load() == p) c = nullptr;
    ++version;
}

int MidiLearn::controllerFor (const juce::String& paramId) const
{
    auto* p = state.getParameter (paramId);
    for (int cc = 0; cc < 128; ++cc)
        if (p != nullptr && ccMap[(size_t) cc].load() == p) return cc;
    return -1;
}

bool MidiLearn::handleController (int cc, int value)
{
    if (auto* learn = learnTarget.exchange (nullptr))
    {
        for (auto& c : ccMap)
            if (c.load() == learn) c = nullptr;
        ccMap[(size_t) cc] = learn;
        ++version;
    }
    if (auto* p = ccMap[(size_t) cc].load())
    {
        p->setValueNotifyingHost ((float) value / 127.0f);
        return true;
    }
    return false;
}

void MidiLearn::save()
{
    juce::StringArray entries;
    for (int cc = 0; cc < 128; ++cc)
        if (auto* p = ccMap[(size_t) cc].load())
            entries.add (juce::String (cc) + "=" + p->getParameterID());
    settings().setValue ("midiMap", entries.joinIntoString (";"));
    settings().saveIfNeeded();
}

void MidiLearn::load()
{
    for (auto& entry : juce::StringArray::fromTokens (settings().getValue ("midiMap"), ";", ""))
    {
        const int cc = entry.upToFirstOccurrenceOf ("=", false, false).getIntValue();
        if (cc >= 0 && cc < 128)
            ccMap[(size_t) cc] = state.getParameter (entry.fromFirstOccurrenceOf ("=", false, false));
    }
}
} // namespace techno
