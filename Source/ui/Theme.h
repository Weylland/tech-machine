#pragma once

#include "Paint.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace techno::colours
{
inline const juce::Colour background { 0xff121215 };
inline const juce::Colour panel { 0xff1b1c21 };
inline const juce::Colour panelEdge { 0xff2b2c33 };
inline const juce::Colour text { 0xffe9e6e0 };
inline const juce::Colour dim { 0xff9a9aa2 };
inline const juce::Colour accent { 0xffff6a2b };
inline const juce::Colour master { 0xffb48cff };
inline const juce::Colour record { 0xffff3b4e };
inline const juce::Colour info { 0xff6c8cff };
inline const juce::Colour lcd { 0xffffb341 };
inline const juce::Colour play { 0xff3ddc6a };
} // namespace techno::colours

namespace techno
{
inline juce::Font uiFont (float size, bool bold = true) { return paint::font (size, bold); }

// Look and feel of one faceplate: knobs, faders, buttons and selectors follow the skin.
class Theme : public juce::LookAndFeel_V4
{
public:
    explicit Theme (const Skin& s = consoleSkin()) : skin (s)
    {
        setColour (juce::ResizableWindow::backgroundColourId, colours::background);
        setColour (juce::Label::textColourId, skin.textDim);
        setColour (juce::TextButton::buttonColourId, skin.caps[0]);
        setColour (juce::TextButton::buttonOnColourId, skin.lit);
        setColour (juce::TextButton::textColourOffId, colours::text);
        setColour (juce::TextButton::textColourOnId, colours::text);
        setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff0b0b0d));
        setColour (juce::ComboBox::outlineColourId, juce::Colours::black);
        setColour (juce::ComboBox::textColourId, colours::lcd);
        setColour (juce::ComboBox::arrowColourId, colours::lcd);
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1b1c20));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff35363e));
        setColour (juce::PopupMenu::textColourId, colours::text);
        setColour (juce::PopupMenu::highlightedTextColourId, colours::lcd);
        setColour (juce::PopupMenu::headerTextColourId, colours::lcd.withAlpha (0.8f));
        setColour (juce::BubbleComponent::backgroundColourId, juce::Colour (0xff1b1c20));
        setColour (juce::BubbleComponent::outlineColourId, juce::Colour (0xff3a3b42));
        setColour (juce::TooltipWindow::backgroundColourId, juce::Colour (0xff1b1c20));
        setColour (juce::TooltipWindow::textColourId, colours::text);
        setColour (juce::TooltipWindow::outlineColourId, juce::Colour (0xff3a3b42));
        setColour (juce::ScrollBar::thumbColourId, juce::Colour (0xff4a4b52));
        setColour (juce::ScrollBar::trackColourId, juce::Colours::transparentBlack);
    }

    const Skin& skin;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float pos,
                           float startAngle, float endAngle, juce::Slider& slider) override
    {
        paint::knob (g, juce::Rectangle<int> (x, y, width, height).toFloat(), pos, startAngle, endAngle, skin,
                     (bool) slider.getProperties()["bipolar"]);
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos,
                           float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style == juce::Slider::LinearVertical)
        {
            const float margin = (float) getSliderThumbRadius (slider);
            paint::fader (g, juce::Rectangle<int> (x, y, width, height).toFloat(), sliderPos,
                          (float) y + (float) height - margin, (float) y + margin, skin);
        }
        else
            LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }

    int getSliderThumbRadius (juce::Slider& slider) override
    {
        return slider.isVertical() ? 8 : LookAndFeel_V4::getSliderThumbRadius (slider);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& background,
                               bool isHighlighted, bool isDown) override
    {
        auto b = button.getLocalBounds().toFloat().reduced (1.0f, 1.5f);
        const float radius = juce::jmin (5.0f, b.getHeight() * 0.25f);
        auto base = button.getToggleState() ? button.findColour (juce::TextButton::buttonOnColourId) : background;
        if (! button.isEnabled()) base = base.withMultipliedSaturation (0.3f).withMultipliedBrightness (0.6f);
        else if (isHighlighted && ! isDown) base = base.brighter (0.08f);

        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (b.translated (0.0f, 1.5f), radius);
        if (isDown) b.translate (0.0f, 1.0f);

        g.setGradientFill (juce::ColourGradient (base.brighter (0.2f), b.getX(), b.getY(), base.darker (isDown ? 0.45f : 0.3f), b.getX(), b.getBottom(), false));
        g.fillRoundedRectangle (b, radius);
        g.setTiledImageFill (paint::noise(), 0, 0, 1.0f);
        g.fillRoundedRectangle (b, radius);
        g.setColour (juce::Colours::white.withAlpha (isDown ? 0.08f : 0.25f));
        g.drawHorizontalLine ((int) b.getY() + 1, b.getX() + radius, b.getRight() - radius);
        g.setColour (juce::Colours::black.withAlpha (0.65f));
        g.drawRoundedRectangle (b, radius, 0.8f);
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool isDown) override
    {
        g.setFont (paint::condensed (juce::jmin (16.0f, (float) button.getHeight() * 0.52f)));
        g.setColour (button.findColour (button.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId)
                         .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));
        g.drawFittedText (button.getButtonText(), button.getLocalBounds().reduced (4, 2).translated (0, isDown ? 1 : 0),
                          juce::Justification::centred, 1, 0.8f);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override
    {
        const auto b = juce::Rectangle<int> (width, height).toFloat().reduced (1.0f);
        paint::recess (g, b, 4.0f);
        g.setColour (box.findColour (juce::ComboBox::textColourId).withAlpha (0.06f));
        g.fillRoundedRectangle (b.reduced (2.0f), 3.0f);

        juce::Path arrow;
        const float ax = b.getRight() - 14.0f, ay = b.getCentreY();
        arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
        g.setColour (box.findColour (juce::ComboBox::arrowColourId));
        g.fillPath (arrow);
    }

    juce::Font getComboBoxFont (juce::ComboBox& box) override
    {
        return paint::condensed (juce::jmin (16.0f, (float) box.getHeight() * 0.55f));
    }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (6, 1, box.getWidth() - 26, box.getHeight() - 2);
        label.setFont (getComboBoxFont (box));
        label.setMinimumHorizontalScale (0.8f);   // long names are squeezed a little before being cut
    }

    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
    {
        g.fillAll (findColour (juce::PopupMenu::backgroundColourId));
        g.setColour (juce::Colour (0xff3a3b42));
        g.drawRect (0, 0, width, height);
    }

    juce::Font getPopupMenuFont() override { return paint::font (15.0f, false); }
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override { return paint::condensed (juce::jmin (16.0f, (float) buttonHeight * 0.52f)); }
    juce::Font getLabelFont (juce::Label& label) override { return label.getFont(); }
};
} // namespace techno
