#include "../PluginProcessor.h"
#include "../core/MidiMap.h"

using namespace techno;

// Exports for music software: the notes as a MIDI file, and one WAV per machine ("stems").

namespace
{
constexpr int ticksPerStep = 240;   // 960 ticks per quarter note

// The parts in the order they play: the song when it is on, otherwise the current part once.
std::vector<int> partsToExport (const TechMachineProcessor& p)
{
    std::vector<int> parts;
    if (p.isSongMode())
        for (auto& entry : p.song())
            for (int r = 0; r < entry.repeats; ++r) parts.push_back (entry.part);
    if (parts.empty()) parts.push_back (p.currentPart());
    return parts;
}

void setParamInXml (juce::XmlElement& state, const juce::String& id, float value)
{
    for (auto* param : state.getChildWithTagNameIterator ("PARAM"))
        if (param->getStringAttribute ("id") == id) param->setAttribute ("value", (double) value);
}

float paramInXml (const juce::XmlElement& state, const juce::String& id)
{
    for (auto* param : state.getChildWithTagNameIterator ("PARAM"))
        if (param->getStringAttribute ("id") == id) return (float) param->getDoubleAttribute ("value");
    return 0.0f;
}
} // namespace

bool TechMachineProcessor::exportMidi (const juce::File& target) const
{
    const int steps = numSteps();
    const double sw = swing->load() * 0.33;
    auto tickOf = [sw] (long long step) { return (double) (step * ticksPerStep) + (step % 2 == 1 ? sw * ticksPerStep : 0.0); };
    const auto parts = partsToExport (*this);

    juce::MidiFile midi;
    midi.setTicksPerQuarterNote (960);
    juce::MidiMessageSequence tempoTrack;
    tempoTrack.addEvent (juce::MidiMessage::tempoMetaEvent ((int) (60000000.0 / bpm->load())), 0.0);
    tempoTrack.addEvent (juce::MidiMessage::timeSignatureMetaEvent (4, 4), 0.0);
    midi.addTrack (tempoTrack);

    for (int t : order)
    {
        if (! isShown (t)) continue;
        const auto& def = machineDef (t);
        const bool drum = def.kind == MachineKind::Drum;
        const int channel = drum ? 10 : 1;
        juce::MidiMessageSequence seq;
        seq.addEvent (juce::MidiMessage::textMetaEvent (3, utf8 (def.name)), 0.0);
        auto add = [&seq, channel] (int note, int velocity, double from, double to)
        {
            if (note < 0 || note > 127) return;
            seq.addEvent (juce::MidiMessage::noteOn (channel, note, (juce::uint8) velocity), from);
            seq.addEvent (juce::MidiMessage::noteOff (channel, note), juce::jmax (from + 1.0, to));
        };

        const int octave = def.params.empty() ? 2 : (int) apvts.getRawParameterValue (pid::synth (t, 0))->load();
        const int base = machineBaseNote (def, currentKey(), octave);
        const int chordType = def.kind == MachineKind::Poly ? (int) apvts.getRawParameterValue (pid::synth (t, 1))->load() : 0;

        for (size_t loop = 0; loop < parts.size(); ++loop)
        {
            const auto& track = partTrack (parts[loop], t);

            // A track with its own length goes on from loop to loop; one with its own speed has more or fewer steps.
            const int own = track.stepsIn (steps);
            const double factor = trackSpeedFactors[(size_t) juce::jlimit (0, numTrackSpeeds - 1, track.speed.load())];
            const double stepTicks = ticksPerStep / factor;
            const long long first = (long long) loop * steps;
            const auto firstCount = (long long) std::ceil ((double) first * factor - 1.0e-9);
            const auto endCount = (long long) std::ceil ((double) (first + steps) * factor - 1.0e-9);
            auto startOf = [&] (long long count) { return juce::exactlyEqual (factor, 1.0) ? tickOf (count) : (double) count * stepTicks; };
            auto stepValue = [&] (size_t st, int setting)
            {
                if (const int lane = track.findLane (automation::stepTarget (def, setting)); lane >= 0)
                    if (const float v = track.lockValue[(size_t) lane][st].load(); v >= 0.0f) return v;
                return automation::stepNormals[setting];
            };

            for (long long count = firstCount; count < endCount; ++count)
            {
                const int s = (int) (count % own);
                const auto st = (size_t) s, prev = (size_t) ((s + own - 1) % own);
                const bool notFirst = count > firstCount;
                // Conditions are followed (no FILL in a file); chance is not: every step that may play is written.
                if (! automation::conditionPlays (juce::roundToInt (stepValue (st, automation::Condition)), count / own, false)) continue;
                const double late = stepValue (st, automation::Delay) * 0.5 * stepTicks;
                const double at = startOf (count) + late;
                auto endOf = [&] (int length) { return startOf (count + length) + late; };
                const int velocity = juce::jlimit (1, 127, juce::roundToInt ((float) (track.accent[st] ? 120 : 96)
                                                                             * stepValue (st, automation::Velocity) / automation::normalVelocity));
                if (drum)
                {
                    for (int r = 0; r < (int) def.rows.size(); ++r)
                    {
                        if (! track.hits[(size_t) r][st]) continue;
                        const int roll = juce::jmax (1, (int) track.rolls[(size_t) r][st].load());
                        for (int k = 0; k < roll; ++k)
                        {
                            const double from = at + (double) k * stepTicks / roll;
                            add (gmNoteForRow (def, r), velocity, from, from + stepTicks / (2.0 * roll));
                        }
                    }
                    continue;
                }
                if (! track.on[st]) continue;

                // How many steps the note lasts: held (slide / hold) steps with the same note go on.
                auto lengthFrom = [&] (int startStep)
                {
                    int length = 1;
                    while (length < own && track.slide[(size_t) ((startStep + length - 1) % own)]
                           && track.on[(size_t) ((startStep + length) % own)]
                           && track.note[(size_t) ((startStep + length) % own)].load() == track.note[(size_t) startStep].load())
                        ++length;
                    return length;
                };
                const bool continued = notFirst && track.on[prev] && track.slide[prev] && track.note[prev].load() == track.note[st].load()
                                    && track.chord[st].load() == 0;
                const std::uint64_t mask = track.chord[st].load();
                if (mask != 0)
                {
                    // Notes written in the piano roll: each starts where it is not tied from the step before.
                    for (int n = 0; n < noteRange; ++n)
                    {
                        if (! ((mask >> n) & 1u)) continue;
                        const bool tiedIn = notFirst && track.on[prev] && ((track.tie[prev].load() >> n) & 1u);
                        if (tiedIn) continue;
                        int length = 1;
                        while (s + length < own && ((track.tie[(size_t) (s + length - 1)].load() >> n) & 1u)) ++length;
                        add (base + snapToScale (currentScale(), n), velocity, at, endOf (length) - stepTicks * 0.5);
                    }
                    continue;
                }
                if (continued) continue;
                const int length = lengthFrom (s);
                const bool glidesOut = track.slide[(size_t) ((s + length - 1) % own)];
                const double end = endOf (length) - (glidesOut ? -20.0 : stepTicks * 0.5);   // a glide overlaps the next note
                const int root = snapToScale (currentScale(), track.note[st].load());
                if (def.kind == MachineKind::Poly)
                {
                    int intervals[5];
                    const int chordSize = chordIntervals (currentScale(), root, chordType, intervals);
                    for (int k = 0; k < chordSize; ++k) add (base + root + intervals[k], velocity, at, end);
                }
                else
                {
                    add (base + root, velocity, at, end);
                }
            }
        }
        seq.updateMatchedPairs();
        midi.addTrack (seq);
    }

    target.deleteFile();
    juce::FileOutputStream out (target);
    return out.openedOk() && midi.writeTo (out);
}

bool TechMachineProcessor::exportStems (const juce::XmlElement& project, const juce::File& folder, int loops,
                                        const std::function<bool (float)>& progress)
{
    if (! folder.createDirectory()) return false;
    std::vector<int> shown;
    for (int t : rackOrder)
        if (paramInXml (project, pid::machine (t, "show")) > 0.5f) shown.push_back (t);

    const auto* patterns = project.getChildByName ("Patterns");
    const auto prefix = juce::File::createLegalFileName (patterns != nullptr ? patterns->getStringAttribute ("name") : juce::String());
    for (size_t i = 0; i < shown.size(); ++i)
    {
        // The same project with only this machine playing (its effects and the shared echo / reverb included).
        juce::XmlElement solo (project);
        for (int t = 0; t < numMachines; ++t) setParamInXml (solo, pid::machine (t, "solo"), 0.0f);
        setParamInXml (solo, pid::machine (shown[i], "solo"), 1.0f);
        setParamInXml (solo, pid::machine (shown[i], "mute"), 0.0f);
        const auto file = folder.getChildFile (prefix + " - " + juce::String ((int) i + 1).paddedLeft ('0', 2) + " "
                                               + juce::File::createLegalFileName (utf8 (machineDef (shown[i]).name)) + ".wav");
        const bool ok = exportWav (solo, file, loops, [&] (float p)
        {
            return ! progress || progress (((float) i + p) / (float) shown.size());
        });
        if (! ok) return false;
    }
    return ! shown.empty();
}
