#include "PluginEditor.h"

#if JucePlugin_Build_Standalone
 #include <juce_audio_utils/juce_audio_utils.h>
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

using namespace techno;

namespace
{
constexpr int consoleH = 118, masterH = 122, railW = 20;
const char* const projectExtension = ".techmachine";

void disableKeyboardFocus (juce::Component& c)
{
    c.setWantsKeyboardFocus (false);
    c.setMouseClickGrabsKeyboardFocus (false);
    for (auto* child : c.getChildren()) disableKeyboardFocus (*child);
}

void rail (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3c3d42), r.getX(), r.getY(), juce::Colour (0xff222327), r.getRight(), r.getY(), false));
    g.fillRect (r);
    g.setTiledImageFill (paint::noise(), 0, 0, 1.0f);
    g.fillRect (r);
    for (float y = r.getY() + 14.0f; y < r.getBottom() - 8.0f; y += 22.0f)
    {
        const auto hole = juce::Rectangle<float> (7.0f, 10.0f).withCentre ({ r.getCentreX(), y });
        g.setColour (juce::Colour (0xff0a0a0b));
        g.fillRoundedRectangle (hole, 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.drawHorizontalLine ((int) hole.getBottom(), hole.getX(), hole.getRight());
    }
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawRect (r, 1.0f);
}

void screws (juce::Graphics& g, juce::Rectangle<int> area)
{
    const auto a = area.toFloat();
    for (auto p : { a.getTopLeft() + juce::Point<float> (10.0f, 10.0f), a.getTopRight() + juce::Point<float> (-10.0f, 10.0f),
                    a.getBottomLeft() + juce::Point<float> (10.0f, -10.0f), a.getBottomRight() + juce::Point<float> (-10.0f, -10.0f) })
        paint::screw (g, p, 4.5f, p.x * 0.013f);
}

juce::Colour genreColour (const juce::String& genre)
{
    static const juce::Colour palette[] { juce::Colour (0xffff6a2b), juce::Colour (0xfff5d442), juce::Colour (0xff5ac8fa), juce::Colour (0xffff4d6d),
                                          juce::Colour (0xffb48cff), juce::Colour (0xff7bd88f), juce::Colour (0xffff7ad9), juce::Colour (0xff4fa3ff),
                                          juce::Colour (0xffd9a066), juce::Colour (0xff59c3ff) };
    return palette[(size_t) (genre.hashCode() & 0x7fffffff) % std::size (palette)];
}

// One example project on the start screen.
class ExampleCard : public juce::Button
{
public:
    explicit ExampleCard (const ExampleInfo& e) : juce::Button (utf8 (e.name)), info (e)
    {
        setTooltip (utf8 (e.description));
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        auto b = getLocalBounds().toFloat().reduced (2.0f);
        if (down) b.translate (0.0f, 1.0f);
        const auto colour = genreColour (utf8 (info.genre));
        paint::plate (g, b, juce::Colour (highlighted ? 0xff34353c : 0xff2a2b31), juce::Colour (0xff1c1d21), 6.0f, false);
        g.setColour (colour);
        g.fillRoundedRectangle ({ b.getX() + 8.0f, b.getY() + 12.0f, 4.0f, b.getHeight() - 24.0f }, 2.0f);

        g.setColour (colours::text);
        g.setFont (paint::display (19.0f));
        g.drawText (utf8 (info.name), (int) b.getX() + 22, (int) b.getY() + 10, (int) b.getWidth() - 34, 24, juce::Justification::centredLeft);
        g.setColour (colour);
        g.setFont (paint::font (13.0f));
        g.drawText (utf8 (info.genre).toUpperCase() + "  ·  " + juce::String (info.bpm) + " BPM",
                    (int) b.getX() + 22, (int) b.getY() + 34, (int) b.getWidth() - 34, 16, juce::Justification::centredLeft);
        g.setColour (colours::dim);
        g.setFont (paint::font (12.5f, false));
        g.drawFittedText (utf8 (info.description), juce::Rectangle<int> ((int) b.getX() + 22, (int) b.getY() + 52, (int) b.getWidth() - 34,
                                                                          (int) b.getHeight() - 58), juce::Justification::topLeft, 2, 0.9f);
        paint::icon (g, "play", { b.getRight() - 40.0f, b.getY() + 12.0f, 22.0f, 22.0f }, colours::text.withAlpha (highlighted ? 0.9f : 0.35f));
    }

private:
    ExampleInfo info;
};
} // namespace

//==============================================================================
class MainView::ExportJob : public juce::ThreadWithProgressWindow
{
public:
    ExportJob (std::unique_ptr<juce::XmlElement> project, const juce::File& target, int loopCount, std::function<void (bool)> done)
        : ThreadWithProgressWindow (utf8 ("Export WAV en cours…"), true, true),
          file (target), xml (std::move (project)), loops (loopCount), onDone (std::move (done))
    {
    }

    void run() override
    {
        ok = TechMachineProcessor::exportWav (*xml, file, loops, [this] (float p)
        {
            setProgress (p);
            return ! threadShouldExit();
        });
    }

    void threadComplete (bool cancelled) override
    {
        if (onDone) onDone (ok && ! cancelled);
    }

    const juce::File file;

private:
    std::unique_ptr<juce::XmlElement> xml;
    int loops;
    std::function<void (bool)> onDone;
    bool ok = false;
};

//==============================================================================
LcdDisplay::LcdDisplay (TechMachineProcessor& p)
    : processor (p), tempo (*p.apvts.getParameter (pid::bpm))
{
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    setTooltip (utf8 ("Tempo : glisse vers le haut ou le bas (ou molette) pour le changer. Double-clic = 130 BPM."));
}

void LcdDisplay::setTempo (float bpm)
{
    tempo.setValueNotifyingHost (tempo.convertTo0to1 (juce::jlimit (60.0f, 200.0f, std::round (bpm))));
}

void LcdDisplay::mouseDown (const juce::MouseEvent&)
{
    if (processor.isHosted()) return;
    dragStartTempo = tempo.convertFrom0to1 (tempo.getValue());
    tempo.beginChangeGesture();
}

void LcdDisplay::mouseDrag (const juce::MouseEvent& e)
{
    if (processor.isHosted()) return;
    setTempo (dragStartTempo - (float) e.getDistanceFromDragStartY() * 0.25f);
}

void LcdDisplay::mouseUp (const juce::MouseEvent&)
{
    if (! processor.isHosted()) tempo.endChangeGesture();
}

void LcdDisplay::mouseDoubleClick (const juce::MouseEvent&)
{
    if (! processor.isHosted()) setTempo (130.0f);
}

void LcdDisplay::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    if (processor.isHosted()) return;
    setTempo (tempo.convertFrom0to1 (tempo.getValue()) + (wheel.deltaY > 0 ? 1.0f : -1.0f));
}

void LcdDisplay::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    paint::recess (g, b, 6.0f);
    const auto glass = b.reduced (5.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a1c06), glass.getX(), glass.getY(), juce::Colour (0xff120c03), glass.getX(), glass.getBottom(), false));
    g.fillRoundedRectangle (glass, 4.0f);

    const auto amber = colours::lcd;
    const float bpm = processor.isHosted() ? processor.displayBpm.load() : tempo.convertFrom0to1 (tempo.getValue());

    auto left = glass.reduced (12.0f, 8.0f).removeFromLeft (glass.getWidth() * 0.4f);
    g.setColour (amber.withAlpha (0.7f));
    g.setFont (paint::condensed (11.0f));
    g.drawText (processor.isHosted() ? utf8 ("TEMPO DAW (SYNC)") : utf8 ("TEMPO"), left.removeFromTop (13.0f), juce::Justification::centredLeft);
    auto digits = left.removeFromTop (left.getHeight() - 14.0f);
    paint::sevenSegment (g, juce::String (juce::roundToInt (bpm)).paddedLeft (' ', 3), digits.withWidth (digits.getHeight() * 1.9f), amber);
    g.setColour (amber.withAlpha (0.7f));
    g.drawText ("BPM", left, juce::Justification::centredLeft);

    auto right = glass.reduced (12.0f, 7.0f).withTrimmedLeft (glass.getWidth() * 0.43f);
    g.setColour (amber);
    g.setFont (paint::display (17.0f));
    g.drawFittedText (processor.projectName().toUpperCase() + (processor.isDirty() ? juce::String::fromUTF8 (" •") : juce::String()),
                      right.removeFromTop (21.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);

    g.setColour (amber.withAlpha (0.85f));
    g.setFont (paint::condensed (13.0f));
    g.drawFittedText (utf8 ("GENRE  ") + utf8 (styles()[(size_t) processor.genre()].name).toUpperCase(),
                      right.removeFromTop (17.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);
    g.drawFittedText (juce::String (keyNames[processor.currentKey()]) + "  " + utf8 (scales()[(size_t) processor.currentScale()].name).toUpperCase(),
                      right.removeFromTop (17.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);

    const int step = processor.currentStep.load();
    const int bars = processor.numSteps() / pageSteps;
    auto bottom = right.removeFromTop (18.0f);
    g.drawText (step >= 0 ? utf8 ("MESURE ") + juce::String (step / pageSteps + 1) + "/" + juce::String (bars) : juce::String ("STOP"),
                bottom.removeFromLeft (90.0f), juce::Justification::centredLeft);
    for (int beat = 0; beat < 4; ++beat)
    {
        const bool on = step >= 0 && (step % pageSteps) / 4 == beat;
        paint::led (g, { bottom.getX() + 8.0f + (float) beat * 16.0f, bottom.getCentreY() }, 3.5f, amber, on);
    }

    for (float y = glass.getY(); y < glass.getBottom(); y += 3.0f)
    {
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        g.drawHorizontalLine ((int) y, glass.getX(), glass.getRight());
    }
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.1f), glass.getX(), glass.getY(),
                                             juce::Colours::transparentWhite, glass.getX(), glass.getCentreY(), false));
    g.fillRoundedRectangle (glass, 4.0f);
}

//==============================================================================
StartScreen::StartScreen (TechMachineProcessor& p) : processor (p)
{
    for (auto* b : { &newButton, &openButton, &resumeButton })
    {
        b->setColour (juce::TextButton::buttonColourId, b == &newButton ? juce::Colour (0xffd2551f) : juce::Colour (0xff3a3b40));
        b->setColour (juce::TextButton::textColourOffId, colours::text);
        addAndMakeVisible (b);
    }
    newButton.onClick = [this] { if (onNew) onNew(); };
    openButton.onClick = [this] { if (onOpen) onOpen(); };
    resumeButton.onClick = [this] { if (onResume) onResume(); };
    newButton.setTooltip (utf8 ("Un projet tout neuf : aucune machine, aucun effet. Ajoute ensuite les machines que tu veux."));
    resumeButton.setTooltip (utf8 ("Retrouve exactement ce que tu faisais la dernière fois."));

    for (int i = 0; i < (int) examples().size(); ++i)
    {
        auto* card = exampleCards.add (new ExampleCard (examples()[(size_t) i]));
        card->onClick = [this, i] { if (onExample) onExample (i); };
        cardsContent.addAndMakeVisible (card);
    }
}

void StartScreen::refresh()
{
    resumeButton.setVisible (! processor.isProjectEmpty());
    recentButtons.clear();
    for (auto& path : TechMachineProcessor::recentProjects())
    {
        if (recentButtons.size() >= 6) break;
        const juce::File f (path);
        auto* b = recentButtons.add (new juce::TextButton (f.getFileNameWithoutExtension()));
        b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff26272c));
        b->setColour (juce::TextButton::textColourOffId, colours::text);
        b->setTooltip (path);
        b->onClick = [this, f] { if (onOpenFile) onOpenFile (f); };
        addAndMakeVisible (b);
    }
    resized();
    repaint();
}

void StartScreen::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.82f));
    paint::plate (g, panel.toFloat(), juce::Colour (0xff2a2b30), juce::Colour (0xff17181b), 10.0f, true);
    screws (g, panel);

    const int x = panel.getX() + 40, y = panel.getY() + 34;
    g.setFont (paint::display (40.0f));
    g.setColour (colours::text);
    g.drawText ("TECH", x, y, 120, 46, juce::Justification::centredLeft);
    g.setColour (colours::accent);
    g.drawText ("MACHINE", x + 100, y, 220, 46, juce::Justification::centredLeft);
    g.setColour (colours::dim);
    g.setFont (paint::font (15.0f, false));
    g.drawText (utf8 ("Groovebox · techno, house, hip-hop, trap… · choisis comment commencer"), x, y + 46, 640, 20, juce::Justification::centredLeft);

    g.setColour (colours::dim);
    g.setFont (paint::condensed (13.0f));
    if (! recentButtons.isEmpty())
        g.drawText (utf8 ("PROJETS RÉCENTS"), x, recentButtons[0]->getY() - 22, 300, 16, juce::Justification::centredLeft);
    if (! exampleCards.isEmpty())
        g.drawText (utf8 ("PROJETS EXEMPLES — un par style, clique pour écouter et modifier (fais défiler)"), cardsView.getX(), cardsView.getY() - 24, 700, 16,
                    juce::Justification::centredLeft);
    g.setFont (paint::font (12.0f, false));
    g.drawText (utf8 ("Tu peux revenir ici à tout moment : menu PROJET › Accueil."), panel.getX() + 40, panel.getBottom() - 34, 600, 16,
                juce::Justification::centredLeft);
}

void StartScreen::resized()
{
    panel = getLocalBounds().withSizeKeepingCentre (juce::jmin (getWidth() - 60, 1260), juce::jmin (getHeight() - 60, 760));
    const int x = panel.getX() + 40;
    int y = panel.getY() + 130;
    newButton.setBounds (x, y, 340, 58);
    y += 70;
    openButton.setBounds (x, y, 340, 46);
    y += 56;
    if (resumeButton.isVisible())
    {
        resumeButton.setBounds (x, y, 340, 46);
        y += 56;
    }
    y += 34;
    for (auto* b : recentButtons)
    {
        b->setBounds (x, y, 340, 32);
        y += 36;
    }

    const int gridX = x + 390, gridY = panel.getY() + 130;
    constexpr int columns = 3, cardH = 92;
    if (cardsView.getViewedComponent() == nullptr)
    {
        cardsView.setViewedComponent (&cardsContent, false);
        cardsView.setScrollBarsShown (true, false);
        cardsView.setScrollBarThickness (10);
        addAndMakeVisible (cardsView);
    }
    cardsView.setBounds (gridX, gridY, panel.getRight() - 40 - gridX, panel.getBottom() - 56 - gridY);
    const int rows = (exampleCards.size() + columns - 1) / columns;
    const int cardW = (cardsView.getWidth() - 14 - 10 * (columns - 1)) / columns;
    cardsContent.setSize (cardsView.getWidth() - 12, rows * (cardH + 8));
    for (int i = 0; i < exampleCards.size(); ++i)
        exampleCards[i]->setBounds ((i % columns) * (cardW + 10), (i / columns) * (cardH + 8), cardW, cardH);
}

//==============================================================================
MainView::MainView (TechMachineProcessor& p)
    : processor (p), lcd (p), startScreen (p)
{
    setLookAndFeel (&theme);
    auto& state = processor.apvts;

    styleButton (playButton, juce::Colour (0xff2f8f4a));
    playButton.onClick = [this] { togglePlay(); };
    playInfo = &addInfo (help ("play"));

    styleButton (recordButton, juce::Colour (0xff4a2a2e));
    recordButton.onClick = [this] { toggleRecord(); };
    recInfo = &addInfo (help ("rec"));

    addAndMakeVisible (lcd);
    lcdInfo = &addInfo (help ("tempo"));

    swingKnob = std::make_unique<Knob> (state, pid::swing, "SWING", help ("swing"));
    addAndMakeVisible (*swingKnob);

    for (int i = 0; i < (int) styles().size(); ++i)
        genreBox.addItem (utf8 (styles()[(size_t) i].name), i + 1);
    genreBox.onChange = [this]
    {
        processor.setGenre (genreBox.getSelectedId() - 1);
        lastGenre = processor.genre();
    };
    genreBox.setTooltip (shortHelp (help ("genre").body));
    genreBox.setSelectedId (processor.genre() + 1, juce::dontSendNotification);
    addAndMakeVisible (genreBox);
    genreInfo = &addInfo (help ("genre"));

    for (int k = 0; k < 12; ++k) keyBox.addItem (keyNames[k], k + 1);
    addAndMakeVisible (keyBox);
    keyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, pid::key, keyBox);
    keyInfo = &addInfo (help ("key"));

    for (int s = 0; s < (int) scales().size(); ++s)
        scaleBox.addItem (utf8 (scales()[(size_t) s].name), s + 1);
    addAndMakeVisible (scaleBox);
    scaleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, pid::scale, scaleBox);
    scaleInfo = &addInfo (help ("scale"));

    styleButton (surpriseButton, juce::Colour (0xff6b46c1));
    surpriseButton.onClick = [this] { processor.generateAll(); };
    surpriseButton.setTooltip (shortHelp (help ("surprise").body));
    surpriseInfo = &addInfo (help ("surprise"));

    styleButton (guideButton, juce::Colour (0xff2f4fa8));
    guideButton.onClick = [this] { showInfo (help ("guide"), guideButton); };
    guideButton.setTooltip (utf8 ("Bien démarrer : le mode d'emploi en 8 étapes"));

    styleButton (projectButton, juce::Colour (0xff3a3b40));
    projectButton.onClick = [this] { showProjectMenu(); };
    projectButton.setTooltip (shortHelp (help ("project").body));
    projectInfo = &addInfo (help ("project"));

    styleButton (undoButton, juce::Colour (0xff3a3b40));
    undoButton.onClick = [this] { processor.undoManager.undo(); };
    undoButton.setTooltip (utf8 ("Annuler (Ctrl+Z)"));
    styleButton (redoButton, juce::Colour (0xff3a3b40));
    redoButton.onClick = [this] { processor.undoManager.redo(); };
    redoButton.setTooltip (utf8 ("Rétablir (Ctrl+Y)"));
    undoInfo = &addInfo (help ("undo"));

    for (int b = 0; b < 4; ++b)
    {
        barButtons[b].setClickingTogglesState (false);
        styleButton (barButtons[b], juce::Colour (0xff3a3b40));
        barButtons[b].onClick = [this, b] { processor.setBars (b + 1); };
        barButtons[b].setTooltip (juce::String (b + 1) + utf8 (b == 0 ? " mesure" : " mesures"));

        pageButtons[b].setClickingTogglesState (false);
        styleButton (pageButtons[b], juce::Colour (0xff3a3b40));
        pageButtons[b].onClick = [this, b]
        {
            follow = false;
            followButton.setToggleState (false, juce::dontSendNotification);
            setPage (b);
        };
        pageButtons[b].setTooltip (utf8 ("Afficher la mesure ") + juce::String (b + 1));
    }
    lengthInfo = &addInfo (help ("length"));

    followButton.setClickingTogglesState (false);
    styleButton (followButton, juce::Colour (0xff3a3b40));
    followButton.onClick = [this]
    {
        follow = ! follow;
        followButton.setToggleState (follow, juce::dontSendNotification);
    };
    followButton.setToggleState (true, juce::dontSendNotification);
    followButton.setTooltip (shortHelp (help ("follow").body));
    pageInfo = &addInfo (help ("page"));

    styleButton (addButton, juce::Colour (0xffd2551f));
    addButton.onClick = [this] { showAddMenu(); };
    addButton.setTooltip (shortHelp (help ("add").body));
    addInfoButton = &addInfo (help ("add"));

    rack.setViewedComponent (&content, false);
    rack.setScrollBarsShown (true, false);
    rack.setScrollBarThickness (10);
    addAndMakeVisible (rack);
    for (int t : rackOrder)
    {
        auto* panel = panels.add (new MachinePanel (processor, t, viewPage));
        panel->onLayoutChange = [this] { layoutRack(); };
        panel->onOpenRoll = [this] (int machineType) { openPianoRoll (machineType); };
        panel->onOpenAutomation = [this] (int machineType) { openAutomation (machineType); };
        content.addChildComponent (panel);
    }

    masterInfo = &addInfo ({ "Master", utf8 ("Les effets et réglages qui s'appliquent à tout le morceau, après le mélange de toutes les machines.") });
    {
        auto* echoTime = new ChoiceControl (state, pid::echoTime, utf8 ("TEMPS ÉCHO"),
                                            { std::begin (echoTimeNames), std::end (echoTimeNames) }, help ("master.echoTime"));
        masterControls.add (echoTime);
        addAndMakeVisible (echoTime);

        struct MasterKnob { const char* id; const char* label; const char* helpKey; bool bipolar; };
        const MasterKnob knobs[] {
            { pid::echoFeedback, "RÉPÉTITIONS", "master.echoFeedback", false }, { pid::echoLevel, "ÉCHO", "master.echoLevel", false },
            { pid::reverbSize, "ESPACE", "master.reverbSize", false }, { pid::reverbLevel, "REVERB", "master.reverbLevel", false },
            { pid::pump, "POMPE", "master.pump", false }, { pid::djFilter, "FILTRE DJ", "master.filter", true },
            { pid::comp, "COMPRESSION", "master.comp", false }, { pid::disto, "DISTO", "master.disto", false },
            { pid::volume, "VOLUME", "master.volume", false },
        };
        for (auto& k : knobs)
        {
            auto* knob = new Knob (state, k.id, utf8 (k.label), help (k.helpKey));
            if (k.bipolar) knob->setBipolar();
            masterControls.add (knob);
            addAndMakeVisible (knob);
        }
    }
    addAndMakeVisible (vu);
    meterInfo = &addInfo (help ("master.meter"));

    startScreen.onNew = [this] { newProject(); };
    startScreen.onOpen = [this] { openProject(); };
    startScreen.onResume = [this] { showStartScreen (false); };
    startScreen.onOpenFile = [this] (const juce::File& f) { openProjectFile (f); };
    startScreen.onExample = [this] (int i) { loadExample (i); };
    addChildComponent (startScreen);

    styleButton (fullScreenButton, juce::Colour (0xff3a3b40));
    fullScreenButton.onClick = [this] { toggleFullScreen(); };
    fullScreenButton.setTooltip (utf8 ("Plein écran (F11)"));
    fullScreenButton.setVisible (! processor.isHosted());

    styleButton (languageButton, juce::Colour (0xff3a3b40));
    languageButton.setButtonText (language() == Language::French ? juce::String::fromUTF8 ("FRANÇAIS") : juce::String ("ENGLISH"));
    languageButton.onClick = [this] { toggleLanguage(); };
    languageButton.setTooltip (utf8 ("Langue de l'interface : français ou anglais"));

    midiRecordButton.setTooltip (utf8 ("Pendant la lecture, ce que tu joues au clavier ou aux pads MIDI s'écrit dans les pas de la machine "
                                       "qui reçoit le MIDI (celle marquée MIDI : clique sur une machine pour la choisir). Ctrl+Z annule la prise."));
    midiRecordButton.onClick = [this] { processor.setRecordNotes (midiRecordButton.getToggleState()); };
    addAndMakeVisible (midiRecordButton);

    automationRecordButton.setTooltip (shortHelp (help ("automation.record").body));
    automationRecordButton.onClick = [this] { processor.setRecordAutomation (automationRecordButton.getToggleState()); };
    addAndMakeVisible (automationRecordButton);
    automationRecordInfo = &addInfo (help ("automation.record"));
    knobMovedHook() = [this] (const juce::String& id, float value) { processor.recordKnob (id, value); };

    auto& hooks = midiLearnHooks();
    hooks.learn = [this] (const juce::String& id)
    {
        processor.midiLearn.start (id);
        showInfo (utf8 ("MIDI learn"), utf8 ("Tourne maintenant le bouton ou le fader de ton contrôleur MIDI : il pilotera ce réglage."), audioButton);
    };
    hooks.forget = [this] (const juce::String& id) { processor.midiLearn.forget (id); };
    hooks.controllerFor = [this] (const juce::String& id) { return processor.midiLearn.controllerFor (id); };

    styleButton (audioButton, juce::Colour (0xff3a3b40));
    audioButton.onClick = [this] { showAudioSettings(); };
    audioButton.setTooltip (utf8 ("Carte son, pilote ASIO, taille du tampon (latence) et clavier ou contrôleur MIDI"));
    audioButton.setVisible (! processor.isHosted());

    disableKeyboardFocus (*this);
    setWantsKeyboardFocus (true);
    addMouseListener (this, true);
    startTimerHz (30);
}

void MainView::toggleFullScreen()
{
    auto& desktop = juce::Desktop::getInstance();
    if (desktop.getKioskModeComponent() != nullptr)
    {
        desktop.setKioskModeComponent (nullptr);
        return;
    }
    if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
        desktop.setKioskModeComponent (window, false);
}

void MainView::openAutomation (int machineType)
{
    closePianoRoll();
    automationEditor = std::make_unique<AutomationEditor> (processor, machineType);
    juce::Component::SafePointer<MainView> safe (this);
    automationEditor->onClose = [safe] { juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->closeAutomation(); }); };
    addAndMakeVisible (*automationEditor);
    automationEditor->setBounds (rack.getBounds().expanded (0, 4));
    automationEditor->grabKeyboardFocus();
}

void MainView::closeAutomation()
{
    automationEditor.reset();
    grabKeyboardFocus();
}

void MainView::openPianoRoll (int machineType)
{
    closeAutomation();
    pianoRoll = std::make_unique<PianoRollEditor> (processor, machineType);
    juce::Component::SafePointer<MainView> safe (this);
    pianoRoll->onClose = [safe] { juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->closePianoRoll(); }); };
    addAndMakeVisible (*pianoRoll);
    pianoRoll->setBounds (rack.getBounds().expanded (0, 4));
    pianoRoll->getGrid().fitAll();
    pianoRoll->getGrid().grabKeyboardFocus();
}

void MainView::closePianoRoll()
{
    pianoRoll.reset();
    grabKeyboardFocus();
}

void MainView::showAudioSettings()
{
#if JucePlugin_Build_Standalone
    if (auto* holder = juce::StandalonePluginHolder::getInstance())
        holder->showAudioSettingsDialog();
#endif
}

void MainView::toggleLanguage()
{
    setLanguage (language() == Language::French ? Language::English : Language::French);
    if (onLanguageChange) onLanguageChange();
}

MainView::~MainView()
{
    midiLearnHooks() = {};
    knobMovedHook() = {};
    removeMouseListener (this);
    setLookAndFeel (nullptr);
}

InfoButton& MainView::addInfo (const HelpText& h)
{
    auto* info = infos.add (new InfoButton());
    info->setHelp (h);
    addAndMakeVisible (info);
    return *info;
}

void MainView::styleButton (juce::Button& button, juce::Colour colour, juce::Colour text)
{
    button.setColour (juce::TextButton::buttonColourId, colour);
    button.setColour (juce::TextButton::buttonOnColourId, colour.brighter (0.15f));
    button.setColour (juce::TextButton::textColourOffId, text);
    button.setColour (juce::TextButton::textColourOnId, text);
    addAndMakeVisible (button);
}

void MainView::mouseDown (const juce::MouseEvent& e)
{
    // Every click or drag starts a new undo step (knob moves, menu choices...).
    processor.undoManager.beginNewTransaction();

    // Clicking a machine makes it the one that receives the MIDI keyboard / pads.
    auto* panel = dynamic_cast<MachinePanel*> (e.eventComponent);
    if (panel == nullptr && e.eventComponent != nullptr) panel = e.eventComponent->findParentComponentOfClass<MachinePanel>();
    if (panel != nullptr) processor.setMidiTarget (panel->type);
}

void MainView::togglePlay()
{
    if (processor.isHosted()) return;
    processor.playing = ! processor.playing.load();
}

void MainView::toggleRecord()
{
    if (processor.isRecording())
    {
        processor.stopRecording();
        showInfo (utf8 ("Enregistrement terminé"),
                  utf8 ("Ton morceau est enregistré ici :\n\n") + processor.getLastRecording().getFullPathName()
                      + utf8 ("\n\nPour retrouver tes enregistrements : menu PROJET › Dossier des enregistrements."),
                  recordButton);
        return;
    }

    processor.startRecording();
    if (processor.isRecording() && ! processor.isHosted()) processor.playing = true;
}

void MainView::showAddMenu()
{
    juce::PopupMenu menu;
    auto section = [&] (const char* title, std::initializer_list<int> types)
    {
        juce::PopupMenu sub;
        int shown = 0;
        for (int t : types)
        {
            const auto& def = machineDef (t);
            sub.addItem (t + 1, utf8 (def.name) + utf8 ("   —   ") + utf8 (def.subtitle), true, processor.isShown (t));
            shown += processor.isShown (t) ? 1 : 0;
        }
        menu.addSubMenu (utf8 (title) + (shown > 0 ? "  (" + juce::String (shown) + ")" : juce::String()), sub);
    };
    section ("Batteries et percussions", { TR909, TR808, TR606, LinnDrum, TR707, DigitalKit, CR78, RhythmBox, MPC, AcousticKit, Simmons,
                                            SynthDrums, BreakKit, Percussion, WorldPerc, SoundFx });
    section ("Tes propres sons", { Sampler });
    section ("Basses", { TB303, SH101, ModelD, Sub808, EBass, Wobble });
    section ("Synthés", { Hoover, Juno106, JP8000, Poly80, CS80, Edm, DX7, LaSynth, Wavetable, M1, SemiMod, Chip, Talkbox, Vocoder });
    section ("Sons à part", { Ether, WestCoast });
    section ("Claviers et cloches", { Piano, EPiano, Organ, Mallets });
    section ("Orchestre, voix et instruments", { Strings, Mellotron, Orchestra, Brass, Winds, Choir, Pluck });
    menu.addSeparator();
    menu.addItem (100, utf8 ("Une coche = machine déjà dans le rack (clique pour la retirer)"), false, false);

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (addButton), [this] (int result)
    {
        if (result <= 0 || result > numMachines) return;
        const int t = result - 1;
        const bool show = ! processor.isShown (t);
        processor.setShown (t, show);
        if (show) scrollToMachine = t;
    });
}

void MainView::showProjectMenu()
{
    auto item = [] (int id, const juce::String& text, const juce::String& shortcut = {}, bool enabled = true)
    {
        juce::PopupMenu::Item i (text);
        i.itemID = id;
        i.shortcutKeyDescription = shortcut;
        i.isEnabled = enabled;
        return i;
    };

    juce::PopupMenu recent, examplesMenu, exportMenu;
    const auto recentFiles = TechMachineProcessor::recentProjects();
    for (int i = 0; i < recentFiles.size(); ++i)
        recent.addItem (100 + i, juce::File (recentFiles[i]).getFileNameWithoutExtension());
    for (int i = 0; i < (int) examples().size(); ++i)
        examplesMenu.addItem (200 + i, utf8 (examples()[(size_t) i].name) + utf8 ("   —   ") + utf8 (examples()[(size_t) i].genre));
    for (int loops : { 1, 2, 4, 8 })
        exportMenu.addItem (300 + loops, juce::String (loops) + utf8 (loops == 1 ? " boucle" : " boucles"));

    juce::PopupMenu menu;
    menu.addItem (item (1, utf8 ("Accueil…")));
    menu.addSeparator();
    menu.addItem (item (2, utf8 ("Nouveau projet vide"), "Ctrl+N"));
    menu.addItem (item (3, utf8 ("Ouvrir…"), "Ctrl+O"));
    menu.addSubMenu (utf8 ("Projets récents"), recent, ! recentFiles.isEmpty());
    menu.addSubMenu (utf8 ("Projets exemples"), examplesMenu);
    menu.addSeparator();
    menu.addItem (item (4, utf8 ("Enregistrer"), "Ctrl+S"));
    menu.addItem (item (5, utf8 ("Enregistrer sous…"), utf8 ("Ctrl+Maj+S")));
    menu.addSeparator();
    menu.addSubMenu (utf8 ("Exporter en WAV"), exportMenu);
    menu.addItem (item (6, utf8 ("Dossier des enregistrements"), {}, processor.getLastRecording().existsAsFile()));

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (projectButton), [this, recentFiles] (int r)
    {
        if (r == 1) { startScreen.refresh(); showStartScreen (true); }
        else if (r == 2) newProject();
        else if (r == 3) openProject();
        else if (r == 4) saveProject();
        else if (r == 5) saveProjectAs();
        else if (r == 6) processor.getLastRecording().revealToUser();
        else if (r >= 100 && r < 100 + recentFiles.size()) askThen ([this, f = juce::File (recentFiles[r - 100])] { openProjectFile (f); });
        else if (r >= 200 && r < 200 + (int) examples().size()) loadExample (r - 200);
        else if (r > 300 && r <= 308) exportWav (r - 300);
    });
}

void MainView::showStartScreen (bool show)
{
    if (show)
    {
        startScreen.refresh();
        startScreen.setBounds (getLocalBounds());
        startScreen.toFront (false);
    }
    startScreen.setVisible (show);
    disableKeyboardFocus (startScreen);
    if (! show) grabKeyboardFocus();
}

void MainView::askThen (std::function<void()> action)
{
    if (! processor.isDirty())
    {
        action();
        return;
    }

    juce::AlertWindow::showYesNoCancelBox (juce::MessageBoxIconType::QuestionIcon, utf8 ("Projet modifié"),
                                           utf8 ("Enregistrer les modifications de « ") + processor.projectName() + utf8 (" » avant de continuer ?"),
                                           utf8 ("Enregistrer"), utf8 ("Ne pas enregistrer"), utf8 ("Annuler"), this,
                                           juce::ModalCallbackFunction::create ([this, action] (int result)
                                           {
                                               if (result == 1) saveProject (action);
                                               else if (result == 2) action();
                                           }));
}

void MainView::newProject()
{
    askThen ([this]
    {
        processor.newProject();
        projectChanged();
        showStartScreen (false);
    });
}

void MainView::openProject()
{
    askThen ([this]
    {
        chooser = std::make_unique<juce::FileChooser> (utf8 ("Ouvrir un projet Tech Machine"), TechMachineProcessor::projectsFolder(),
                                                       juce::String ("*") + projectExtension);
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc)
                              {
                                  const auto f = fc.getResult();
                                  if (f.existsAsFile()) openProjectFile (f);
                              });
    });
}

void MainView::openProjectFile (const juce::File& f)
{
    if (! processor.loadProject (f))
    {
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, utf8 ("Impossible d'ouvrir le projet"),
                                                utf8 ("Ce fichier n'est pas un projet Tech Machine valide :\n") + f.getFullPathName());
        return;
    }
    projectChanged();
    showStartScreen (false);
}

void MainView::loadExample (int index)
{
    askThen ([this, index]
    {
        processor.loadExample (index);
        projectChanged();
        showStartScreen (false);
    });
}

void MainView::saveProject (std::function<void()> then)
{
    if (processor.projectFile().existsAsFile())
    {
        if (processor.saveProject (processor.projectFile()) && then) then();
        return;
    }
    saveProjectAs (std::move (then));
}

void MainView::saveProjectAs (std::function<void()> then)
{
    const auto suggested = TechMachineProcessor::projectsFolder().getChildFile (juce::File::createLegalFileName (processor.projectName()) + projectExtension);
    chooser = std::make_unique<juce::FileChooser> (utf8 ("Enregistrer le projet"), suggested, juce::String ("*") + projectExtension);
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this, then] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f == juce::File()) return;
                              f = f.withFileExtension (projectExtension);
                              if (! processor.saveProject (f))
                              {
                                  juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, utf8 ("Enregistrement impossible"),
                                                                          utf8 ("Impossible d'écrire le fichier :\n") + f.getFullPathName());
                                  return;
                              }
                              if (then) then();
                          });
}

void MainView::exportWav (int loops)
{
    const auto suggested = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Tech Machine")
                               .getChildFile (juce::File::createLegalFileName (processor.projectName()) + ".wav");
    chooser = std::make_unique<juce::FileChooser> (utf8 ("Exporter en WAV"), suggested, "*.wav");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this, loops] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f == juce::File()) return;
                              f = f.withFileExtension (".wav");
                              f.getParentDirectory().createDirectory();
                              exportJob = std::make_unique<ExportJob> (processor.createProjectXml(), f, loops, [this] (bool ok)
                              {
                                  juce::MessageManager::callAsync ([this, ok]
                                  {
                                      if (exportJob == nullptr) return;
                                      const auto file = exportJob->file;
                                      exportJob.reset();
                                      if (ok) file.revealToUser();
                                      else juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, utf8 ("Export interrompu"),
                                                                                   utf8 ("Le fichier WAV n'a pas été créé."));
                                  });
                              });
                              exportJob->launchThread();
                          });
}

void MainView::projectChanged()
{
    lastGenre = -1;
    lastShownMask = ~0ull;
    lastBars = -1;
    setPage (0);
    layoutRack();
}

void MainView::setPage (int newPage)
{
    viewPage = juce::jlimit (0, processor.numSteps() / pageSteps - 1, newPage);
    for (int b = 0; b < 4; ++b)
        pageButtons[b].setToggleState (b == viewPage, juce::dontSendNotification);
    for (auto* panel : panels)
    {
        panel->refreshCells();
        panel->setPlayhead (processor.currentStep.load());
    }
}

void MainView::layoutRack()
{
    const int width = rack.getWidth() - rack.getScrollBarThickness() - 4;
    int y = 0, scrollY = -1;
    bool any = false;
    for (auto* panel : panels)
    {
        const bool shown = processor.isShown (panel->type);
        panel->setVisible (shown);
        if (! shown) continue;
        any = true;
        if (panel->type == scrollToMachine) scrollY = y;
        const int h = panel->preferredHeight (width);
        panel->setBounds (0, y, width, h);
        y += h + 14;
    }
    content.empty = ! any;
    content.setSize (juce::jmax (1, width), juce::jmax (y, rack.getHeight()));
    content.repaint();

    if (scrollY >= 0)
    {
        rack.setViewPosition (0, scrollY);
        scrollToMachine = -1;
    }
}

bool MainView::keyPressed (const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    const int code = key.getKeyCode();

    if (key == juce::KeyPress::spaceKey) { togglePlay(); return true; }
    if (key == juce::KeyPress::F11Key) { toggleFullScreen(); return true; }
    if (! mods.isCommandDown()) return false;

    if (code == 'Z' && mods.isShiftDown()) { processor.undoManager.redo(); return true; }
    if (code == 'Z') { processor.undoManager.undo(); return true; }
    if (code == 'Y') { processor.undoManager.redo(); return true; }
    if (code == 'S' && mods.isShiftDown()) { saveProjectAs(); return true; }
    if (code == 'S') { saveProject(); return true; }
    if (code == 'O') { openProject(); return true; }
    if (code == 'N') { newProject(); return true; }
    return false;
}

void MainView::timerCallback()
{
    if (processor.midiLearn.version.load() != lastMidiMapVersion)
    {
        lastMidiMapVersion = processor.midiLearn.version.load();
        processor.midiLearn.save();
    }
    if (const int target = processor.midiTargetMachine(); target != lastMidiTarget)
    {
        lastMidiTarget = target;
        for (auto* panel : panels) panel->setMidiTarget (panel->type == target);
    }
    if (pianoRoll != nullptr)
    {
        auto& grid = pianoRoll->getGrid();
        grid.setPlayhead (processor.currentStep.load());
        if (processor.patternVersion.load() != grid.knownVersion) grid.reload();
    }
    if (processor.sampleVersion() != lastSampleVersion)
    {
        lastSampleVersion = processor.sampleVersion();
        for (auto* panel : panels) panel->refreshSampleNames();
    }
    if (automationRecordButton.getToggleState() != processor.isRecordingAutomation())
        automationRecordButton.setToggleState (processor.isRecordingAutomation(), juce::dontSendNotification);
    if (midiRecordButton.getToggleState() != processor.isRecordingNotes())
        midiRecordButton.setToggleState (processor.isRecordingNotes(), juce::dontSendNotification);

    if (! focusGrabbed && isShowing())
    {
        grabKeyboardFocus();
        focusGrabbed = true;
    }

    std::uint64_t mask = 0;
    for (int t = 0; t < numMachines; ++t)
        if (processor.isShown (t)) mask |= 1ull << t;
    if (mask != lastShownMask || scrollToMachine >= 0)
    {
        if (pianoRoll != nullptr)
        {
            // The piano roll follows the rack: closed when its machine leaves, list of synths kept up to date.
            if (! processor.isShown (pianoRoll->getGrid().machine())) closePianoRoll();
            else pianoRoll->refreshMachines();
        }
        if (automationEditor != nullptr && ! processor.isShown (automationEditor->machine())) closeAutomation();
        lastShownMask = mask;
        layoutRack();
    }

    const int bars = processor.numSteps() / pageSteps;
    if (bars != lastBars)
    {
        lastBars = bars;
        for (int b = 0; b < 4; ++b)
        {
            barButtons[b].setToggleState (b == bars - 1, juce::dontSendNotification);
            pageButtons[b].setEnabled (b < bars);
        }
        setPage (juce::jmin (viewPage, bars - 1));
    }

    const bool playing = processor.playing.load();
    const int step = processor.currentStep.load();
    if (follow && playing && step >= 0 && step / pageSteps != viewPage)
        setPage (step / pageSteps);

    if (step != lastStep)
    {
        lastStep = step;
        for (auto* panel : panels)
            if (panel->isVisible()) panel->setPlayhead (step);
    }

    // Note and chord names depend on the key, the scale and each synth's octave and chord type.
    int labelHash = processor.currentKey() + 12 * processor.currentScale();
    for (int t = 0; t < numMachines; ++t)
    {
        const auto& def = machineDef (t);
        if (def.kind == MachineKind::Drum) continue;
        labelHash = labelHash * 31 + (int) processor.apvts.getRawParameterValue (pid::synth (t, 0))->load() * 16;
        if (def.kind == MachineKind::Poly) labelHash += (int) processor.apvts.getRawParameterValue (pid::synth (t, 1))->load();
    }

    const int version = processor.patternVersion.load();
    if (version != lastVersion || labelHash != lastLabelHash)
    {
        if (pianoRoll != nullptr && labelHash != lastLabelHash) pianoRoll->getGrid().repaint();
        lastVersion = version;
        lastLabelHash = labelHash;
        for (auto* panel : panels)
            if (panel->isVisible()) panel->refreshCells();
    }

    if (processor.genre() != lastGenre)
    {
        lastGenre = processor.genre();
        genreBox.setSelectedId (lastGenre + 1, juce::dontSendNotification);
        const auto& style = styles()[(size_t) lastGenre];
        genreInfo->setHelp (utf8 ("Genre : ") + utf8 (style.name), utf8 (style.help) + "\n\n" + help ("genre").body);
    }

    const auto& scale = scales()[(size_t) processor.currentScale()];
    scaleInfo->setHelp (utf8 ("Gamme : ") + utf8 (scale.name), utf8 (scale.help) + "\n\n" + help ("scale").body);

    undoButton.setEnabled (processor.undoManager.canUndo());
    redoButton.setEnabled (processor.undoManager.canRedo());

    playButton.setEnabled (! processor.isHosted());
    playButton.setButtonText (playing ? utf8 ("STOP") : utf8 ("LECTURE"));
    playButton.setIcon (playing ? "stop" : "play");
    playButton.setColour (juce::TextButton::buttonColourId, playing ? juce::Colour (0xff8f2f2f) : juce::Colour (0xff2f8f4a));
    if (processor.isHosted())
    {
        playButton.setTooltip (utf8 ("En plugin, la lecture et le tempo suivent ton logiciel de musique."));
        lcd.setTooltip (utf8 ("Tempo synchronisé sur ton logiciel de musique."));
    }

    const bool recording = processor.isRecording();
    const bool blink = (juce::Time::getMillisecondCounter() / 500) % 2 == 0;
    recordButton.setColour (juce::TextButton::buttonColourId,
                            recording ? (blink ? juce::Colour (0xffd8303f) : juce::Colour (0xff6a1a22)) : juce::Colour (0xff4a2a2e));

    const auto title = utf8 ("Tech Machine  —  ") + processor.projectName() + (processor.isDirty() ? juce::String::fromUTF8 ("  •") : juce::String());
    if (title != lastTitle)
    {
        lastTitle = title;
        if (auto* window = findParentComponentOfClass<juce::DocumentWindow>()) window->setName (title);
    }

    lcd.repaint();
    vu.push (processor.outputPeak.exchange (0.0f));
}

void MainView::printLabel (juce::Graphics& g, const juce::Component& c, const char* text) const
{
    g.setColour (colours::dim);
    g.setFont (paint::condensed (12.0f));
    g.drawText (utf8 (text), c.getX(), c.getY() - 17, c.getWidth() - 18, 15, juce::Justification::centredLeft);
}

void MainView::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1b1b1f), b.getCentreX(), b.getCentreY(), juce::Colour (0xff09090a), 0.0f, 0.0f, true));
    g.fillAll();
    g.setTiledImageFill (paint::noise(), 0, 0, 1.0f);
    g.fillAll();

    const auto rackArea = rack.getBounds().toFloat();
    rail (g, { rackArea.getX() - railW - 6.0f, rackArea.getY() - 6.0f, (float) railW, rackArea.getHeight() + 12.0f });
    rail (g, { rackArea.getRight() + 6.0f, rackArea.getY() - 6.0f, (float) railW, rackArea.getHeight() + 12.0f });

    paint::plate (g, consoleArea.toFloat(), juce::Colour (0xff2e2f35), juce::Colour (0xff1c1d21), 6.0f, true);
    screws (g, consoleArea);

    g.setColour (colours::text);
    g.setFont (paint::display (28.0f));
    g.drawText ("TECH", consoleArea.getX() + 26, consoleArea.getY() + 12, 80, 30, juce::Justification::centredLeft);
    g.setColour (colours::accent);
    g.drawText ("MACHINE", consoleArea.getX() + 26 + 70, consoleArea.getY() + 12, 140, 30, juce::Justification::centredLeft);
    g.setColour (colours::text.withAlpha (0.75f));
    g.setFont (paint::condensed (15.0f));
    g.drawText (juce::String::fromUTF8 ("GROOVEBOX  ·  v") + JucePlugin_VersionString, consoleArea.getX() + 27, consoleArea.getY() + 40, 200, 16, juce::Justification::centredLeft);
    g.setColour (colours::accent);
    g.fillRect (consoleArea.getX() + 27, consoleArea.getY() + 56, 150, 2);

    printLabel (g, genreBox, "GENRE DU HASARD");
    printLabel (g, keyBox, "TONALITÉ");
    printLabel (g, scaleBox, "GAMME");
    g.setColour (colours::dim);
    g.setFont (paint::condensed (12.0f));
    g.drawText (utf8 ("MESURES"), barButtons[0].getX() - 60, barButtons[0].getY(), 56, barButtons[0].getHeight(), juce::Justification::centredRight);
    g.drawText ("PAGE", pageButtons[0].getX() - 46, pageButtons[0].getY(), 42, pageButtons[0].getHeight(), juce::Justification::centredRight);

    paint::plate (g, masterArea.toFloat(), juce::Colour (0xff2e2f35), juce::Colour (0xff1c1d21), 6.0f, true);
    screws (g, masterArea);
    g.setColour (colours::master);
    g.fillRect (masterArea.getX() + 26, masterArea.getY() + 16, 3, masterArea.getHeight() - 32);
    g.setColour (colours::text);
    g.setFont (paint::display (22.0f));
    g.drawText ("MASTER", masterArea.getX() + 38, masterArea.getY() + 16, 110, 26, juce::Justification::centredLeft);
    g.setColour (colours::dim);
    g.setFont (paint::condensed (12.0f));
    g.drawFittedText (utf8 ("Effets sur tout\nle morceau"), masterArea.getX() + 38, masterArea.getY() + 44, 110, 32, juce::Justification::topLeft, 2);
}

void MainView::resized()
{
    const int w = getWidth();
    const bool narrow = w < 1560;   // down to the minimum width (1440), a few things get smaller
    consoleArea = { 10, 8, w - 20, consoleH };
    const int cy = consoleArea.getY();

    // Row 1: transport, display, harmony, generation.
    playButton.setBounds (236, cy + 14, 120, 48);
    playInfo->setBounds (playButton.getRight() + 2, cy + 14, 15, 15);
    recordButton.setBounds (376, cy + 14, 84, 48);
    recInfo->setBounds (recordButton.getRight() + 2, cy + 14, 15, 15);

    lcd.setBounds (482, cy + 10, narrow ? 270 : 300, consoleH - 20);
    lcdInfo->setBounds (lcd.getRight() + 2, cy + 12, 15, 15);

    const int gap = narrow ? 10 : 12;
    int x = lcd.getRight() + (narrow ? 18 : 26);
    genreBox.setBounds (x, cy + 32, narrow ? 140 : 150, 32);
    genreInfo->setBounds (genreBox.getRight() - 15, cy + 13, 15, 15);
    x = genreBox.getRight() + gap;
    keyBox.setBounds (x, cy + 32, 74, 32);
    keyInfo->setBounds (keyBox.getRight() - 15, cy + 13, 15, 15);
    x = keyBox.getRight() + gap;
    scaleBox.setBounds (x, cy + 32, 180, 32);
    scaleInfo->setBounds (scaleBox.getRight() - 15, cy + 13, 15, 15);
    x = scaleBox.getRight() + gap;
    swingKnob->setBounds (x, cy + 10, 60, 62);
    x = swingKnob->getRight() + 10;
    surpriseButton.setBounds (x, cy + 14, juce::jmin (190, consoleArea.getRight() - 34 - x), 48);
    surpriseInfo->setBounds (surpriseButton.getRight() + 2, cy + 14, 15, 15);

    // Row 2: project, undo, pattern length and pages, rack.
    const int y2 = cy + 80;
    projectButton.setBounds (26, y2, 100, 28);
    projectInfo->setBounds (projectButton.getRight() + 2, y2 + 7, 15, 15);
    undoButton.setBounds (150, y2, 34, 28);
    redoButton.setBounds (188, y2, 34, 28);
    undoInfo->setBounds (redoButton.getRight() + 2, y2 + 7, 15, 15);
    x = 308;
    for (auto& bar : barButtons) { bar.setBounds (x, y2, 34, 28); x += 37; }
    lengthInfo->setBounds (x - 2, y2 + 7, 15, 15);

    x = lcd.getRight() + (narrow ? 50 : 72);
    for (auto& page : pageButtons) { page.setBounds (x, y2, narrow ? 33 : 36, 28); x += narrow ? 36 : 39; }
    followButton.setBounds (x + 2, y2, narrow ? 76 : 82, 28);
    pageInfo->setBounds (followButton.getRight() + 2, y2 + 7, 15, 15);
    automationRecordButton.setBounds (pageInfo->getRight() + (narrow ? 8 : 10), y2, narrow ? 112 : 130, 28);
    automationRecordInfo->setBounds (automationRecordButton.getRight() + 2, y2 + 7, 15, 15);

    addInfoButton->setBounds (consoleArea.getRight() - 16 - 15, y2 + 7, 15, 15);
    addButton.setBounds (addInfoButton->getX() - 4 - (narrow ? 130 : 150), y2, narrow ? 130 : 150, 28);
    guideButton.setBounds (addButton.getX() - 12 - 44, y2, 44, 28);

    // Master strip.
    masterArea = { 10, getHeight() - masterH - 6, w - 20, masterH };
    masterInfo->setBounds (masterArea.getX() + 142, masterArea.getY() + 20, 15, 15);
    vu.setBounds (masterArea.getX() + 168, masterArea.getY() + 12, narrow ? 124 : 170, masterArea.getHeight() - 24);
    meterInfo->setBounds (vu.getRight() + 3, masterArea.getY() + 12, 15, 15);

    // Settings column at the right of the master strip.
    auto column = juce::Rectangle<int> (masterArea.getRight() - 16 - 248, masterArea.getY() + 14, 248, masterArea.getHeight() - 28);
    const int columnX = column.getX();
    {
        auto top = column.removeFromTop ((column.getHeight() - 6) / 2);
        column.removeFromTop (6);
        audioButton.setBounds (top.removeFromLeft (122));
        midiRecordButton.setBounds (top.withTrimmedLeft (4));
        fullScreenButton.setBounds (column.removeFromLeft (122));
        languageButton.setBounds (column.withTrimmedLeft (4));
    }

    // Master controls share the space left between the meter and the settings.
    int mx = vu.getRight() + (narrow ? 18 : 30);
    const int knobStep = juce::jlimit (76, 96, (columnX - 12 - mx - 130) / juce::jmax (1, masterControls.size() - 1));
    for (int i = 0; i < masterControls.size(); ++i)
    {
        const int width = i == 0 ? 130 : knobStep;
        masterControls[i]->setBounds (mx, masterArea.getY() + (i == 0 ? 30 : 14), width - 6, i == 0 ? 52 : 86);
        mx += width;
    }

    const int rackTop = consoleArea.getBottom() + 16;
    rack.setBounds (10 + railW + 12, rackTop, w - 2 * (10 + railW + 12), masterArea.getY() - rackTop - 16);
    layoutRack();

    startScreen.setBounds (getLocalBounds());
    if (pianoRoll != nullptr) pianoRoll->setBounds (rack.getBounds().expanded (0, 4));
    if (automationEditor != nullptr) automationEditor->setBounds (rack.getBounds().expanded (0, 4));
}

//==============================================================================
TechMachineEditor::TechMachineEditor (TechMachineProcessor& p)
    : AudioProcessorEditor (p), techProcessor (p)
{
    setResizable (true, true);
    setResizeLimits (1440, 700, 3200, 2400);

    // The standalone app always opens on the start screen; the plugin only when its project is empty.
    rebuild (! techProcessor.isHosted() || techProcessor.isProjectEmpty());
    setSize (1600, 880);
}

TechMachineEditor::~TechMachineEditor() = default;

void TechMachineEditor::rebuild (bool showStart)
{
    view = std::make_unique<MainView> (techProcessor);
    juce::Component::SafePointer<TechMachineEditor> safe (this);
    view->onLanguageChange = [safe]
    {
        // Recreate the interface outside of the button callback that asked for it.
        if (safe == nullptr) return;
        const bool start = safe->view->isStartScreenShown();
        juce::MessageManager::callAsync ([safe, start]
        {
            if (safe != nullptr) safe->rebuild (start);
        });
    };
    addAndMakeVisible (*view);
    view->setBounds (getLocalBounds());
    view->showStartScreen (showStart);
}

void TechMachineEditor::resized()
{
    if (view != nullptr) view->setBounds (getLocalBounds());
}
