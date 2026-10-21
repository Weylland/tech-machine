#include "PluginEditor.h"

#if JucePlugin_Build_Standalone
 #include <juce_audio_utils/juce_audio_utils.h>
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

using namespace techno;

namespace
{
constexpr int consoleH = 156, masterH = 122, railW = 20;
const char* const projectExtension = ".techmachine";

void disableKeyboardFocus (juce::Component& c)
{
    c.setWantsKeyboardFocus (false);
    c.setMouseClickGrabsKeyboardFocus (false);
    for (auto* child : c.getChildren()) disableKeyboardFocus (*child);
}

// Keyboard navigation: Tab goes from control to control (buttons, knobs, selectors, steps), never by a click.
void enableKeyboardFocus (juce::Component& c)
{
    const bool control = dynamic_cast<juce::Button*> (&c) != nullptr || dynamic_cast<juce::Slider*> (&c) != nullptr
                      || dynamic_cast<juce::ComboBox*> (&c) != nullptr || dynamic_cast<techno::StepCell*> (&c) != nullptr
                      || dynamic_cast<techno::InfoButton*> (&c) != nullptr;
    c.setWantsKeyboardFocus (control);
    c.setMouseClickGrabsKeyboardFocus (false);
    for (auto* child : c.getChildren()) enableKeyboardFocus (*child);
}

// Screen readers: a control without a name of its own takes the text of its tooltip.
void nameFromTooltips (juce::Component& c)
{
    if (auto* tip = dynamic_cast<juce::SettableTooltipClient*> (&c); tip != nullptr && c.getTitle().isEmpty())
    {
        const auto button = dynamic_cast<juce::Button*> (&c);
        if (button == nullptr || button->getButtonText().isEmpty()) c.setTitle (tip->getTooltip().upToFirstOccurrenceOf ("  (", false, false));
    }
    for (auto* child : c.getChildren()) nameFromTooltips (*child);
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
    using Task = std::function<bool (const juce::XmlElement&, const juce::File&, const std::function<bool (float)>&)>;

    ExportJob (std::unique_ptr<juce::XmlElement> project, const juce::File& target, Task exportTask, std::function<void (bool)> done)
        : ThreadWithProgressWindow (utf8 ("Export en cours…"), true, true),
          file (target), xml (std::move (project)), task (std::move (exportTask)), onDone (std::move (done))
    {
    }

    void run() override
    {
        ok = task (*xml, file, [this] (float p)
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
    Task task;
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
    g.drawText (processor.isHosted() ? utf8 ("TEMPO DAW (SYNC)") : processor.isFollowingMidiClock() ? utf8 ("TEMPO MIDI (SYNC)") : utf8 ("TEMPO"),
                left.removeFromTop (13.0f), juce::Justification::centredLeft);
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

    // The part being played (and in song mode, where the song is).
    juce::String part = partName (processor.currentPart());
    if (processor.isSongMode() && processor.playing.load() && processor.songPosition() >= 0)
        part << " " << juce::String (processor.songPosition() + 1) << "/" << juce::String ((int) processor.song().size());
    g.setFont (paint::condensed (13.0f));
    const float partWidth = juce::GlyphArrangement::getStringWidth (g.getCurrentFont(), part) + 12.0f;
    const auto badge = juce::Rectangle<float> (glass.getRight() - partWidth - 8.0f, glass.getY() + 7.0f, partWidth, 17.0f);
    g.setColour (amber.withAlpha (0.8f));
    g.drawRoundedRectangle (badge, 3.0f, 1.2f);
    g.drawText (part, badge, juce::Justification::centred);

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
    guideButton.onClick = [this] { showHelpMenu(); };
    guideButton.setTooltip (utf8 ("Aide : le mode d'emploi, le tutoriel, les raccourcis et la taille de l'interface"));

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
        panel->onOpenSampleEditor = [this] (int slot) { openSampleEditor (slot); };
        panel->onReorderDrag = [this] (int machineType, int y, bool finished) { reorderDrag (machineType, y, finished); };
        content.addChildComponent (panel);
    }

    masterInfo = &addInfo ({ "Master", utf8 ("Les effets et réglages qui s'appliquent à tout le morceau, après le mélange de toutes les machines.") });
    {
        // Four selectors in a square, then the knobs.
        struct MasterChoice { const char* id; const char* label; std::vector<const char*> choices; const char* helpKey; };
        const MasterChoice choices[] {
            { pid::echoTime, "TEMPS ÉCHO", { std::begin (echoTimeNames), std::end (echoTimeNames) }, "master.echoTime" },
            { pid::echoType, "TYPE ÉCHO", { std::begin (echoTypeNames), std::end (echoTypeNames) }, "master.echoType" },
            { pid::reverbType, "SALLE", { std::begin (reverbTypeNames), std::end (reverbTypeNames) }, "master.reverbType" },
            { pid::pumpMode, "MODE POMPE", { std::begin (pumpModeNames), std::end (pumpModeNames) }, "master.pumpMode" },
        };
        for (auto& c : choices)
        {
            auto* control = new ChoiceControl (state, c.id, utf8 (c.label), c.choices, help (c.helpKey));
            masterChoices.add (control);
            addAndMakeVisible (control);
        }

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

    startScreen.onNew = [this]
    {
        newProject();
        // The very first time, a guided tour.
        if (! settings().getBoolValue ("tutorialSeen", false))
        {
            settings().setValue ("tutorialSeen", true);
            startTutorial();
        }
    };
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
    languageButton.setButtonText (languageName (language()));
    languageButton.onClick = [this] { toggleLanguage(); };
    languageButton.setTooltip (utf8 ("Langue de l'interface : français, anglais, espagnol, allemand ou portugais"));

    midiRecordButton.setTooltip (utf8 ("Pendant la lecture, ce que tu joues au clavier ou aux pads MIDI s'écrit dans les pas de la machine "
                                       "qui reçoit le MIDI (celle marquée MIDI : clique sur une machine pour la choisir). Ctrl+Z annule la prise."));
    midiRecordButton.onClick = [this] { processor.setRecordNotes (midiRecordButton.getToggleState()); };
    addAndMakeVisible (midiRecordButton);

    automationRecordButton.setTooltip (shortHelp (help ("automation.record").body));
    automationRecordButton.onClick = [this] { processor.setRecordAutomation (automationRecordButton.getToggleState()); };
    addAndMakeVisible (automationRecordButton);
    automationRecordInfo = &addInfo (help ("automation.record"));
    knobMovedHook() = [this] (const juce::String& id, float value) { processor.recordKnob (id, value); };

    for (int part = 0; part < TechMachineProcessor::numParts; ++part)
    {
        auto& b = partButtons[(size_t) part];
        b.setButtonText (partName (part));
        b.setClickingTogglesState (false);
        b.setTooltip (utf8 ("Partie ") + partName (part) + utf8 (" : clic = la jouer / l'afficher, clic droit = copier ou effacer"));
        b.onClick = [this, part] { processor.selectPart (part); refreshParts(); };
        b.onRightClick = [this, part] { showPartMenu (part); };
        addAndMakeVisible (b);
    }
    partsInfo = &addInfo (help ("parts"));
    songPlayButton.setTooltip (shortHelp (help ("song").body));
    songPlayButton.onClick = [this] { processor.setSongMode (songPlayButton.getToggleState()); };
    addAndMakeVisible (songPlayButton);
    styleButton (songEditButton, juce::Colour (0xff3a3b40));
    songEditButton.setTooltip (utf8 ("Ouvre la liste des parties du morceau"));
    songEditButton.onClick = [this] { openSong(); };
    songInfo = &addInfo (help ("song"));
    refreshParts();

    // A newer version on GitHub: a small button under the name, to its download page.
    updateButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2f8f4a));
    updateButton.setColour (juce::TextButton::textColourOffId, colours::text);
    updateButton.setTooltip (utf8 ("Une nouvelle version de Tech Machine est sortie : clique pour ouvrir sa page de téléchargement"));
    updateButton.onClick = [this] { updatePage.launchInDefaultBrowser(); };
    addChildComponent (updateButton);
    {
        juce::Component::SafePointer<MainView> safe (this);
        updates.start ([safe] (const juce::String& version, const juce::URL& page)
        {
            if (safe == nullptr) return;
            safe->updatePage = page;
            safe->updateButton.setButtonText (utf8 ("NOUVELLE VERSION ") + version.trimCharactersAtStart ("vV") + utf8 (" ›"));
            safe->updateButton.setVisible (true);
        });
    }

    pcKeysButton.setTooltip (shortHelp (help ("pckeys").body));
    pcKeysButton.onClick = [this]
    {
        for (auto& [code, note] : keysDown) processor.playKey (note, false);
        keysDown.clear();
        grabKeyboardFocus();
    };
    addAndMakeVisible (pcKeysButton);
    pcKeysInfo = &addInfo (help ("pckeys"));

    fillButton.setTooltip (shortHelp (help ("fill").body));
    fillButton.onClick = [this] { processor.armFill (fillButton.getToggleState()); };
    addAndMakeVisible (fillButton);
    fillInfo = &addInfo (help ("fill"));
    for (int scene = 0; scene < TechMachineProcessor::numScenes; ++scene)
    {
        auto& b = sceneButtons[(size_t) scene];
        b.setButtonText (juce::String (scene + 1));
        b.setClickingTogglesState (false);
        b.setTooltip (utf8 ("Scène ") + juce::String (scene + 1) + utf8 (" : clic = la rappeler (une scène vide garde les machines coupées maintenant), clic droit = la remplacer ou l'effacer"));
        b.onClick = [this, scene]
        {
            if (processor.hasScene (scene)) processor.recallScene (scene);
            else processor.storeScene (scene);
            refreshScenes();
        };
        b.onRightClick = [this, scene] { showSceneMenu (scene); };
        addAndMakeVisible (b);
    }
    scenesInfo = &addInfo (help ("scenes"));
    refreshScenes();

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

    Accessibility::stepMarks = settings().getBoolValue ("stepMarks", false);
    nameFromTooltips (*this);
    applyKeyboardNavigation();
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

void MainView::openSong()
{
    closePianoRoll();
    closeAutomation();
    closeSampleEditor();
    songEditor = std::make_unique<SongEditor> (processor);
    juce::Component::SafePointer<MainView> safe (this);
    songEditor->onClose = [safe] { juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->closeSong(); }); };
    addAndMakeVisible (*songEditor);
    songEditor->setBounds (rack.getBounds().expanded (0, 4));
    songEditor->grabKeyboardFocus();
}

void MainView::closeSong()
{
    songEditor.reset();
    grabKeyboardFocus();
}

void MainView::showPartMenu (int part)
{
    juce::PopupMenu copyTo;
    for (int to = 0; to < TechMachineProcessor::numParts; ++to)
        if (to != part)
            copyTo.addItem (100 + to, utf8 ("Partie ") + partName (to) + (processor.isPartEmpty (to) ? utf8 (" (vide)") : juce::String()));
    juce::PopupMenu menu;
    menu.addSectionHeader (utf8 ("Partie ") + partName (part));
    menu.addSubMenu (utf8 ("Copier dans"), copyTo);
    menu.addItem (1, utf8 ("Effacer cette partie"), ! processor.isPartEmpty (part));
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (partButtons[(size_t) part]), [this, part] (int result)
    {
        if (result == 1) processor.clearPart (part);
        else if (result >= 100) processor.copyPart (part, result - 100);
        refreshParts();
    });
}

void MainView::showSceneMenu (int scene)
{
    juce::PopupMenu menu;
    menu.addSectionHeader (utf8 ("Scène ") + juce::String (scene + 1));
    menu.addItem (1, utf8 ("Garder les machines coupées maintenant dans cette scène"));
    menu.addItem (2, utf8 ("Effacer cette scène"), processor.hasScene (scene));
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (sceneButtons[(size_t) scene]), [this, scene] (int result)
    {
        if (result == 1) processor.storeScene (scene);
        else if (result == 2) processor.clearScene (scene);
        refreshScenes();
    });
}

// Scenes: the last one recalled is lit, the one waiting for the next bar blinks, the empty ones are dim.
// FILL stays lit until its fill has played, and blinks while it plays.
void MainView::refreshScenes()
{
    const bool blink = (juce::Time::getMillisecondCounter() / 250) % 2 == 0;
    const int current = processor.lastScene(), cued = processor.cuedSceneIndex();
    for (int scene = 0; scene < TechMachineProcessor::numScenes; ++scene)
    {
        auto& b = sceneButtons[(size_t) scene];
        const bool stored = processor.hasScene (scene);
        const bool on = (scene == current && cued < 0) || (scene == cued && blink);
        b.setColour (juce::TextButton::buttonColourId, on ? juce::Colour (0xffd8892a) : stored ? juce::Colour (0xff4a3a26) : juce::Colour (0xff2c2d32));
        b.setColour (juce::TextButton::textColourOffId, stored ? colours::text : colours::dim);
    }
    const bool armed = processor.isFillArmed();
    const bool lit = armed && ! (processor.isFillPlaying() && ! blink);
    if (fillButton.getToggleState() != lit) fillButton.setToggleState (lit, juce::dontSendNotification);
}

// Parts: the one playing is lit, the one waiting for the end of the loop blinks, the empty ones are dim.
void MainView::refreshParts()
{
    const int current = processor.currentPart(), cued = processor.cuedPartIndex();
    const bool blink = (juce::Time::getMillisecondCounter() / 250) % 2 == 0;
    const bool checkEmpty = processor.patternVersion.load() != lastPartsVersion;
    lastPartsVersion = processor.patternVersion.load();
    for (int part = 0; part < TechMachineProcessor::numParts; ++part)
    {
        auto& b = partButtons[(size_t) part];
        const auto colour = partColour (part);
        const bool on = part == current || (part == cued && blink);
        b.setColour (juce::TextButton::buttonColourId, on ? colour.withMultipliedBrightness (0.8f) : juce::Colour (0xff2c2d32));
        if (checkEmpty)
            b.setColour (juce::TextButton::textColourOffId, processor.isPartEmpty (part) && part != current ? colours::dim : colours::text);
    }
    if (songPlayButton.getToggleState() != processor.isSongMode())
        songPlayButton.setToggleState (processor.isSongMode(), juce::dontSendNotification);
}

void MainView::openSampleEditor (int slot)
{
    closePianoRoll();
    closeSong();
    closeAutomation();
    sampleEditor = std::make_unique<SampleEditor> (processor, slot);
    juce::Component::SafePointer<MainView> safe (this);
    sampleEditor->onClose = [safe] { juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->closeSampleEditor(); }); };
    addAndMakeVisible (*sampleEditor);
    sampleEditor->setBounds (rack.getBounds().expanded (0, 4));
    sampleEditor->grabKeyboardFocus();
}

void MainView::closeSampleEditor()
{
    sampleEditor.reset();
    grabKeyboardFocus();
}

void MainView::openAutomation (int machineType)
{
    closePianoRoll();
    closeSong();
    closeSampleEditor();
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
    closeSong();
    closeSampleEditor();
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
    juce::PopupMenu menu;
    for (int i = 0; i < numLanguages; ++i) menu.addItem (i + 1, languageName ((Language) i), true, language() == (Language) i);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (languageButton), [this] (int result)
    {
        if (result < 1 || (Language) (result - 1) == language()) return;
        setLanguage ((Language) (result - 1));
        if (onLanguageChange) onLanguageChange();
    });
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
    section ("Tes propres sons", { Sampler, SampleKeys });
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
    juce::PopupMenu stemsMenu;
    for (int loops : { 1, 2, 4, 8 })
    {
        exportMenu.addItem (300 + loops, juce::String (loops) + utf8 (loops == 1 ? " boucle" : " boucles"));
        stemsMenu.addItem (400 + loops, juce::String (loops) + utf8 (loops == 1 ? " boucle" : " boucles"));
    }

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
    menu.addSubMenu (utf8 ("Exporter les pistes séparées (un WAV par machine)"), stemsMenu);
    menu.addItem (item (7, utf8 ("Exporter les notes en MIDI…")));
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
        else if (r > 400 && r <= 408) exportStems (r - 400);
        else if (r == 7) exportMidi();
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

// One WAV per machine, in a folder chosen by the user.
void MainView::exportStems (int loops)
{
    const auto suggested = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Tech Machine");
    chooser = std::make_unique<juce::FileChooser> (utf8 ("Dossier des pistes séparées"), suggested);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                          [this, loops] (const juce::FileChooser& fc)
                          {
                              const auto folder = fc.getResult();
                              if (folder == juce::File()) return;
                              auto stems = [loops] (const juce::XmlElement& x, const juce::File& out, const std::function<bool (float)>& progress)
                              {
                                  return TechMachineProcessor::exportStems (x, out, loops, progress);
                              };
                              exportJob = std::make_unique<ExportJob> (processor.createProjectXml(), folder, stems, [this] (bool ok)
                              {
                                  juce::MessageManager::callAsync ([this, ok]
                                  {
                                      if (exportJob == nullptr) return;
                                      const auto target = exportJob->file;
                                      exportJob.reset();
                                      if (ok) target.revealToUser();
                                      else juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, utf8 ("Export interrompu"),
                                                                                   utf8 ("Les pistes n'ont pas toutes été créées."));
                                  });
                              });
                              exportJob->launchThread();
                          });
}

void MainView::exportMidi()
{
    const auto suggested = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Tech Machine")
                               .getChildFile (juce::File::createLegalFileName (processor.projectName()) + ".mid");
    chooser = std::make_unique<juce::FileChooser> (utf8 ("Exporter les notes en MIDI"), suggested, "*.mid");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f == juce::File()) return;
                              f = f.withFileExtension (".mid");
                              f.getParentDirectory().createDirectory();
                              if (processor.exportMidi (f)) f.revealToUser();
                              else juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, utf8 ("Export MIDI"),
                                                                           utf8 ("Le fichier MIDI n'a pas pu être écrit."));
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
                              auto wav = [loops] (const juce::XmlElement& x, const juce::File& out, const std::function<bool (float)>& progress)
                              {
                                  return TechMachineProcessor::exportWav (x, out, loops, progress);
                              };
                              exportJob = std::make_unique<ExportJob> (processor.createProjectXml(), f, wav, [this] (bool ok)
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
        panel->setPlayhead (processor.machine (panel->type).playStep.load());
    }
}

techno::MachinePanel* MainView::panelFor (int machineType) const
{
    for (auto* panel : panels)
        if (panel->type == machineType) return panel;
    return nullptr;
}

// A machine dragged by its name band: a line shows where it goes, it moves there on release.
void MainView::reorderDrag (int machineType, int y, bool finished)
{
    int before = -1, lineY = content.getHeight() - 7;
    for (int t : processor.machineOrder())
    {
        auto* panel = panelFor (t);
        if (panel == nullptr || ! panel->isVisible()) continue;
        if (y < panel->getBounds().getCentreY())
        {
            before = t;
            lineY = panel->getY() - 7;
            break;
        }
        lineY = panel->getBottom() + 7;
    }
    content.dropLineY = finished ? -1 : juce::jmax (2, lineY);
    content.repaint();
    if (! finished) return;
    if (before != machineType) processor.moveMachine (machineType, before);
    layoutRack();
}

void MainView::layoutRack()
{
    lastOrderVersion = processor.orderVersion.load();
    const int width = rack.getWidth() - rack.getScrollBarThickness() - 4;
    int y = 0, scrollY = -1;
    bool any = false;
    for (int t : processor.machineOrder())
    {
        auto* panel = panelFor (t);
        if (panel == nullptr) continue;
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

// Keyboard navigation: a ring around the control that has the focus (the steps draw their own).
void MainView::paintOverChildren (juce::Graphics& g)
{
    auto* focused = lastFocused.getComponent();
    if (! keyboardNavigation || focused == nullptr || dynamic_cast<StepCell*> (focused) != nullptr || ! focused->isShowing()) return;
    g.setColour (colours::info);
    g.drawRoundedRectangle (getLocalArea (focused, focused->getLocalBounds()).toFloat().expanded (2.0f), 4.0f, 2.0f);
}

void MainView::applyKeyboardNavigation()
{
    keyboardNavigation = settings().getBoolValue ("keyboardNavigation", false);
    if (keyboardNavigation) enableKeyboardFocus (*this);
    else disableKeyboardFocus (*this);
    setWantsKeyboardFocus (true);
    repaint();
}

bool MainView::keyPressed (const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    const int code = key.getKeyCode();

    // With keyboard navigation, Space and Enter belong to the focused control; Ctrl+Space plays.
    if (key == juce::KeyPress (juce::KeyPress::spaceKey, juce::ModifierKeys::commandModifier, 0)) { togglePlay(); return true; }
    if (keyboardNavigation && (key == juce::KeyPress::spaceKey || key == juce::KeyPress::returnKey))
    {
        if (auto* focused = juce::Component::getCurrentlyFocusedComponent(); focused != nullptr && focused != this && isParentOf (focused))
        {
            if (auto* button = dynamic_cast<juce::Button*> (focused)) { button->triggerClick(); return true; }
        }
    }
    if (key == juce::KeyPress::spaceKey) { togglePlay(); return true; }
    if (key == juce::KeyPress::F11Key) { toggleFullScreen(); return true; }

    // The computer keyboard as a piano: a key held down plays until it is released (keyStateChanged).
    if (pcKeysButton.getToggleState() && ! mods.isCommandDown() && ! mods.isAltDown())
    {
        const auto character = juce::CharacterFunctions::toLowerCase (key.getTextCharacter());
        const bool azerty = language() == Language::French, qwertz = language() == Language::German;
        if (character == (juce::juce_wchar) (azerty ? 'w' : qwertz ? 'y' : 'z')) { keysOctave = juce::jmax (-3, keysOctave - 1); return true; }
        if (character == 'x') { keysOctave = juce::jmin (3, keysOctave + 1); return true; }
        if (const int note = noteForKey (character); note >= 0)
        {
            if (keysDown.count (code) == 0)
            {
                keysDown[code] = note;
                processor.playKey (note, true);
            }
            return true;
        }
    }
    if (code == 'F' && ! mods.isCommandDown() && ! mods.isAltDown())
    {
        processor.armFill (! processor.isFillArmed());
        refreshScenes();
        return true;
    }
    if (! mods.isCommandDown()) return false;

    // Ctrl + / Ctrl - / Ctrl 0: size of the interface.
    if (onZoom && currentZoom && (code == '=' || code == '+' || code == '-' || code == '0' || code == juce::KeyPress::numberPadAdd
                                  || code == juce::KeyPress::numberPadSubtract))
    {
        const float now = currentZoom();
        const auto& steps = TechMachineEditor::zoomSteps;
        int index = 3;
        for (int i = 0; i < (int) std::size (steps); ++i)
            if (std::abs (steps[i] - now) < 0.01f) index = i;
        if (code == '0') index = 3;
        else if (code == '-' || code == juce::KeyPress::numberPadSubtract) index = juce::jmax (0, index - 1);
        else index = juce::jmin ((int) std::size (steps) - 1, index + 1);
        onZoom (steps[index]);
        return true;
    }

    if (code == 'Z' && mods.isShiftDown()) { processor.undoManager.redo(); return true; }
    if (code == 'Z') { processor.undoManager.undo(); return true; }
    if (code == 'Y') { processor.undoManager.redo(); return true; }
    if (code == 'S' && mods.isShiftDown()) { saveProjectAs(); return true; }
    if (code == 'S') { saveProject(); return true; }
    if (code == 'O') { openProject(); return true; }
    if (code == 'N') { newProject(); return true; }
    return false;
}

// AZERTY (French) or QWERTY (English): the row of letters is the white keys, the row above the black keys.
int MainView::noteForKey (juce::juce_wchar character) const
{
    static const juce::String azerty ("qzsedftgyhujkolpm"), qwerty ("awsedftgyhujkolp;"),
        qwertz (juce::CharPointer_UTF8 ("awsedftgzhujkolp\xc3\xb6")), spanish (juce::CharPointer_UTF8 ("awsedftgyhujkolp\xc3\xb1")),
        portuguese (juce::CharPointer_UTF8 ("awsedftgyhujkolp\xc3\xa7"));
    const auto lang = language();
    const auto& layout = lang == Language::French ? azerty : lang == Language::German ? qwertz : lang == Language::Spanish ? spanish
                       : lang == Language::Portuguese ? portuguese : qwerty;
    const int index = layout.indexOfChar (character);
    if (index < 0) return -1;
    const int target = processor.midiTargetMachine();
    const bool drum = target >= 0 && machineDef (target).kind == MachineKind::Drum;
    return juce::jlimit (0, 127, (drum ? 36 : 60) + 12 * keysOctave + index);
}

bool MainView::keyStateChanged (bool)
{
    bool handled = false;
    for (auto it = keysDown.begin(); it != keysDown.end();)
    {
        if (juce::KeyPress::isKeyCurrentlyDown (it->first)) { ++it; continue; }
        processor.playKey (it->second, false);
        it = keysDown.erase (it);
        handled = true;
    }
    return handled;
}

int MainView::tutorialDrums() const
{
    for (int t : processor.machineOrder())
        if (processor.isShown (t) && machineDef (t).kind == MachineKind::Drum && t != Sampler) return t;
    return -1;
}

// "Your first beat in 2 minutes": add drums, a kick on each beat, play, hats, a bass or chords, then turn the knobs.
void MainView::startTutorial()
{
    auto hitsOf = [this] (std::initializer_list<Role> roles)
    {
        const int t = tutorialDrums();
        if (t < 0) return 0;
        const auto& def = machineDef (t);
        int count = 0;
        for (int r = 0; r < (int) def.rows.size(); ++r)
            if (std::find (roles.begin(), roles.end(), def.rows[(size_t) r].role) != roles.end())
                for (int s = 0; s < processor.numSteps(); ++s) count += processor.machine (t).track.hits[(size_t) r][(size_t) s].load() ? 1 : 0;
        return count;
    };
    auto drumPanel = [this]() -> juce::Component* { return panelFor (tutorialDrums()); };
    std::vector<TutorialOverlay::Step> steps {
        { utf8 ("Bienvenue ! On fait un beat ensemble. Clique sur « + MACHINE » › Batteries et percussions, puis choisis une boîte "
                "à rythmes : la 909 par exemple."),
          [this] { return &addButton; }, [this] { return tutorialDrums() >= 0; } },
        { utf8 ("Clique sur 4 pas de la ligne KICK : 1, 5, 9 et 13. Un kick sur chaque temps : c'est la base de la house et de la techno."),
          drumPanel, [hitsOf] { return hitsOf ({ Role::Kick }) >= 3; } },
        { utf8 ("Écoute : clique sur LECTURE (ou appuie sur la barre Espace)."),
          [this] { return &playButton; }, [this] { return processor.playing.load(); } },
        { utf8 ("Ajoute des charleys : sur la ligne HAT, clique entre les kicks (pas 3, 7, 11 et 15). Ça groove déjà !"),
          drumPanel, [hitsOf] { return hitsOf ({ Role::ClosedHat, Role::OpenHat }) >= 2; } },
        { utf8 ("Une basse et des accords ? Clique sur SURPRISE ! : la machine ajoute tout un groupe qui va avec le genre choisi."),
          [this] { return &surpriseButton; }, [this]
          {
              for (int t = 0; t < numMachines; ++t)
                  if (processor.isShown (t) && machineDef (t).kind != MachineKind::Drum && ! processor.machine (t).track.isEmpty()) return true;
              return false;
          } },
        { utf8 ("Bravo, ton premier morceau tourne ! Tourne les boutons pendant que ça joue (FILTRE, ÉCHO, REVERB…), essaie FILL, "
                "puis enregistre ton projet avec Ctrl+S. Le petit « i » à côté de chaque élément explique à quoi il sert."),
          nullptr, nullptr },
    };
    tutorial = std::make_unique<TutorialOverlay> (std::move (steps));
    juce::Component::SafePointer<MainView> safe (this);
    tutorial->onFinish = [safe] { juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->tutorial.reset(); }); };
    addAndMakeVisible (*tutorial);
    tutorial->setBounds (getLocalBounds());
    tutorial->toFront (false);
}

void MainView::showHelpMenu()
{
    juce::PopupMenu menu, sizes;
    const float now = currentZoom ? currentZoom() : 1.0f;
    int id = 100;
    for (float z : TechMachineEditor::zoomSteps)
        sizes.addItem (id++, juce::String (juce::roundToInt (z * 100.0f)) + " %", true, std::abs (z - now) < 0.01f);
    menu.addItem (1, utf8 ("Bien démarrer : le mode d'emploi"));
    menu.addItem (2, utf8 ("Tutoriel : ton premier beat en 2 minutes"));
    menu.addItem (3, utf8 ("Raccourcis clavier"));
    menu.addSeparator();
    menu.addSubMenu (utf8 ("Taille de l'interface  (Ctrl + / Ctrl -)"), sizes);
    menu.addItem (4, utf8 ("Navigation au clavier (Tab, Entrée)"), true, settings().getBoolValue ("keyboardNavigation", false));
    menu.addItem (5, utf8 ("Repères sur les pas actifs (daltonisme)"), true, Accessibility::stepMarks);
    menu.addItem (6, utf8 ("Vérifier les mises à jour au démarrage"), true, UpdateCheck::enabled());
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (guideButton), [this] (int result)
    {
        if (result == 1) showInfo (help ("guide"), guideButton);
        if (result == 2) startTutorial();
        if (result == 3) showInfo (help ("shortcuts"), guideButton);
        if (result == 4)
        {
            settings().setValue ("keyboardNavigation", ! settings().getBoolValue ("keyboardNavigation", false));
            applyKeyboardNavigation();
            if (settings().getBoolValue ("keyboardNavigation", false)) showInfo (help ("keyboardNavigation"), guideButton);
        }
        if (result == 6) UpdateCheck::setEnabled (! UpdateCheck::enabled());
        if (result == 5)
        {
            Accessibility::stepMarks = ! Accessibility::stepMarks;
            settings().setValue ("stepMarks", Accessibility::stepMarks);
            for (auto* panel : panels) panel->refreshCells();
        }
        if (result >= 100 && result < 100 + (int) std::size (TechMachineEditor::zoomSteps) && onZoom)
            onZoom (TechMachineEditor::zoomSteps[result - 100]);
    });
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
        grid.setPlayhead (processor.machine (grid.machine()).playStep.load());
        if (processor.patternVersion.load() != grid.knownVersion) grid.reload();
    }
    if (processor.sampleVersion() != lastSampleVersion)
    {
        lastSampleVersion = processor.sampleVersion();
        for (auto* panel : panels) panel->refreshSampleNames();
    }
    refreshParts();
    refreshScenes();
    if (keyboardNavigation)
        if (auto* focused = juce::Component::getCurrentlyFocusedComponent(); focused != lastFocused.getComponent())
        {
            if (auto* old = lastFocused.getComponent(); old != nullptr && old->isShowing()) repaint (getLocalArea (old, old->getLocalBounds()).expanded (4));
            lastFocused = focused != nullptr && isParentOf (focused) ? focused : nullptr;
            if (auto* now = lastFocused.getComponent()) repaint (getLocalArea (now, now->getLocalBounds()).expanded (4));
        }
    if (processor.orderVersion.load() != lastOrderVersion) layoutRack();
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

    // Each track shows its own step: a track can be shorter or faster than the sequence.
    for (auto* panel : panels)
        if (panel->isVisible()) panel->setPlayhead (processor.machine (panel->type).playStep.load());

    // Note and chord names depend on the key, the scale and each synth's octave and chord type.
    auto labelHash = (juce::uint32) (processor.currentKey() + 12 * processor.currentScale());
    for (int t = 0; t < numMachines; ++t)
    {
        const auto& def = machineDef (t);
        if (def.kind == MachineKind::Drum) continue;
        labelHash = labelHash * 31u + (juce::uint32) processor.apvts.getRawParameterValue (pid::synth (t, 0))->load() * 16;
        if (def.kind == MachineKind::Poly) labelHash += (juce::uint32) processor.apvts.getRawParameterValue (pid::synth (t, 1))->load();
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
    g.drawText (utf8 ("PARTIES"), partButtons[0].getX() - 70, partButtons[0].getY(), 64, partButtons[0].getHeight(), juce::Justification::centredRight);
    g.drawText (utf8 ("SCÈNES"), sceneButtons[0].getX() - 70, sceneButtons[0].getY(), 64, sceneButtons[0].getHeight(), juce::Justification::centredRight);

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

    lcd.setBounds (482, cy + 10, narrow ? 270 : 300, 98);
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

    // Row 3: the parts of the song.
    const int y3 = cy + 118;
    x = 96;
    for (auto& part : partButtons) { part.setBounds (x, y3, narrow ? 36 : 40, 28); x += narrow ? 40 : 44; }
    partsInfo->setBounds (x, y3 + 7, 15, 15);
    x += 30;
    songPlayButton.setBounds (x, y3, 170, 28);
    songEditButton.setBounds (songPlayButton.getRight() + 8, y3, 130, 28);
    songInfo->setBounds (songEditButton.getRight() + 2, y3 + 7, 15, 15);
    x = songInfo->getRight() + (narrow ? 22 : 30);
    fillButton.setBounds (x, y3, narrow ? 62 : 70, 28);
    fillInfo->setBounds (fillButton.getRight() + 2, y3 + 7, 15, 15);
    x = fillInfo->getRight() + (narrow ? 70 : 78);
    for (auto& scene : sceneButtons) { scene.setBounds (x, y3, narrow ? 30 : 34, 28); x += narrow ? 33 : 37; }
    scenesInfo->setBounds (x, y3 + 7, 15, 15);
    pcKeysInfo->setBounds (consoleArea.getRight() - 16 - 15, y3 + 7, 15, 15);
    updateButton.setBounds (consoleArea.getX() + 22, cy + 59, 190, 18);
    pcKeysButton.setBounds (pcKeysInfo->getX() - 4 - (narrow ? 112 : 130), y3, narrow ? 112 : 130, 28);
    guideButton.setBounds (addButton.getX() - 12 - 44, y2, 44, 28);

    // Master strip.
    masterArea = { 10, getHeight() - masterH - 6, w - 20, masterH };
    masterInfo->setBounds (masterArea.getX() + 142, masterArea.getY() + 20, 15, 15);
    vu.setBounds (masterArea.getX() + 168, masterArea.getY() + 12, narrow ? 76 : 170, masterArea.getHeight() - 24);
    meterInfo->setBounds (vu.getRight() + 3, masterArea.getY() + 12, 15, 15);

    // Settings column at the right of the master strip.
    const int columnW = 248;
    auto column = juce::Rectangle<int> (masterArea.getRight() - 16 - columnW, masterArea.getY() + 14, columnW, masterArea.getHeight() - 28);
    const int columnX = column.getX();
    {
        auto top = column.removeFromTop ((column.getHeight() - 6) / 2);
        column.removeFromTop (6);
        audioButton.setBounds (top.removeFromLeft (columnW / 2 - 2));
        midiRecordButton.setBounds (top.withTrimmedLeft (4));
        fullScreenButton.setBounds (column.removeFromLeft (columnW / 2 - 2));
        languageButton.setBounds (column.withTrimmedLeft (4));
    }

    // Master controls share the space left between the meter and the settings: the selectors two by two, then the knobs.
    int mx = meterInfo->getRight() + 6;
    const int choiceW = narrow ? 106 : 128;
    for (int i = 0; i < masterChoices.size(); ++i)
        masterChoices[i]->setBounds (mx + (i % 2) * choiceW, masterArea.getY() + 10 + (i / 2) * 50, choiceW - 6, 48);
    mx += 2 * choiceW + (narrow ? 2 : 12);
    const int knobStep = juce::jlimit (70, 96, (columnX - 12 - mx) / juce::jmax (1, masterControls.size()));
    for (int i = 0; i < masterControls.size(); ++i)
    {
        masterControls[i]->setBounds (mx, masterArea.getY() + 14, knobStep - 1, 86);
        mx += knobStep;
    }

    const int rackTop = consoleArea.getBottom() + 16;
    rack.setBounds (10 + railW + 12, rackTop, w - 2 * (10 + railW + 12), masterArea.getY() - rackTop - 16);
    layoutRack();

    startScreen.setBounds (getLocalBounds());
    if (pianoRoll != nullptr) pianoRoll->setBounds (rack.getBounds().expanded (0, 4));
    if (automationEditor != nullptr) automationEditor->setBounds (rack.getBounds().expanded (0, 4));
    if (sampleEditor != nullptr) sampleEditor->setBounds (rack.getBounds().expanded (0, 4));
    if (tutorial != nullptr) tutorial->setBounds (getLocalBounds());
    if (songEditor != nullptr) songEditor->setBounds (rack.getBounds().expanded (0, 4));
}

//==============================================================================
TechMachineEditor::TechMachineEditor (TechMachineProcessor& p)
    : AudioProcessorEditor (p), techProcessor (p)
{
    setResizable (true, true);

    // The size chosen before; the first time, small enough for the screen (a laptop is often narrower than 1440 points).
    float startZoom = (float) settings().getDoubleValue ("zoom", 0.0);
    if (startZoom <= 0.0f)
    {
        startZoom = 1.0f;
        if (const auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
            for (float z : zoomSteps)
                if (z <= 1.0f && display->userBounds.getWidth() >= 1460.0f * z) startZoom = z;
    }
    zoom = juce::jlimit (zoomSteps[0], zoomSteps[std::size (zoomSteps) - 1], startZoom);
    setResizeLimits ((int) (1440 * zoom), (int) (700 * zoom), 3200, 2400);

    // The standalone app always opens on the start screen; the plugin only when its project is empty.
    rebuild (! techProcessor.isHosted() || techProcessor.isProjectEmpty());
    setSize ((int) (1600 * zoom), (int) (880 * zoom));
}

void TechMachineEditor::setZoom (float newZoom)
{
    const float previous = zoom;
    zoom = juce::jlimit (zoomSteps[0], zoomSteps[std::size (zoomSteps) - 1], newZoom);
    settings().setValue ("zoom", zoom);
    setResizeLimits ((int) (1440 * zoom), (int) (700 * zoom), 3200, 2400);
    setSize (juce::jmax ((int) (1440 * zoom), (int) ((float) getWidth() * zoom / previous)),
             juce::jmax ((int) (700 * zoom), (int) ((float) getHeight() * zoom / previous)));
    resized();
}

TechMachineEditor::~TechMachineEditor() = default;

void TechMachineEditor::rebuild (bool showStart)
{
    view = std::make_unique<MainView> (techProcessor);
    juce::Component::SafePointer<TechMachineEditor> safe (this);
    view->onZoom = [safe] (float z) { if (safe != nullptr) safe->setZoom (z); };
    view->currentZoom = [safe] { return safe != nullptr ? safe->zoom : 1.0f; };
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
    resized();
    view->showStartScreen (showStart);
}

void TechMachineEditor::resized()
{
    if (view == nullptr) return;
    view->setTransform (juce::AffineTransform::scale (zoom));
    view->setBounds (0, 0, juce::roundToInt ((float) getWidth() / zoom), juce::roundToInt ((float) getHeight() / zoom));
}
