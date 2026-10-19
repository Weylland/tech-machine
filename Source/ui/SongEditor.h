#pragma once

#include "../PluginProcessor.h"
#include "Widgets.h"

namespace techno
{
inline juce::Colour partColour (int part)
{
    static const juce::uint32 colours[TechMachineProcessor::numParts] {
        0xffff7a2a, 0xff4db6e8, 0xff9ccc45, 0xffb48cff, 0xffffc02e, 0xffff4f8b, 0xff2fd1b8, 0xffd9d4c4 };
    return juce::Colour (colours[juce::jlimit (0, TechMachineProcessor::numParts - 1, part)]);
}

inline juce::String partName (int part) { return juce::String::charToString ((juce::juce_wchar) ('A' + part)); }

// Full-screen editor of the song: the list of parts played in order, each repeated a number of times.
class SongEditor : public juce::Component, private juce::Timer
{
public:
    explicit SongEditor (TechMachineProcessor& p) : processor (p)
    {
        setWantsKeyboardFocus (true);
        entries = processor.song();
        if (! entries.empty()) selected = 0;

        playButton.setTooltip (utf8 ("Allumé : la lecture suit le morceau, partie après partie. Éteint : la partie choisie tourne en boucle."));
        playButton.setToggleState (processor.isSongMode(), juce::dontSendNotification);
        playButton.onClick = [this] { processor.setSongMode (playButton.getToggleState()); };
        addAndMakeVisible (playButton);

        for (int part = 0; part < TechMachineProcessor::numParts; ++part)
        {
            auto& b = addButtons[(size_t) part];
            b.setButtonText ("+ " + partName (part));
            b.setColour (juce::TextButton::buttonColourId, partColour (part).withMultipliedBrightness (0.45f));
            b.setColour (juce::TextButton::textColourOffId, colours::text);
            b.setTooltip (utf8 ("Ajoute la partie ") + partName (part) + utf8 (" à la fin du morceau"));
            b.onClick = [this, part]
            {
                if ((int) entries.size() >= TechMachineProcessor::maxSongEntries) return;
                entries.push_back ({ part, 2 });
                selected = (int) entries.size() - 1;
                commit();
            };
            addAndMakeVisible (b);
        }

        auto tool = [this] (IconButton& b, const char* tip, std::function<void()> action)
        {
            b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3b40));
            b.setColour (juce::TextButton::textColourOffId, colours::text);
            b.setTooltip (utf8 (tip));
            b.onClick = [this, action] { action(); grabKeyboardFocus(); };
            addAndMakeVisible (b);
        };
        tool (fewer, "Joue la partie sélectionnée une fois de moins  (−)", [this] { changeRepeats (-1); });
        tool (more, "Joue la partie sélectionnée une fois de plus  (+)", [this] { changeRepeats (1); });
        tool (earlier, "Avance la partie sélectionnée dans le morceau", [this] { move (-1); });
        tool (later, "Recule la partie sélectionnée dans le morceau", [this] { move (1); });
        tool (remove, "Retire la partie sélectionnée du morceau  (Suppr)", [this] { removeSelected(); });
        tool (close, "Revenir au rack  (Échap)", [this] { if (onClose) onClose(); });
        close.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff6b46c1));
        startTimerHz (15);
    }

    std::function<void()> onClose;
    const std::vector<TechMachineProcessor::SongEntry>& songEntries() const { return entries; }

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colour (0xff101113));
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
        g.setColour (colours::text);
        g.setFont (paint::display (22.0f));
        g.drawText (utf8 ("MORCEAU"), 16, 8, 160, 30, juce::Justification::centredLeft);

        const auto area = blockArea();
        if (entries.empty())
        {
            g.setColour (colours::dim);
            g.setFont (paint::font (15.0f, false));
            g.drawFittedText (utf8 ("Ajoute des parties avec les boutons + A à + H : par exemple A (intro), B (couplet) ×4, C (break), B ×4.\n"
                                    "Écris d'abord tes parties dans le rack : choisis la partie avec les boutons PARTIES en haut."),
                              area.reduced (20), juce::Justification::centred, 3);
        }

        const int playingEntry = processor.isSongMode() && processor.playing.load() ? processor.songPosition() : -1;
        for (int i = 0; i < (int) entries.size(); ++i)
        {
            const auto r = blockBounds (i).toFloat();
            const auto colour = partColour (entries[(size_t) i].part);
            g.setColour (colour.withMultipliedBrightness (i == playingEntry ? 0.9f : 0.45f));
            g.fillRoundedRectangle (r, 6.0f);
            if (i == selected)
            {
                g.setColour (juce::Colours::white);
                g.drawRoundedRectangle (r.reduced (1.0f), 6.0f, 2.5f);
            }
            g.setColour (juce::Colours::white);
            g.setFont (paint::display (34.0f));
            g.drawText (partName (entries[(size_t) i].part), r.withTrimmedBottom (r.getHeight() * 0.35f), juce::Justification::centred);
            g.setFont (paint::condensed (15.0f));
            g.drawText (juce::String (juce::CharPointer_UTF8 ("\xc3\x97 ")) + juce::String (entries[(size_t) i].repeats),
                        r.withTrimmedTop (r.getHeight() * 0.6f), juce::Justification::centred);
            if (i == playingEntry)
            {
                juce::Path triangle;
                triangle.addTriangle (r.getX() + 8.0f, r.getY() + 8.0f, r.getX() + 8.0f, r.getY() + 20.0f, r.getX() + 18.0f, r.getY() + 14.0f);
                g.fillPath (triangle);
            }
        }

        // Total length
        int loops = 0;
        for (auto& e : entries) loops += e.repeats;
        const double seconds = processor.songSeconds();
        g.setColour (colours::text);
        g.setFont (paint::condensed (15.0f));
        const int bars = processor.numSteps() / pageSteps;
        g.drawText (utf8 ("Durée : ") + juce::String ((int) seconds / 60) + " min " + juce::String ((int) seconds % 60).paddedLeft ('0', 2)
                        + juce::String (juce::CharPointer_UTF8 (" \xc2\xb7 ")) + juce::String (loops) + utf8 (loops > 1 ? " boucles de " : " boucle de ")
                        + juce::String (bars) + utf8 (bars > 1 ? " mesures" : " mesure"),
                    getLocalBounds().reduced (16, 0).removeFromBottom (54).removeFromTop (24), juce::Justification::centredLeft);

        g.setColour (colours::dim);
        g.setFont (paint::font (12.5f, false));
        g.drawFittedText (utf8 ("Clic = sélectionner · double-clic = afficher cette partie dans le rack · clic droit = retirer · "
                                "+ / − = nombre de fois · JOUER LE MORCEAU = la lecture suit la liste au lieu de boucler"),
                          getLocalBounds().reduced (16, 0).removeFromBottom (26), juce::Justification::centredLeft, 1, 0.8f);
    }

    void resized() override
    {
        auto row = getLocalBounds().reduced (12, 0).withTrimmedTop (8).removeFromTop (30);
        row.removeFromLeft (140);
        close.setBounds (row.removeFromRight (110));
        row.removeFromRight (8);
        playButton.setBounds (row.removeFromRight (170));

        row = getLocalBounds().reduced (12, 0).withTrimmedTop (46).removeFromTop (28);
        for (auto& b : addButtons)
        {
            b.setBounds (row.removeFromLeft (52));
            row.removeFromLeft (6);
        }
        row.removeFromLeft (20);
        for (auto* b : { &fewer, &more, &earlier, &later, &remove })
        {
            b->setBounds (row.removeFromLeft (b == &remove ? 118 : 84));
            row.removeFromLeft (6);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const int i = blockAt (e.getPosition());
        if (i < 0) return;
        selected = i;
        if (e.mods.isPopupMenu()) removeSelected();
        else if (e.getNumberOfClicks() >= 2) processor.selectPart (entries[(size_t) i].part);
        repaint();
    }

    bool keyPressed (const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::escapeKey) { if (onClose) onClose(); return true; }
        if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey) { removeSelected(); return true; }
        if (key.getTextCharacter() == '+') { changeRepeats (1); return true; }
        if (key.getTextCharacter() == '-') { changeRepeats (-1); return true; }
        if (key == juce::KeyPress::leftKey) { selected = juce::jmax (0, selected - 1); repaint(); return true; }
        if (key == juce::KeyPress::rightKey) { selected = juce::jmin ((int) entries.size() - 1, selected + 1); repaint(); return true; }
        return false;
    }

    juce::Point<int> blockCentre (int index) const { return blockBounds (index).getCentre(); }

private:
    juce::Rectangle<int> blockArea() const { return getLocalBounds().reduced (12, 0).withTrimmedTop (86).withTrimmedBottom (60); }

    juce::Rectangle<int> blockBounds (int index) const
    {
        const auto area = blockArea();
        constexpr int w = 92, h = 84, gap = 10;
        const int perRow = juce::jmax (1, (area.getWidth() + gap) / (w + gap));
        return { area.getX() + (index % perRow) * (w + gap), area.getY() + (index / perRow) * (h + gap), w, h };
    }

    int blockAt (juce::Point<int> pos) const
    {
        for (int i = 0; i < (int) entries.size(); ++i)
            if (blockBounds (i).contains (pos)) return i;
        return -1;
    }

    void changeRepeats (int delta)
    {
        if (selected < 0 || selected >= (int) entries.size()) return;
        auto& e = entries[(size_t) selected];
        e.repeats = juce::jlimit (1, 64, e.repeats + delta);
        commit();
    }

    void move (int direction)
    {
        const int to = selected + direction;
        if (selected < 0 || to < 0 || to >= (int) entries.size()) return;
        std::swap (entries[(size_t) selected], entries[(size_t) to]);
        selected = to;
        commit();
    }

    void removeSelected()
    {
        if (selected < 0 || selected >= (int) entries.size()) return;
        entries.erase (entries.begin() + selected);
        selected = juce::jmin (selected, (int) entries.size() - 1);
        commit();
    }

    void commit()
    {
        processor.setSong (entries);
        repaint();
    }

    void timerCallback() override
    {
        if (playButton.getToggleState() != processor.isSongMode())
            playButton.setToggleState (processor.isSongMode(), juce::dontSendNotification);
        repaint();
    }

    TechMachineProcessor& processor;
    std::vector<TechMachineProcessor::SongEntry> entries;
    int selected = -1;
    LedButton playButton { utf8 ("JOUER LE MORCEAU"), juce::Colour (0xff3dff6a) };
    std::array<juce::TextButton, TechMachineProcessor::numParts> addButtons;
    IconButton fewer { "", utf8 ("− 1 FOIS") }, more { "", utf8 ("+ 1 FOIS") },
               earlier { "", utf8 ("AVANCER") }, later { "", utf8 ("RECULER") }, remove { "trash", utf8 ("RETIRER") },
               close { "close", utf8 ("FERMER") };
};
} // namespace techno
