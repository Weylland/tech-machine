#pragma once

#include "Machines.h"
#include "../dsp/Effects.h"

// The two ways a synth plays its steps: one voice that glides (mono), or several voices for chords and free notes (poly).
// The instruments (Instruments.cpp) are poly machines with their own voices and settings.
namespace techno
{
class SynthMachine : public Machine
{
public:
    using Machine::Machine;

    void bind (juce::AudioProcessorValueTreeState& state) override
    {
        for (size_t i = 0; i < def.params.size(); ++i)
            params.push_back (state.getRawParameterValue (pid::synth (type, (int) i)));
        values.resize (def.params.size());
    }

    void readParams() override
    {
        for (size_t i = 0; i < params.size(); ++i)
            values[i] = setting (automation::synthTarget ((int) i), params[i]->load());
        octave = (int) values[0];
        applyParams();
    }

protected:
    virtual void applyParams() = 0;

    int gateSamples (const StepInfo& s) const
    {
        const auto st = (size_t) s.step;
        return track.slide[st] ? (int) (s.stepSamples + s.nominalStepSamples * 0.5)
                               : std::max (1, (int) (s.stepSamples * gateFraction));
    }

    // Each sound of a selector at the same loudness: one gain per choice, applied to the machine's output.
    template <size_t N>
    void trimFor (const float (&gains)[N], float choice) { outputTrim = gains[(size_t) juce::jlimit (0, (int) N - 1, (int) choice)]; }

    std::vector<std::atomic<float>*> params;
    std::vector<float> values;
    int octave = 2, gateCountdown = 0;
    float gateFraction = 0.5f, outputTrim = 1.0f;
};

class MonoMachine : public SynthMachine
{
public:
    MonoMachine (int machineType, std::unique_ptr<MonoVoice> v, float gateLength = 0.5f, int typeParam = -1, std::vector<float> typeGains = {})
        : SynthMachine (machineType), voice (std::move (v)), typeIndex (typeParam), gains (std::move (typeGains))
    {
        gateFraction = gateLength;
    }

    void prepare (float sampleRate) override { voice->prepare (sampleRate * 2.0f); }

    void onStep (const StepInfo& s) override
    {
        voice->syncStep (s.step, s.nominalStepSamples);
        const auto st = (size_t) s.step, prev = (size_t) s.prevStep;
        if (! track.on[st]) return;

        const bool glide = track.on[prev] && track.slide[prev];
        const int semitone = snapToScale (s.scale, track.note[st]);
        voice->noteOn (machineBaseNote (def, s.key, octave) + semitone, track.accent[st], glide);
        gateCountdown = gateSamples (s);
    }

    void tick() override
    {
        if (gateCountdown > 0 && --gateCountdown == 0) voice->gateOff();
    }

    // The voice runs at twice the sample rate to keep the resonant filter and oscillators clean.
    void render (Frame& f) override
    {
        const float a = voice->process();
        const float b = voice->process();
        f.left = f.right = decimator.process (a, b) * outputTrim;
    }

    void stop() override
    {
        voice->gateOff();
        gateCountdown = 0;
        heldCount = 0;
    }

    // Mono: the last key held plays; releasing it returns to the previous one with a glide (legato).
    void liveNoteOn (int note, float velocity) override
    {
        const bool legato = heldCount > 0;
        if (heldCount < (int) std::size (held)) held[heldCount++] = note;
        voice->noteOn (note, velocity > 0.85f, legato);
        gateCountdown = 0;
    }

    void liveNoteOff (int note) override
    {
        int k = 0;
        for (int i = 0; i < heldCount; ++i)
            if (held[i] != note) held[k++] = held[i];
        heldCount = k;
        if (heldCount > 0) voice->noteOn (held[heldCount - 1], false, true);
        else voice->gateOff();
    }

private:
    void applyParams() override
    {
        voice->setParams (values.data());
        if (typeIndex >= 0 && ! gains.empty())
            outputTrim = gains[(size_t) juce::jlimit (0, (int) gains.size() - 1, (int) values[(size_t) typeIndex])];
    }

    std::unique_ptr<MonoVoice> voice;
    int typeIndex;
    std::vector<float> gains;
    Decimator2x decimator;
    int held[16] {}, heldCount = 0;
};

class PolyMachine : public SynthMachine
{
public:
    using SynthMachine::SynthMachine;

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        for (auto& v : voices) v->prepare (sampleRate * 2.0f);
        previousChord = {};
    }

    void onStep (const StepInfo& s) override
    {
        const auto st = (size_t) s.step, prev = (size_t) s.prevStep;

        // Notes written in the piano roll: each one starts, goes on (tie) and stops on its own.
        const std::uint64_t mask = track.on[st] ? track.chord[st].load() : 0;
        if (mask != 0 || written != 0)
        {
            const std::uint64_t continuing = written & mask & (track.on[prev] ? track.tie[prev].load() : 0);
            releaseNotes (written & ~continuing);
            written = 0;
            if (mask != 0)
            {
                const int base = machineBaseNote (def, s.key, octave);
                int count = 0;
                for (int n = 0; n < noteRange; ++n) count += (int) ((mask >> n) & 1u);
                const float velocity = (track.accent[st] ? 1.0f : 0.8f) * 1.6f / std::sqrt ((float) juce::jmax (1, count));
                for (int n = 0; n < noteRange; ++n)
                {
                    if (! ((mask >> n) & 1u) || ((continuing >> n) & 1u)) continue;
                    writtenNote[n] = base + snapToScale (s.scale, n);
                    startVoice (writtenNote[n], velocity);
                }
                written = mask;
                endingAtGate = mask & ~track.tie[st].load();
                gateOpen = false;
                gateCountdown = juce::jmax (1, (int) (s.stepSamples * gateFraction));
                return;
            }
        }

        const bool held = gateOpen && track.on[prev] && track.slide[prev] && track.on[st]
                       && track.note[prev].load() == track.note[st].load() && track.chord[prev].load() == track.chord[st].load();
        if (held)
        {
            gateCountdown = gateSamples (s);
            return;
        }
        if (! track.on[st]) return;

        releaseAll();
        Chord chord;
        const int root = snapToScale (s.scale, track.note[st]);
        int intervals[5];
        chord.count = chordIntervals (s.scale, root, (int) values[1], intervals);
        const int base = machineBaseNote (def, s.key, octave) + root;
        for (int k = 0; k < chord.count; ++k) chord.notes[k] = base + intervals[k];
        if (chord.count >= 3) chord = closestVoicing (chord);
        const int count = chord.count;
        previousChord = chord;

        // Same loudness whether one note or a five-note chord is played.
        const float velocity = (track.accent[st] ? 1.0f : 0.8f) * 1.6f / std::sqrt ((float) count);
        for (int k = 0; k < chord.count; ++k) startVoice (chord.notes[k], velocity);

        gateOpen = true;
        gateCountdown = gateSamples (s);
    }

    void tick() override
    {
        if (gateCountdown > 0 && --gateCountdown == 0)
        {
            if (written != 0)
            {
                releaseNotes (endingAtGate);
                written &= ~endingAtGate;
                return;
            }
            releaseAll();
            gateOpen = false;
        }
    }

    void stop() override
    {
        releaseAll();
        gateOpen = false;
        gateCountdown = 0;
        written = 0;
    }

    void render (Frame& f) override
    {
        float l0 = 0, r0 = 0, l1 = 0, r1 = 0;
        for (auto& v : voices)
        {
            if (! v->isActive()) continue;
            v->process (l0, r0);
            v->process (l1, r1);
        }
        f.left = decimatorL.process (l0, l1);
        f.right = decimatorR.process (r0, r1);
        postProcess (f.left, f.right);
        f.left *= outputTrim;
        f.right *= outputTrim;
    }

    void liveNoteOn (int note, float velocity) override { startVoice (note, 0.5f + velocity * 0.7f); }

    void liveNoteOff (int note) override
    {
        for (auto& v : voices)
            if (v->isActive() && v->note == note) v->release();
    }

protected:
    virtual void postProcess (float&, float&) {}

    std::vector<std::unique_ptr<PolyVoice>> voices;
    float fs = 44100.0f;

private:
    struct Chord
    {
        static constexpr int maxNotes = 8;
        int notes[maxNotes] {};
        int count = 0;

        float mean() const
        {
            float m = 0;
            for (int i = 0; i < count; ++i) m += (float) notes[i];
            return count > 0 ? m / (float) count : 0.0f;
        }
    };

    // Picks the inversion of the chord that moves the least from the previous one (smooth voice leading),
    // without drifting away from the register of the root position. No allocation: runs on the audio thread.
    Chord closestVoicing (const Chord& rootPosition) const
    {
        if (previousChord.count == 0) return rootPosition;

        const float centre = rootPosition.mean();
        Chord best = rootPosition;
        float bestCost = 1.0e9f;

        for (int shift : { -12, 0, 12 })
        {
            Chord candidate = rootPosition;
            for (int i = 0; i < candidate.count; ++i) candidate.notes[i] += shift;
            for (int rotation = 0; rotation < candidate.count; ++rotation)
            {
                std::sort (candidate.notes, candidate.notes + candidate.count);
                float cost = std::abs (candidate.mean() - centre) * 0.6f;
                for (int i = 0; i < candidate.count; ++i)
                    cost += (float) std::abs (candidate.notes[i] - previousChord.notes[juce::jmin (i, previousChord.count - 1)]);
                if (cost < bestCost)
                {
                    bestCost = cost;
                    best = candidate;
                }
                candidate.notes[0] += 12;
            }
        }
        return best;
    }

    void startVoice (int note, float velocity)
    {
        PolyVoice* chosen = nullptr;
        for (auto& v : voices)
            if (! v->isActive()) { chosen = v.get(); break; }
        if (chosen == nullptr)
            for (auto& v : voices)
                if (chosen == nullptr || v->age < chosen->age) chosen = v.get();

        chosen->age = ++ageCounter;
        chosen->start (note, velocity);
    }

    void releaseAll()
    {
        for (auto& v : voices)
            if (v->isActive()) v->release();
    }

    void releaseNotes (std::uint64_t bits)
    {
        for (int n = 0; n < noteRange; ++n)
            if ((bits >> n) & 1u)
                for (auto& v : voices)
                    if (v->isActive() && v->note == writtenNote[n]) v->release();
    }

    std::uint64_t written = 0, endingAtGate = 0;   // piano roll notes sounding, and those that stop at the end of this step
    int writtenNote[noteRange] {};

    bool gateOpen = false;
    unsigned int ageCounter = 0;
    Chord previousChord;
    Decimator2x decimatorL, decimatorR;
};
} // namespace techno
