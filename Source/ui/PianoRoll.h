#pragma once

#include "../PluginProcessor.h"
#include "Skin.h"
#include "Widgets.h"

namespace techno
{
// Piano roll of a synth machine: pitches (rows) by steps (columns of the current page), aligned with the step keys.
// Click = add a note, drag to the right = make it longer, right-click or click on a note = remove it.
// Mono synths: one note per step (held steps glide / tie). Poly synths: any notes per step (free chords).
class PianoRoll : public juce::Component
{
public:
    static constexpr int rowH = 11;

    PianoRoll (TechMachineProcessor& p, int machineType, const int& viewPage, const Skin& s, std::function<int (int)> columnX, int cellWidth)
        : processor (p), type (machineType), def (machineDef (machineType)), page (viewPage), skin (s), xOf (std::move (columnX)), cellW (cellWidth)
    {
    }

    static int preferredHeight() { return noteRange * rowH + 2; }

    // x of the left edge of the grid in this component's coordinates (the keyboard is drawn to its left).
    int gridLeft = 120;

    void setPlayhead (int step)
    {
        if (step == playhead) return;
        playhead = step;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto& track = processor.machine (type).track;
        const int scale = processor.currentScale();
        const int base = baseNote();
        const int gridRight = columnX (pageSteps - 1) + cellW;

        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 4.0f);

        for (int n = 0; n < noteRange; ++n)
        {
            const int y = rowY (n);
            const int pc = (base + n) % 12;
            const bool black = pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
            const bool allowed = inScale (scale, n);

            // Keyboard on the left.
            g.setColour (black ? juce::Colour (0xff1c1c1e) : juce::Colour (0xffe9e6df));
            g.fillRect (gridLeft - 46, y, 44, rowH - 1);
            if (pc == 0 || n == 0)
            {
                g.setColour (black ? juce::Colours::white : juce::Colour (0xff333333));
                g.setFont (paint::condensed (10.0f));
                g.drawText (noteName (base + n), gridLeft - 46, y, 40, rowH - 1, juce::Justification::centredRight);
            }

            // Lane: notes of the scale are lighter, the others are dark (they cannot be played).
            g.setColour (allowed ? juce::Colours::white.withAlpha (black ? 0.05f : 0.09f) : juce::Colours::black.withAlpha (0.35f));
            g.fillRect (gridLeft, y, gridRight - gridLeft, rowH - 1);
        }

        for (int c = 0; c <= pageSteps; ++c)
        {
            const int x = c < pageSteps ? columnX (c) - 2 : gridRight + 1;
            g.setColour (juce::Colours::white.withAlpha (c % 4 == 0 ? 0.22f : 0.07f));
            g.drawVerticalLine (x, 2.0f, (float) getHeight() - 2.0f);
        }

        if (playhead >= page * pageSteps && playhead < (page + 1) * pageSteps)
        {
            g.setColour (skin.led.withAlpha (0.18f));
            g.fillRect (columnX (playhead - page * pageSteps) - 2, 0, cellW + 4, getHeight());
        }

        // Notes.
        for (int c = 0; c < pageSteps; ++c)
        {
            const int s = page * pageSteps + c;
            if (! track.on[(size_t) s]) continue;
            const auto notes = notesAt (s);
            const bool continues = s + 1 < processor.numSteps() && track.slide[(size_t) s] && track.on[(size_t) s + 1] && notesAt (s + 1) == notes;
            const bool continued = s > 0 && track.slide[(size_t) s - 1] && track.on[(size_t) s - 1] && notesAt (s - 1) == notes;
            for (int n = 0; n < noteRange; ++n)
            {
                if (! ((notes >> n) & 1u)) continue;
                const int x0 = columnX (c) - (continued ? 4 : 0), x1 = columnX (c) + cellW + (continues ? 4 : 0);
                auto r = juce::Rectangle<int> (x0, rowY (n) + 1, x1 - x0, rowH - 3).toFloat();
                g.setColour (track.chord[(size_t) s].load() != 0 || ! isPoly() ? skin.lit : skin.lit.withAlpha (0.55f));
                g.fillRoundedRectangle (r, 2.0f);
                if (track.accent[(size_t) s] && ! continued)
                {
                    g.setColour (juce::Colours::white.withAlpha (0.7f));
                    g.fillRect (r.removeFromLeft (3.0f));
                }
            }
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const int c = columnAt (e.x), n = noteAt (e.y);
        if (c < 0 || n < 0 || ! inScale (processor.currentScale(), n)) return;
        const int s = page * pageSteps + c;
        processor.beginPatternEdit();

        auto& track = processor.machine (type).track;
        const bool present = track.on[(size_t) s] && ((notesAt (s) >> n) & 1u);
        if (e.mods.isPopupMenu() || present)
        {
            removeNote (s, n);
            dragStart = -1;
        }
        else
        {
            addNote (s, n);
            dragStart = s;
            dragNote = n;
        }
        processor.touchPattern();
        repaint();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (dragStart < 0) return;
        const int c = columnAt (e.x);
        if (c < 0) return;
        setLength (dragStart, page * pageSteps + c);
        processor.touchPattern();
        repaint();
    }

private:
    bool isPoly() const { return def.kind == MachineKind::Poly; }

    int baseNote() const
    {
        const int octave = (int) processor.apvts.getRawParameterValue (pid::synth (type, 0))->load();
        return machineBaseNote (def, processor.currentKey(), octave);
    }

    static bool inScale (int scale, int n) { return snapToScale (scale, n) == n; }

    int rowY (int n) const { return 1 + (noteRange - 1 - n) * rowH; }
    int columnX (int c) const { return xOf (c) - getX(); }

    int columnAt (int x) const
    {
        for (int c = 0; c < pageSteps; ++c)
            if (x >= columnX (c) - 2 && x < columnX (c) + cellW + 3) return c;
        return -1;
    }

    int noteAt (int y) const
    {
        const int n = noteRange - 1 - (y - 1) / rowH;
        return n >= 0 && n < noteRange ? n : -1;
    }

    // The notes a step plays, one bit per semitone. For a poly step without free notes: its chord.
    std::uint64_t notesAt (int s) const
    {
        auto& track = processor.machine (type).track;
        const int scale = processor.currentScale();
        if (isPoly())
        {
            if (const auto mask = track.chord[(size_t) s].load(); mask != 0) return mask;
            const int root = snapToScale (scale, track.note[(size_t) s].load());
            int intervals[5];
            const int count = chordIntervals (scale, root, (int) processor.apvts.getRawParameterValue (pid::synth (type, 1))->load(), intervals);
            std::uint64_t bits = 0;
            for (int k = 0; k < count; ++k)
                if (root + intervals[k] >= 0 && root + intervals[k] < noteRange) bits |= 1ull << (root + intervals[k]);
            return bits;
        }
        return 1ull << snapToScale (scale, track.note[(size_t) s].load());
    }

    void addNote (int s, int n)
    {
        auto& track = processor.machine (type).track;
        if (isPoly())
        {
            const auto current = track.on[(size_t) s] ? notesAt (s) : 0;
            track.chord[(size_t) s] = current | (1ull << n);
            track.note[(size_t) s] = lowest (track.chord[(size_t) s].load());
        }
        else
        {
            track.note[(size_t) s] = n;
        }
        track.on[(size_t) s] = true;
        track.slide[(size_t) s] = false;
    }

    void removeNote (int s, int n)
    {
        auto& track = processor.machine (type).track;
        // Removing a note removes the whole held note it belongs to.
        int start = s;
        const auto notes = notesAt (s);
        while (start > 0 && track.on[(size_t) start - 1] && track.slide[(size_t) start - 1] && notesAt (start - 1) == notes) --start;
        for (int t = start; t < processor.numSteps(); ++t)
        {
            const bool next = track.slide[(size_t) t] && t + 1 < processor.numSteps() && track.on[(size_t) t + 1] && notesAt (t + 1) == notes;
            if (isPoly())
            {
                const auto left = notes & ~(1ull << n);
                track.chord[(size_t) t] = left;
                track.on[(size_t) t] = left != 0;
                if (left != 0) track.note[(size_t) t] = lowest (left);
            }
            else
            {
                track.on[(size_t) t] = false;
            }
            if (! track.on[(size_t) t]) track.slide[(size_t) t] = false;
            if (! next) break;
        }
        if (start > 0 && ! track.on[(size_t) start]) track.slide[(size_t) start - 1] = false;
    }

    // Holds the note started at `start` until `end` (inclusive).
    void setLength (int start, int end)
    {
        auto& track = processor.machine (type).track;
        end = juce::jlimit (start, processor.numSteps() - 1, end);
        const auto notes = notesAt (start);
        for (int t = start + 1; t <= end; ++t)
        {
            track.on[(size_t) t] = true;
            track.note[(size_t) t] = track.note[(size_t) start].load();
            track.chord[(size_t) t] = track.chord[(size_t) start].load();
            track.slide[(size_t) t - 1] = true;
        }
        // Steps after the new end that still carried this note are released.
        for (int t = end + 1; t < processor.numSteps() && track.slide[(size_t) t - 1] && notesAt (t) == notes; ++t)
        {
            track.slide[(size_t) t - 1] = false;
            track.on[(size_t) t] = false;
            track.chord[(size_t) t] = 0;
        }
        track.slide[(size_t) end] = false;
        juce::ignoreUnused (dragNote);
    }

    static int lowest (std::uint64_t bits)
    {
        for (int n = 0; n < noteRange; ++n)
            if ((bits >> n) & 1u) return n;
        return 0;
    }

    TechMachineProcessor& processor;
    const int type;
    const MachineDef& def;
    const int& page;
    const Skin& skin;
    std::function<int (int)> xOf;
    int cellW, playhead = -1, dragStart = -1, dragNote = -1;
};
} // namespace techno
