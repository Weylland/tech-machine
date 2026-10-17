#pragma once

#include "PluginProcessor.h"
#include "ui/MachinePanel.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

// Amber display in the middle of the console: tempo (drag to change), style, key, position.
class LcdDisplay : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit LcdDisplay (TechMachineProcessor&);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    void setTempo (float bpm);

    TechMachineProcessor& processor;
    juce::RangedAudioParameter& tempo;
    float dragStartTempo = 0.0f;
};

// Analog VU meter with needle ballistics.
class VuMeter : public juce::Component
{
public:
    void push (float peak)
    {
        const float db = juce::Decibels::gainToDecibels (peak, -60.0f) + 8.0f;
        level += (db - level) * (db > level ? 0.35f : 0.12f);
        repaint();
    }

    void paint (juce::Graphics& g) override { techno::paint::vuMeter (g, getLocalBounds().toFloat(), level); }

private:
    float level = -40.0f;
};

class RackContent : public juce::Component
{
public:
    bool empty = true;

    void paint (juce::Graphics& g) override
    {
        if (! empty) return;
        g.setColour (techno::colours::dim);
        g.setFont (techno::paint::font (20.0f));
        g.drawFittedText (techno::utf8 ("Ton rack est vide.\nClique sur « + AJOUTER UNE MACHINE » pour choisir tes instruments."),
                          getLocalBounds().reduced (40), juce::Justification::centred, 3);
    }
};

class TechMachineEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TechMachineEditor (TechMachineProcessor&);
    ~TechMachineEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    void timerCallback() override;
    void togglePlay();
    void toggleRecord();
    void showAddMenu();
    void setPage (int newPage);
    void layoutRack();
    techno::InfoButton& addInfo (const techno::HelpText&);
    void styleButton (juce::Button&, juce::Colour colour, juce::Colour text = techno::colours::text);
    void printLabel (juce::Graphics&, const juce::Component&, const char* text) const;

    TechMachineProcessor& processor;
    techno::Theme theme;
    juce::TooltipWindow tooltips { this, 450 };

    techno::IconButton playButton { "play", "LECTURE" }, recordButton { "rec", "REC" }, folderButton { "", "FICHIERS" };
    techno::IconButton surpriseButton { "dice", "SURPRISE !" }, addButton { "plus", "AJOUTER UNE MACHINE" };
    techno::IconButton recommendedButton { "plus", utf8text ("CONSEILLÉES") }, guideButton { "", "?" };
    techno::LedButton barButtons[4] { { "1", techno::colours::lcd }, { "2", techno::colours::lcd }, { "3", techno::colours::lcd }, { "4", techno::colours::lcd } };
    techno::LedButton pageButtons[4] { { "1", techno::colours::info }, { "2", techno::colours::info }, { "3", techno::colours::info }, { "4", techno::colours::info } };
    techno::LedButton followButton { "SUIVRE", techno::colours::info };

    LcdDisplay lcd;
    std::unique_ptr<techno::Knob> swingKnob;
    juce::ComboBox styleBox, keyBox, scaleBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttachment, scaleAttachment;
    juce::OwnedArray<techno::InfoButton> infos;
    techno::InfoButton* playInfo = nullptr, *lcdInfo = nullptr, *styleInfo = nullptr, *keyInfo = nullptr, *scaleInfo = nullptr;
    techno::InfoButton* surpriseInfo = nullptr, *recInfo = nullptr, *addInfoButton = nullptr, *recommendedInfo = nullptr;
    techno::InfoButton* lengthInfo = nullptr, *pageInfo = nullptr, *masterInfo = nullptr, *meterInfo = nullptr;

    juce::Viewport rack;
    RackContent content;
    juce::OwnedArray<techno::MachinePanel> panels;

    juce::OwnedArray<juce::Component> masterControls;
    VuMeter vu;
    juce::Rectangle<int> consoleArea, masterArea;

    int viewPage = 0, lastStep = -1, lastVersion = -1, lastShownMask = -1, lastBars = -1, lastLabelHash = -1, lastStyle = -1;
    int scrollToMachine = -1;
    bool follow = true, focusGrabbed = false;

    static juce::String utf8text (const char* t) { return juce::String::fromUTF8 (t); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TechMachineEditor)
};
