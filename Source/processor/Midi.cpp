#include "../PluginProcessor.h"

using namespace techno;

// MIDI keyboard and pads: what they play, and the recording of what is played into the steps.

int TechMachineProcessor::midiTargetMachine() const
{
    const int t = midiTarget.load();
    if (t >= 0 && t < numMachines && isShown (t)) return t;
    for (int candidate : rackOrder)
        if (isShown (candidate)) return candidate;
    return -1;
}

void TechMachineProcessor::previewNote (int type, int midiNote, bool on)
{
    int start1, size1, start2, size2;
    previewFifo.prepareToWrite (1, start1, size1, start2, size2);
    if (size1 > 0) previewQueue[(size_t) start1] = { type, midiNote, on };
    else if (size2 > 0) previewQueue[(size_t) start2] = { type, midiNote, on };
    previewFifo.finishedWrite (size1 + size2);
}

void TechMachineProcessor::setRecordNotes (bool on)
{
    if (on && ! recordNotes)
    {
        undoManager.beginNewTransaction();
        beginPatternEdit();
    }
    for (auto& s : recordStart) s = -1;
    recordNotes = on;
}

int TechMachineProcessor::quantizedStep() const
{
    if (stepIndex < 0) return 0;
    const double nominal = sampleRate * stepSeconds();
    return samplesToNextStep < nominal * 0.5 ? (stepIndex + 1) % lengthSteps : stepIndex;
}

void TechMachineProcessor::handleMidi (const juce::MidiBuffer& midi)
{
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.isController())
        {
            const bool assigned = midiLearn.handleController (m.getControllerNumber(), m.getControllerValue());
            if (! assigned && m.getControllerNumber() == 1)   // modulation wheel: vibrato
                if (const int t = midiTargetMachine(); t >= 0) machines[(size_t) t]->setModWheel ((float) m.getControllerValue() / 127.0f);
            continue;
        }
        if (m.isPitchWheel())
        {
            if (const int t = midiTargetMachine(); t >= 0)
                machines[(size_t) t]->setPitchBend ((float) (m.getPitchWheelValue() - 8192) / 8192.0f * 2.0f);
            continue;
        }
        if (! m.isNoteOnOrOff()) continue;

        const int t = midiTargetMachine();
        if (t < 0) continue;
        const int note = m.getNoteNumber();
        ++midiActivity;
        if (m.isNoteOn())
        {
            machines[(size_t) t]->liveNoteOn (note, m.getFloatVelocity());
            if (recordNotes && wasPlaying) recordNoteOn (t, note, m.getFloatVelocity());
        }
        else
        {
            machines[(size_t) t]->liveNoteOff (note);
            if (recordNotes && wasPlaying) recordNoteOff (t, note);
        }
    }
}

// The step note (0 = the key's root at the synth's octave) of a key pressed on the keyboard, folded into the range.
int TechMachineProcessor::recordedSemitone (int type, int note) const
{
    const auto& def = machineDef (type);
    const int octave = def.params.empty() ? 2 : (int) apvts.getRawParameterValue (pid::synth (type, 0))->load();
    int semitone = note - machineBaseNote (def, currentKey(), octave);
    while (semitone < 0) semitone += 12;
    while (semitone >= noteRange) semitone -= 12;
    return semitone;
}

void TechMachineProcessor::recordNoteOn (int type, int note, float velocity)
{
    const auto& def = machineDef (type);
    auto& track = machines[(size_t) type]->track;
    const int step = quantizedStep();
    const auto s = (size_t) step;
    if (const int lane = track.laneFor (automation::velocityTarget (def), true); lane >= 0)
        track.lockValue[(size_t) lane][s] = juce::jlimit (0.05f, 1.0f, velocity * 0.9375f);   // velocity 0.8 (about 100) = normal
    if (def.kind == MachineKind::Drum)
    {
        const int row = machines[(size_t) type]->rowForMidiNote (note);
        if (row < 0) return;
        track.hits[(size_t) row][s] = true;
        if (velocity > 0.85f) track.accent[s] = true;
        touchPattern();
        return;
    }

    const int semitone = recordedSemitone (type, note);
    if (def.kind == MachineKind::Poly)
    {
        // Every key is kept: a chord played on the keyboard is written note by note, as in the piano roll.
        const std::uint64_t bit = 1ull << semitone;
        const bool fresh = ! track.on[s] || track.chord[s].load() == 0;
        track.chord[s] = (fresh ? 0 : track.chord[s].load()) | bit;
        track.tie[s] = fresh ? 0 : track.tie[s].load() & ~bit;
        const auto prev = (size_t) ((step + lengthSteps - 1) % lengthSteps);
        track.tie[prev] = track.tie[prev].load() & ~bit;
        if (fresh) track.note[s] = semitone;
    }
    else
    {
        track.note[s] = semitone;
        track.slide[s] = false;
    }
    track.on[s] = true;
    track.accent[s] = velocity > 0.85f;
    recordStart[note & 127] = step;
    touchPattern();
}

void TechMachineProcessor::recordNoteOff (int type, int note)
{
    const int start = recordStart[note & 127];
    recordStart[note & 127] = -1;
    const auto& def = machineDef (type);
    if (start < 0 || def.kind == MachineKind::Drum) return;

    // Held keys become held steps: tied notes on the poly synths, a glide on the mono ones.
    auto& track = machines[(size_t) type]->track;
    const int length = (quantizedStep() - start + lengthSteps) % lengthSteps;
    const std::uint64_t bit = 1ull << recordedSemitone (type, note);
    for (int k = 1; k < length; ++k)
    {
        const auto s = (size_t) ((start + k) % lengthSteps), prev = (size_t) ((start + k - 1) % lengthSteps);
        if (def.kind == MachineKind::Poly)
        {
            const bool fresh = ! track.on[s] || track.chord[s].load() == 0;
            track.chord[s] = (fresh ? 0 : track.chord[s].load()) | bit;
            if (fresh)
            {
                track.tie[s] = 0;
                track.note[s] = track.note[(size_t) start].load();
                track.accent[s] = false;
            }
            track.tie[prev] = track.tie[prev].load() | bit;
        }
        else
        {
            track.note[s] = track.note[(size_t) start].load();
            track.slide[prev] = true;
        }
        track.on[s] = true;
    }
    touchPattern();
}
