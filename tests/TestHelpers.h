#pragma once

// Shared helpers of the test program: offline renders, fingerprints, interface snapshots and audits.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include "dsp/Reverb.h"
#include "dsp/DrumVoices.h"
#include "core/Language.h"
#include "ui/MachinePanel.h"
#include <iostream>

using namespace techno;
static const juce::File outDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();

struct Stats
{
    float peak = 0;
    double rmsDb = -120;
    int nans = 0;
    double cpu = 0;
};

static Stats run (TechMachineProcessor& p, double seconds, const juce::String& wavName = {}, double rate = 48000.0)
{
    const int bs = 512;
    p.setPlayConfigDetails (0, 2, rate, bs);
    p.prepareToPlay (rate, bs);
    p.playing = true;
    const int total = (int) (seconds * rate);
    juce::AudioBuffer<float> out (2, total), block (2, bs);
    juce::MidiBuffer midi;
    const auto t0 = juce::Time::getMillisecondCounterHiRes();
    for (int pos = 0; pos < total; pos += bs)
    {
        const int n = juce::jmin (bs, total - pos);
        block.setSize (2, n, false, false, true);
        p.processBlock (block, midi);
        for (int ch = 0; ch < 2; ++ch) out.copyFrom (ch, pos, block, ch, 0, n);
    }
    Stats st;
    st.cpu = (juce::Time::getMillisecondCounterHiRes() - t0) / 1000.0 / seconds * 100.0;
    double sum = 0;
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < total; ++i)
        {
            const float v = out.getSample (ch, i);
            if (! std::isfinite (v)) { ++st.nans; continue; }
            st.peak = std::max (st.peak, std::abs (v));
            sum += v * v;
        }
    st.rmsDb = juce::Decibels::gainToDecibels ((float) std::sqrt (sum / (2.0 * total)), -120.0f);
    if (wavName.isNotEmpty())
    {
        auto f = outDir.getChildFile (wavName + ".wav");
        f.deleteFile();
        std::unique_ptr<juce::OutputStream> stream = f.createOutputStream();
        juce::WavAudioFormat wav;
        auto w = wav.createWriterFor (stream, juce::AudioFormatWriterOptions {}.withSampleRate (rate).withNumChannels (2).withBitsPerSample (24));
        w->writeFromAudioSampleBuffer (out, 0, total);
    }
    return st;
}

// A WAV file holding a sine (mono, 48 kHz), to load into the sampler.
static void writeSine (const juce::File& file, float hz, double seconds)
{
    const int length = (int) (seconds * 48000.0);
    juce::AudioBuffer<float> audio (1, length);
    for (int i = 0; i < length; ++i) audio.setSample (0, i, 0.5f * std::sin (2.0f * juce::MathConstants<float>::pi * hz * (float) i / 48000.0f));
    file.deleteFile();
    std::unique_ptr<juce::OutputStream> stream = file.createOutputStream();
    juce::WavAudioFormat wav;
    auto writer = wav.createWriterFor (stream, juce::AudioFormatWriterOptions {}.withSampleRate (48000.0).withNumChannels (1).withBitsPerSample (24));
    writer->writeFromAudioSampleBuffer (audio, 0, length);
}

// The pitch of a held note: the first strong repetition (autocorrelation) between 50 and 1000 Hz.
static double estimatePitch (const std::vector<float>& v, double rate)
{
    const int n = (int) v.size();
    std::vector<double> c (962, 0.0);
    double bestValue = -1e9;
    for (int lag = 47; lag <= 961 && lag < n / 2; ++lag)
    {
        double sum = 0.0;
        for (int i = 0; i + lag < n; ++i) sum += (double) v[(size_t) i] * v[(size_t) (i + lag)];
        c[(size_t) lag] = sum / (double) (n - lag);
        bestValue = std::max (bestValue, c[(size_t) lag]);
    }
    int best = 48;
    for (int lag = 48; lag <= 960; ++lag)
        if (c[(size_t) lag] > 0.85 * bestValue && c[(size_t) lag] >= c[(size_t) lag - 1] && c[(size_t) lag] >= c[(size_t) lag + 1]) { best = lag; break; }
    const double a = c[(size_t) best - 1], b = c[(size_t) best], d = c[(size_t) best + 1];
    const double curve = a - 2.0 * b + d;
    const double shift = std::abs (curve) > 1e-12 ? 0.5 * (a - d) / curve : 0.0;
    return rate / ((double) best + shift);
}

static void print (const juce::String& name, const Stats& s)
{
    printf ("%-28s peak=%.3f rms=%6.1f dB nan=%d cpu=%.1f%%\n", name.toRawUTF8(), s.peak, s.rmsDb, s.nans, s.cpu);
}

static void savePng (const juce::Image& img, const juce::String& name)
{
    auto f = outDir.getChildFile (name + ".png");
    f.deleteFile();
    juce::FileOutputStream out (f);
    juce::PNGImageFormat().writeImageToStream (img, out);
}

static void dump (TechMachineProcessor& p, int type)
{
    auto& t = p.machine (type).track;
    const auto& def = machineDef (type);
    printf ("  [%s]\n", def.id);
    if (def.kind == MachineKind::Drum)
    {
        for (size_t r = 0; r < def.rows.size(); ++r)
        {
            juce::String line;
            for (int s = 0; s < 32; ++s) line << (t.hits[r][(size_t) s] ? "x" : ".") << (s % 16 == 15 ? " " : "");
            if (line.containsChar ('x')) printf ("   %-4s %s\n", def.rows[r].id, line.toRawUTF8());
        }
    }
    else
    {
        juce::String notes;
        for (int s = 0; s < 32; ++s)
            notes << (t.on[(size_t) s] ? juce::String (t.note[(size_t) s].load()).paddedLeft (' ', 2) + (t.slide[(size_t) s] ? "~" : " ") : " . ")
                  << (s % 16 == 15 ? "| " : "");
        printf ("   %s\n", notes.toRawUTF8());
    }
}

static void hideStartScreen (juce::Component& root)
{
    for (auto* c : root.getChildren())
    {
        if (auto* view = dynamic_cast<MainView*> (c); view != nullptr && view->isStartScreenShown())
            for (auto* child : view->getChildren())
                if (dynamic_cast<StartScreen*> (child) != nullptr) child->setVisible (false);
        hideStartScreen (*c);
    }
}

// Bit-exact fingerprint of what a processor plays (FNV-1a over the output samples).
static juce::uint64 fingerprint (TechMachineProcessor& p, double seconds)
{
    const int bs = 512;
    p.setPlayConfigDetails (0, 2, 48000.0, bs);
    p.prepareToPlay (48000.0, bs);
    p.playing = true;
    juce::AudioBuffer<float> block (2, bs);
    juce::MidiBuffer midi;
    juce::uint64 h = 1469598103934665603ull;
    for (int pos = 0; pos < (int) (seconds * 48000.0); pos += bs)
    {
        p.processBlock (block, midi);
        for (int ch = 0; ch < 2; ++ch)
        {
            auto* bytes = reinterpret_cast<const unsigned char*> (block.getReadPointer (ch));
            for (int i = 0; i < bs * (int) sizeof (float); ++i) { h ^= bytes[i]; h *= 1099511628211ull; }
        }
    }
    return h;
}

// Layout audit: texts wider than their control, and visible siblings that overlap.
static void auditLayout (juce::Component& c, juce::LookAndFeel& lf, const juce::String& where, juce::StringArray& issues)
{
    auto describe = [] (juce::Component* x)
    {
        juce::String text;
        if (auto* b = dynamic_cast<juce::Button*> (x)) text = b->getButtonText();
        else if (auto* l = dynamic_cast<juce::Label*> (x)) text = l->getText();
        else if (auto* cb = dynamic_cast<juce::ComboBox*> (x)) text = cb->getText();
        juce::String type = typeid (*x).name();
        type = type.fromLastOccurrenceOf (" ", false, false).fromLastOccurrenceOf (":", false, false);
        return type + " \"" + text.substring (0, 40) + "\" " + x->getBounds().toString();
    };

    if (auto* b = dynamic_cast<juce::TextButton*> (&c); b != nullptr && b->getButtonText().isNotEmpty() && dynamic_cast<techno::IconButton*> (&c) == nullptr)
    {
        const float w = juce::GlyphArrangement::getStringWidth (lf.getTextButtonFont (*b, b->getHeight()), b->getButtonText());
        if (w > (float) b->getWidth() - 6.0f) issues.add (where + "  text too wide (" + juce::String ((int) w) + "px): " + describe (&c));
    }
    if (auto* l = dynamic_cast<juce::Label*> (&c); l != nullptr && l->getText().isNotEmpty() && dynamic_cast<juce::ComboBox*> (l->getParentComponent()) == nullptr)
    {
        const auto border = l->getBorderSize();
        const float w = juce::GlyphArrangement::getStringWidth (l->getFont(), l->getText());
        const int lines = juce::jmax (1, (int) ((float) (l->getHeight() - border.getTopAndBottom()) / l->getFont().getHeight()));
        if (w > (float) (l->getWidth() - border.getLeftAndRight()) * (float) lines + 1.0f)
            issues.add (where + "  label too narrow (" + juce::String ((int) w) + "px): " + describe (&c));
    }
    if (auto* cl = dynamic_cast<techno::ControlLabel*> (&c); cl != nullptr && ! cl->fits())
        issues.add (where + "  knob name cut: \"" + cl->getText() + "\" " + juce::String (c.getWidth()) + "px in " + describe (c.getParentComponent()));
    if (auto* cb = dynamic_cast<juce::ComboBox*> (&c))
        for (int i = 0; i < cb->getNumItems(); ++i)
        {
            const float w = juce::GlyphArrangement::getStringWidth (lf.getComboBoxFont (*cb), cb->getItemText (i));
            if (w * 0.8f > (float) cb->getWidth() - 32.0f)
                issues.add (where + "  list item cut (" + juce::String ((int) w) + "px): \"" + cb->getItemText (i) + "\" in " + describe (&c));
        }

    juce::Array<juce::Component*> visible;
    for (auto* child : c.getChildren())
        if (child->isVisible() && ! child->getBounds().isEmpty()) visible.add (child);
    for (int i = 0; i < visible.size(); ++i)
        for (int j = i + 1; j < visible.size(); ++j)
        {
            const auto overlap = visible[i]->getBounds().getIntersection (visible[j]->getBounds());
            const bool contains = visible[i]->getBounds().contains (visible[j]->getBounds()) || visible[j]->getBounds().contains (visible[i]->getBounds());
            if (overlap.getWidth() > 2 && overlap.getHeight() > 2 && ! contains)
                issues.add (where + "  overlap: " + describe (visible[i]) + "  <>  " + describe (visible[j]));
        }
    for (auto* child : visible) auditLayout (*child, lf, where, issues);
}

static MainView* findMainView (juce::Component& root)
{
    if (auto* v = dynamic_cast<MainView*> (&root)) return v;
    for (auto* c : root.getChildren())
        if (auto* v = findMainView (*c)) return v;
    return nullptr;
}

static void findPanels (juce::Component& root, juce::Array<MachinePanel*>& out)
{
    if (auto* p = dynamic_cast<MachinePanel*> (&root)) out.add (p);
    for (auto* c : root.getChildren()) findPanels (*c, out);
}

// What a component shows, as a number: changes whenever one pixel changes.
static juce::uint64 imageHash (juce::Component& c)
{
    auto img = c.createComponentSnapshot (c.getLocalBounds(), true, 1.0f);
    juce::Image::BitmapData data (img, juce::Image::BitmapData::readOnly);
    juce::uint64 h = 1469598103934665603ull;
    for (int y = 0; y < data.height; ++y)
        for (int x = 0; x < data.width * data.pixelStride; ++x) { h ^= data.getLinePointer (y)[x]; h *= 1099511628211ull; }
    return h;
}

// Lets the interface timers run, as the message loop would.
static void pumpTimers()
{
    for (int i = 0; i < 6; ++i)
    {
        juce::Thread::sleep (40);
        juce::Timer::callPendingTimersSynchronously();
    }
}

template <typename T>
static T* findChild (juce::Component& root)
{
    if (auto* t = dynamic_cast<T*> (&root)) return t;
    for (auto* c : root.getChildren())
        if (auto* t = findChild<T> (*c)) return t;
    return nullptr;
}

// Loudness as heard: the loudest 100 ms window of the render, in dB.
static double momentaryLoudness (TechMachineProcessor& p, double seconds)
{
    const int bs = 480;
    p.setPlayConfigDetails (0, 2, 48000.0, bs);
    p.prepareToPlay (48000.0, bs);
    p.playing = true;
    juce::AudioBuffer<float> block (2, bs);
    juce::MidiBuffer midi;
    double best = 0, window = 0;
    int count = 0;
    for (int pos = 0; pos < (int) (seconds * 48000.0); pos += bs)
    {
        p.processBlock (block, midi);
        for (int i = 0; i < bs; ++i)
        {
            const double l = block.getSample (0, i), r = block.getSample (1, i);
            window += 0.5 * (l * l + r * r);
        }
        if (++count == 10)
        {
            best = std::max (best, window / (10.0 * bs));
            window = 0;
            count = 0;
        }
    }
    return 10.0 * std::log10 (std::max (best, 1.0e-12));
}

struct FakeHead : juce::AudioPlayHead
{
    double ppq = 0, bpm = 120;
    bool playing = true;
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo info;
        info.setBpm (bpm);
        info.setPpqPosition (ppq);
        info.setIsPlaying (playing);
        return info;
    }
};
