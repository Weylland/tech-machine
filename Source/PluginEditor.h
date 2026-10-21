#pragma once

#include "PluginProcessor.h"
#include "ui/MachinePanel.h"
#include "ui/AutomationEditor.h"
#include "ui/SongEditor.h"
#include "ui/PianoRoll.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

// Amber display in the middle of the console: tempo (drag to change), project, genre, key, position.
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
        g.drawFittedText (techno::utf8 ("Ton rack est vide.\nClique sur « + MACHINE » pour ajouter une boîte à rythmes, une basse ou des accords."),
                          getLocalBounds().reduced (40), juce::Justification::centred, 3);
    }
};

// Shown when the app starts: new project, open, recent projects and examples.
class StartScreen : public juce::Component
{
public:
    explicit StartScreen (TechMachineProcessor&);

    void refresh();
    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void()> onNew, onOpen, onResume;
    std::function<void (const juce::File&)> onOpenFile;
    std::function<void (int)> onExample;

private:
    TechMachineProcessor& processor;
    techno::IconButton newButton { "plus", techno::utf8 ("NOUVEAU PROJET VIDE") };
    techno::IconButton openButton { "folder", techno::utf8 ("OUVRIR UN PROJET…") };
    techno::IconButton resumeButton { "play", techno::utf8 ("REPRENDRE LA DERNIÈRE SESSION") };
    juce::OwnedArray<juce::TextButton> recentButtons;
    juce::OwnedArray<juce::Button> exampleCards;
    juce::Viewport cardsView;
    juce::Component cardsContent;
    juce::Rectangle<int> panel;
};

// The whole interface. Rebuilt from scratch when the language changes.
class MainView : public juce::Component, private juce::Timer
{
public:
    explicit MainView (TechMachineProcessor&);
    ~MainView() override;

    std::function<void()> onLanguageChange;
    bool isStartScreenShown() const { return startScreen.isVisible(); }
    void showStartScreen (bool show);

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    class ExportJob;

    void timerCallback() override;
    void togglePlay();
    void toggleRecord();
    void showAddMenu();
    void showProjectMenu();
    void toggleFullScreen();
    void toggleLanguage();
    void showAudioSettings();
    void openPianoRoll (int machineType);
    void closePianoRoll();
    void openAutomation (int machineType);
    void closeAutomation();
    void openSong();
    void closeSong();
    void showPartMenu (int part);
    void refreshParts();
    std::unique_ptr<techno::SongEditor> songEditor;
    std::unique_ptr<techno::PianoRollEditor> pianoRoll;
    std::unique_ptr<techno::AutomationEditor> automationEditor;
    void setPage (int newPage);
    void layoutRack();
    techno::InfoButton& addInfo (const techno::HelpText&);
    void styleButton (juce::Button&, juce::Colour colour, juce::Colour text = techno::colours::text);
    void printLabel (juce::Graphics&, const juce::Component&, const char* text) const;

    // Project actions; askThen first offers to save unsaved changes.
    void askThen (std::function<void()> action);
    void newProject();
    void openProject();
    void openProjectFile (const juce::File&);
    void loadExample (int index);
    void saveProject (std::function<void()> then = {});
    void saveProjectAs (std::function<void()> then = {});
    void exportWav (int loops);
    void exportStems (int loops);
    void exportMidi();
    void projectChanged();

    TechMachineProcessor& processor;
    techno::Theme theme;
    juce::TooltipWindow tooltips { this, 450 };

    techno::IconButton playButton { "play", techno::utf8 ("LECTURE") }, recordButton { "rec", "REC" };
    techno::IconButton surpriseButton { "dice", "SURPRISE !" }, addButton { "plus", "MACHINE" }, guideButton { "", "?" };
    techno::IconButton projectButton { "folder", techno::utf8 ("PROJET") }, undoButton { "undo" }, redoButton { "redo" };
    techno::IconButton fullScreenButton { "fullscreen", techno::utf8 ("PLEIN ÉCRAN") }, languageButton { "globe", juce::String::fromUTF8 ("FRANÇAIS") };
    techno::IconButton audioButton { "gear", techno::utf8 ("AUDIO / MIDI") };
    techno::LedButton midiRecordButton { techno::utf8 ("ENREG. MIDI"), juce::Colour (0xffff3b1f) };
    techno::LedButton automationRecordButton { techno::utf8 ("ENREG. BOUTONS"), juce::Colour (0xffff3b1f) };
    techno::InfoButton* automationRecordInfo = nullptr;
    std::array<techno::MenuTextButton, TechMachineProcessor::numParts> partButtons;
    techno::LedButton songPlayButton { techno::utf8 ("JOUER LE MORCEAU"), juce::Colour (0xff3dff6a) };
    techno::IconButton songEditButton { "patterns", techno::utf8 ("MORCEAU") };
    techno::InfoButton* partsInfo = nullptr;
    techno::InfoButton* songInfo = nullptr;
    techno::LedButton fillButton { "FILL", juce::Colour (0xffff9a1f) };
    std::array<techno::MenuTextButton, TechMachineProcessor::numScenes> sceneButtons;
    techno::InfoButton* fillInfo = nullptr;
    techno::InfoButton* scenesInfo = nullptr;
    void showSceneMenu (int scene);
    void refreshScenes();
    int lastPartsVersion = -1;
    int lastMidiMapVersion = 0, lastMidiTarget = -2;
    std::uint64_t lastShownMask = ~0ull;
    int lastSampleVersion = -1;
    techno::LedButton barButtons[4] { { "1", techno::colours::lcd }, { "2", techno::colours::lcd }, { "3", techno::colours::lcd }, { "4", techno::colours::lcd } };
    techno::LedButton pageButtons[4] { { "1", techno::colours::info }, { "2", techno::colours::info }, { "3", techno::colours::info }, { "4", techno::colours::info } };
    techno::LedButton followButton { techno::utf8 ("SUIVRE"), techno::colours::info };

    LcdDisplay lcd;
    std::unique_ptr<techno::Knob> swingKnob;
    juce::ComboBox genreBox, keyBox, scaleBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttachment, scaleAttachment;
    juce::OwnedArray<techno::InfoButton> infos;
    techno::InfoButton* playInfo = nullptr, *lcdInfo = nullptr, *genreInfo = nullptr, *keyInfo = nullptr, *scaleInfo = nullptr;
    techno::InfoButton* surpriseInfo = nullptr, *recInfo = nullptr, *addInfoButton = nullptr, *projectInfo = nullptr, *undoInfo = nullptr;
    techno::InfoButton* lengthInfo = nullptr, *pageInfo = nullptr, *masterInfo = nullptr, *meterInfo = nullptr;

    juce::Viewport rack;
    RackContent content;
    juce::OwnedArray<techno::MachinePanel> panels;

    juce::OwnedArray<juce::Component> masterControls, masterChoices;
    VuMeter vu;
    juce::Rectangle<int> consoleArea, masterArea;

    StartScreen startScreen;
    std::unique_ptr<juce::FileChooser> chooser;
    std::unique_ptr<ExportJob> exportJob;

    int viewPage = 0, lastVersion = -1, lastBars = -1, lastLabelHash = -1, lastGenre = -1;
    int scrollToMachine = -1;
    bool follow = true, focusGrabbed = false;
    juce::String lastTitle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainView)
};

class TechMachineEditor : public juce::AudioProcessorEditor
{
public:
    explicit TechMachineEditor (TechMachineProcessor&);
    ~TechMachineEditor() override;

    void resized() override;

private:
    void rebuild (bool showStart);

    TechMachineProcessor& techProcessor;   // AudioProcessorEditor::processor, with its real type
    std::unique_ptr<MainView> view;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TechMachineEditor)
};
