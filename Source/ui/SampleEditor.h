#pragma once

#include "../PluginProcessor.h"
#include "../core/MidiMap.h"
#include "Widgets.h"

namespace techno
{
inline juce::String slotTitle (int slot)
{
    return slot == TechMachineProcessor::keysSlot ? utf8 ("SAMPLE KEYS") : utf8 ("PAD ") + juce::String (slot + 1);
}

// Full-screen editor of a sound: its waveform, where it starts and ends, and (SAMPLE KEYS) the part that loops.
class SampleEditor : public juce::Component, private juce::Timer
{
public:
    enum Handle { Start, End, LoopStart, LoopEnd, none };

    SampleEditor (TechMachineProcessor& p, int slotIndex) : processor (p), slot (slotIndex)
    {
        setWantsKeyboardFocus (true);
        auto button = [this] (IconButton& b, const char* tip, std::function<void()> action)
        {
            b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3b40));
            b.setColour (juce::TextButton::textColourOffId, colours::text);
            b.setTooltip (utf8 (tip));
            b.onClick = [this, action] { action(); grabKeyboardFocus(); };
            addAndMakeVisible (b);
        };
        button (listen, "Joue le son tel qu'il est réglé", [this] { preview(); });
        button (trim, "Coupe le silence au début et à la fin", [this] { trimSilence(); });
        button (whole, "Revient au son entier", [this]
        {
            if (const auto* d = processor.sampleData (slot)) commit (0, d->audio->getNumSamples() - 1, 0, d->audio->getNumSamples() - 1);
        });
        button (close, "Revenir au rack  (Échap)", [this] { if (onClose) onClose(); });
        close.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff6b46c1));
        startTimerHz (15);
    }

    std::function<void()> onClose;
    int sampleSlot() const { return slot; }
    bool showsLoop() const { return slot == TechMachineProcessor::keysSlot; }

    // Where a handle sits on screen (used by the tests).
    juce::Point<int> pointFor (Handle h) const
    {
        const auto* d = processor.sampleData (slot);
        if (d == nullptr) return {};
        const int positions[4] { d->start, d->end, d->loopStart, d->loopEnd };
        return { xFor (positions[h]), waveArea().getCentreY() };
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colour (0xff101113));
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
        const auto* d = processor.sampleData (slot);
        g.setColour (colours::text);
        g.setFont (paint::display (22.0f));
        g.drawText (utf8 ("SON · ") + slotTitle (slot), 16, 8, 260, 30, juce::Justification::centredLeft);
        g.setFont (paint::condensed (15.0f));
        g.setColour (colours::dim);
        g.drawFittedText (d != nullptr ? d->name : juce::String(), 280, 8, juce::jmax (40, listen.getX() - 290), 30, juce::Justification::centredLeft, 1, 0.8f);

        const auto area = waveArea();
        g.setColour (juce::Colour (0xff17181b));
        g.fillRect (area);
        if (d == nullptr)
        {
            g.setColour (colours::dim);
            g.setFont (paint::font (15.0f, false));
            g.drawFittedText (utf8 ("Pas encore de son. Glisse un fichier audio sur la machine, ou passe par le menu ⋯ "
                                    "(charger un son, enregistrer au micro)."), area.reduced (20), juce::Justification::centred, 3);
            return;
        }

        // The waveform: the part that plays is bright, the rest is dim.
        refreshPeaks (*d);
        const float mid = (float) area.getCentreY(), half = (float) area.getHeight() * 0.45f;
        for (int x = 0; x < (int) peaks.size(); ++x)
        {
            const int sample = sampleFor (area.getX() + x);
            const bool inside = sample >= d->start && sample <= d->end;
            g.setColour (inside ? juce::Colour (0xff5aa8ff) : juce::Colour (0xff3a4656));
            g.drawVerticalLine (area.getX() + x, mid - peaks[(size_t) x].second * half, mid - peaks[(size_t) x].first * half + 1.0f);
        }
        if (showsLoop())
        {
            const bool looping = processor.apvts.getRawParameterValue (pid::synth (SampleKeys, pKeys::Loop))->load() > 0.5f;
            g.setColour (juce::Colour (0xff3dff6a).withAlpha (looping ? 0.14f : 0.06f));
            g.fillRect (juce::Rectangle<int>::leftTopRightBottom (xFor (d->loopStart), area.getY(), xFor (d->loopEnd), area.getBottom()));
        }

        // Handles
        const char* const names[4] { "DÉBUT", "FIN", "BOUCLE", "BOUCLE" };
        const juce::Colour handleColours[4] { juce::Colour (0xff3dff6a), juce::Colour (0xffff5a4a), juce::Colour (0xff3dd6ff), juce::Colour (0xff3dd6ff) };
        const int positions[4] { d->start, d->end, d->loopStart, d->loopEnd };
        g.setFont (paint::condensed (12.0f));
        for (int h = 0; h < (showsLoop() ? 4 : 2); ++h)
        {
            const int x = xFor (positions[h]);
            g.setColour (handleColours[h]);
            g.fillRect (x - 1, area.getY(), 3, area.getHeight());
            const auto label = utf8 (names[h]) + "  " + juce::String ((double) positions[h] / d->sampleRate, 2) + " s";
            const int top = h < 2 ? area.getY() + 4 : area.getBottom() - 20;
            const bool right = h == End || h == LoopEnd;
            g.drawText (label, right ? x - 128 : x + 6, top, 122, 16, right ? juce::Justification::centredRight : juce::Justification::centredLeft);
        }

        g.setColour (colours::dim);
        g.setFont (paint::font (12.5f, false));
        g.drawFittedText (utf8 (showsLoop()
                                    ? "Glisse les traits : DÉBUT et FIN = la partie jouée, BOUCLE = la partie qui se répète quand BOUCLE est sur Oui · "
                                      "AUTO = coupe le silence"
                                    : "Glisse les traits : DÉBUT et FIN = la partie du son que joue le pad · AUTO = coupe le silence au début et à la fin"),
                          getLocalBounds().reduced (16, 0).removeFromBottom (24), juce::Justification::centredLeft, 1, 0.8f);
    }

    void resized() override
    {
        auto row = getLocalBounds().reduced (12, 0).withTrimmedTop (8).removeFromTop (30);
        close.setBounds (row.removeFromRight (110));
        row.removeFromRight (8);
        whole.setBounds (row.removeFromRight (110));
        row.removeFromRight (8);
        trim.setBounds (row.removeFromRight (90));
        row.removeFromRight (8);
        listen.setBounds (row.removeFromRight (110));
        peaksWidth = -1;
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        dragged = none;
        const auto* d = processor.sampleData (slot);
        if (d == nullptr || ! waveArea().expanded (0, 4).contains (e.getPosition())) return;
        const int positions[4] { d->start, d->end, d->loopStart, d->loopEnd };
        int best = 1 << 30;
        for (int h = 0; h < (showsLoop() ? 4 : 2); ++h)
        {
            // The loop handles answer in the lower half, the others in the upper half, when they sit at the same place.
            const int distance = std::abs (xFor (positions[h]) - e.x) + ((h >= 2) != (e.y > waveArea().getCentreY()) ? 6 : 0);
            if (distance < best) { best = distance; dragged = (Handle) h; }
        }
        draft = { d->start, d->end, d->loopStart, d->loopEnd };
        mouseDrag (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (dragged == none) return;
        const auto* d = processor.sampleData (slot);
        if (d == nullptr) return;
        const int last = d->audio->getNumSamples() - 1;
        const int s = juce::jlimit (0, last, sampleFor (e.x));
        auto& [start, end, loopStart, loopEnd] = draft;
        switch (dragged)
        {
            case Start:     start = juce::jmin (s, end - 64); break;
            case End:       end = juce::jmax (s, start + 64); break;
            case LoopStart: loopStart = juce::jmin (s, loopEnd - 64); break;
            case LoopEnd:   loopEnd = juce::jmax (s, loopStart + 64); break;
            case none:      break;
        }
        loopStart = juce::jlimit (start, end, loopStart);
        loopEnd = juce::jlimit (loopStart, end, loopEnd);
        commit (start, end, loopStart, loopEnd);
    }

    void mouseUp (const juce::MouseEvent&) override { dragged = none; }

    bool keyPressed (const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::escapeKey) { if (onClose) onClose(); return true; }
        if (key == juce::KeyPress::spaceKey) { preview(); return true; }
        return false;
    }

    // Cuts the silence (under 1 % of the loudest point) at both ends.
    void trimSilence()
    {
        const auto* d = processor.sampleData (slot);
        if (d == nullptr) return;
        const float* x = d->audio->getReadPointer (0);
        const int last = d->audio->getNumSamples() - 1;
        float peak = 0.0f;
        for (int i = 0; i <= last; ++i) peak = std::max (peak, std::abs (x[i]));
        if (peak <= 0.0f) return;
        int first = 0, lastLoud = last;
        while (first < last && std::abs (x[first]) < peak * 0.01f) ++first;
        while (lastLoud > first && std::abs (x[lastLoud]) < peak * 0.01f) --lastLoud;
        const int margin = (int) (d->sampleRate * 0.005);
        const int from = juce::jmax (0, first - margin), to = juce::jmin (last, lastLoud + margin * 4);
        commit (from, to, from, to);
    }

private:
    juce::Rectangle<int> waveArea() const { return getLocalBounds().reduced (16, 0).withTrimmedTop (56).withTrimmedBottom (34); }

    int xFor (int sample) const
    {
        const auto* d = processor.sampleData (slot);
        const auto area = waveArea();
        if (d == nullptr) return area.getX();
        return area.getX() + (int) ((double) sample / (double) juce::jmax (1, d->audio->getNumSamples() - 1) * (area.getWidth() - 1));
    }

    int sampleFor (int x) const
    {
        const auto* d = processor.sampleData (slot);
        const auto area = waveArea();
        if (d == nullptr) return 0;
        return (int) ((double) (x - area.getX()) / (double) juce::jmax (1, area.getWidth() - 1) * (d->audio->getNumSamples() - 1));
    }

    void refreshPeaks (const SampleData& d)
    {
        const int width = waveArea().getWidth();
        if (d.audio.get() == peaksOf && width == peaksWidth) return;
        peaksOf = d.audio.get();
        peaksWidth = width;
        peaks.assign ((size_t) juce::jmax (0, width), { 0.0f, 0.0f });
        const float* x = d.audio->getReadPointer (0);
        const int n = d.audio->getNumSamples();
        float loudest = 0.0f;
        for (int i = 0; i < n; ++i) loudest = std::max (loudest, std::abs (x[i]));
        const float scale = loudest > 0.0f ? 1.0f / loudest : 1.0f;
        for (int c = 0; c < width; ++c)
        {
            const int from = (int) ((juce::int64) c * n / width), to = juce::jmax (from + 1, (int) ((juce::int64) (c + 1) * n / width));
            float lo = 0.0f, hi = 0.0f;
            for (int i = from; i < to && i < n; ++i) { lo = std::min (lo, x[i]); hi = std::max (hi, x[i]); }
            peaks[(size_t) c] = { lo * scale, hi * scale };
        }
    }

    void commit (int start, int end, int loopStart, int loopEnd)
    {
        processor.setSampleRange (slot, start, end, loopStart, loopEnd);
        repaint();
    }

    void preview()
    {
        const bool keys = showsLoop();
        const int type = keys ? SampleKeys : Sampler;
        const int note = keys ? keysLowestRoot + (int) processor.apvts.getRawParameterValue (pid::synth (SampleKeys, pKeys::Root))->load()
                              : gmNoteForRow (machineDef (Sampler), slot);
        processor.previewNote (type, note, true);
        juce::Component::SafePointer<SampleEditor> safe (this);
        juce::Timer::callAfterDelay (1500, [safe, type, note] { if (safe != nullptr) safe->processor.previewNote (type, note, false); });
    }

    void timerCallback() override
    {
        if (processor.sampleVersion() != knownVersion)
        {
            knownVersion = processor.sampleVersion();
            repaint();
        }
    }

    TechMachineProcessor& processor;
    const int slot;
    Handle dragged = none;
    std::tuple<int, int, int, int> draft;
    std::vector<std::pair<float, float>> peaks;
    const void* peaksOf = nullptr;
    int peaksWidth = -1, knownVersion = -1;
    IconButton listen { "play", utf8 ("ÉCOUTER") }, trim { "", "AUTO" }, whole { "undo", utf8 ("SON ENTIER") }, close { "close", utf8 ("FERMER") };
};

// Recording from the microphone into a pad or SAMPLE KEYS, in a bubble.
class MicRecorder : public juce::Component, private juce::Timer
{
public:
    MicRecorder (TechMachineProcessor& p, int slotIndex) : processor (p), slot (slotIndex)
    {
        record.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffb8322a));
        record.setColour (juce::TextButton::textColourOffId, colours::text);
        record.onClick = [this] { toggle(); };
        addAndMakeVisible (record);
        setSize (420, 172);
        startTimerHz (20);
    }

    ~MicRecorder() override
    {
        if (processor.isSampling()) processor.stopSampling();
    }

    void toggle()
    {
        if (processor.isSampling())
        {
            const auto file = processor.stopSampling();
            message = file.existsAsFile() ? utf8 ("Enregistré et chargé : ") + file.getFileNameWithoutExtension()
                                          : utf8 ("Rien n'a été entendu : rien n'est chargé.");
        }
        else if (! processor.startSampling (slot))
        {
            message = utf8 ("Pas d'entrée audio. Dans l'application : AUDIO / MIDI › choisis ton micro. "
                            "Dans un logiciel de musique : active l'entrée (sidechain) de Tech Machine.");
        }
        else
        {
            message = {};
        }
        record.setButtonText (processor.isSampling() ? utf8 ("ARRÊTER") : utf8 ("ENREGISTRER"));
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff1c1d21));
        g.setColour (colours::text);
        g.setFont (paint::font (15.0f));
        g.drawText (utf8 ("MICRO › ") + slotTitle (slot), 14, 8, getWidth() - 28, 20, juce::Justification::centredLeft);

        const auto meter = juce::Rectangle<float> (14.0f, 70.0f, (float) getWidth() - 28.0f, 12.0f);
        g.setColour (juce::Colour (0xff2c2d32));
        g.fillRoundedRectangle (meter, 3.0f);
        const float level = juce::jlimit (0.0f, 1.0f, std::sqrt (shownLevel));
        g.setColour (level > 0.9f ? juce::Colour (0xffff5a4a) : juce::Colour (0xff3dff6a));
        g.fillRoundedRectangle (meter.withWidth (meter.getWidth() * level), 3.0f);

        g.setColour (colours::dim);
        g.setFont (paint::font (13.0f, false));
        g.drawText (juce::String (processor.samplingSeconds(), 1) + " s / 30 s", 160, 36, getWidth() - 174, 24, juce::Justification::centredRight);
        juce::String text = message;
        if (text.isEmpty())
            text = processor.isSampling() && silentFor > 30
                       ? utf8 ("Aucun son n'arrive : vérifie le micro dans AUDIO / MIDI et décoche « Mute audio input ».")
                       : utf8 ("Parle, chante ou joue près du micro. Le silence du début est coupé, le niveau est ajusté.");
        g.drawFittedText (text, 14, 90, getWidth() - 28, 72, juce::Justification::topLeft, 4, 0.9f);
    }

    void resized() override { record.setBounds (14, 34, 140, 28); }

private:
    void timerCallback() override
    {
        shownLevel = std::max (processor.samplingLevel(), shownLevel * 0.8f);
        silentFor = processor.isSampling() && processor.samplingLevel() < 0.002f ? silentFor + 1 : 0;
        if (processor.isSampling() && processor.samplingSeconds() >= 29.9) toggle();
        repaint();
    }

    TechMachineProcessor& processor;
    const int slot;
    juce::TextButton record { utf8 ("ENREGISTRER") };
    juce::String message;
    float shownLevel = 0.0f;
    int silentFor = 0;
};
} // namespace techno
