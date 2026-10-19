#pragma once

#include "../PluginProcessor.h"
#include "Widgets.h"

namespace techno
{
// Full-screen editor of a machine's automation: pick a setting, then draw its value step by step.
// A step without a value follows the knob.
class AutomationEditor : public juce::Component, private juce::Timer
{
public:
    AutomationEditor (TechMachineProcessor& p, int machineType) : processor (p), type (machineType)
    {
        setWantsKeyboardFocus (true);

        targetBox.setTooltip (utf8 ("Le réglage dont tu dessines l'évolution. ● = déjà automatisé."));
        targetBox.onChange = [this]
        {
            if (targetBox.getSelectedId() > 0) target = targetBox.getSelectedId() - 1;
            repaint();
        };
        addAndMakeVisible (targetBox);

        auto button = [this] (IconButton& b, const char* tip, std::function<void()> action)
        {
            b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3b40));
            b.setColour (juce::TextButton::textColourOffId, colours::text);
            b.setTooltip (utf8 (tip));
            b.onClick = [this, action] { action(); grabKeyboardFocus(); };
            addAndMakeVisible (b);
        };
        button (clearOne, "Efface l'automation de ce réglage : il suit de nouveau son bouton", [this]
        {
            processor.undoManager.beginNewTransaction();
            processor.beginPatternEdit();
            processor.clearAutomation (type, target);
        });
        button (clearAll, "Efface l'automation de tous les réglages de cette machine", [this]
        {
            processor.undoManager.beginNewTransaction();
            processor.beginPatternEdit();
            processor.machine (type).track.clearLocks();
            processor.touchPattern();
        });
        button (close, "Revenir au rack  (Échap)", [this] { if (onClose) onClose(); });
        close.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff6b46c1));

        // Start on the first automated setting, otherwise on the filter.
        target = automation::Filter;
        for (auto& t : processor.machine (type).track.lockTarget)
            if (t.load() >= 0) { target = t.load(); break; }
        refreshTargets();
        startTimerHz (30);
    }

    std::function<void()> onClose;
    int machine() const { return type; }
    int selectedTarget() const { return target; }

    void selectTarget (int newTarget)
    {
        target = newTarget;
        refreshTargets();
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colour (0xff101113));
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
        g.setColour (colours::text);
        g.setFont (paint::display (22.0f));
        g.drawText ("AUTOMATION", 16, 8, 170, 30, juce::Justification::centredLeft);
        g.setFont (paint::condensed (15.0f));
        g.setColour (colours::dim);
        g.drawText (utf8 (machineDef (type).name), 190, 8, 150, 30, juce::Justification::centredLeft);

        const auto area = gridArea();
        g.setColour (juce::Colour (0xff17181b));
        g.fillRect (area);

        const int steps = processor.numSteps();
        const float w = (float) area.getWidth() / (float) steps;
        const int choices = automation::choiceCount (type, target);

        // Ruler and bar lines
        g.setFont (paint::condensed (12.0f));
        for (int s = 0; s < steps; ++s)
        {
            const float x = (float) area.getX() + (float) s * w;
            if (s % 4 == 0)
            {
                g.setColour (s % 16 == 0 ? juce::Colour (0xff4a4c52) : juce::Colour (0xff2a2c30));
                g.drawVerticalLine ((int) x, (float) area.getY(), (float) area.getBottom());
            }
            if (s % 16 == 0)
            {
                g.setColour (colours::text);
                g.drawText (juce::String (s / 16 + 1), (int) x + 4, area.getY() - 20, 30, 18, juce::Justification::centredLeft);
            }
        }

        // The knob's own value: what the steps without a value play.
        const float knob = normalised (processor.apvts.getRawParameterValue (automation::paramId (type, target))->load());
        const float knobY = yFor (knob);
        g.setColour (colours::dim.withAlpha (0.8f));
        const float dash[] { 6.0f, 5.0f };
        g.drawDashedLine ({ (float) area.getX(), knobY, (float) area.getRight(), knobY }, dash, 2, 1.5f);

        auto& track = processor.machine (type).track;
        const int lane = track.laneFor (target, false);
        const auto accent = juce::Colour (0xff8a7dff);
        for (int s = 0; s < steps; ++s)
        {
            const float v = lane >= 0 ? track.lockValue[(size_t) lane][(size_t) s].load() : noLock;
            const auto cell = juce::Rectangle<float> ((float) area.getX() + (float) s * w + 1.0f, (float) area.getY(), w - 2.0f, (float) area.getHeight());
            if (v < 0.0f)
            {
                g.setColour (colours::dim.withAlpha (0.35f));
                g.fillEllipse (juce::Rectangle<float> (4.0f, 4.0f).withCentre ({ cell.getCentreX(), knobY }));
                continue;
            }
            const float y = yFor (normalised (v));
            g.setColour (accent.withAlpha (0.55f));
            g.fillRect (cell.withTop (y));
            g.setColour (accent.brighter (0.4f));
            g.fillRect (cell.withTop (y).withHeight (3.0f));
            if (choices > 0 && w > 34.0f)
            {
                g.setColour (colours::text);
                g.setFont (paint::condensed (11.0f));
                const auto& names = machineDefParamChoices();
                const int index = juce::jlimit (0, choices - 1, juce::roundToInt (v));
                g.drawFittedText (utf8 (names[(size_t) index]), cell.withTop (y - 16.0f).withHeight (14.0f).toNearestInt(),
                                  juce::Justification::centred, 1, 0.7f);
            }
        }

        // Playhead
        const int step = processor.currentStep.load();
        if (processor.playing.load() && step >= 0 && step < steps)
        {
            g.setColour (juce::Colours::white.withAlpha (0.12f));
            g.fillRect (juce::Rectangle<float> ((float) area.getX() + (float) step * w, (float) area.getY(), w, (float) area.getHeight()));
        }

        g.setColour (colours::dim);
        g.setFont (paint::font (12.5f, false));
        g.drawFittedText (utf8 ("Clic glissé = dessiner la valeur de chaque pas · clic droit = effacer (le pas suit de nouveau le bouton) · "
                                "pointillés = position du bouton · ENREG. BOUTONS en haut = enregistrer en tournant les boutons pendant la lecture"),
                          getLocalBounds().reduced (16, 0).removeFromBottom (24), juce::Justification::centredLeft, 1, 0.8f);
    }

    void resized() override
    {
        auto row = getLocalBounds().reduced (12, 0).withTrimmedTop (8).removeFromTop (30);
        row.removeFromLeft (340);
        close.setBounds (row.removeFromRight (110));
        row.removeFromRight (8);
        clearAll.setBounds (row.removeFromRight (140));
        row.removeFromRight (8);
        clearOne.setBounds (row.removeFromRight (190));
        targetBox.setBounds (row.removeFromLeft (juce::jmin (260, row.getWidth())));
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (! gridArea().contains (e.getPosition())) return;
        processor.undoManager.beginNewTransaction();
        processor.beginPatternEdit();
        erasing = e.mods.isPopupMenu();
        lastStep = -1;
        draw (e.getPosition());
    }

    void mouseDrag (const juce::MouseEvent& e) override { if (lastStep != -2) draw (e.getPosition()); }
    void mouseUp (const juce::MouseEvent&) override { lastStep = -2; refreshTargets(); }

    bool keyPressed (const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::escapeKey)
        {
            if (onClose) onClose();
            return true;
        }
        return false;
    }

    // The centre of a step's column at a value (0..1), used by the tests.
    juce::Point<int> pointFor (int step, float value) const
    {
        const auto area = gridArea();
        const float w = (float) area.getWidth() / (float) processor.numSteps();
        return { (int) ((float) area.getX() + ((float) step + 0.5f) * w), (int) yFor (value) };
    }

private:
    juce::Rectangle<int> gridArea() const { return getLocalBounds().reduced (12, 0).withTrimmedTop (70).withTrimmedBottom (30); }

    float yFor (float normalisedValue) const
    {
        const auto area = gridArea().toFloat().reduced (0.0f, 6.0f);
        return area.getBottom() - normalisedValue * area.getHeight();
    }

    float normalised (float raw) const
    {
        const int choices = automation::choiceCount (type, target);
        return choices > 1 ? raw / (float) (choices - 1) : juce::jlimit (0.0f, 1.0f, raw);
    }

    float rawFor (float normalisedValue) const
    {
        const int choices = automation::choiceCount (type, target);
        if (choices > 1) return (float) juce::roundToInt (normalisedValue * (float) (choices - 1));
        return juce::jlimit (0.0f, 1.0f, normalisedValue);
    }

    const std::vector<const char*>& machineDefParamChoices() const
    {
        return machineDef (type).params[(size_t) (target - automation::stripCount)].choices;
    }

    // Writes the steps under the mouse; a fast drag fills the steps it jumped over.
    void draw (juce::Point<int> pos)
    {
        const auto area = gridArea().toFloat().reduced (0.0f, 6.0f);
        const int steps = processor.numSteps();
        const int step = juce::jlimit (0, steps - 1, (int) ((float) (pos.x - gridArea().getX()) / ((float) gridArea().getWidth() / (float) steps)));
        const float value = juce::jlimit (0.0f, 1.0f, (area.getBottom() - (float) pos.y) / area.getHeight());
        const int from = lastStep < 0 ? step : lastStep;
        const float fromValue = lastStep < 0 ? value : lastValue;
        const int count = std::abs (step - from);
        for (int k = 0; k <= count; ++k)
        {
            const int s = from + (step > from ? k : -k);
            const float v = count == 0 ? value : fromValue + (value - fromValue) * (float) k / (float) count;
            processor.setAutomation (type, target, s, erasing ? noLock : rawFor (v));
        }
        lastStep = step;
        lastValue = value;
        repaint();
    }

    void refreshTargets()
    {
        targetBox.clear (juce::dontSendNotification);
        auto& track = processor.machine (type).track;
        for (int t = 0; t < automation::targetCount (type); ++t)
        {
            const bool automated = track.laneFor (t, false) >= 0;
            targetBox.addItem ((automated ? juce::String (juce::CharPointer_UTF8 ("\xe2\x97\x8f ")) : juce::String()) + automation::label (type, t), t + 1);
        }
        targetBox.setSelectedId (target + 1, juce::dontSendNotification);
        repaint();
    }

    void timerCallback() override
    {
        const int version = processor.patternVersion.load();
        if (version != knownVersion)
        {
            knownVersion = version;
            if (lastStep == -2) refreshTargets();
        }
        repaint();
    }

    TechMachineProcessor& processor;
    const int type;
    int target = 0, lastStep = -2, knownVersion = -1;
    float lastValue = 0.0f;
    bool erasing = false;
    juce::ComboBox targetBox;
    IconButton clearOne { "trash", utf8 ("EFFACER CE RÉGLAGE") }, clearAll { "trash", utf8 ("TOUT EFFACER") }, close { "close", utf8 ("FERMER") };
};
} // namespace techno
