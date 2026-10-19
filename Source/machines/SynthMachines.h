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

    // Keyboard expression, on the machine that receives the MIDI keyboard (audio thread).
    void setPitchBend (float semitones) override { bend = semitones; }
    void setModWheel (float amount) override { modulation = amount; }

protected:
    virtual void applyParams() = 0;

    // Pitch wheel plus the vibrato of the modulation wheel, in semitones. False while there is none to apply.
    bool expression (float& semitones)
    {
        const bool active = ! juce::exactlyEqual (bend, 0.0f) || ! juce::exactlyEqual (modulation, 0.0f);
        if (! active && ! expressionApplied) return false;
        expressionApplied = active;
        advance (vibratoPhase, 5.5f / expressionRate);
        semitones = bend + modulation * 0.6f * std::sin (2.0f * pi * vibratoPhase);
        return true;
    }

    float bend = 0.0f, modulation = 0.0f, vibratoPhase = 0.0f, expressionRate = 44100.0f;
    bool expressionApplied = false;

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

    void prepare (float sampleRate) override
    {
        voice->prepare (sampleRate * 2.0f);
        expressionRate = sampleRate;
    }

    void onStep (const StepInfo& s) override
    {
        voice->syncStep (s.step, s.nominalStepSamples);
        const auto st = (size_t) s.step, prev = (size_t) s.prevStep;
        if (! track.on[st] || silentStep) return;

        const bool glide = track.on[prev] && track.slide[prev];
        if (! glide) noteGain = velocityScale();
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
        if (float semitones; expression (semitones)) voice->setBend (semitones);
        const float a = voice->process();
        const float b = voice->process();
        f.left = f.right = decimator.process (a, b) * outputTrim * noteGain;
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
    float noteGain = 1.0f;   // velocity of the note playing
    int typeIndex;
    std::vector<float> gains;
    Decimator2x decimator;
    int held[16] {}, heldCount = 0;
};

class PolyMachine : public SynthMachine
{
public:
    using SynthMachine::SynthMachine;

    void bind (juce::AudioProcessorValueTreeState& state) override
    {
        SynthMachine::bind (state);
        if (! hasArpeggiator (def)) return;
        arpModeParam = state.getRawParameterValue (pid::machine (type, "arp"));
        arpRateParam = state.getRawParameterValue (pid::machine (type, "arprate"));
        arpOctavesParam = state.getRawParameterValue (pid::machine (type, "arpoctaves"));
    }

    void readParams() override
    {
        SynthMachine::readParams();
        if (arpModeParam == nullptr) return;
        const int mode = (int) arpModeParam->load();
        if (mode != arpeggio.mode)
        {
            releaseAll();
            arpeggio.clear();
        }
        arpeggio.mode = mode;
        arpeggio.rate = (int) arpRateParam->load();
        arpeggio.octaves = (int) arpOctavesParam->load();
    }

    void prepare (float sampleRate) override
    {
        fs = sampleRate;
        expressionRate = sampleRate;
        for (auto& v : voices) v->prepare (sampleRate * 2.0f);
        previousChord = {};
    }

    void onStep (const StepInfo& s) override
    {
        const auto st = (size_t) s.step, prev = (size_t) s.prevStep;

        // Notes written in the piano roll: each one starts, goes on (tie) and stops on its own.
        const std::uint64_t mask = track.on[st] && ! silentStep ? track.chord[st].load() : 0;
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
                const float velocity = (track.accent[st] ? 1.0f : 0.8f) * 1.6f / std::sqrt ((float) juce::jmax (1, count)) * velocityScale();
                for (int n = 0; n < noteRange; ++n)
                {
                    if (! ((mask >> n) & 1u) || ((continuing >> n) & 1u)) continue;
                    writtenNote[n] = base + snapToScale (s.scale, n);
                    play (writtenNote[n], velocity);
                }
                written = mask;
                endingAtGate = mask & ~track.tie[st].load();
                gateOpen = false;
                gateCountdown = juce::jmax (1, (int) (s.stepSamples * gateFraction));
                return;
            }
        }

        const bool held = gateOpen && ! silentStep && track.on[prev] && track.slide[prev] && track.on[st]
                       && track.note[prev].load() == track.note[st].load() && track.chord[prev].load() == track.chord[st].load();
        if (held)
        {
            gateCountdown = gateSamples (s);
            return;
        }
        if (! track.on[st] || silentStep) return;

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
        const float velocity = (track.accent[st] ? 1.0f : 0.8f) * 1.6f / std::sqrt ((float) count) * velocityScale();
        for (int k = 0; k < chord.count; ++k) play (chord.notes[k], velocity);

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
        applyExpression();
        if (arpeggio.mode > 0) runArpeggio();
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

    void liveNoteOn (int note, float velocity) override
    {
        if (arpeggio.mode > 0)
        {
            if (arpeggio.held.count == 0) arpeggio.restart = true;
            arpeggio.held.add (note, 0.5f + velocity * 0.7f);
            return;
        }
        startVoice (note, 0.5f + velocity * 0.7f);
    }

    void liveNoteOff (int note) override
    {
        arpeggio.held.remove (note);
        releaseNote (note);
    }

protected:
    virtual void postProcess (float&, float&) {}

    void applyExpression()
    {
        if (float semitones; expression (semitones))
        {
            bendRatio = std::exp2 (semitones / 12.0f);
            for (auto& v : voices) v->setBend (bendRatio);
        }
    }

    float bendRatio = 1.0f;

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
        if (! juce::exactlyEqual (bendRatio, 1.0f)) chosen->setBend (bendRatio);
    }

    void releaseAll()
    {
        for (auto& v : voices)
            if (v->isActive()) v->release();
        arpeggio.sequenced.clear();
        arpeggio.restart = true;   // the next chord starts its arpeggio on the beat
    }

    void releaseNotes (std::uint64_t bits)
    {
        for (int n = 0; n < noteRange; ++n)
            if ((bits >> n) & 1u)
            {
                releaseNote (writtenNote[n]);
                arpeggio.sequenced.remove (writtenNote[n]);
            }
    }

    void releaseNote (int note)
    {
        for (auto& v : voices)
            if (v->isActive() && v->note == note) v->release();
    }

    // A note of a step: played at once, or given to the arpeggiator.
    void play (int note, float velocity)
    {
        if (arpeggio.mode == 0) startVoice (note, velocity);
        else arpeggio.sequenced.add (note, velocity);
    }

    // Arpeggiator: plays the notes held (keyboard first, else the chord of the step) one after the other.
    struct Arpeggio
    {
        struct Notes
        {
            static constexpr int capacity = 24;
            int notes[capacity] {};
            float velocities[capacity] {};
            int count = 0;

            void clear() { count = 0; }

            void add (int note, float velocity)
            {
                for (int i = 0; i < count; ++i)
                    if (notes[i] == note) return;
                if (count == capacity) return;
                notes[count] = note;
                velocities[count++] = velocity;
            }

            void remove (int note)
            {
                int k = 0;
                for (int i = 0; i < count; ++i)
                    if (notes[i] != note)
                    {
                        notes[k] = notes[i];
                        velocities[k++] = velocities[i];
                    }
                count = k;
            }
        };

        void clear()
        {
            sequenced.clear();
            held.clear();
            sounding = -1;
            restart = true;
        }

        Notes sequenced, held;
        int mode = 0, rate = 1, octaves = 0, position = 0, sounding = -1;
        double countdown = 0.0, gate = 0.0;
        bool restart = true;
        juce::Random random { 0xa4be };
    };

    void runArpeggio()
    {
        auto& a = arpeggio;
        const auto& pool = a.held.count > 0 ? a.held : a.sequenced;
        if (a.gate > 0.0 && (a.gate -= 1.0) <= 0.0 && a.sounding >= 0)
        {
            releaseNote (a.sounding);
            a.sounding = -1;
        }
        if (pool.count == 0)
        {
            a.restart = true;
            return;
        }
        if (a.restart)
        {
            a.restart = false;
            a.countdown = 0.0;
            a.position = 0;
        }
        if ((a.countdown -= 1.0) > 0.0) return;

        static constexpr double rateSteps[] { 2.0, 1.0, 2.0 / 3.0, 0.5 };
        const double interval = juce::jmax (8.0, tempoStepSamples * rateSteps[juce::jlimit (0, 3, a.rate)]);
        a.countdown += interval;
        if (a.sounding >= 0) releaseNote (a.sounding);

        // The notes in order (low to high, or as played), repeated an octave higher for each octave.
        int order[Arpeggio::Notes::capacity];
        for (int i = 0; i < pool.count; ++i) order[i] = i;
        if (a.mode != 5)
            std::sort (order, order + pool.count, [&pool] (int x, int y) { return pool.notes[x] < pool.notes[y]; });
        const int span = pool.count * (juce::jlimit (0, 2, a.octaves) + 1);
        int index = 0;
        switch (a.mode)
        {
            case 2: index = span - 1 - a.position % span; break;
            case 3:
            {
                const int period = juce::jmax (1, 2 * span - 2);
                const int p = a.position % period;
                index = span == 1 ? 0 : (p < span ? p : period - p);
                break;
            }
            case 4: index = a.random.nextInt (span); break;
            default: index = a.position % span; break;
        }
        ++a.position;
        const int which = order[index % pool.count];
        const int note = pool.notes[which] + 12 * (index / pool.count);
        startVoice (note, pool.velocities[which]);
        a.sounding = note;
        a.gate = interval * 0.6;
    }

    std::atomic<float>* arpModeParam = nullptr, *arpRateParam = nullptr, *arpOctavesParam = nullptr;
    Arpeggio arpeggio;

    std::uint64_t written = 0, endingAtGate = 0;   // piano roll notes sounding, and those that stop at the end of this step
    int writtenNote[noteRange] {};

    bool gateOpen = false;
    unsigned int ageCounter = 0;
    Chord previousChord;
    Decimator2x decimatorL, decimatorR;
};
} // namespace techno
