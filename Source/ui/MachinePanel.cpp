#include "MachinePanel.h"

namespace techno
{
namespace
{
constexpr int headerH = 54;
constexpr int labelW = 112;
constexpr int cellW = 38, cellGap = 4, groupGap = 10;
constexpr int drumTop = headerH + 30, drumRowH = 30, drumColumnW = 64;
constexpr int knobW = 64, knobH = 72, faderW = 60, faderH = 126, choiceW = 136;
constexpr int boxPad = 10, boxTitle = 16, fxW = 44;

const char* const drumColumnKeys[4] { "drum.level", "drum.tune", "drum.decay", "drum.tone" };
const char* const drumColumnNames[4] { "NIVEAU", "HAUTEUR", "DURÉE", "TIMBRE" };
} // namespace

MachinePanel::MachinePanel (TechMachineProcessor& p, int machineType, const int& viewPage)
    : type (machineType), processor (p), def (machineDef (machineType)), skin (skinFor (machineType)), theme (skin), page (viewPage)
{
    setLookAndFeel (&theme);
    setBufferedToImage (true);
    auto& state = processor.apvts;

    machineInfo.setHelp (utf8 (def.name), utf8 (def.subtitle) + "\n\n" + utf8 (def.help));
    addAndMakeVisible (machineInfo);

    muteButton = std::make_unique<ParamLed> (state, pid::machine (type, "mute"), "MUTE", juce::Colour (0xffff3b1f));
    soloButton = std::make_unique<ParamLed> (state, pid::machine (type, "solo"), "SOLO", juce::Colour (0xffffd23a));
    addHeaderButton (*muteButton, "machine.mute");
    addHeaderButton (*soloButton, "machine.solo");

    soundButton.onClick = [this] { showSoundMenu(); };
    addHeaderButton (soundButton, "machine.sounds");   // factory sounds and the user's own
    patternButton.onClick = [this] { showPatternMenu(); };
    if (! patternKindsFor (def).empty()) addHeaderButton (patternButton, "machine.patterns");
    diceButton.onClick = [this] { showDiceMenu(); };
    if (type != Sampler) addHeaderButton (diceButton, "machine.diceMenu");
    automationButton.onClick = [this] { if (onOpenAutomation) onOpenAutomation (type); };
    automationButton.setClickingTogglesState (false);
    addHeaderButton (automationButton, "machine.automation");
    moreButton.onClick = [this] { showMoreMenu(); };
    addHeaderButton (moreButton, "machine.more");

    collapseButton.setTooltip (help ("machine.collapse").title + " : " + help ("machine.collapse").body);
    collapseButton.onClick = [this]
    {
        collapsed = ! collapsed;
        collapseButton.setIcon (collapsed ? "right" : "down");
        if (onLayoutChange) onLayoutChange();
    };
    addAndMakeVisible (collapseButton);

    removeButton.setTooltip (help ("machine.remove").title + " : " + shortHelp (help ("machine.remove").body));
    removeButton.onClick = [this] { processor.setShown (type, false); };
    addAndMakeVisible (removeButton);

    for (auto* b : { &collapseButton, &removeButton })
    {
        b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3b40));
        b->setColour (juce::TextButton::textColourOffId, colours::text);
    }

    ledStrip.columnX = [this] (int c) { return cellX (c); };
    ledStrip.columnWidth = cellW;
    addAndMakeVisible (ledStrip);

    auto& track = processor.machine (type).track;

    if (isDrum())
    {
        rows.resize ((size_t) numRows() + 1);
        for (int r = 0; r < numRows(); ++r)
        {
            const auto& row = def.rows[(size_t) r];
            const juce::String label = utf8 (row.label);

            auto* mute = rowMutes.add (new ParamLed (state, pid::row (type, r, "mute"), label, skin.led, LedButton::Style::Row, true));
            mute->setTooltip (utf8 ("Clic : couper / remettre ce son (la LED s'éteint quand il est coupé)"));
            addAndMakeVisible (mute);

            rowInfoButtons.push_back (&addInfo ({ label, utf8 (row.help) + "\n\n" + help ("drum.rowMute").body }));

            auto* fx = fxButtons.add (new FxButton (state, pid::row (type, r, "")));
            fx->setTooltip (help ("drum.fx").title + " : " + shortHelp (help ("drum.fx").body));
            fx->onClick = [this, r] { showRowFx (r); };
            addAndMakeVisible (fx);

            const char* suffixes[4] { "level", "tune", "decay", "tone" };
            for (int k = 0; k < 4; ++k)
            {
                auto h = help (drumColumnKeys[k]);
                if (k == 3) h.body = utf8 (row.toneHelp);
                h.title = label + " · " + h.title;
                addAndMakeVisible (rowKnobs.add (new Knob (state, pid::row (type, r, suffixes[k]), {}, h, false)));
            }

            for (int c = 0; c < pageSteps; ++c)
            {
                auto& cell = addCell (c, r, StepCell::Mode::Toggle);
                cell.isOn = [this, &track, r, c] { return track.hits[(size_t) r][(size_t) stepFor (c)].load(); };
                cell.setOn = [this, &track, r, c] (bool on)
                {
                    track.hits[(size_t) r][(size_t) stepFor (c)] = on;
                    if (! on) track.rolls[(size_t) r][(size_t) stepFor (c)] = 0;
                };
                cell.getRoll = [this, &track, r, c] { return (int) track.rolls[(size_t) r][(size_t) stepFor (c)].load(); };
                cell.setRoll = [this, &track, r, c] (int roll) { track.rolls[(size_t) r][(size_t) stepFor (c)] = (std::uint8_t) roll; };
                cell.setTooltip (utf8 ("Clic : son on / off · Clic droit ou molette : roulement ×2, ×3, ×4"));
            }
        }

        rowInfoButtons.push_back (&addInfo (help ("row.accent")));
        for (int c = 0; c < pageSteps; ++c)
        {
            auto& cell = addCell (c, numRows(), StepCell::Mode::Toggle, true);
            cell.isOn = [this, &track, c] { return track.accent[(size_t) stepFor (c)].load(); };
            cell.setOn = [this, &track, c] (bool on) { track.accent[(size_t) stepFor (c)] = on; };
        }

        for (int k = 0; k < 4; ++k)
            columnInfos.push_back (&addInfo (help (drumColumnKeys[k])));
    }
    else
    {
        const bool poly = isPoly();
        const int numSeqRows = poly ? 2 : 3;
        rows.resize ((size_t) numSeqRows);

        const char* helpKeys[3] { poly ? "row.chords" : "row.notes", poly ? "row.tie" : "row.synthAccent", "row.slide" };
        for (int r = 0; r < numSeqRows; ++r)
            rowInfoButtons.push_back (&addInfo (help (helpKeys[r])));

        for (int c = 0; c < pageSteps; ++c)
        {
            auto& noteCell = addCell (c, 0, StepCell::Mode::Value);
            noteCell.isOn = [this, &track, c] { return track.on[(size_t) stepFor (c)].load(); };
            noteCell.setOn = [this, &track, c] (bool on)
            {
                track.on[(size_t) stepFor (c)] = on;
                if (! on) track.chord[(size_t) stepFor (c)] = 0;
            };
            noteCell.getLabel = [this, c] { return cellLabel (stepFor (c)); };
            noteCell.nudge = [this, c] (int delta) { nudgeNote (stepFor (c), delta); };

            if (poly)
            {
                auto& tie = addCell (c, 1, StepCell::Mode::Toggle, true);
                tie.isOn = [this, &track, c] { return track.slide[(size_t) stepFor (c)].load(); };
                tie.setOn = [this, &track, c] (bool on) { track.slide[(size_t) stepFor (c)] = on; };
            }
            else
            {
                auto& accent = addCell (c, 1, StepCell::Mode::Toggle, true);
                accent.isOn = [this, &track, c] { return track.accent[(size_t) stepFor (c)].load(); };
                accent.setOn = [this, &track, c] (bool on) { track.accent[(size_t) stepFor (c)] = on; };

                auto& slide = addCell (c, 2, StepCell::Mode::Toggle, true);
                slide.isOn = [this, &track, c] { return track.slide[(size_t) stepFor (c)].load(); };
                slide.setOn = [this, &track, c] (bool on) { track.slide[(size_t) stepFor (c)] = on; };
            }
        }

        rollButton.onClick = [this] { if (onOpenRoll) onOpenRoll (type); };
        addHeaderButton (rollButton, "machine.roll");

        // Selectors first, then the knobs or faders.
        for (int pass = 0; pass < 2; ++pass)
        {
            for (int i = 0; i < (int) def.params.size(); ++i)
            {
                const auto& param = def.params[(size_t) i];
                const bool isChoice = ! param.choices.empty();
                if (isChoice != (pass == 0)) continue;

                const HelpText h { utf8 (param.label), utf8 (param.help) };
                juce::Component* control;
                if (isChoice)
                {
                    auto* choice = new ChoiceControl (state, pid::synth (type, i), utf8 (param.label), param.choices, h);
                    control = soundControls.add (choice);
                    soundItems.push_back ({ control, juce::jlimit (choiceW, 230, choice->preferredWidth() + 4), 50 });
                }
                else
                {
                    control = soundControls.add (new Knob (state, pid::synth (type, i), utf8 (param.label), h, true, skin.faders));
                    const int width = juce::jmax (skin.faders ? faderW : knobW, ControlLabel::widthFor (utf8 (param.label)) + 4);
                    soundItems.push_back ({ control, width, skin.faders ? faderH : knobH });
                }
                addAndMakeVisible (control);
            }
        }

        // The arpeggiator, at the end of the sound section like on the real synths.
        if (hasArpeggiator (def))
        {
            struct Arp { const char* suffix; const char* label; const std::vector<const char*>& choices; };
            const Arp arps[] { { "arp", "ARPÈGE", arpModeNames() }, { "arprate", "VITESSE ARP", arpRateNames() },
                               { "arpoctaves", "OCTAVES ARP", arpOctaveNames() } };
            for (auto& a : arps)
            {
                auto h = help ("arp");
                h.title = utf8 (a.label);
                auto* choice = new ChoiceControl (state, pid::machine (type, a.suffix), utf8 (a.label), a.choices, h);
                soundControls.add (choice);
                soundItems.push_back ({ choice, juce::jlimit (choiceW, 230, choice->preferredWidth() + 4), 50 });
                addAndMakeVisible (choice);
            }
        }
    }

    // Settings of a whole drum machine (the sampler character): a SON box under the pads.
    if (isDrum())
    {
        for (int i = 0; i < (int) def.params.size(); ++i)
        {
            const auto& param = def.params[(size_t) i];
            const HelpText h { utf8 (param.label), utf8 (param.help) };
            if (param.choices.empty())
            {
                auto* knob = soundControls.add (new Knob (state, pid::synth (type, i), utf8 (param.label), h));
                soundItems.push_back ({ knob, juce::jmax (knobW, ControlLabel::widthFor (utf8 (param.label)) + 4), knobH });
                addAndMakeVisible (knob);
            }
            else
            {
                auto* choice = new ChoiceControl (state, pid::synth (type, i), utf8 (param.label), param.choices, h);
                soundControls.add (choice);
                soundItems.push_back ({ choice, juce::jlimit (choiceW, 230, choice->preferredWidth() + 4), knobH });
                addAndMakeVisible (choice);
            }
        }
    }

    struct StripDef { const char* suffix; const char* label; const char* helpKey; bool bipolar; };
    const StripDef stripDefs[] {
        { "accent", "ACCENT", "strip.accent", false }, { "drive", "DISTO", "strip.drive", false },
        { "crush", "CRUSH", "strip.crush", false }, { "filter", "FILTRE", "strip.filter", true },
        { "echo", "ÉCHO", "strip.echo", false }, { "reverb", "REVERB", "strip.reverb", false },
        { "phaser", "PHASER", "strip.phaser", false }, { "flanger", "FLANGER", "strip.flanger", false },
        { "volume", "VOLUME", "strip.volume", false }, { "pan", "PAN", "strip.pan", true },
    };
    for (auto& s : stripDefs)
    {
        if (! isDrum() && juce::String (s.suffix) == "accent") continue;
        auto* k = new Knob (state, pid::machine (type, s.suffix), utf8 (s.label), help (s.helpKey));
        if (s.bipolar) k->setBipolar();
        stripControls.add (k);
        stripItems.push_back ({ k, juce::jmax (knobW, ControlLabel::widthFor (utf8 (s.label)) + 4), knobH });
        addAndMakeVisible (k);
    }
}

MachinePanel::~MachinePanel()
{
    setLookAndFeel (nullptr);
}

StepCell& MachinePanel::addCell (int column, int row, StepCell::Mode mode, bool dimmed)
{
    auto* cell = cells.add (new StepCell (column, row, mode, dimmed));
    cell->isOutside = [this, column] { return stepFor (column) >= processor.machine (type).track.stepsIn (processor.numSteps()); };
    cell->onEditStart = [this]
    {
        processor.undoManager.beginNewTransaction();
        processor.beginPatternEdit();
    };
    rows[(size_t) row].push_back (cell);
    addAndMakeVisible (cell);
    return *cell;
}

InfoButton& MachinePanel::addInfo (const HelpText& h)
{
    auto* info = infos.add (new InfoButton());
    info->setHelp (h);
    addAndMakeVisible (info);
    return *info;
}

namespace
{
// Bubble with the effects of one drum sound.
class RowFxPanel : public juce::Component
{
public:
    RowFxPanel (juce::AudioProcessorValueTreeState& state, int type, int row, Theme& panelTheme)
        : theme (panelTheme), title (utf8 (machineDef (type).rows[(size_t) row].label) + utf8 (" · EFFETS"))
    {
        setLookAndFeel (&theme);
        struct Def { const char* suffix; const char* label; const char* helpKey; bool bipolar; };
        const Def defs[] { { "drive", "DISTO", "strip.drive", false }, { "filter", "FILTRE", "strip.filter", true },
                           { "pan", "PAN", "strip.pan", true }, { "echo", "ÉCHO", "strip.echo", false },
                           { "reverb", "REVERB", "strip.reverb", false } };
        for (auto& d : defs)
        {
            auto h = help (d.helpKey);
            h.title = utf8 (machineDef (type).rows[(size_t) row].label) + " · " + h.title;
            auto* k = knobs.add (new Knob (state, pid::row (type, row, d.suffix), utf8 (d.label), h));
            if (d.bipolar) k->setBipolar();
            addAndMakeVisible (k);
        }
        setSize (5 * 70 + 24, 128);
    }

    ~RowFxPanel() override { setLookAndFeel (nullptr); }

    void paint (juce::Graphics& g) override
    {
        const auto& skin = theme.skin;
        paint::plate (g, getLocalBounds().toFloat(), skin.plateTop, skin.plateBottom, 6.0f, skin.lightPlate);
        g.setColour (skin.text);
        g.setFont (paint::font (14.0f));
        g.drawText (title, 14, 8, getWidth() - 28, 20, juce::Justification::centredLeft);
    }

    void resized() override
    {
        for (int i = 0; i < knobs.size(); ++i)
            knobs[i]->setBounds (12 + i * 70, 36, 66, 82);
    }

private:
    Theme& theme;
    juce::String title;
    juce::OwnedArray<Knob> knobs;
};
} // namespace

void MachinePanel::showRowFx (int row)
{
    juce::CallOutBox::launchAsynchronously (std::make_unique<RowFxPanel> (processor.apvts, type, row, theme),
                                            fxButtons[row]->getScreenBounds(), nullptr);
}

void MachinePanel::showDiceMenu()
{
    juce::PopupMenu menu;
    const juce::String genre = utf8 (styles()[(size_t) processor.genre()].name);
    menu.addSectionHeader (utf8 ("HASARD · GENRE ") + genre.toUpperCase());
    menu.addItem (1, utf8 ("Nouvelle idée"));
    menu.addItem (2, utf8 ("Varier un peu"));
    menu.addItem (3, utf8 ("Plus de notes"));
    menu.addItem (4, utf8 ("Moins de notes"));
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (diceButton), [this] (int result)
    {
        const GenerateMode modes[] { GenerateMode::NewIdea, GenerateMode::Vary, GenerateMode::Denser, GenerateMode::Sparser };
        if (result >= 1 && result <= 4) processor.generate (type, modes[result - 1]);
    });
}

void MachinePanel::showSoundMenu()
{
    enum { saveId = 900, folderId = 901, mineId = 1000, deleteId = 2000 };
    juce::PopupMenu menu;
    const auto& list = presetsFor (type);
    if (! list.empty()) menu.addSectionHeader (utf8 ("SONS PRÊTS À JOUER"));
    for (int i = 0; i < (int) list.size(); ++i) menu.addItem (i + 1, utf8 (list[(size_t) i].name));

    const auto mine = TechMachineProcessor::userSounds (type);
    menu.addSectionHeader (utf8 ("MES SONS"));
    for (int i = 0; i < mine.size(); ++i) menu.addItem (mineId + i, mine[i]);
    menu.addItem (saveId, utf8 ("Enregistrer ce son…"));
    if (! mine.isEmpty())
    {
        juce::PopupMenu remove;
        for (int i = 0; i < mine.size(); ++i) remove.addItem (deleteId + i, mine[i]);
        menu.addSubMenu (utf8 ("Supprimer un de mes sons"), remove);
        menu.addItem (folderId, utf8 ("Ouvrir le dossier de mes sons"));
    }
    menu.addSeparator();
    menu.addItem (-1, utf8 ("Remplace les réglages de son de cette machine (pas les notes)"), false, false);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (soundButton), [this, mine] (int result)
    {
        const auto& presets = presetsFor (type);
        if (result >= 1 && result <= (int) presets.size()) processor.applyPreset (type, presets[(size_t) result - 1]);
        else if (result >= mineId && result < mineId + mine.size()) processor.loadUserSound (type, mine[result - mineId]);
        else if (result >= deleteId && result < deleteId + mine.size()) TechMachineProcessor::deleteUserSound (type, mine[result - deleteId]);
        else if (result == folderId) TechMachineProcessor::userSoundsFolder (type).revealToUser();
        else if (result == saveId) askSoundName();
    });
}

// Asks for the name of the sound to save.
void MachinePanel::askSoundName()
{
    auto* window = new juce::AlertWindow (utf8 ("Enregistrer ce son"),
                                          utf8 ("Donne un nom à ce son : tu le retrouveras dans SONS › MES SONS, dans tous tes projets."),
                                          juce::MessageBoxIconType::NoIcon);
    window->addTextEditor ("name", utf8 (machineDef (type).name) + " " + juce::String (TechMachineProcessor::userSounds (type).size() + 1));
    window->addButton (utf8 ("ENREGISTRER"), 1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton (utf8 ("ANNULER"), 0, juce::KeyPress (juce::KeyPress::escapeKey));
    juce::Component::SafePointer<MachinePanel> safe (this);
    window->enterModalState (true, juce::ModalCallbackFunction::create ([safe, window] (int result)
    {
        if (result == 1 && safe != nullptr) safe->processor.saveUserSound (safe->type, window->getTextEditorContents ("name"));
    }), true);
}

bool MachinePanel::isInterestedInFileDrag (const juce::StringArray& files)
{
    if (type != Sampler) return false;
    for (auto& f : files)
        if (juce::File (f).hasFileExtension ("wav;aif;aiff;flac;ogg")) return true;
    return false;
}

// A file dropped on a pad goes on that pad; dropped elsewhere, it goes on the first empty pad (several files fill several pads).
void MachinePanel::filesDropped (const juce::StringArray& files, int, int y)
{
    int pad = (y - drumTop) / drumRowH;
    const bool onPad = y >= drumTop && pad >= 0 && pad < TechMachineProcessor::numPads;
    if (! onPad)
    {
        pad = 0;
        while (pad < TechMachineProcessor::numPads - 1 && processor.samplePadName (pad).isNotEmpty()) ++pad;
    }
    for (auto& f : files)
    {
        if (pad >= TechMachineProcessor::numPads) break;
        if (processor.loadSamplePad (pad, juce::File (f))) ++pad;
    }
}

void MachinePanel::chooseSampleFile (std::function<void (const juce::File&)> then)
{
    chooser = std::make_unique<juce::FileChooser> (utf8 ("Choisis un fichier audio"), juce::File(), "*.wav;*.aif;*.aiff;*.flac;*.ogg");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [then] (const juce::FileChooser& fc)
                          {
                              if (fc.getResult().existsAsFile()) then (fc.getResult());
                          });
}

void MachinePanel::refreshSampleNames()
{
    if (type != Sampler) return;
    for (int r = 0; r < rowMutes.size() && r < TechMachineProcessor::numPads; ++r)
    {
        const auto name = processor.samplePadName (r);
        rowMutes[r]->setButtonText (name.isNotEmpty() ? name.toUpperCase().substring (0, 14) : utf8 (def.rows[(size_t) r].label));
    }
}

void MachinePanel::showPatternMenu()
{
    juce::PopupMenu menu;
    std::vector<const Pattern*> items;
    const char* const kindTitles[] { "RYTHMES CÉLÈBRES", "LIGNES DE BASSE", "ACCORDS", "MÉLODIES ET ARPÈGES" };

    for (auto kind : patternKindsFor (def))
    {
        const auto categories = patternCategories (kind);
        if (kind == PatternKind::Drums) menu.addSectionHeader (utf8 (kindTitles[(int) kind]));
        for (auto& category : categories)
        {
            juce::PopupMenu sub;
            for (auto& p : patterns())
            {
                if (p.kind != kind || p.category != category || patternCoverage (def, p) < 0.75f) continue;
                items.push_back (&p);
                const juce::String name = utf8 (p.name);
                sub.addItem ((int) items.size(), p.bpm > 0 ? name + "   (" + juce::String (p.bpm) + " BPM)" : name);
            }
            if (sub.getNumItems() > 0) menu.addSubMenu (utf8 (category), sub);
        }
    }
    menu.addSeparator();
    menu.addItem (-1, utf8 ("Remplace la partie de cette machine · Ctrl+Z pour annuler"), false, false);

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (patternButton), [this, items] (int result)
    {
        if (result >= 1 && result <= (int) items.size()) processor.applyPattern (type, *items[(size_t) result - 1]);
    });
}

void MachinePanel::showMoreMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, utf8 ("Copier la partie"));
    menu.addItem (2, utf8 ("Coller la partie"), processor.canPaste (type));
    menu.addSeparator();
    menu.addItem (3, utf8 ("Dupliquer la mesure 1 sur toute la séquence"), processor.numSteps() > pageSteps);
    menu.addItem (4, utf8 ("Décaler d'un pas vers la gauche ←"));
    menu.addItem (5, utf8 ("Décaler d'un pas vers la droite →"));
    menu.addSeparator();
    menu.addItem (6, utf8 ("Tout effacer"));

    // Length and speed of this track: polyrhythms, triplets.
    const auto& track = processor.machine (type).track;
    juce::PopupMenu lengths, speeds;
    lengths.addItem (400, utf8 ("Comme la séquence (") + juce::String (processor.numSteps()) + utf8 (" pas)"), true, track.length.load() == 0);
    for (int n : { 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 20, 24, 28, 32, 48, 64 })
        lengths.addItem (400 + n, juce::String (n) + utf8 (" pas"), true, track.length.load() == n);
    const char* const speedNames[numTrackSpeeds] { "Normale (doubles croches)", "Double (×2)", "Moitié (×½)", "Triolets (×1,5)", "Triolets lents (×¾)" };
    for (int i = 0; i < numTrackSpeeds; ++i) speeds.addItem (500 + i, utf8 (speedNames[i]), true, track.speed.load() == i);
    menu.addSeparator();
    menu.addSubMenu (utf8 ("Longueur de cette piste"), lengths);
    menu.addSubMenu (utf8 ("Vitesse de cette piste"), speeds);
    if (processor.isHosted())
    {
        // Where this machine leaves in the music software.
        juce::PopupMenu outputs;
        const int current = (int) processor.apvts.getRawParameterValue (pid::machine (type, "output"))->load();
        outputs.addItem (300, utf8 ("Master (avec les effets du master)"), true, current == 0);
        for (int bus = 1; bus <= TechMachineProcessor::numAuxOutputs; ++bus)
            outputs.addItem (300 + bus, utf8 ("Sortie ") + juce::String (bus + 1), true, current == bus);
        menu.addSeparator();
        menu.addSubMenu (utf8 ("Sortie dans le logiciel de musique"), outputs);
    }
    if (type == Sampler)
    {
        menu.addSeparator();
        juce::PopupMenu pads;
        for (int i = 0; i < TechMachineProcessor::numPads; ++i)
        {
            const auto name = processor.samplePadName (i);
            pads.addItem (100 + i, utf8 ("Pad ") + juce::String (i + 1) + (name.isNotEmpty() ? "   (" + name + ")" : juce::String()));
        }
        menu.addSubMenu (utf8 ("Charger un son sur un pad…"), pads);
        menu.addItem (200, utf8 ("Découper une boucle sur les 8 pads…"));
        menu.addItem (201, utf8 ("Vider les pads"));
    }
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (moreButton), [this] (int result)
    {
        if (result >= 300 && result <= 300 + TechMachineProcessor::numAuxOutputs)
        {
            processor.setParam (pid::machine (type, "output"), (float) (result - 300));
            return;
        }
        const auto& t = processor.machine (type).track;
        if (result >= 400 && result <= 400 + maxSteps)
        {
            processor.setTrackTiming (type, result - 400, t.speed.load());
            return;
        }
        if (result >= 500 && result < 500 + numTrackSpeeds)
        {
            processor.setTrackTiming (type, t.length.load(), result - 500);
            return;
        }
        switch (result)
        {
            case 1: processor.copyMachine (type); break;
            case 2: processor.pasteMachine (type); break;
            case 3: processor.duplicateFirstBar (type); break;
            case 4: processor.shiftMachine (type, -1); break;
            case 5: processor.shiftMachine (type, 1); break;
            case 6: processor.clearMachine (type); break;
            case 200: chooseSampleFile ([this] (const juce::File& f) { processor.chopLoop (f); }); break;
            case 201: processor.clearSamplePads(); break;
            case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107:
                chooseSampleFile ([this, pad = result - 100] (const juce::File& f) { processor.loadSamplePad (pad, f); });
                break;
            default: break;
        }
    });
}

void MachinePanel::addHeaderButton (juce::Button& button, const juce::String& helpKey)
{
    button.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3b40));
    button.setColour (juce::TextButton::textColourOffId, colours::text);
    button.setTooltip (shortHelp (help (helpKey).body));
    addAndMakeVisible (button);
    auto* info = &addInfo (help (helpKey));
    headerButtons.push_back ({ &button, info });
}

int MachinePanel::cellX (int column) const
{
    return stepsX() + column * (cellW + cellGap) + (column / 4) * (groupGap - cellGap);
}

juce::String MachinePanel::cellLabel (int step) const
{
    auto& track = processor.machine (type).track;
    const int scale = processor.currentScale();
    const int semitone = snapToScale (scale, track.note[(size_t) step].load());
    const int octave = (int) processor.apvts.getRawParameterValue (pid::synth (type, 0))->load();
    const int note = machineBaseNote (def, processor.currentKey(), octave) + semitone;

    if (isPoly())
    {
        const int chord = (int) processor.apvts.getRawParameterValue (pid::synth (type, 1))->load();
        return chordLabel (scale, note, semitone, chord);
    }
    return noteName (note);
}

void MachinePanel::nudgeNote (int step, int delta)
{
    processor.machine (type).track.chord[(size_t) step] = 0;   // back to the chord built from the note
    auto& note = processor.machine (type).track.note[(size_t) step];
    note = juce::jlimit (0, noteRange - 1, moveInScale (processor.currentScale(), note.load(), delta));
}

void MachinePanel::setPlayhead (int step)
{
    if (step == shownStep && page == shownPage) return;
    shownStep = step;
    shownPage = page;
    const int column = step - page * pageSteps;
    ledStrip.setStep (column >= 0 && column < pageSteps ? column : -1);
    for (auto* cell : cells) cell->setPlayhead (stepFor (cell->column) == step);
}

void MachinePanel::refreshCells()
{
    bool automated = false;
    for (auto& t : processor.machine (type).track.lockTarget) automated = automated || t.load() >= 0;
    automationButton.setToggleState (automated, juce::dontSendNotification);
    automationButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff5b4bb8));
    repaint();
}

std::array<int, 3> MachinePanel::synthRowY() const
{
    return { headerH + 30, headerH + 92, headerH + 124 };
}

int MachinePanel::sequencerBottom() const
{
    if (isDrum())
    {
        const int grid = drumTop + (numRows() + 1) * drumRowH + 18;
        if (soundItems.empty()) return grid;
        const int width = cellX (15) + cellW - (side() + 10);
        return grid + boxTitle + layoutFlow (soundItems, { 0, 0, width - 2 * boxPad, 0 }, false) + 22;
    }
    const int stripTop = synthRowY()[(size_t) (isPoly() ? 1 : 2)] + 26 + 30;
    const int stripW = cellX (15) + cellW - (side() + 10);
    return stripTop + boxTitle + layoutFlow (stripItems, { 0, 0, stripW - 2 * boxPad, 0 }, false) + 18;
}

int MachinePanel::layoutFlow (const std::vector<Item>& items, juce::Rectangle<int> area, bool apply) const
{
    int x = 0, y = 0, rowHeight = 0;
    for (auto& item : items)
    {
        if (x > 0 && x + item.width > area.getWidth())
        {
            x = 0;
            y += rowHeight;
            rowHeight = 0;
        }
        if (apply) item.component->setBounds (area.getX() + x, area.getY() + y, item.width - 4, item.height - 4);
        x += item.width;
        rowHeight = juce::jmax (rowHeight, item.height);
    }
    return y + rowHeight;
}

int MachinePanel::preferredHeight (int width) const
{
    if (collapsed) return headerH + 6;

    const int available = width - side() - rightX() - 12;
    int right;
    if (isDrum())
    {
        const int stripW = available - 4 * drumColumnW - fxW - 14;
        right = layoutFlow (stripItems, { 0, 0, stripW - 2 * boxPad, 0 }, false);
    }
    else
    {
        right = layoutFlow (soundItems, { 0, 0, available - 2 * boxPad, 0 }, false);
    }
    return juce::jmax (sequencerBottom(), headerH + 18 + boxTitle + right + 14);
}

void MachinePanel::layoutRow (int row, int y, int height)
{
    for (auto* cell : rows[(size_t) row])
        cell->setBounds (cellX (cell->column), y, cellW, height);
}

void MachinePanel::resized()
{
    auto header = getLocalBounds().removeFromTop (headerH).withTrimmedLeft (side()).withTrimmedRight (side()).reduced (12, 12);
    removeButton.setBounds (header.removeFromRight (30));
    header.removeFromRight (4);
    collapseButton.setBounds (header.removeFromRight (30));
    header.removeFromRight (14);

    for (auto it = headerButtons.rbegin(); it != headerButtons.rend(); ++it)
    {
        it->second->setBounds (header.removeFromRight (16).withSizeKeepingCentre (15, 15));
        header.removeFromRight (3);
        const int width = it->first == &moreButton ? 36 : it->first == &diceButton ? 104 : it->first == &patternButton ? 114 : it->first == &soundButton ? 92 : it->first == &rollButton ? 114 : it->first == &automationButton ? 120 : 76;
        it->first->setBounds (header.removeFromRight (width));
        header.removeFromRight (10);
    }

    subtitleRight = header.getRight() - 6;
    const auto modelWidth = (int) juce::GlyphArrangement::getStringWidth (paint::display (30.0f), utf8 (skin.model));
    const auto brandWidth = (int) juce::GlyphArrangement::getStringWidth (paint::condensed (13.0f), utf8 (skin.brand));
    // The printed brand ("ANALOG DRUM MACHINE"...) is decoration: it gives way to the buttons when space is short.
    showBrand = side() + 18 + modelWidth + 12 + brandWidth + 10 + 16 <= subtitleRight;
    machineInfo.setBounds (side() + 18 + modelWidth + (showBrand ? 12 + brandWidth : 0) + 10, 13, 16, 16);

    const bool showBody = ! collapsed;
    for (auto* c : getChildren())
    {
        bool inHeader = c == &machineInfo || c == &removeButton || c == &collapseButton;
        for (auto& h : headerButtons) inHeader = inHeader || c == h.first || c == h.second;
        if (! inHeader) c->setVisible (showBody);
    }
    if (collapsed) return;

    const int labelX = side() + 14;
    const int available = getWidth() - side() - rightX() - 12;
    ledStrip.setBounds (cellX (0), headerH + 10, cellX (15) + cellW - cellX (0), 14);

    if (isDrum())
    {
        for (int r = 0; r <= numRows(); ++r)
        {
            const int y = drumTop + r * drumRowH;
            if (r < numRows())
            {
                rowMutes[r]->setBounds (labelX, y, labelW, drumRowH - 3);
                for (int k = 0; k < 4; ++k)
                    rowKnobs[r * 4 + k]->setBounds (rightX() + k * drumColumnW + 12, y - 1, 30, 30);
                fxButtons[r]->setBounds (rightX() + 4 * drumColumnW + 4, y + 2, fxW - 8, drumRowH - 8);
            }
            rowInfoButtons[(size_t) r]->setBounds (labelX + labelW + 4, y + 7, 14, 14);
            layoutRow (r, y, drumRowH - 3);
        }
        for (int k = 0; k < 4; ++k)
            columnInfos[(size_t) k]->setBounds (rightX() + k * drumColumnW + 48, headerH + 11, 13, 13);

        const int stripX = rightX() + 4 * drumColumnW + fxW + 14;
        stripBox = { stripX, headerH + 18, available - 4 * drumColumnW - fxW - 14, getHeight() - headerH - 30 };
        layoutFlow (stripItems, stripBox.reduced (boxPad, 0).withTrimmedTop (boxTitle), true);
        soundBox = {};
        if (! soundItems.empty())
        {
            const int top = drumTop + (numRows() + 1) * drumRowH + 18;
            const int width = cellX (15) + cellW - (side() + 10);
            const int height = boxTitle + layoutFlow (soundItems, { 0, 0, width - 2 * boxPad, 0 }, false) + 8;
            soundBox = { side() + 10, top, width, height };
            layoutFlow (soundItems, soundBox.reduced (boxPad, 0).withTrimmedTop (boxTitle), true);
        }
    }
    else
    {
        const auto y = synthRowY();
        const int heights[3] { 56, 26, 26 };
        for (int r = 0; r < (int) rows.size(); ++r)
        {
            rowInfoButtons[(size_t) r]->setBounds (labelX + labelW + 4, y[(size_t) r] + heights[r] / 2 - 7, 14, 14);
            layoutRow (r, y[(size_t) r], heights[r]);
        }

        const int stripTop = y[(size_t) (isPoly() ? 1 : 2)] + 26 + 30;
        soundBox = { rightX(), headerH + 18, available, getHeight() - headerH - 30 };
        stripBox = { side() + 10, stripTop, cellX (15) + cellW - (side() + 10), getHeight() - stripTop - 12 };
        layoutFlow (soundItems, soundBox.reduced (boxPad, 0).withTrimmedTop (boxTitle), true);
        // The box fits its knobs, even when the sound section on the right makes the panel taller.
        stripBox.setHeight (juce::jmin (stripBox.getHeight(), boxTitle + layoutFlow (stripItems, stripBox.reduced (boxPad, 0).withTrimmedTop (boxTitle), true) + 8));
    }
}

void MachinePanel::paintOverChildren (juce::Graphics& g)
{
    if (! midiTarget) return;
    const auto badge = juce::Rectangle<float> ((float) machineInfo.getRight() + 10.0f, 12.0f, 54.0f, 18.0f);
    g.setColour (juce::Colour (0xff2fb8ff));
    g.fillRoundedRectangle (badge, 9.0f);
    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.setFont (paint::condensed (12.0f));
    g.drawText ("MIDI", badge, juce::Justification::centred);
}

void MachinePanel::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const int s = side();

    paint::plate (g, bounds, skin.plateTop, skin.plateBottom, 4.0f, skin.lightPlate);

    if (skin.wood)
    {
        paint::wood (g, bounds.withWidth ((float) s), 11 + type);
        paint::wood (g, bounds.withTrimmedLeft (bounds.getWidth() - (float) s), 29 + type);
    }
    else
    {
        paint::rackEar (g, bounds.withWidth ((float) s));
        paint::rackEar (g, bounds.withTrimmedLeft (bounds.getWidth() - (float) s));
    }

    const auto bandArea = juce::Rectangle<float> ((float) s, 0.0f, bounds.getWidth() - 2.0f * (float) s, (float) headerH - 2.0f);
    paint::band (g, bandArea, skin, 0.0f);

    const float modelWidth = juce::GlyphArrangement::getStringWidth (paint::display (30.0f), utf8 (skin.model));
    g.setColour (skin.bandText);
    g.setFont (paint::display (30.0f));
    g.drawText (utf8 (skin.model), s + 18, 4, (int) modelWidth + 4, 32, juce::Justification::centredLeft);
    g.setFont (paint::condensed (13.0f));
    g.setColour (skin.bandText.withAlpha (0.85f));
    if (showBrand) g.drawText (utf8 (skin.brand), s + 18 + (int) modelWidth + 12, 13, 400, 16, juce::Justification::centredLeft);
    g.setColour (skin.bandText.withAlpha (0.55f));
    g.setFont (paint::font (11.0f, false));
    g.drawFittedText (utf8 (def.subtitle), s + 18, 34, juce::jmax (60, subtitleRight - (s + 18)), 14, juce::Justification::centredLeft, 1, 0.85f);

    if (collapsed) return;

    if (skin.stripes)
        paint::stripes (g, { (float) s, (float) headerH, bounds.getWidth() - 2.0f * (float) s, 6.0f });

    const int labelX = s + 14;
    g.setColour (skin.textDim);
    g.setFont (paint::condensed (12.0f));
    // A track with its own length or speed says so next to its steps.
    const auto& track = processor.machine (type).track;
    juce::String steps = utf8 ("PAS");
    if (track.length.load() > 0) steps << utf8 (" · ") << track.length.load();
    const char* const speedMarks[numTrackSpeeds] { "", " · ×2", " · ×½", " · ×1,5", " · ×¾" };
    steps << utf8 (speedMarks[(size_t) juce::jlimit (0, numTrackSpeeds - 1, track.speed.load())]);
    g.drawText (steps, labelX, headerH + 9, labelW, 16, juce::Justification::centredLeft);

    if (isDrum())
    {
        g.drawText ("ACCENT", labelX + 20, drumTop + numRows() * drumRowH, labelW - 20, drumRowH - 3, juce::Justification::centredLeft);
        for (int k = 0; k < 4; ++k)
            g.drawText (utf8 (drumColumnNames[k]), rightX() + k * drumColumnW - 10, headerH + 9, 56, 16, juce::Justification::centredRight);

        // Printed group lines under every 4 steps, like on the real machines.
        g.setColour (skin.line);
        g.setFont (paint::condensed (10.0f));
        const int y = drumTop + (numRows() + 1) * drumRowH + 1;
        for (int group = 0; group < 4; ++group)
        {
            const int x0 = cellX (group * 4), x1 = cellX (group * 4 + 3) + cellW;
            g.fillRect (x0 + 2, y, x1 - x0 - 4, 2);
            g.drawText (juce::String (group * 4 + 1 + page * pageSteps), x0, y + 3, 24, 11, juce::Justification::centredLeft);
        }
    }
    else
    {
        const auto y = synthRowY();
        const bool chords = isPoly() && processor.apvts.getRawParameterValue (pid::synth (type, 1))->load() > 0.5f;
        const char* labels[3] { chords ? "ACCORDS" : "NOTES", isPoly() ? "TENIR" : "ACCENT", "SLIDE" };
        const int heights[3] { 56, 26, 26 };
        for (int r = 0; r < (int) rows.size(); ++r)
        {
            g.setColour (r == 0 ? skin.text : skin.textDim);
            g.setFont (paint::condensed (r == 0 ? 15.0f : 13.0f));
            g.drawText (utf8 (labels[r]), labelX, y[(size_t) r], labelW, heights[r], juce::Justification::centredLeft);
        }
    }

    if (! soundBox.isEmpty()) paint::printedBox (g, soundBox.toFloat(), utf8 ("SON"), skin);
    if (! stripBox.isEmpty()) paint::printedBox (g, stripBox.toFloat(), utf8 ("EFFETS & MIX"), skin);
}
} // namespace techno
