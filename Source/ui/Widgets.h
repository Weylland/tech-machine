#pragma once

#include "../core/Help.h"
#include "../core/Music.h"
#include "Theme.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace techno
{
inline const Skin& skinOf (juce::Component& c)
{
    if (auto* theme = dynamic_cast<Theme*> (&c.getLookAndFeel())) return theme->skin;
    return consoleSkin();
}

// Content of an info bubble: a title and wrapped explanatory text.
class InfoPanel : public juce::Component
{
public:
    InfoPanel (const juce::String& titleText, const juce::String& bodyText) : title (titleText)
    {
        juce::AttributedString text;
        text.append (bodyText, paint::font (15.5f, false), colours::text);
        text.setLineSpacing (3.0f);
        layout.createLayout (text, (float) width - 36.0f);
        setSize (width, 62 + (int) std::ceil (layout.getHeight()) + 18);
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (colours::info);
        g.fillEllipse (18.0f, 19.0f, 18.0f, 18.0f);
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions ("Georgia", 15.0f, juce::Font::bold | juce::Font::italic));
        g.drawText ("i", 18, 19, 18, 18, juce::Justification::centred);

        g.setColour (colours::text);
        g.setFont (paint::display (19.0f));
        g.drawText (title, 44, 14, getWidth() - 62, 28, juce::Justification::centredLeft);
        g.setColour (colours::info.withAlpha (0.4f));
        g.drawHorizontalLine (48, 18.0f, (float) getWidth() - 18.0f);
        layout.draw (g, { 18.0f, 58.0f, (float) getWidth() - 36.0f, layout.getHeight() });
    }

private:
    static constexpr int width = 440;
    juce::String title;
    juce::TextLayout layout;
};

inline void showInfo (const juce::String& title, const juce::String& body, juce::Component& target)
{
    juce::CallOutBox::launchAsynchronously (std::make_unique<InfoPanel> (title, body), target.getScreenBounds(), nullptr);
}

inline void showInfo (const HelpText& help, juce::Component& target) { showInfo (help.title, help.body, target); }

// Small round "i" printed on the faceplate; opens an explanation.
class InfoButton : public juce::Component, public juce::SettableTooltipClient
{
public:
    InfoButton() { setMouseCursor (juce::MouseCursor::PointingHandCursor); }

    void setHelp (const juce::String& titleText, const juce::String& bodyText)
    {
        title = titleText;
        body = bodyText;
        setTooltip (utf8 ("Clique pour comprendre à quoi ça sert"));
    }

    void setHelp (const HelpText& h) { setHelp (h.title, h.body); }

    void paint (juce::Graphics& g) override
    {
        const auto& skin = skinOf (*this);
        const auto b = getLocalBounds().toFloat();
        const float d = juce::jmin (b.getWidth(), b.getHeight()) - 2.0f;
        const auto circle = juce::Rectangle<float> (d, d).withCentre (b.getCentre());
        const auto ink = skin.lightPlate ? juce::Colour (0xff2a2a2c) : juce::Colour (0xffe9e6e0);

        g.setColour (hover ? colours::info : ink.withAlpha (0.75f));
        g.fillEllipse (circle);
        g.setColour (hover ? juce::Colours::white : (skin.lightPlate ? juce::Colour (0xffe9e6e0) : juce::Colour (0xff1a1a1c)));
        g.setFont (juce::FontOptions ("Georgia", d * 0.82f, juce::Font::bold | juce::Font::italic));
        g.drawText ("i", circle.translated (0.0f, 0.5f), juce::Justification::centred);
    }

    void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override { hover = false; repaint(); }
    void mouseUp (const juce::MouseEvent& e) override
    {
        if (contains (e.getPosition())) showInfo (title, body, *this);
    }

private:
    juce::String title, body;
    bool hover = false;
};

// Printed label under a control, with its "i".
class ControlLabel : public juce::Component
{
public:
    ControlLabel (const juce::String& text, const HelpText& help) : name (text)
    {
        info.setHelp (help);
        addAndMakeVisible (info);
    }

    void resized() override { info.setBounds (getLocalBounds().removeFromRight (14).withSizeKeepingCentre (13, 13)); }

    void paint (juce::Graphics& g) override
    {
        g.setColour (skinOf (*this).text);
        g.setFont (paint::condensed (12.0f));
        g.drawFittedText (name, getLocalBounds().withTrimmedRight (15), juce::Justification::centred, 1, 0.75f);
    }

private:
    juce::String name;
    InfoButton info;
};

// Knob (or fader) bound to a parameter, with a printed name and an "i". Right-click also opens the help.
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& name,
          const HelpText& helpText, bool showLabel = true, bool asFader = false)
        : help (helpText), label (name, helpText), fader (asFader)
    {
        slider.setSliderStyle (fader ? juce::Slider::LinearVertical : juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setPopupDisplayEnabled (true, true, nullptr);
        slider.setTooltip (help.title + " : " + shortHelp (help.body));
        slider.addMouseListener (this, false);
        addAndMakeVisible (slider);
        if (showLabel) addAndMakeVisible (label);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, paramId, slider);
        if (auto* p = state.getParameter (paramId))
            slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
    }

    void setBipolar() { slider.getProperties().set ("bipolar", true); }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu()) showInfo (help, *this);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        if (label.isVisible()) label.setBounds (b.removeFromBottom (16));
        slider.setBounds (fader ? b.withSizeKeepingCentre (juce::jmin (b.getWidth(), 40), b.getHeight()) : b);
    }

private:
    HelpText help;
    juce::Slider slider;
    ControlLabel label;
    bool fader;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// A choice parameter as a small display-style selector with its printed name.
class ChoiceControl : public juce::Component
{
public:
    ChoiceControl (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& name,
                   const std::vector<const char*>& choices, const HelpText& help)
        : label (name, help)
    {
        int id = 1;
        for (auto* c : choices) box.addItem (utf8 (c), id++);
        box.setTooltip (help.title + " : " + shortHelp (help.body));
        addAndMakeVisible (box);
        addAndMakeVisible (label);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, paramId, box);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromBottom (16));
        box.setBounds (b.withSizeKeepingCentre (b.getWidth() - 4, 28));
    }

private:
    juce::ComboBox box;
    ControlLabel label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

// Hardware button with an LED. Row style = LED + printed name (used to mute a drum sound).
class LedButton : public juce::Button
{
public:
    enum class Style { Hardware, Row };

    LedButton (const juce::String& text, juce::Colour ledColour, Style s = Style::Hardware, bool ledWhenOff = false)
        : juce::Button (text), led (ledColour), style (s), invert (ledWhenOff)
    {
        setClickingTogglesState (true);
    }

    void setIconName (const juce::String& name) { iconName = name; }

    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        const auto& skin = skinOf (*this);
        const bool lit = getToggleState() != invert;
        auto b = getLocalBounds().toFloat();

        if (style == Style::Row)
        {
            if (highlighted) { g.setColour (skin.text.withAlpha (0.08f)); g.fillRoundedRectangle (b, 4.0f); }
            paint::led (g, { b.getX() + 9.0f, b.getCentreY() }, 4.0f, led, lit);
            g.setColour (lit ? skin.text : skin.textDim.withAlpha (0.6f));
            g.setFont (paint::condensed (14.0f));
            g.drawFittedText (getButtonText(), b.withTrimmedLeft (20.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.8f);
            return;
        }

        getLookAndFeel().drawButtonBackground (g, *this, findColour (juce::TextButton::buttonColourId), highlighted, down);
        const float y = down ? 1.0f : 0.0f;
        paint::led (g, { b.getX() + 10.0f, b.getCentreY() + y }, 3.5f, led, lit);

        auto textArea = b.withTrimmedLeft (18.0f).translated (0.0f, y);
        if (iconName.isNotEmpty())
        {
            paint::icon (g, iconName, textArea.removeFromLeft (b.getHeight() * 0.6f).reduced (2.0f), findColour (juce::TextButton::textColourOffId));
        }
        g.setColour (findColour (juce::TextButton::textColourOffId));
        g.setFont (paint::condensed (juce::jmin (15.0f, b.getHeight() * 0.52f)));
        g.drawFittedText (getButtonText(), textArea.toNearestInt(), juce::Justification::centred, 1, 0.8f);
    }

private:
    juce::Colour led;
    Style style;
    bool invert;
    juce::String iconName;
};

// LED button bound to a boolean parameter.
class ParamLed : public LedButton
{
public:
    ParamLed (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& text,
              juce::Colour ledColour, Style s = Style::Hardware, bool ledWhenOff = false)
        : LedButton (text, ledColour, s, ledWhenOff)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, paramId, *this);
    }

private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
};

// Hardware button with a drawn icon and text.
class IconButton : public juce::TextButton
{
public:
    IconButton (const juce::String& iconName, const juce::String& text = {}) : juce::TextButton (text), icon (iconName) {}

    void setIcon (const juce::String& name) { icon = name; repaint(); }

    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        getLookAndFeel().drawButtonBackground (g, *this, findColour (buttonColourId), highlighted, down);
        auto b = getLocalBounds().toFloat().reduced (6.0f, 4.0f).translated (0.0f, down ? 1.0f : 0.0f);
        const auto colour = findColour (textColourOffId);
        if (getButtonText().isEmpty())
        {
            paint::icon (g, icon, b, colour);
            return;
        }
        if (icon.isEmpty())
        {
            g.setColour (colour);
            g.setFont (paint::condensed (juce::jmin (16.0f, b.getHeight() * 0.62f)));
            g.drawFittedText (getButtonText(), b.toNearestInt(), juce::Justification::centred, 1, 0.8f);
            return;
        }
        const float iconSize = juce::jmin (b.getHeight(), 22.0f);
        const float textWidth = juce::GlyphArrangement::getStringWidth (paint::condensed (juce::jmin (16.0f, b.getHeight() * 0.62f)), getButtonText());
        const float total = iconSize + 6.0f + textWidth;
        auto row = b.withSizeKeepingCentre (juce::jmin (total, b.getWidth()), b.getHeight());
        paint::icon (g, icon, row.removeFromLeft (iconSize), colour);
        row.removeFromLeft (6.0f);
        g.setColour (colour);
        g.setFont (paint::condensed (juce::jmin (16.0f, b.getHeight() * 0.62f)));
        g.drawFittedText (getButtonText(), row.toNearestInt(), juce::Justification::centredLeft, 1, 0.8f);
    }

private:
    juce::String icon;
};

// One sequencer step drawn as a hardware key. Toggle keys can be painted by dragging along a row;
// value keys (notes, chords) are switched on by a click and changed by dragging up/down or the wheel.
class StepCell : public juce::Component, public juce::SettableTooltipClient
{
public:
    enum class Mode { Toggle, Value };

    StepCell (int columnIndex, int rowId, Mode m, bool dimmedKey = false) : column (columnIndex), row (rowId), mode (m), dimmed (dimmedKey) {}

    std::function<bool()> isOn;
    std::function<void (bool)> setOn;
    std::function<juce::String()> getLabel;
    std::function<void (int)> nudge;
    std::function<void()> onEditStart;   // called once at the start of each edit gesture (for undo)
    std::function<int()> getRoll;         // drum cells only: 0 = single hit, 2..4 = roll
    std::function<void (int)> setRoll;

    void cycleRoll()
    {
        const int next = getRoll() >= 4 ? 0 : getRoll() == 0 ? 2 : getRoll() + 1;
        if (! isOn()) setOn (true);
        setRoll (next);
        repaint();
    }

    const int column;

    void setPlayhead (bool shouldShow)
    {
        if (playhead == shouldShow) return;
        playhead = shouldShow;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto& skin = skinOf (*this);
        const bool on = isOn();
        auto cap = skin.caps[(size_t) ((column / 4) % 4)];
        auto lit = skin.lit;
        if (skin.litUsesCap)
        {
            lit = cap.brighter (0.3f);
            cap = cap.withMultipliedBrightness (0.5f).withMultipliedSaturation (0.75f);
        }
        if (dimmed)
        {
            cap = cap.withMultipliedBrightness (0.8f);
            lit = lit.withMultipliedSaturation (0.7f).withMultipliedBrightness (0.85f);
        }
        paint::pad (g, getLocalBounds().toFloat(), cap, lit, on, playhead);
        if (skin.litUsesCap && on && mode == Mode::Toggle)
            paint::led (g, { (float) getWidth() * 0.5f, 6.0f }, 2.6f, skin.led, true);

        if (getRoll && on)
        {
            const int roll = getRoll();
            g.setColour (skin.capText.withAlpha (0.85f));
            for (int k = 0; k < roll; ++k)
                g.fillRoundedRectangle ((float) getWidth() * 0.5f - (float) roll * 3.0f + (float) k * 6.0f + 1.0f, (float) getHeight() - 9.0f, 4.0f, 4.0f, 1.0f);
        }

        if (mode == Mode::Value && on && getLabel)
        {
            g.setColour (skin.capText);
            g.setFont (paint::condensed (getWidth() < 40 ? 13.0f : 14.0f));
            g.drawFittedText (getLabel(), getLocalBounds().reduced (2), juce::Justification::centred, 2, 0.7f);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (onEditStart) onEditStart();
        if (getRoll && e.mods.isPopupMenu())
        {
            cycleRoll();
            return;
        }
        if (mode == Mode::Toggle)
        {
            paintState = ! isOn();
            setOn (paintState);
        }
        else
        {
            wasOnAtDown = isOn();
            if (! wasOnAtDown) setOn (true);
            dragStartY = e.getPosition().y;
            dragSteps = 0;
            dragged = false;
        }
        repaint();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu()) return;
        if (mode == Mode::Value)
        {
            const int steps = (dragStartY - e.getPosition().y) / 9;
            if (steps != dragSteps)
            {
                nudge (steps - dragSteps);
                dragSteps = steps;
                dragged = true;
                repaint();
            }
            return;
        }

        auto* parent = getParentComponent();
        if (auto* other = dynamic_cast<StepCell*> (parent->getComponentAt (e.getEventRelativeTo (parent).getPosition())))
        {
            if (other->row == row && other->isOn() != paintState)
            {
                other->setOn (paintState);
                other->repaint();
            }
        }
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (mode == Mode::Value && wasOnAtDown && ! dragged)
        {
            setOn (false);
            repaint();
        }
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        if (getRoll && isOn() && wheel.deltaY != 0.0f)
        {
            const auto now = juce::Time::getMillisecondCounter();
            if (onEditStart && now - lastWheelEdit > 600) onEditStart();
            lastWheelEdit = now;
            cycleRoll();
            return;
        }
        if (mode != Mode::Value || ! isOn() || wheel.deltaY == 0.0f)
        {
            Component::mouseWheelMove (e, wheel);
            return;
        }
        const auto now = juce::Time::getMillisecondCounter();
        if (onEditStart && now - lastWheelEdit > 600) onEditStart();
        lastWheelEdit = now;
        nudge (wheel.deltaY > 0.0f ? 1 : -1);
        repaint();
    }

private:
    inline static bool paintState = true;

    const int row;
    const Mode mode;
    const bool dimmed;
    bool playhead = false, wasOnAtDown = false, dragged = false;
    int dragStartY = 0, dragSteps = 0;
    juce::uint32 lastWheelEdit = 0;
};

// Small "FX" button of a drum sound: lit when one of its effects is active.
class FxButton : public juce::TextButton, private juce::Timer
{
public:
    FxButton (juce::AudioProcessorValueTreeState& state, const juce::String& rowPrefix) : juce::TextButton ("FX")
    {
        for (auto* suffix : { "drive", "filter", "pan", "echo", "reverb" })
            params.push_back (state.getRawParameterValue (rowPrefix + suffix));
        startTimerHz (4);
    }

    bool isActive() const
    {
        return params[0]->load() > 0.001f || std::abs (params[1]->load() - 0.5f) > 0.02f || std::abs (params[2]->load() - 0.5f) > 0.02f
            || params[3]->load() > 0.001f || params[4]->load() > 0.001f;
    }

    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        const auto& skin = skinOf (*this);
        const bool active = isActive();
        getLookAndFeel().drawButtonBackground (g, *this, active ? skin.accent.withMultipliedBrightness (0.8f) : juce::Colour (0xff3a3b40),
                                               highlighted, down);
        g.setColour (active ? juce::Colours::white : colours::text.withAlpha (0.75f));
        g.setFont (paint::condensed (12.0f));
        g.drawText ("FX", getLocalBounds().translated (0, down ? 1 : 0), juce::Justification::centred);
    }

private:
    void timerCallback() override
    {
        const bool active = isActive();
        if (active != wasActive) repaint();
        wasActive = active;
    }

    std::vector<std::atomic<float>*> params;
    bool wasActive = false;
};

// Row of running LEDs above the steps: shows which step is playing.
class LedStrip : public juce::Component
{
public:
    std::function<int (int)> columnX;
    int columnWidth = 38;

    void setStep (int newColumn)
    {
        if (newColumn == column) return;
        column = newColumn;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto& skin = skinOf (*this);
        for (int c = 0; c < 16; ++c)
        {
            const float x = (float) (columnX (c) - getX()) + (float) columnWidth * 0.5f;
            paint::led (g, { x, (float) getHeight() * 0.5f }, 3.2f, skin.led, c == column);
        }
    }

private:
    int column = -1;
};
} // namespace techno
