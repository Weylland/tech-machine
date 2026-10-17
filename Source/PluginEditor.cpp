#include "PluginEditor.h"

using namespace techno;

namespace
{
constexpr int consoleH = 118, masterH = 122, railW = 20;

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
} // namespace

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

void LcdDisplay::mouseUp (const juce::MouseEvent&) { if (! processor.isHosted()) tempo.endChangeGesture(); }

void LcdDisplay::mouseDoubleClick (const juce::MouseEvent&) { if (! processor.isHosted()) setTempo (130.0f); }

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
    const float bpm = processor.displayBpm.load();

    auto left = glass.reduced (12.0f, 8.0f).removeFromLeft (glass.getWidth() * 0.42f);
    g.setColour (amber.withAlpha (0.7f));
    g.setFont (paint::condensed (11.0f));
    g.drawText (processor.isHosted() ? "TEMPO DAW (SYNC)" : "TEMPO", left.removeFromTop (13.0f), juce::Justification::centredLeft);
    auto digits = left.removeFromTop (left.getHeight() - 14.0f);
    paint::sevenSegment (g, juce::String (juce::roundToInt (bpm)).paddedLeft (' ', 3), digits.withWidth (digits.getHeight() * 1.9f), amber);
    g.setColour (amber.withAlpha (0.7f));
    g.drawText ("BPM", left, juce::Justification::centredLeft);

    auto right = glass.reduced (12.0f, 8.0f).withTrimmedLeft (glass.getWidth() * 0.45f);
    const auto& style = styles()[(size_t) processor.styleIndex];
    g.setColour (amber.withAlpha (0.7f));
    g.setFont (paint::condensed (11.0f));
    g.drawText ("STYLE", right.removeFromTop (13.0f), juce::Justification::centredLeft);
    g.setColour (amber);
    g.setFont (paint::display (19.0f));
    g.drawFittedText (utf8 (style.name).toUpperCase(), right.removeFromTop (22.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);

    g.setFont (paint::condensed (14.0f));
    g.drawFittedText (juce::String (keyNames[processor.currentKey()]) + "  " + utf8 (scales()[(size_t) processor.currentScale()].name).toUpperCase(),
                      right.removeFromTop (18.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);

    const int step = processor.currentStep.load();
    const int bars = processor.numSteps() / pageSteps;
    auto bottom = right.removeFromTop (18.0f);
    g.setColour (amber.withAlpha (0.85f));
    g.drawText (step >= 0 ? "MESURE " + juce::String (step / pageSteps + 1) + "/" + juce::String (bars) : juce::String ("STOP"),
                bottom.removeFromLeft (90.0f), juce::Justification::centredLeft);
    for (int beat = 0; beat < 4; ++beat)
    {
        const bool on = step >= 0 && (step % pageSteps) / 4 == beat;
        paint::led (g, { bottom.getX() + 8.0f + (float) beat * 16.0f, bottom.getCentreY() }, 3.5f, amber, on);
    }

    juce::Random rnd (3);
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
TechMachineEditor::TechMachineEditor (TechMachineProcessor& p)
    : AudioProcessorEditor (p), processor (p), lcd (p)
{
    setLookAndFeel (&theme);
    auto& state = processor.apvts;

    styleButton (playButton, juce::Colour (0xff2f8f4a));
    playButton.onClick = [this] { togglePlay(); };
    playInfo = &addInfo (help ("play"));

    styleButton (recordButton, juce::Colour (0xff4a2a2e));
    recordButton.onClick = [this] { toggleRecord(); };
    recInfo = &addInfo (help ("rec"));
    styleButton (folderButton, juce::Colour (0xff3a3b40));
    folderButton.onClick = [this] { processor.getLastRecording().revealToUser(); };
    folderButton.setTooltip (help ("files").body);
    folderButton.setVisible (false);

    addAndMakeVisible (lcd);
    lcdInfo = &addInfo (help ("tempo"));

    swingKnob = std::make_unique<Knob> (state, pid::swing, "SWING", help ("swing"));
    addAndMakeVisible (*swingKnob);

    for (int i = 0; i < (int) styles().size(); ++i)
        styleBox.addItem (utf8 (styles()[(size_t) i].name), i + 1);
    styleBox.setSelectedId (processor.styleIndex + 1, juce::dontSendNotification);
    styleBox.onChange = [this] { processor.applyStyle (styleBox.getSelectedId() - 1); };
    addAndMakeVisible (styleBox);
    styleInfo = &addInfo (help ("style"));

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
    surpriseButton.onClick = [this] { processor.regenerateAll(); };
    surpriseButton.setTooltip (shortHelp (help ("surprise").body));
    surpriseInfo = &addInfo (help ("surprise"));

    styleButton (guideButton, juce::Colour (0xff2f4fa8));
    guideButton.onClick = [this] { showInfo (help ("guide"), guideButton); };
    guideButton.setTooltip (utf8 ("Bien démarrer : le mode d'emploi en 7 étapes"));

    for (int b = 0; b < 4; ++b)
    {
        barButtons[b].setClickingTogglesState (false);
        styleButton (barButtons[b], juce::Colour (0xff3a3b40));
        barButtons[b].onClick = [this, b] { processor.setBars (b + 1); };
        barButtons[b].setTooltip (juce::String (b + 1) + (b == 0 ? " mesure" : " mesures"));

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

    styleButton (recommendedButton, juce::Colour (0xff3a3b40));
    recommendedButton.onClick = [this] { processor.addRecommendedMachines(); };
    recommendedInfo = &addInfo (help ("recommended"));
    styleButton (addButton, juce::Colour (0xffd2551f));
    addButton.onClick = [this] { showAddMenu(); };
    addInfoButton = &addInfo (help ("add"));

    rack.setViewedComponent (&content, false);
    rack.setScrollBarsShown (true, false);
    rack.setScrollBarThickness (10);
    addAndMakeVisible (rack);
    for (int t = 0; t < numMachines; ++t)
    {
        auto* panel = panels.add (new MachinePanel (processor, t, viewPage));
        panel->onLayoutChange = [this] { layoutRack(); };
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
            { pid::echoFeedback, "RÉPÉTITIONS", "master.echoFeedback", false }, { pid::echoLevel, "NIVEAU ÉCHO", "master.echoLevel", false },
            { pid::reverbSize, "TAILLE REVERB", "master.reverbSize", false }, { pid::reverbLevel, "NIVEAU REVERB", "master.reverbLevel", false },
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

    disableKeyboardFocus (*this);
    setWantsKeyboardFocus (true);

    setResizable (true, true);
    setResizeLimits (1440, 700, 3200, 2400);
    setSize (1600, 880);
    startTimerHz (30);
}

TechMachineEditor::~TechMachineEditor()
{
    setLookAndFeel (nullptr);
}

InfoButton& TechMachineEditor::addInfo (const HelpText& h)
{
    auto* info = infos.add (new InfoButton());
    info->setHelp (h);
    addAndMakeVisible (info);
    return *info;
}

void TechMachineEditor::styleButton (juce::Button& button, juce::Colour colour, juce::Colour text)
{
    button.setColour (juce::TextButton::buttonColourId, colour);
    button.setColour (juce::TextButton::buttonOnColourId, colour.brighter (0.15f));
    button.setColour (juce::TextButton::textColourOffId, text);
    button.setColour (juce::TextButton::textColourOnId, text);
    addAndMakeVisible (button);
}

void TechMachineEditor::togglePlay()
{
    if (processor.isHosted()) return;
    processor.playing = ! processor.playing.load();
}

void TechMachineEditor::toggleRecord()
{
    if (processor.isRecording())
    {
        processor.stopRecording();
        showInfo (utf8 ("Enregistrement terminé"),
                  utf8 ("Ton morceau est enregistré ici :\n\n") + processor.getLastRecording().getFullPathName()
                      + utf8 ("\n\nClique sur FICHIERS pour ouvrir le dossier."),
                  recordButton);
        return;
    }

    processor.startRecording();
    if (processor.isRecording()) processor.playing = true;
}

void TechMachineEditor::showAddMenu()
{
    juce::PopupMenu menu;
    auto section = [&] (const char* title, std::initializer_list<int> types)
    {
        menu.addSectionHeader (utf8 (title));
        for (int t : types)
        {
            const auto& def = machineDef (t);
            menu.addItem (t + 1, utf8 (def.name) + utf8 ("   —   ") + utf8 (def.subtitle), true, processor.isShown (t));
        }
    };
    section ("BOÎTES À RYTHMES", { TR909, TR808, BreakKit });
    section ("BASSES ET SYNTHÉS MONOPHONIQUES (une note à la fois)", { TB303, SH101, ModelD });
    section ("SYNTHÉS POLYPHONIQUES (accords et nappes)", { Juno106, JP8000 });
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

void TechMachineEditor::setPage (int newPage)
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

void TechMachineEditor::layoutRack()
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

bool TechMachineEditor::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        togglePlay();
        return true;
    }
    return false;
}

void TechMachineEditor::timerCallback()
{
    if (! focusGrabbed && isShowing())
    {
        grabKeyboardFocus();
        focusGrabbed = true;
    }

    int mask = 0;
    for (int t = 0; t < numMachines; ++t)
        if (processor.isShown (t)) mask |= 1 << t;
    if (mask != lastShownMask || scrollToMachine >= 0)
    {
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

    int labelHash = processor.currentKey() + 12 * processor.currentScale();
    for (int t : { TB303, SH101, ModelD, Juno106, JP8000 })
        labelHash = labelHash * 31 + (int) processor.apvts.getRawParameterValue (pid::synth (t, 0))->load() * 16
                  + (int) processor.apvts.getRawParameterValue (pid::synth (t, 1))->load();

    const int version = processor.patternVersion.load();
    if (version != lastVersion || labelHash != lastLabelHash)
    {
        lastVersion = version;
        lastLabelHash = labelHash;
        for (auto* panel : panels)
            if (panel->isVisible()) panel->refreshCells();
    }

    if (processor.styleIndex != lastStyle)
    {
        lastStyle = processor.styleIndex;
        styleBox.setSelectedId (lastStyle + 1, juce::dontSendNotification);
        const auto& style = styles()[(size_t) lastStyle];
        juce::String machines;
        for (int t : style.recommended) machines << (machines.isEmpty() ? "" : ", ") << utf8 (machineDef (t).name);
        styleInfo->setHelp (utf8 ("Style : ") + utf8 (style.name), utf8 (style.help) + "\n\n" + help ("style").body);
        styleBox.setTooltip (utf8 (style.name) + " : " + shortHelp (utf8 (style.help)));
        recommendedButton.setTooltip (utf8 ("Ajoute : ") + machines);
    }

    const auto& scale = scales()[(size_t) processor.currentScale()];
    scaleInfo->setHelp (utf8 ("Gamme : ") + utf8 (scale.name), utf8 (scale.help) + "\n\n" + help ("scale").body);

    playButton.setEnabled (! processor.isHosted());
    playButton.setButtonText (playing ? "STOP" : "LECTURE");
    playButton.setIcon (playing ? "stop" : "play");
    playButton.setColour (juce::TextButton::buttonColourId, playing ? juce::Colour (0xff8f2f2f) : juce::Colour (0xff2f8f4a));

    const bool recording = processor.isRecording();
    const bool blink = (juce::Time::getMillisecondCounter() / 500) % 2 == 0;
    recordButton.setColour (juce::TextButton::buttonColourId,
                            recording ? (blink ? juce::Colour (0xffd8303f) : juce::Colour (0xff6a1a22)) : juce::Colour (0xff4a2a2e));
    folderButton.setVisible (processor.getLastRecording().existsAsFile() && ! recording);

    if (processor.isHosted())
    {
        playButton.setTooltip (utf8 ("En plugin, la lecture et le tempo suivent ton logiciel de musique."));
        lcd.setTooltip (utf8 ("Tempo synchronisé sur ton logiciel de musique."));
    }
    lcd.repaint();
    vu.push (processor.outputPeak.exchange (0.0f));
}

void TechMachineEditor::printLabel (juce::Graphics& g, const juce::Component& c, const char* text) const
{
    g.setColour (colours::dim);
    g.setFont (paint::condensed (12.0f));
    g.drawText (utf8 (text), c.getX(), c.getY() - 17, c.getWidth() - 18, 15, juce::Justification::centredLeft);
}

void TechMachineEditor::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1b1b1f), b.getCentreX(), b.getCentreY(), juce::Colour (0xff09090a), 0.0f, 0.0f, true));
    g.fillAll();
    g.setTiledImageFill (paint::noise(), 0, 0, 1.0f);
    g.fillAll();

    const auto rackArea = rack.getBounds().toFloat();
    rail (g, { rackArea.getX() - railW - 6.0f, rackArea.getY() - 6.0f, (float) railW, rackArea.getHeight() + 12.0f });
    rail (g, { rackArea.getRight() + 6.0f, rackArea.getY() - 6.0f, (float) railW, rackArea.getHeight() + 12.0f });

    // Console
    paint::plate (g, consoleArea.toFloat(), juce::Colour (0xff2e2f35), juce::Colour (0xff1c1d21), 6.0f, true);
    for (auto p : { juce::Point<float> (10.0f, 10.0f), juce::Point<float> ((float) consoleArea.getWidth() - 10.0f, 10.0f),
                    juce::Point<float> (10.0f, (float) consoleArea.getHeight() - 10.0f),
                    juce::Point<float> ((float) consoleArea.getWidth() - 10.0f, (float) consoleArea.getHeight() - 10.0f) })
        paint::screw (g, p + consoleArea.getPosition().toFloat(), 4.5f, p.x * 0.01f);

    g.setColour (colours::text);
    g.setFont (paint::display (28.0f));
    g.drawText ("TECH", consoleArea.getX() + 26, consoleArea.getY() + 14, 80, 30, juce::Justification::centredLeft);
    g.setColour (colours::accent);
    g.drawText ("MACHINE", consoleArea.getX() + 26 + 70, consoleArea.getY() + 14, 140, 30, juce::Justification::centredLeft);
    g.setColour (colours::dim);
    g.setFont (paint::condensed (12.0f));
    g.drawText (utf8 ("GROOVEBOX  ·  v0.3"), consoleArea.getX() + 27, consoleArea.getY() + 42, 200, 14, juce::Justification::centredLeft);
    g.setColour (colours::accent);
    g.fillRect (consoleArea.getX() + 27, consoleArea.getY() + 58, 150, 2);

    printLabel (g, styleBox, "STYLE");
    printLabel (g, keyBox, "TONALITÉ");
    printLabel (g, scaleBox, "GAMME");
    g.setColour (colours::dim);
    g.setFont (paint::condensed (12.0f));
    g.drawText ("MESURES", barButtons[0].getX() - 70, barButtons[0].getY(), 64, barButtons[0].getHeight(), juce::Justification::centredRight);
    g.drawText ("PAGE", pageButtons[0].getX() - 50, pageButtons[0].getY(), 44, pageButtons[0].getHeight(), juce::Justification::centredRight);

    // Master
    paint::plate (g, masterArea.toFloat(), juce::Colour (0xff2e2f35), juce::Colour (0xff1c1d21), 6.0f, true);
    for (auto p : { juce::Point<float> (10.0f, 10.0f), juce::Point<float> ((float) masterArea.getWidth() - 10.0f, 10.0f),
                    juce::Point<float> (10.0f, (float) masterArea.getHeight() - 10.0f),
                    juce::Point<float> ((float) masterArea.getWidth() - 10.0f, (float) masterArea.getHeight() - 10.0f) })
        paint::screw (g, p + masterArea.getPosition().toFloat(), 4.5f, p.y * 0.02f);
    g.setColour (colours::master);
    g.fillRect (masterArea.getX() + 26, masterArea.getY() + 16, 3, masterArea.getHeight() - 32);
    g.setColour (colours::text);
    g.setFont (paint::display (22.0f));
    g.drawText ("MASTER", masterArea.getX() + 38, masterArea.getY() + 16, 110, 26, juce::Justification::centredLeft);
    g.setColour (colours::dim);
    g.setFont (paint::condensed (12.0f));
    g.drawFittedText (utf8 ("Effets sur tout\nle morceau"), masterArea.getX() + 38, masterArea.getY() + 44, 110, 32, juce::Justification::topLeft, 2);
}

void TechMachineEditor::resized()
{
    const int w = getWidth();
    consoleArea = { 10, 8, w - 20, consoleH };
    const int cy = consoleArea.getY();

    // Row 1: transport, display, harmony.
    playButton.setBounds (236, cy + 14, 120, 48);
    playInfo->setBounds (playButton.getRight() + 2, cy + 14, 15, 15);
    recordButton.setBounds (376, cy + 14, 84, 48);
    recInfo->setBounds (recordButton.getRight() + 2, cy + 14, 15, 15);

    lcd.setBounds (482, cy + 10, 330, consoleH - 20);
    lcdInfo->setBounds (lcd.getRight() + 2, cy + 12, 15, 15);

    int x = lcd.getRight() + 26;
    styleBox.setBounds (x, cy + 32, 170, 32);
    styleInfo->setBounds (styleBox.getRight() - 15, cy + 13, 15, 15);
    x = styleBox.getRight() + 12;
    keyBox.setBounds (x, cy + 32, 72, 32);
    keyInfo->setBounds (keyBox.getRight() - 15, cy + 13, 15, 15);
    x = keyBox.getRight() + 12;
    scaleBox.setBounds (x, cy + 32, 196, 32);
    scaleInfo->setBounds (scaleBox.getRight() - 15, cy + 13, 15, 15);
    x = scaleBox.getRight() + 12;
    swingKnob->setBounds (x, cy + 8, 66, 66);
    x = swingKnob->getRight() + 10;
    guideButton.setBounds (consoleArea.getRight() - 16 - 44, cy + 14, 44, 48);
    surpriseButton.setBounds (x, cy + 14, juce::jmin (170, guideButton.getX() - 26 - x), 48);
    surpriseInfo->setBounds (surpriseButton.getRight() + 2, cy + 14, 15, 15);

    // Row 2: pattern length and pages, recordings, rack.
    const int y2 = cy + 80;
    x = 100;
    for (auto& b : barButtons) { b.setBounds (x, y2, 42, 28); x += 45; }
    lengthInfo->setBounds (x + 2, y2 + 7, 15, 15);
    folderButton.setBounds (lengthInfo->getRight() + 12, y2, 100, 28);

    x = lcd.getRight() + 70;
    for (auto& b : pageButtons) { b.setBounds (x, y2, 40, 28); x += 43; }
    followButton.setBounds (x + 4, y2, 86, 28);
    pageInfo->setBounds (followButton.getRight() + 2, y2 + 7, 15, 15);

    addInfoButton->setBounds (consoleArea.getRight() - 16 - 15, y2 + 7, 15, 15);
    addButton.setBounds (addInfoButton->getX() - 4 - 210, y2, 210, 28);
    recommendedInfo->setBounds (addButton.getX() - 20, y2 + 7, 15, 15);
    recommendedButton.setBounds (recommendedInfo->getX() - 4 - 150, y2, 150, 28);

    // Master strip.
    masterArea = { 10, getHeight() - masterH - 6, w - 20, masterH };
    masterInfo->setBounds (masterArea.getX() + 142, masterArea.getY() + 20, 15, 15);
    vu.setBounds (masterArea.getX() + 168, masterArea.getY() + 12, 170, masterArea.getHeight() - 24);
    meterInfo->setBounds (vu.getRight() + 3, masterArea.getY() + 12, 15, 15);
    int mx = vu.getRight() + 30;
    for (int i = 0; i < masterControls.size(); ++i)
    {
        const int width = i == 0 ? 130 : 92;
        masterControls[i]->setBounds (mx, masterArea.getY() + (i == 0 ? 30 : 14), width - 6, i == 0 ? 52 : 86);
        mx += width;
    }

    const int rackTop = consoleArea.getBottom() + 16;
    rack.setBounds (10 + railW + 12, rackTop, w - 2 * (10 + railW + 12), masterArea.getY() - rackTop - 16);
    layoutRack();
}
