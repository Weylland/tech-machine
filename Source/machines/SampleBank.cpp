#include "SampleBank.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <map>

namespace techno
{
SampleBank::Audio SampleBank::read (const juce::File& source, double& sampleRate)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (source));
    if (reader == nullptr || reader->lengthInSamples <= 0) return nullptr;

    // Up to 30 seconds, mixed down to mono.
    const int length = (int) juce::jmin<juce::int64> (reader->lengthInSamples, (juce::int64) (reader->sampleRate * 30.0));
    juce::AudioBuffer<float> stereo ((int) juce::jmax (1u, reader->numChannels), length);
    reader->read (&stereo, 0, length, 0, true, true);
    auto mono = std::make_shared<juce::AudioBuffer<float>> (1, length + 1);
    mono->clear();
    for (int ch = 0; ch < stereo.getNumChannels(); ++ch)
        mono->addFrom (0, 0, stereo, ch, 0, length, 1.0f / (float) stereo.getNumChannels());
    sampleRate = reader->sampleRate;
    return mono;
}

void SampleBank::set (int pad, Audio audio, double rate, int start, int end, const juce::String& padName, const juce::String& path,
                      int loopStart, int loopEnd)
{
    auto data = std::make_unique<SampleData>();
    data->audio = std::move (audio);
    data->sampleRate = rate;
    data->start = start;
    data->end = end;
    data->loopStart = loopStart >= 0 ? juce::jlimit (start, end, loopStart) : start;
    data->loopEnd = loopEnd >= 0 ? juce::jlimit (data->loopStart, end, loopEnd) : end;
    data->name = padName;
    data->path = path;
    slots[(size_t) pad] = data.get();
    store.push_back (std::move (data));
    ++version;
}

bool SampleBank::load (int pad, const juce::File& source)
{
    if (pad < 0 || pad >= numSlots) return false;
    double rate = 44100.0;
    auto audio = read (source, rate);
    if (audio == nullptr) return false;
    const int end = audio->getNumSamples() - 1;
    set (pad, std::move (audio), rate, 0, end, source.getFileNameWithoutExtension(), source.getFullPathName());
    return true;
}

bool SampleBank::chop (const juce::File& loop)
{
    double rate = 44100.0;
    auto audio = read (loop, rate);
    if (audio == nullptr) return false;
    const int length = audio->getNumSamples() - 1;
    for (int i = 0; i < numPads; ++i)
        set (i, audio, rate, length * i / numPads, length * (i + 1) / numPads,
             loop.getFileNameWithoutExtension() + " " + juce::String (i + 1), loop.getFullPathName());
    return true;
}

void SampleBank::clear()
{
    for (auto& s : slots) s = nullptr;
    ++version;
}

void SampleBank::clearPads()
{
    for (int i = 0; i < numPads; ++i) slots[(size_t) i] = nullptr;
    ++version;
}

// A new start, end and loop on the same sound (the audio is shared, not copied).
void SampleBank::setRange (int slot, int start, int end, int loopStart, int loopEnd)
{
    const auto* old = data (slot);
    if (old == nullptr) return;
    const int last = old->audio->getNumSamples() - 1;
    start = juce::jlimit (0, last - 1, start);
    end = juce::jlimit (start + 1, last, end);
    set (slot, old->audio, old->sampleRate, start, end, old->name, old->path, loopStart, loopEnd);
}

juce::String SampleBank::name (int pad) const
{
    if (auto* d = slots[(size_t) pad].load()) return d->name;
    return {};
}

juce::String SampleBank::describe (int pad) const
{
    if (auto* d = slots[(size_t) pad].load())
        return d->path + "|" + juce::String (d->start) + "|" + juce::String (d->end) + "|" + d->name
             + "|" + juce::String (d->loopStart) + "|" + juce::String (d->loopEnd);
    return {};
}

void SampleBank::restore (const juce::StringArray& descriptions)
{
    clear();

    // Each file is read once, even when its slices sit on several pads.
    std::map<juce::String, std::pair<Audio, double>> cache;
    for (int i = 0; i < numSlots && i < descriptions.size(); ++i)
    {
        const auto parts = juce::StringArray::fromTokens (descriptions[i], "|", "");
        if (parts.size() < 4) continue;
        auto& cached = cache[parts[0]];
        if (cached.first == nullptr) cached.first = read (juce::File (parts[0]), cached.second);
        if (cached.first == nullptr) continue;
        const int last = cached.first->getNumSamples() - 1;
        set (i, cached.first, cached.second, juce::jlimit (0, last, parts[1].getIntValue()),
             juce::jlimit (0, last, parts[2].getIntValue()), parts[3], parts[0],
             parts.size() >= 6 ? parts[4].getIntValue() : -1, parts.size() >= 6 ? parts[5].getIntValue() : -1);
    }
}
} // namespace techno
