#pragma once

#include "../PluginProcessor.h"
#include "Skin.h"
#include "Widgets.h"

namespace techno
{
// Full-size piano roll of a synth machine, shown over the rack.
// Grid: pitches (rows) by steps (columns) over the whole sequence, with zoom and scrolling.
// Mouse: click = add a note (drag to set its length), drag a note = move it, drag its right edge = resize,
// right-click or double-click = delete, Ctrl+drag = select a zone, Alt+drag = copy.
// Keys: Delete, Ctrl+A, Ctrl+C / Ctrl+V, Ctrl+D (duplicate), arrows (move / transpose, Shift = bar / octave), Esc = close.
class PianoRollGrid : public juce::Component
{
public:
    struct Note
    {
        int start = 0, length = 1, pitch = 0;
        bool selected = false;
    };

    PianoRollGrid (TechMachineProcessor& p, int machineType) : processor (p)
    {
        setWantsKeyboardFocus (true);
        setMachine (machineType);
    }

    std::function<void()> onClose, onChange;
    int defaultLength = 2;
    int stackSize = 1;   // notes placed by one click: 1 = a note, 3 = a chord of the scale, 4 = a seventh chord

    void setMachine (int machineType)
    {
        type = machineType;
        skin = &skinFor (type);
        stepW = 0.0f;   // fit the whole sequence again
        reload();
    }

    int machine() const { return type; }
    int knownVersion = -1;

    // Reads the notes from the machine's steps (after an undo, a recording, a pattern...).
    void reload()
    {
        notes.clear();
        glide.assign ((size_t) maxSteps, false);
        auto& t = track();
        const int len = steps();
        if (isPoly())
        {
            for (int s = 0; s < len; ++s)
            {
                const auto mask = maskAt (s);
                for (int n = 0; n < noteRange; ++n)
                {
                    if (! ((mask >> n) & 1u)) continue;
                    if (s > 0 && ((maskAt (s - 1) >> n) & 1u) && ((tieAt (s - 1) >> n) & 1u)) continue;   // continuation
                    int length = 1;
                    while (s + length < len && ((tieAt (s + length - 1) >> n) & 1u) && ((maskAt (s + length) >> n) & 1u)) ++length;
                    notes.push_back ({ s, length, n, false });
                }
            }
        }
        else
        {
            const int scale = processor.currentScale();
            auto pitch = [&] (int s) { return snapToScale (scale, t.note[(size_t) s].load()); };
            for (int s = 0; s < len; ++s)
            {
                if (! t.on[(size_t) s]) continue;
                if (s > 0 && t.on[(size_t) s - 1] && t.slide[(size_t) s - 1] && pitch (s - 1) == pitch (s)) continue;
                int length = 1;
                while (s + length < len && t.slide[(size_t) (s + length - 1)] && t.on[(size_t) (s + length)] && pitch (s + length) == pitch (s)) ++length;
                notes.push_back ({ s, length, pitch (s), false });
                const int last = s + length - 1;
                if (last + 1 < len && t.slide[(size_t) last] && t.on[(size_t) last + 1]) glide[(size_t) last] = true;
            }
        }
        knownVersion = processor.patternVersion.load();
        repaint();
    }

    // Centre of a cell of the grid (for tests and tools).
    juce::Point<float> cellCentre (int step, int pitch) const
    {
        return { stepX (step) + stepW * 0.5f, (float) rowY (pitch) + (float) rowH * 0.5f };
    }

    void setPlayhead (int step)
    {
        if (step == playhead) return;
        playhead = step;
        repaint();
    }

    //==========================================================================
    // Actions (also used by the toolbar).

    void selectAll()
    {
        for (auto& n : notes) n.selected = true;
        repaint();
    }

    void deleteSelection()
    {
        beginEdit();
        notes.erase (std::remove_if (notes.begin(), notes.end(), [] (const Note& n) { return n.selected; }), notes.end());
        commit();
    }

    void transposeSelection (int semitones)
    {
        beginEdit();
        for (auto& n : notes)
            if (n.selected || noneSelected()) n.pitch = juce::jlimit (0, noteRange - 1, n.pitch + semitones);
        commit();
    }

    void duplicateSelection()
    {
        auto source = selectedOrAll();
        if (source.empty()) return;
        int first = steps(), last = 0;
        for (auto& n : source) { first = juce::jmin (first, n.start); last = juce::jmax (last, n.start + n.length); }
        // Duplicate after the block, rounded up to whole beats so rhythms stay in place.
        const int span = ((last - first + 3) / 4) * 4;
        beginEdit();
        for (auto& n : notes) n.selected = false;
        for (auto n : source)
        {
            n.start += span;
            n.selected = true;
            if (n.start < steps()) notes.push_back (n);
        }
        commit();
    }

    void clearAll()
    {
        beginEdit();
        notes.clear();
        commit();
    }

    //==========================================================================
    void paint (juce::Graphics& g) override
    {
        layout();
        const int len = steps();
        const int scale = processor.currentScale();
        const int base = baseNote();
        const auto grid = gridArea();

        g.fillAll (juce::Colour (0xff17181b));

        // Rows.
        g.saveState();
        g.reduceClipRegion (juce::Rectangle<int> (0, grid.getY(), getWidth(), grid.getHeight()));
        for (int n = 0; n < noteRange; ++n)
        {
            const int y = rowY (n);
            if (y + rowH < grid.getY() || y > grid.getBottom()) continue;
            const int pc = ((base + n) % 12 + 12) % 12;
            const bool black = pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
            const bool allowed = inScale (scale, n);
            const bool tonic = pc == ((base % 12) + 12) % 12;

            juce::Colour lane = allowed ? juce::Colour (black ? 0xff222428 : 0xff2a2c31) : juce::Colour (0xff141518);
            if (tonic && allowed) lane = lane.brighter (0.12f);
            g.setColour (lane);
            g.fillRect (grid.getX(), y, grid.getWidth(), rowH - 1);

            // Keyboard.
            g.setColour (black ? juce::Colour (0xff1e1e20) : juce::Colour (0xffeae7e0));
            g.fillRect (2, y, keyW - 6, rowH - 1);
            if (pressedKey == n) { g.setColour (skin->lit.withAlpha (0.8f)); g.fillRect (2, y, keyW - 6, rowH - 1); }
            if (pc == 0 || tonic || rowH >= 18)
            {
                g.setColour (black ? juce::Colours::white.withAlpha (0.85f) : juce::Colour (0xff222222));
                g.setFont (paint::condensed (juce::jmin (12.0f, (float) rowH - 3.0f)));
                g.drawText (noteName (base + n), 4, y, keyW - 12, rowH - 1, juce::Justification::centredRight);
            }
            if (! allowed)
            {
                g.setColour (juce::Colours::black.withAlpha (0.45f));
                g.fillRect (2, y, keyW - 6, rowH - 1);
            }
        }

        // Steps, beats and bars.
        for (int s = 0; s <= len; ++s)
        {
            const float x = stepX (s);
            if (x < (float) grid.getX() - 1.0f || x > (float) grid.getRight() + 1.0f) continue;
            g.setColour (juce::Colours::white.withAlpha (s % 16 == 0 ? 0.35f : s % 4 == 0 ? 0.14f : 0.05f));
            g.drawVerticalLine ((int) x, (float) grid.getY(), (float) grid.getBottom());
        }
        if (stepX (len) < (float) grid.getRight())
        {
            g.setColour (juce::Colours::black.withAlpha (0.5f));
            g.fillRect (juce::Rectangle<float> (stepX (len), (float) grid.getY(), (float) grid.getRight() - stepX (len), (float) grid.getHeight()));
        }

        // Playhead.
        if (playhead >= 0 && playhead < len)
        {
            g.setColour (skin->led.withAlpha (0.16f));
            g.fillRect (juce::Rectangle<float> (stepX (playhead), (float) grid.getY(), stepW, (float) grid.getHeight()));
        }

        // Notes.
        for (auto& note : notes)
        {
            auto r = noteRect (note);
            if (! r.intersects (grid.toFloat())) continue;
            g.setColour (note.selected ? skin->lit.brighter (0.35f) : skin->lit);
            g.fillRoundedRectangle (r.reduced (1.0f, 1.0f), 3.0f);
            g.setColour (note.selected ? juce::Colours::white : juce::Colours::black.withAlpha (0.45f));
            g.drawRoundedRectangle (r.reduced (1.0f, 1.0f), 3.0f, note.selected ? 1.6f : 1.0f);
            // Resize handle.
            g.setColour (juce::Colours::black.withAlpha (0.25f));
            g.fillRect (r.getRight() - 5.0f, r.getY() + 3.0f, 2.0f, r.getHeight() - 6.0f);
            if (r.getWidth() > 34.0f && rowH >= 13)
            {
                g.setColour (juce::Colours::black.withAlpha (0.75f));
                g.setFont (paint::condensed (juce::jmin (11.0f, (float) rowH - 4.0f)));
                g.drawText (noteName (base + note.pitch), r.reduced (5.0f, 0.0f), juce::Justification::centredLeft);
            }
            if (! isPoly() && glide[(size_t) (note.start + note.length - 1)])
            {
                g.setColour (juce::Colours::white.withAlpha (0.8f));
                g.drawArrow ({ r.getRight() - 6.0f, r.getCentreY(), r.getRight() + 4.0f, r.getCentreY() }, 1.5f, 5.0f, 5.0f);
            }
        }
        g.restoreState();

        // Rubber band.
        if (dragMode == Drag::Select)
        {
            g.setColour (juce::Colours::white.withAlpha (0.12f));
            g.fillRect (selectionRect);
            g.setColour (juce::Colours::white.withAlpha (0.6f));
            g.drawRect (selectionRect, 1.0f);
        }

        // Ruler with bar and beat numbers.
        const auto ruler = juce::Rectangle<int> (keyW, 0, getWidth() - keyW, rulerH);
        g.setColour (juce::Colour (0xff222327));
        g.fillRect (ruler);
        g.setFont (paint::condensed (12.0f));
        for (int s = 0; s < len; s += 4)
        {
            const float x = stepX (s);
            if (x < (float) keyW - 1.0f || x > (float) getWidth()) continue;
            g.setColour (s % 16 == 0 ? colours::text : colours::dim);
            g.drawText (s % 16 == 0 ? juce::String (s / 16 + 1) : juce::String (s / 16 + 1) + "." + juce::String ((s % 16) / 4 + 1),
                        (int) x + 3, 0, 40, rulerH, juce::Justification::centredLeft);
        }
        g.setColour (colours::dim);
        g.drawText (utf8 ("MESURE"), 4, 0, keyW - 8, rulerH, juce::Justification::centredRight);

        // Lanes: accent (and glide on mono synths).
        auto& t = track();
        for (int lane = 0; lane < numLanes(); ++lane)
        {
            const auto area = laneArea (lane);
            g.setColour (juce::Colour (0xff1d1e22));
            g.fillRect (area);
            g.setColour (colours::dim);
            g.setFont (paint::condensed (11.0f));
            g.drawText (lane == 0 ? utf8 ("ACCENT") : utf8 ("GLISSÉ"), 4, area.getY(), keyW - 10, area.getHeight(), juce::Justification::centredRight);
            for (int s = 0; s < len; ++s)
            {
                const float x = stepX (s);
                if (x + stepW < (float) keyW || x > (float) getWidth()) continue;
                const bool on = lane == 0 ? t.accent[(size_t) s].load() : glide[(size_t) s];
                auto cell = juce::Rectangle<float> (x + 2.0f, (float) area.getY() + 4.0f, juce::jmax (3.0f, stepW - 4.0f), (float) area.getHeight() - 8.0f);
                g.setColour (on ? (lane == 0 ? juce::Colour (0xffffb020) : juce::Colour (0xff59c3ff)) : juce::Colours::white.withAlpha (0.06f));
                g.fillRoundedRectangle (cell, 2.0f);
            }
        }

        if (notes.empty())
        {
            g.setColour (colours::dim);
            g.setFont (paint::font (15.0f, false));
            g.drawText (utf8 ("Clique dans la grille pour poser une note, glisse vers la droite pour l'allonger."), grid, juce::Justification::centred);
        }
    }

    //==========================================================================
    void mouseDown (const juce::MouseEvent& e) override
    {
        grabKeyboardFocus();
        layout();
        const auto pos = e.position;
        downPos = pos;

        // Keyboard: listen to the note.
        if (pos.x < (float) keyW && pos.y >= (float) rulerH && pos.y < (float) gridArea().getBottom())
        {
            pressedKey = noteAtY (pos.y);
            if (pressedKey >= 0) preview (pressedKey, true);
            repaint();
            return;
        }

        // Lanes.
        for (int lane = 0; lane < numLanes(); ++lane)
        {
            if (! laneArea (lane).toFloat().contains (pos)) continue;
            const int s = stepAt (pos.x);
            if (s < 0) return;
            beginEdit();
            if (lane == 0) track().accent[(size_t) s] = ! track().accent[(size_t) s].load();
            else glide[(size_t) s] = ! glide[(size_t) s];
            laneValue = lane == 0 ? track().accent[(size_t) s].load() : glide[(size_t) s];
            dragLane = lane;
            commit();
            return;
        }

        if (! gridArea().toFloat().contains (pos)) return;
        const int s = stepAt (pos.x), n = noteAtY (pos.y);
        if (s < 0 || n < 0) return;
        const int hit = noteIndexAt (pos);

        if (e.mods.isPopupMenu())
        {
            beginEdit();
            dragMode = Drag::Erase;
            if (hit >= 0) notes.erase (notes.begin() + hit);
            commit();
            return;
        }

        if (e.mods.isCommandDown() && hit < 0)
        {
            dragMode = Drag::Select;
            if (! e.mods.isShiftDown())
                for (auto& note : notes) note.selected = false;
            selectionRect = {};
            repaint();
            return;
        }

        beginEdit();
        if (hit >= 0)
        {
            auto& note = notes[(size_t) hit];
            if (e.mods.isShiftDown()) note.selected = ! note.selected;
            else if (! note.selected)
            {
                for (auto& other : notes) other.selected = false;
                note.selected = true;
            }
            if (e.mods.isAltDown())
            {
                // Alt+drag: the selected notes are copied, the copies move.
                std::vector<Note> copies;
                for (auto& other : notes)
                    if (other.selected) { copies.push_back (other); other.selected = false; }
                for (auto& c : copies) notes.push_back (c);
            }
            const bool onEdge = pos.x > noteRect (notes[(size_t) noteIndexAt (pos)]).getRight() - 7.0f;
            dragMode = onEdge ? Drag::Resize : Drag::Move;
            grabbed = noteIndexAt (pos);
            original = notes;
            previewPitch = notes[(size_t) grabbed].pitch;
            preview (previewPitch, true);
        }
        else
        {
            if (! inScale (processor.currentScale(), n)) { dragMode = Drag::None; return; }
            for (auto& note : notes) note.selected = false;
            const int length = juce::jmin (defaultLength, steps() - s);
            const int count = isPoly() ? stackSize : 1;
            // A chord: every other note of the scale above the clicked one (1-3-5, then the 7th).
            for (int k = 0; k < count; ++k)
            {
                const int pitch = k == 0 ? n : moveInScale (processor.currentScale(), n, 2 * k);
                if (pitch < noteRange) notes.push_back ({ s, length, pitch, true });
                if (k > 0 && pitch < noteRange)
                {
                    preview (pitch, true);
                    chordPreview.push_back (pitch);
                }
            }
            grabbed = (int) notes.size() - count;
            original = notes;
            dragMode = Drag::Create;
            previewPitch = n;
            preview (n, true);
        }
        downStep = s;
        downPitch = n;
        writeNotes();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const auto pos = e.position;
        if (dragLane >= 0)
        {
            if (const int s = stepAt (pos.x); s >= 0)
            {
                if (dragLane == 0) track().accent[(size_t) s] = laneValue;
                else glide[(size_t) s] = laneValue;
                writeNotes();
            }
            return;
        }

        const int s = juce::jlimit (0, steps() - 1, stepAtUnclamped (pos.x));
        const int n = juce::jlimit (0, noteRange - 1, noteAtYUnclamped (pos.y));
        switch (dragMode)
        {
            case Drag::Create:
            {
                const int start = notes[(size_t) grabbed].start;
                const int length = juce::jmax (1, juce::jmin (s - start + 1, steps() - start));
                for (auto& note : notes)
                    if (note.selected) note.length = length;
                break;
            }
            case Drag::Move:
            {
                const int dx = s - downStep;
                const int dy = n - downPitch;
                int minStart = steps(), maxEnd = 0, minPitch = noteRange, maxPitch = 0;
                for (auto& o : original)
                    if (o.selected) { minStart = juce::jmin (minStart, o.start); maxEnd = juce::jmax (maxEnd, o.start + o.length); minPitch = juce::jmin (minPitch, o.pitch); maxPitch = juce::jmax (maxPitch, o.pitch); }
                const int cx = juce::jlimit (-minStart, steps() - maxEnd, dx);
                const int cy = juce::jlimit (-minPitch, noteRange - 1 - maxPitch, dy);
                for (size_t i = 0; i < notes.size() && i < original.size(); ++i)
                {
                    if (! original[i].selected) continue;
                    notes[i].start = original[i].start + cx;
                    notes[i].pitch = toScale (original[i].pitch + cy, cy);
                }
                if (grabbed >= 0 && notes[(size_t) grabbed].pitch != previewPitch)
                {
                    preview (previewPitch, false);
                    previewPitch = notes[(size_t) grabbed].pitch;
                    preview (previewPitch, true);
                }
                break;
            }
            case Drag::Resize:
            {
                const int dx = s - downStep;
                for (size_t i = 0; i < notes.size() && i < original.size(); ++i)
                    if (original[i].selected)
                        notes[i].length = juce::jlimit (1, steps() - original[i].start, original[i].length + dx);
                break;
            }
            case Drag::Erase:
            {
                if (const int hit = noteIndexAt (pos); hit >= 0) notes.erase (notes.begin() + hit);
                break;
            }
            case Drag::Select:
            {
                selectionRect = juce::Rectangle<float> (downPos, pos);
                for (auto& note : notes)
                    note.selected = noteRect (note).intersects (selectionRect) || (e.mods.isShiftDown() && note.selected);
                repaint();
                return;
            }
            case Drag::None: return;
        }
        writeNotes();
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (pressedKey >= 0) { preview (pressedKey, false); pressedKey = -1; repaint(); }
        if (previewPitch >= 0) { preview (previewPitch, false); previewPitch = -1; }
        for (int pitch : chordPreview) preview (pitch, false);
        chordPreview.clear();
        if (dragMode == Drag::Create || dragMode == Drag::Resize)
            if (grabbed >= 0 && grabbed < (int) notes.size()) defaultLength = notes[(size_t) grabbed].length;
        if (dragMode != Drag::None && dragMode != Drag::Select)
        {
            resolveMono();
            writeNotes();
        }
        dragMode = Drag::None;
        dragLane = -1;
        grabbed = -1;
        repaint();
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        if (const int hit = noteIndexAt (e.position); hit >= 0)
        {
            beginEdit();
            notes.erase (notes.begin() + hit);
            commit();
        }
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const int hit = noteIndexAt (e.position);
        const bool edge = hit >= 0 && e.position.x > noteRect (notes[(size_t) hit]).getRight() - 7.0f;
        setMouseCursor (edge ? juce::MouseCursor::LeftRightResizeCursor
                             : hit >= 0 ? juce::MouseCursor::DraggingHandCursor : juce::MouseCursor::NormalCursor);
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w) override
    {
        layout();
        if (e.mods.isCommandDown())
        {
            // Zoom around the mouse.
            const float before = (e.position.x - (float) keyW + scrollX) / stepW;
            stepW = juce::jlimit (8.0f, 120.0f, stepW * (w.deltaY > 0 ? 1.15f : 1.0f / 1.15f));
            scrollX = before * stepW - (e.position.x - (float) keyW);
        }
        else if (e.mods.isShiftDown() || std::abs (w.deltaX) > std::abs (w.deltaY))
        {
            scrollX -= (std::abs (w.deltaX) > 0.0f ? w.deltaX : w.deltaY) * 300.0f;
        }
        else
        {
            scrollY -= w.deltaY * 240.0f;
        }
        clampScroll();
        repaint();
    }

    bool keyPressed (const juce::KeyPress& key) override
    {
        const auto mods = key.getModifiers();
        const int code = key.getKeyCode();
        if (key == juce::KeyPress::escapeKey) { if (onClose) onClose(); return true; }
        if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey) { deleteSelection(); return true; }
        if (mods.isCommandDown() && code == 'A') { selectAll(); return true; }
        if (mods.isCommandDown() && code == 'D') { duplicateSelection(); return true; }
        if (mods.isCommandDown() && code == 'C') { clipboard = selectedOrAll(); return true; }
        if (mods.isCommandDown() && code == 'V' && ! clipboard.empty())
        {
            int last = 0;
            for (auto& n : notes) if (n.selected) last = juce::jmax (last, n.start + n.length);
            int first = steps();
            for (auto& n : clipboard) first = juce::jmin (first, n.start);
            beginEdit();
            for (auto& n : notes) n.selected = false;
            for (auto n : clipboard)
            {
                n.start = n.start - first + last;
                n.selected = true;
                if (n.start < steps()) { n.length = juce::jmin (n.length, steps() - n.start); notes.push_back (n); }
            }
            commit();
            return true;
        }
        if (key.isKeyCode (juce::KeyPress::upKey) || key.isKeyCode (juce::KeyPress::downKey))
        {
            const int dir = key.isKeyCode (juce::KeyPress::upKey) ? 1 : -1;
            beginEdit();
            for (auto& n : notes)
                if (n.selected)
                    n.pitch = mods.isShiftDown() ? juce::jlimit (0, noteRange - 1, n.pitch + 12 * dir) : toScale (n.pitch + dir, dir);
            commit();
            return true;
        }
        if (key.isKeyCode (juce::KeyPress::leftKey) || key.isKeyCode (juce::KeyPress::rightKey))
        {
            const int amount = (key.isKeyCode (juce::KeyPress::rightKey) ? 1 : -1) * (mods.isShiftDown() ? 16 : 1);
            beginEdit();
            for (auto& n : notes)
                if (n.selected) n.start = juce::jlimit (0, juce::jmax (0, steps() - n.length), n.start + amount);
            commit();
            return true;
        }
        return false;   // space, Ctrl+Z... go to the main window
    }

    void resized() override
    {
        layout();
        if (stepW <= 0.0f) fitAll();
        clampScroll();
    }

    void fitAll()
    {
        layout();
        stepW = juce::jlimit (8.0f, 80.0f, (float) (getWidth() - keyW - 12) / (float) juce::jmax (1, steps()));
        scrollX = 0.0f;
        // Centre the notes vertically (or the middle of the range).
        int low = noteRange, high = 0;
        for (auto& n : notes) { low = juce::jmin (low, n.pitch); high = juce::jmax (high, n.pitch); }
        const int centre = notes.empty() ? noteRange / 2 : (low + high) / 2;
        scrollY = (float) ((noteRange - 1 - centre) * rowH) - (float) gridArea().getHeight() * 0.5f;
        clampScroll();
        repaint();
    }

private:
    enum class Drag { None, Create, Move, Resize, Erase, Select };
    static constexpr int keyW = 70, rulerH = 22, laneH = 24;
    int rowH = 16;

    Track& track() const { return processor.machine (type).track; }
    bool isPoly() const { return machineDef (type).kind == MachineKind::Poly; }
    int steps() const { return processor.numSteps(); }
    int numLanes() const { return isPoly() ? 1 : 2; }

    int baseNote() const
    {
        const int octave = (int) processor.apvts.getRawParameterValue (pid::synth (type, 0))->load();
        return machineBaseNote (machineDef (type), processor.currentKey(), octave);
    }

    static bool inScale (int scale, int n) { return snapToScale (scale, n) == n; }

    // Moves a pitch onto the scale, in the direction of the move.
    int toScale (int pitch, int direction) const
    {
        const int scale = processor.currentScale();
        pitch = juce::jlimit (0, noteRange - 1, pitch);
        for (int k = 0; k < 12; ++k)
        {
            const int up = pitch + (direction >= 0 ? k : -k), down = pitch + (direction >= 0 ? -k : k);
            if (up >= 0 && up < noteRange && inScale (scale, up)) return up;
            if (down >= 0 && down < noteRange && inScale (scale, down)) return down;
        }
        return pitch;
    }

    void layout()
    {
        rowH = juce::jlimit (12, 22, (getHeight() - rulerH - 2 * laneH - 8) / 24);
    }

    juce::Rectangle<int> gridArea() const
    {
        return { keyW, rulerH, getWidth() - keyW, getHeight() - rulerH - numLanes() * laneH - 4 };
    }

    juce::Rectangle<int> laneArea (int lane) const
    {
        const int top = getHeight() - numLanes() * laneH + lane * laneH;
        return { 0, top, getWidth(), laneH };
    }

    float stepX (int s) const { return (float) keyW + (float) s * stepW - scrollX; }
    int rowY (int n) const { return rulerH + (noteRange - 1 - n) * rowH - (int) scrollY; }

    int stepAtUnclamped (float x) const { return (int) std::floor ((x - (float) keyW + scrollX) / stepW); }
    int stepAt (float x) const
    {
        const int s = stepAtUnclamped (x);
        return s >= 0 && s < steps() ? s : -1;
    }
    int noteAtYUnclamped (float y) const { return noteRange - 1 - (int) std::floor ((y - (float) rulerH + scrollY) / (float) rowH); }
    int noteAtY (float y) const
    {
        const int n = noteAtYUnclamped (y);
        return n >= 0 && n < noteRange ? n : -1;
    }

    juce::Rectangle<float> noteRect (const Note& n) const
    {
        return { stepX (n.start), (float) rowY (n.pitch), stepW * (float) n.length, (float) rowH };
    }

    int noteIndexAt (juce::Point<float> p) const
    {
        for (int i = (int) notes.size() - 1; i >= 0; --i)
            if (noteRect (notes[(size_t) i]).contains (p)) return i;
        return -1;
    }

    void clampScroll()
    {
        const float maxX = juce::jmax (0.0f, (float) steps() * stepW - (float) (getWidth() - keyW) + 20.0f);
        const float maxY = juce::jmax (0.0f, (float) (noteRange * rowH) - (float) gridArea().getHeight());
        scrollX = juce::jlimit (0.0f, maxX, scrollX);
        scrollY = juce::jlimit (0.0f, maxY, scrollY);
    }

    bool noneSelected() const
    {
        for (auto& n : notes) if (n.selected) return false;
        return true;
    }

    std::vector<Note> selectedOrAll() const
    {
        std::vector<Note> out;
        for (auto& n : notes) if (n.selected) out.push_back (n);
        return out.empty() ? notes : out;
    }

    std::uint64_t maskAt (int s) const
    {
        auto& t = track();
        if (! t.on[(size_t) s]) return 0;
        if (const auto m = t.chord[(size_t) s].load(); m != 0) return m;
        // A step written with a chord type: its chord.
        const int scale = processor.currentScale();
        const int root = snapToScale (scale, t.note[(size_t) s].load());
        int intervals[5];
        const int count = chordIntervals (scale, root, (int) processor.apvts.getRawParameterValue (pid::synth (type, 1))->load(), intervals);
        std::uint64_t bits = 0;
        for (int k = 0; k < count; ++k)
            if (root + intervals[k] >= 0 && root + intervals[k] < noteRange) bits |= 1ull << (root + intervals[k]);
        return bits;
    }

    std::uint64_t tieAt (int s) const
    {
        auto& t = track();
        if (t.chord[(size_t) s].load() != 0) return t.tie[(size_t) s].load();
        // Chord-type steps hold with SLIDE ("hold") when the next step plays the same chord.
        if (s + 1 < steps() && t.slide[(size_t) s] && t.on[(size_t) s + 1] && t.chord[(size_t) s + 1].load() == 0
            && t.note[(size_t) s].load() == t.note[(size_t) s + 1].load())
            return maskAt (s);
        return 0;
    }

    // A mono synth plays one note at a time: a later note cuts the one before it.
    void resolveMono()
    {
        if (isPoly()) return;
        std::sort (notes.begin(), notes.end(), [] (const Note& a, const Note& b) { return a.start < b.start; });
        for (size_t i = 0; i + 1 < notes.size(); ++i)
        {
            auto& a = notes[i];
            const auto& b = notes[i + 1];
            if (a.start == b.start) { a.length = 0; continue; }
            if (a.start + a.length > b.start) a.length = b.start - a.start;
        }
        notes.erase (std::remove_if (notes.begin(), notes.end(), [] (const Note& n) { return n.length <= 0; }), notes.end());
    }

    // Writes the notes back into the machine's steps (the whole sequence).
    void writeNotes()
    {
        auto& t = track();
        const int len = steps();
        for (int s = 0; s < len; ++s)
        {
            const auto i = (size_t) s;
            t.on[i] = false;
            t.slide[i] = false;
            t.chord[i] = 0;
            t.tie[i] = 0;
        }
        if (isPoly())
        {
            for (auto& n : notes)
                for (int k = 0; k < n.length && n.start + k < len; ++k)
                {
                    const auto i = (size_t) (n.start + k);
                    t.chord[i] = t.chord[i].load() | (1ull << n.pitch);
                    if (k < n.length - 1) t.tie[i] = t.tie[i].load() | (1ull << n.pitch);
                }
            for (int s = 0; s < len; ++s)
            {
                const auto mask = t.chord[(size_t) s].load();
                if (mask == 0) continue;
                t.on[(size_t) s] = true;
                for (int n = 0; n < noteRange; ++n)
                    if ((mask >> n) & 1u) { t.note[(size_t) s] = n; break; }
            }
        }
        else
        {
            auto sorted = notes;
            std::sort (sorted.begin(), sorted.end(), [] (const Note& a, const Note& b) { return a.start < b.start; });
            for (auto& n : sorted)
                for (int k = 0; k < n.length && n.start + k < len; ++k)
                {
                    const auto i = (size_t) (n.start + k);
                    t.on[i] = true;
                    t.note[i] = n.pitch;
                    t.slide[i] = k < n.length - 1;
                }
            for (int s = 0; s + 1 < len; ++s)
                if (glide[(size_t) s] && t.on[(size_t) s] && t.on[(size_t) s + 1]) t.slide[(size_t) s] = true;
        }
        t.tile (len);
        processor.touchPattern();
        knownVersion = processor.patternVersion.load();
        if (onChange) onChange();
        repaint();
    }

    void beginEdit()
    {
        processor.undoManager.beginNewTransaction();
        processor.beginPatternEdit();
    }

    void commit()
    {
        resolveMono();
        writeNotes();
    }

    void preview (int pitch, bool on)
    {
        if (pitch < 0) return;
        processor.previewNote (type, baseNote() + snapToScale (processor.currentScale(), pitch), on);
    }

    TechMachineProcessor& processor;
    int type = 0;
    const Skin* skin = nullptr;
    std::vector<Note> notes, original, clipboard;
    std::vector<int> chordPreview;
    std::vector<bool> glide;
    float stepW = 0.0f, scrollX = 0.0f, scrollY = 0.0f;
    int playhead = -1, grabbed = -1, downStep = 0, downPitch = 0, previewPitch = -1, pressedKey = -1, dragLane = -1;
    bool laneValue = false;
    Drag dragMode = Drag::None;
    juce::Point<float> downPos;
    juce::Rectangle<float> selectionRect;
};

// The piano roll screen: a toolbar above the grid.
class PianoRollEditor : public juce::Component
{
public:
    PianoRollEditor (TechMachineProcessor& p, int machineType) : processor (p), grid (p, machineType)
    {
        addAndMakeVisible (grid);
        grid.onClose = [this] { if (onClose) onClose(); };

        for (int t : rackOrder)
            if (machineDef (t).kind != MachineKind::Drum) machineBox.addItem (utf8 (machineDef (t).name), t + 1);
        machineBox.setSelectedId (machineType + 1, juce::dontSendNotification);
        machineBox.onChange = [this] { grid.setMachine (machineBox.getSelectedId() - 1); grid.fitAll(); };
        machineBox.setTooltip (utf8 ("La machine dont tu écris les notes"));
        addAndMakeVisible (machineBox);

        lengthBox.addItem ("1/16", 1);
        lengthBox.addItem ("1/8", 2);
        lengthBox.addItem ("1/4", 4);
        lengthBox.addItem ("1/2", 8);
        lengthBox.addItem (utf8 ("1 mesure"), 16);
        lengthBox.setSelectedId (2, juce::dontSendNotification);
        lengthBox.onChange = [this] { grid.defaultLength = lengthBox.getSelectedId(); };
        lengthBox.setTooltip (utf8 ("Longueur des nouvelles notes (elle suit aussi la dernière note que tu as allongée)"));
        addAndMakeVisible (lengthBox);

        stackBox.addItem (utf8 ("Poser : une note"), 1);
        stackBox.addItem (utf8 ("Poser : un accord"), 3);
        stackBox.addItem (utf8 ("Poser : un accord de 7e"), 4);
        stackBox.setSelectedId (1, juce::dontSendNotification);
        stackBox.onChange = [this] { grid.stackSize = stackBox.getSelectedId(); };
        stackBox.setTooltip (utf8 ("Un clic pose une note, ou un accord entier qui sonne juste dans la gamme (synthés polyphoniques)"));
        addAndMakeVisible (stackBox);
        auto updateStack = [this]
        {
            const bool poly = machineDef (grid.machine()).kind == MachineKind::Poly;
            stackBox.setEnabled (poly);
            if (! poly) stackBox.setSelectedId (1, juce::sendNotificationSync);
        };
        updateStack();
        machineBox.onChange = [this, updateStack] { grid.setMachine (machineBox.getSelectedId() - 1); grid.fitAll(); updateStack(); };

        auto button = [this] (IconButton& b, const char* tip, std::function<void()> action)
        {
            b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3b40));
            b.setColour (juce::TextButton::textColourOffId, colours::text);
            b.setTooltip (utf8 (tip));
            b.onClick = [this, action] { action(); grid.grabKeyboardFocus(); };
            addAndMakeVisible (b);
        };
        button (octaveDown, "Descend les notes sélectionnées (ou toutes) d'une octave", [this] { grid.transposeSelection (-12); });
        button (octaveUp, "Monte les notes sélectionnées (ou toutes) d'une octave", [this] { grid.transposeSelection (12); });
        button (duplicate, "Recopie les notes sélectionnées (ou toutes) juste après elles  (Ctrl+D)", [this] { grid.duplicateSelection(); });
        button (selectAll, "Sélectionne toutes les notes  (Ctrl+A)", [this] { grid.selectAll(); });
        button (fit, "Affiche toute la séquence", [this] { grid.fitAll(); });
        button (erase, "Efface les notes sélectionnées  (Suppr)", [this] { grid.deleteSelection(); });
        button (close, "Revenir au rack  (Échap)", [this] { if (onClose) onClose(); });
        close.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff6b46c1));
    }

    std::function<void()> onClose;
    PianoRollGrid& getGrid() { return grid; }

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colour (0xff101113));
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
        g.setColour (colours::text);
        g.setFont (paint::display (22.0f));
        g.drawText ("PIANO ROLL", 16, 8, 160, 30, juce::Justification::centredLeft);
        g.setColour (colours::dim);
        g.setFont (paint::font (12.5f, false));
        g.drawText (utf8 ("Clic = poser · glisser = déplacer · bord droit = allonger · clic droit ou double-clic = effacer · "
                          "Ctrl+glisser = sélectionner · Alt+glisser = copier · flèches = déplacer / transposer (Maj = mesure / octave) · "
                          "Ctrl+molette = zoom · Maj+molette = défiler"),
                    16, 40, getWidth() - 32, 16, juce::Justification::centredLeft);
    }

    void resized() override
    {
        auto bar = getLocalBounds().reduced (12, 8).removeFromTop (30);
        bar.removeFromLeft (160);
        close.setBounds (bar.removeFromRight (110));
        bar.removeFromRight (10);
        machineBox.setBounds (bar.removeFromLeft (170));
        bar.removeFromLeft (14);
        lengthBox.setBounds (bar.removeFromLeft (90));
        bar.removeFromLeft (10);
        stackBox.setBounds (bar.removeFromLeft (170));
        bar.removeFromLeft (14);
        for (auto* b : { &octaveDown, &octaveUp, &duplicate, &selectAll, &fit, &erase })
        {
            b->setBounds (bar.removeFromLeft (b == &selectAll ? 140 : b == &fit || b == &erase ? 94 : 102));
            bar.removeFromLeft (6);
        }
        grid.setBounds (getLocalBounds().reduced (10).withTrimmedTop (54));
    }

private:
    TechMachineProcessor& processor;
    PianoRollGrid grid;
    juce::ComboBox machineBox, lengthBox, stackBox;
    IconButton octaveDown { "down", utf8 ("OCTAVE −") }, octaveUp { "plus", utf8 ("OCTAVE +") }, duplicate { "patterns", utf8 ("DUPLIQUER") },
               selectAll { "", utf8 ("TOUT SÉLECTIONNER") }, fit { "fullscreen", utf8 ("TOUT VOIR") }, erase { "trash", utf8 ("EFFACER") },
               close { "close", utf8 ("FERMER") };
};
} // namespace techno
