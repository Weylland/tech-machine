#pragma once

#include "Widgets.h"

namespace techno
{
// A guided tour: a bubble next to the part of the interface to use, which moves on by itself once the step is done.
// Only the bubble catches the mouse: the rest of the interface stays usable underneath.
class TutorialOverlay : public juce::Component, private juce::Timer
{
public:
    struct Step
    {
        juce::String text;
        std::function<juce::Component*()> target;   // nullptr: the bubble sits in the middle
        std::function<bool()> done;                 // nullptr: only the button moves on
    };

    explicit TutorialOverlay (std::vector<Step> tourSteps) : steps (std::move (tourSteps))
    {
        setInterceptsMouseClicks (true, true);
        for (auto* b : { &skip, &next })
        {
            b->setColour (juce::TextButton::buttonColourId, b == &next ? juce::Colour (0xff2f4fa8) : juce::Colour (0xff3a3b40));
            b->setColour (juce::TextButton::textColourOffId, colours::text);
            addAndMakeVisible (b);
        }
        skip.onClick = [this] { finish(); };
        next.onClick = [this] { advance(); };
        startTimerHz (8);
        update();
    }

    std::function<void()> onFinish;
    int currentStep() const { return index; }
    int numSteps() const { return (int) steps.size(); }
    juce::Rectangle<int> bubbleBounds() const { return bubble; }

    void advance()
    {
        if (index + 1 >= (int) steps.size()) { finish(); return; }
        ++index;
        update();
    }

    bool hitTest (int x, int y) override { return bubble.contains (x, y); }

    void paint (juce::Graphics& g) override
    {
        if (! ring.isEmpty())
        {
            g.setColour (juce::Colour (0xff3dd6ff).withAlpha (0.25f));
            g.fillRoundedRectangle (ring.toFloat().expanded (6.0f), 8.0f);
            g.setColour (juce::Colour (0xff3dd6ff));
            g.drawRoundedRectangle (ring.toFloat().expanded (6.0f), 8.0f, 3.0f);
        }
        g.setColour (juce::Colour (0xf0202a3a));
        g.fillRoundedRectangle (bubble.toFloat(), 10.0f);
        g.setColour (juce::Colour (0xff3dd6ff));
        g.drawRoundedRectangle (bubble.toFloat().reduced (0.5f), 10.0f, 1.5f);
        g.setFont (paint::condensed (13.0f));
        g.drawText (utf8 ("TUTORIEL  ") + juce::String (index + 1) + " / " + juce::String ((int) steps.size()), bubble.getX() + 16,
                    bubble.getY() + 10, 200, 18, juce::Justification::centredLeft);
        g.setColour (colours::text);
        g.setFont (paint::font (15.0f, false));
        g.drawFittedText (steps[(size_t) index].text, bubble.reduced (16).withTrimmedTop (22).withTrimmedBottom (34),
                          juce::Justification::topLeft, 6, 0.9f);
    }

private:
    void finish()
    {
        stopTimer();
        setVisible (false);
        if (onFinish) onFinish();
    }

    // Places the bubble under its target (or above, if there is no room), inside the window.
    void update()
    {
        auto& step = steps[(size_t) index];
        auto* target = step.target ? step.target() : nullptr;
        const auto oldRing = ring, oldBubble = bubble;
        ring = target != nullptr && target->isShowing() ? getLocalArea (target, target->getLocalBounds()) : juce::Rectangle<int>();
        const int w = 400, h = 168;
        auto area = getLocalBounds().reduced (12);
        juce::Rectangle<int> b (w, h);
        if (ring.isEmpty()) b.setCentre (area.getCentre());
        else
        {
            // Big targets (a whole machine): the bubble goes inside their top-right corner.
            if (ring.getHeight() > 240) b.setPosition (ring.getRight() - w - 30, ring.getY() + 70);
            else b.setPosition (ring.getCentreX() - w / 2, ring.getBottom() + 14 + h <= area.getBottom() ? ring.getBottom() + 14 : ring.getY() - 14 - h);
        }
        bubble = b.constrainedWithin (area);
        if (ring == oldRing && bubble == oldBubble && index == shownIndex) return;   // only what moved is redrawn
        shownIndex = index;
        for (auto r : { oldRing, ring }) repaint (r.expanded (12));
        for (auto r : { oldBubble, bubble }) repaint (r);
        next.setButtonText (index + 1 >= (int) steps.size() ? utf8 ("TERMINER") : utf8 ("SUIVANT"));
        next.setBounds (bubble.getRight() - 16 - 110, bubble.getBottom() - 40, 110, 28);
        skip.setBounds (next.getX() - 8 - 150, bubble.getBottom() - 40, 150, 28);
    }

    void timerCallback() override
    {
        auto& step = steps[(size_t) index];
        if (step.done && step.done()) advance();
        else update();   // the target may have moved (scrolling, a machine added)
    }

    std::vector<Step> steps;
    int index = 0, shownIndex = -1;
    juce::Rectangle<int> ring, bubble;
    juce::TextButton skip { utf8 ("ARRÊTER LE TUTORIEL") }, next { utf8 ("SUIVANT") };
};
} // namespace techno
