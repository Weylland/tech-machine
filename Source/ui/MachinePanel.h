#pragma once

#include "../PluginProcessor.h"
#include "PianoRoll.h"
#include "SampleEditor.h"
#include "Widgets.h"

namespace techno
{
// One machine of the rack, drawn as its hardware faceplate.
class MachinePanel : public juce::Component, public juce::FileDragAndDropTarget
{
public:
    MachinePanel (TechMachineProcessor&, int machineType, const int& viewPage);
    ~MachinePanel() override;

    int preferredHeight (int width) const;
    void setPlayhead (int step);
    void refreshCells();

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;

    std::function<void()> onLayoutChange;
    const int type;

private:
    struct Item
    {
        juce::Component* component;
        int width, height;
    };

    StepCell& addCell (int column, int row, StepCell::Mode, bool dimmed = false);
    void showDiceMenu();
    void showPatternMenu();
    void showSoundMenu();

public:
    // Sampler: drop audio files on the pads, and show the names of the loaded sounds.
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    void refreshSampleNames();
    void askSoundName();

    // Opens the piano roll screen for this machine (set by the main view).
    std::function<void (int machineType)> onOpenRoll;
    std::function<void (int machineType)> onOpenAutomation;
    std::function<void (int slot)> onOpenSampleEditor;

    // Moving the machine in the rack: drag its name band (y in the rack's coordinates; finished on release).
    std::function<void (int machineType, int y, bool finished)> onReorderDrag;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

    // Shows the MIDI badge on the machine that receives the keyboard.
    void setMidiTarget (bool isTarget)
    {
        if (midiTarget == isTarget) return;
        midiTarget = isTarget;
        repaint();
    }

private:
    bool midiTarget = false;
    bool pressedOnBand = false, reordering = false;
    int shownStep = -2, shownPage = -1;
    int subtitleRight = 600;   // the subtitle stops before the header buttons
    bool showBrand = true;
    std::unique_ptr<juce::FileChooser> chooser;
    void chooseSampleFile (std::function<void (const juce::File&)> then);
    void showMicRecorder (int slot);
    IconButton rollButton { "keys", "PIANO ROLL" };
    IconButton automationButton { "curve", "AUTOMATION" };
    void showMoreMenu();
    void showRowFx (int row);
    InfoButton& addInfo (const HelpText&);
    void addHeaderButton (juce::Button&, const juce::String& helpKey);
    int stepFor (int column) const { return page * pageSteps + column; }
    int cellX (int column) const;
    juce::String cellLabel (int step) const;
    void nudgeNote (int step, int delta);
    int layoutFlow (const std::vector<Item>&, juce::Rectangle<int> area, bool apply) const;
    void layoutRow (int row, int y, int height);

    bool isDrum() const { return def.kind == MachineKind::Drum; }
    bool isPoly() const { return def.kind == MachineKind::Poly; }
    int numRows() const { return (int) def.rows.size(); }
    int side() const { return skin.wood ? 28 : 22; }
    int stepsX() const { return side() + 150; }
    int rightX() const { return side() + 858; }
    int sequencerBottom() const;
    std::array<int, 3> synthRowY() const;

    TechMachineProcessor& processor;
    const MachineDef& def;
    const Skin& skin;
    Theme theme;
    const int& page;
    bool collapsed = false;

    InfoButton machineInfo;
    std::unique_ptr<ParamLed> muteButton, soloButton;
    IconButton soundButton { "sound", utf8 ("SONS") }, patternButton { "patterns", "PATTERNS" }, diceButton { "dice", utf8 ("HASARD") }, moreButton { "more" }, collapseButton { "down" }, removeButton { "close" };
    std::vector<std::pair<juce::Button*, InfoButton*>> headerButtons;

    juce::OwnedArray<InfoButton> infos;
    juce::OwnedArray<ParamLed> rowMutes;
    juce::OwnedArray<FxButton> fxButtons;
    juce::OwnedArray<Knob> rowKnobs;
    std::vector<InfoButton*> rowInfoButtons, columnInfos;
    juce::OwnedArray<StepCell> cells;
    std::vector<std::vector<StepCell*>> rows;
    LedStrip ledStrip;

    juce::OwnedArray<juce::Component> soundControls, stripControls;
    std::vector<Item> soundItems, stripItems;
    juce::Rectangle<int> soundBox, stripBox;
};
} // namespace techno
