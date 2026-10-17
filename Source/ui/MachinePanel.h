#pragma once

#include "../PluginProcessor.h"
#include "Widgets.h"

namespace techno
{
// One machine of the rack, drawn as its hardware faceplate.
class MachinePanel : public juce::Component
{
public:
    MachinePanel (TechMachineProcessor&, int machineType, const int& viewPage);
    ~MachinePanel() override;

    int preferredHeight (int width) const;
    void setPlayhead (int step);
    void refreshCells();

    void paint (juce::Graphics&) override;
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
    IconButton diceButton { "dice", "HASARD" }, clearButton { "trash", "EFFACER" }, collapseButton { "down" }, removeButton { "close" };
    std::vector<std::pair<juce::Button*, InfoButton*>> headerButtons;

    juce::OwnedArray<InfoButton> infos;
    juce::OwnedArray<ParamLed> rowMutes;
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
