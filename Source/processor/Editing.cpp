#include "../PluginProcessor.h"

#include <cstring>

using namespace techno;

//==============================================================================
// Undo of note edits: a snapshot of every part, taken just before the change.

class TechMachineProcessor::PatternAction : public juce::UndoableAction
{
public:
    explicit PatternAction (TechMachineProcessor& p, int part) : processor (p), before (p.snapshotOf (part)) {}

    bool perform() override
    {
        if (! performed)
        {
            performed = true;
            return true;
        }
        processor.restore (after);
        return true;
    }

    bool undo() override
    {
        after = processor.snapshotOf (before.part);
        processor.restore (before);
        return true;
    }

    int getSizeInUnits() override { return 100; }

private:
    TechMachineProcessor& processor;
    Snapshot before, after;
    bool performed = false;
};

TechMachineProcessor::Snapshot TechMachineProcessor::snapshot() const { return snapshotOf (activePart.load()); }

TechMachineProcessor::Snapshot TechMachineProcessor::snapshotOf (int part) const
{
    Snapshot s;
    s.part = part;
    s.tracks.resize (machines.size());
    for (int t = 0; t < numMachines; ++t) s.tracks[(size_t) t] = partTrack (part, t).snapshot();
    return s;
}

// Undo works in the part where the change was made, even after switching to another one.
void TechMachineProcessor::restore (const Snapshot& s)
{
    for (int t = 0; t < numMachines; ++t) partTrack (s.part, t).restore (s.tracks[(size_t) t]);
    touchPattern();
}

void TechMachineProcessor::beginPatternEdit() { beginPartEdit (activePart.load()); }

void TechMachineProcessor::beginPartEdit (int part)
{
    undoManager.perform (new PatternAction (*this, part));
    dirty = true;
}

//==============================================================================
// Generation

GenerateContext TechMachineProcessor::context() const
{
    GenerateContext ctx { styles()[(size_t) genreIndex], numSteps(), currentScale(), progression, {} };

    // Where the kick plays: taken from the drum machines in the rack, otherwise four on the floor.
    bool anyKick = false;
    for (int t = 0; t < numMachines; ++t)
    {
        const auto& def = machineDef (t);
        if (! isShown (t) || def.kind != MachineKind::Drum) continue;
        for (size_t r = 0; r < def.rows.size(); ++r)
        {
            if (def.rows[r].role != Role::Kick) continue;
            for (int s = 0; s < ctx.steps; ++s)
            {
                if (machines[(size_t) t]->track.hits[r][(size_t) s])
                {
                    ctx.kick[(size_t) s] = true;
                    anyKick = true;
                }
            }
        }
    }
    if (! anyKick)
        for (int s = 0; s < ctx.steps; s += 4) ctx.kick[(size_t) s] = true;
    return ctx;
}

void TechMachineProcessor::chooseProgression()
{
    const auto& options = styles()[(size_t) genreIndex].progressions;
    progression = options[(size_t) random.nextInt ((int) options.size())];
}

void TechMachineProcessor::setGenre (int index)
{
    genreIndex = juce::jlimit (0, (int) styles().size() - 1, index);
    chooseProgression();

    // On an empty project, the genre also brings its usual tempo, swing and scale.
    if (isProjectEmpty())
    {
        const auto& style = styles()[(size_t) genreIndex];
        setParam (pid::bpm, style.bpm);
        setParam (pid::swing, style.swing);
        setParam (pid::scale, (float) style.scale);
    }
    dirty = true;
}

void TechMachineProcessor::generate (int type, GenerateMode mode)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    auto& track = machines[(size_t) type]->track;
    const auto kept = track.snapshot();   // new notes, same automation
    techno::generate (track, machineDef (type), context(), mode, random);
    track.restoreLocks (kept);
    touchPattern();
}

void TechMachineProcessor::generateAll()
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    chooseProgression();

    // Empty rack: set up the usual band of the genre, with its tempo, swing and scale, over 4 bars.
    bool anyShown = false;
    for (int t = 0; t < numMachines; ++t) anyShown = anyShown || isShown (t);
    if (! anyShown)
    {
        const auto& style = styles()[(size_t) genreIndex];
        setParam (pid::bpm, style.bpm);
        setParam (pid::swing, style.swing);
        setParam (pid::scale, (float) style.scale);
        setBars (4);
        for (auto& member : defaultBand (style))
        {
            setShown (member.machine, true);
            if (member.preset == nullptr) continue;
            for (auto& preset : presetsFor (member.machine))
                if (std::strcmp (preset.name, member.preset) == 0) applyPreset (member.machine, preset);
        }
    }

    // Drums first, so the bass lines know where the kicks are.
    for (int pass = 0; pass < 2; ++pass)
    {
        for (int t = 0; t < numMachines; ++t)
        {
            const bool drum = machineDef (t).kind == MachineKind::Drum;
            if (isShown (t) && drum == (pass == 0))
            {
                auto& track = machines[(size_t) t]->track;
                const auto kept = track.snapshot();
                techno::generate (track, machineDef (t), context(), GenerateMode::NewIdea, random);
                track.restoreLocks (kept);
            }
        }
    }
    touchPattern();
}

void TechMachineProcessor::clearMachine (int type)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    machines[(size_t) type]->track.clear();
    touchPattern();
}

void TechMachineProcessor::applyPattern (int type, const Pattern& pattern)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    auto& track = machines[(size_t) type]->track;
    const auto kept = track.snapshot();
    techno::applyPattern (track, machineDef (type), context(), pattern);
    track.restoreLocks (kept);
    touchPattern();
}

//==============================================================================
// Sounds and editing tools

juce::RangedAudioParameter* TechMachineProcessor::presetParameter (int type, const char* key)
{
    juce::String k (key);
    if (k.startsWithChar ('@')) return apvts.getParameter (pid::machine (type, k.substring (1).toRawUTF8()));
    return apvts.getParameter (juce::String (machineDef (type).id) + "_" + k.replaceCharacter ('.', '_'));
}

void TechMachineProcessor::applyPreset (int type, const SoundPreset& preset)
{
    // Back to the factory sound first, so a preset only has to list what it changes.
    resetSound (type);

    for (auto& [key, value] : preset.values)
        if (auto* p = presetParameter (type, key))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
    dirty = true;
}

void TechMachineProcessor::shiftMachine (int type, int direction)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    shiftTrack (machines[(size_t) type]->track, numSteps(), direction);
    touchPattern();
}

void TechMachineProcessor::duplicateFirstBar (int type)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    machines[(size_t) type]->track.tile (pageSteps);
    touchPattern();
}

void TechMachineProcessor::copyMachine (int type)
{
    clipboard = machines[(size_t) type]->track.snapshot();
    clipboardKind = type;
}

bool TechMachineProcessor::canPaste (int type) const
{
    return clipboard.has_value() && clipboardKind == type;
}

void TechMachineProcessor::pasteMachine (int type)
{
    if (! canPaste (type)) return;
    undoManager.beginNewTransaction();
    beginPatternEdit();
    machines[(size_t) type]->track.restore (*clipboard);
    touchPattern();
}

bool TechMachineProcessor::isProjectEmpty() const
{
    for (int part = 0; part < numParts; ++part)
        if (! isPartEmpty (part)) return false;
    return true;
}

void TechMachineProcessor::setBars (int newBars)
{
    const int oldSteps = numSteps();
    const int newSteps = newBars * pageSteps;
    if (newSteps > oldSteps)
        for (auto& m : machines) m->track.tile (oldSteps);
    setParam (pid::bars, (float) (newBars - 1));
    touchPattern();
}

//==============================================================================
// Automation

void TechMachineProcessor::setRecordAutomation (bool on)
{
    if (on && ! recordAutomation)
    {
        undoManager.beginNewTransaction();
        beginPatternEdit();
    }
    recordAutomation = on;
}

void TechMachineProcessor::recordKnob (const juce::String& paramId, float value)
{
    if (! recordAutomation || ! playing) return;
    const auto it = automationIndex.find (paramId);
    if (it == automationIndex.end()) return;
    const int step = machines[(size_t) it->second.first]->playStep.load();
    if (step < 0) return;
    auto& track = machines[(size_t) it->second.first]->track;
    const int lane = track.laneFor (it->second.second, true);
    if (lane < 0) return;   // already 8 automated settings on this machine
    track.lockValue[(size_t) lane][(size_t) step] = value;
    touchPattern();
}

void TechMachineProcessor::setTrackTiming (int type, int length, int speed)
{
    undoManager.beginNewTransaction();
    beginPatternEdit();
    auto& track = machines[(size_t) type]->track;
    track.length = juce::jlimit (0, maxSteps, length);
    track.speed = juce::jlimit (0, numTrackSpeeds - 1, speed);
    touchPattern();
}

void TechMachineProcessor::setAutomation (int type, int target, int step, float value)
{
    auto& track = machines[(size_t) type]->track;
    const int lane = track.laneFor (target, value >= 0.0f);
    if (lane < 0) return;
    track.lockValue[(size_t) lane][(size_t) step] = value;
    if (value < 0.0f) track.releaseEmptyLanes();
    touchPattern();
}

void TechMachineProcessor::clearAutomation (int type, int target)
{
    auto& track = machines[(size_t) type]->track;
    if (const int lane = track.laneFor (target, false); lane >= 0)
    {
        for (auto& v : track.lockValue[(size_t) lane]) v = noLock;
        track.lockTarget[(size_t) lane] = -1;
    }
    touchPattern();
}
