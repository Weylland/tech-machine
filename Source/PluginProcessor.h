#pragma once

#include "core/Examples.h"
#include "core/MidiLearn.h"
#include "core/PatternLibrary.h"
#include "core/Presets.h"
#include "core/Styles.h"
#include "dsp/Effects.h"
#include "dsp/Reverb.h"
#include "machines/Machines.h"
#include "machines/SampleBank.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <map>
#include <juce_audio_processors/juce_audio_processors.h>

class TechMachineProcessor : public juce::AudioProcessor, private juce::ValueTree::Listener
{
public:
    TechMachineProcessor();
    ~TechMachineProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Tech Machine"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    techno::Machine& machine (int type) { return *machines[(size_t) type]; }
    bool isShown (int type) const { return strips[(size_t) type].show->load() > 0.5f; }
    void setShown (int type, bool shown);
    int numSteps() const { return ((int) bars->load() + 1) * techno::pageSteps; }
    bool isHosted() const { return wrapperType != wrapperType_Standalone && wrapperType != wrapperType_Undefined; }
    int currentScale() const { return (int) scaleParam->load(); }
    int currentKey() const { return (int) keyParam->load(); }

    void setParam (const juce::String& id, float value);
    void setBars (int newBars);
    void touchPattern() { ++patternVersion; dirty = true; }

    // Generation ("hasard") and editing tools. Each one can be undone.
    int genre() const { return genreIndex; }
    void setGenre (int index);
    void generate (int machineType, techno::GenerateMode);
    void generateAll();
    void clearMachine (int machineType);
    void applyPattern (int machineType, const techno::Pattern&);
    void applyPreset (int machineType, const techno::SoundPreset&);

    // MIDI: the machine that receives the keyboard / pads, recording of what is played (MIDI learn: see midiLearn).
    void setMidiTarget (int machineType) { midiTarget = machineType; }
    int midiTargetMachine() const;
    void setRecordNotes (bool on);
    void previewNote (int machineType, int midiNote, bool on);   // plays a note right away (piano roll)
    bool isRecordingNotes() const { return recordNotes.load(); }
    std::atomic<int> midiActivity { 0 };

    // Automation recording: while it is on and the sequencer plays, moving a knob writes its value into the steps.
    void setRecordAutomation (bool on);
    bool isRecordingAutomation() const { return recordAutomation.load(); }
    void recordKnob (const juce::String& paramId, float value);   // message thread
    void setAutomation (int machineType, int target, int step, float value);   // noLock erases; message thread
    void clearAutomation (int machineType, int target);

    // Sampler pads.
    static constexpr int numPads = techno::SampleBank::numPads;
    bool loadSamplePad (int pad, const juce::File&);
    bool chopLoop (const juce::File& loop);
    void clearSamplePads();
    juce::String samplePadName (int pad) const { return samples.name (pad); }
    int sampleVersion() const { return samples.version.load(); }

    juce::RangedAudioParameter* presetParameter (int machineType, const char* key);
    void shiftMachine (int machineType, int direction);
    void duplicateFirstBar (int machineType);
    void copyMachine (int machineType);
    void pasteMachine (int machineType);
    bool canPaste (int machineType) const;
    bool isProjectEmpty() const;

    // Call before changing notes from the UI so the change can be undone.
    void beginPatternEdit();
    juce::UndoManager undoManager;

    // Projects
    void newProject();
    bool saveProject (const juce::File&);
    bool loadProject (const juce::File&);
    void loadExample (int index);
    juce::String projectName() const { return name; }
    juce::File projectFile() const { return file; }
    bool isDirty() const { return dirty.load(); }
    static juce::StringArray recentProjects();
    static juce::File projectsFolder();

    // The whole project as XML (also used for the plugin state).
    std::unique_ptr<juce::XmlElement> createProjectXml();

    // Offline render of a project to a WAV file. Safe to call from a background thread.
    static bool exportWav (const juce::XmlElement& project, const juce::File&, int loops, const std::function<bool (float)>& progress);

    void startRecording();
    void stopRecording();
    bool isRecording() const { return activeWriter.load() != nullptr; }
    juce::File getLastRecording() const { return lastRecording; }

    juce::AudioProcessorValueTreeState apvts;
    techno::MidiLearn midiLearn { apvts };
    std::atomic<bool> playing { false };
    std::atomic<int> currentStep { -1 };
    std::atomic<int> patternVersion { 0 };
    std::atomic<float> outputPeak { 0.0f };
    std::atomic<float> displayBpm { 130.0f };

private:
    struct Strip
    {
        std::atomic<float>* show, *mute, *solo, *drive, *crush, *filter, *echo, *reverb, *volume, *pan;
        techno::Crusher crusher;
        techno::DjFilter djFilter;
        float gain = 0.0f;
        bool shown = false, audible = false;
        float driveAmount = 0, crushAmount = 0, echoSend = 0, reverbSend = 0, target = 0, panLeft = 1, panRight = 1;
    };

    using Snapshot = std::vector<techno::TrackData>;   // one per machine, on the heap: it is large
    class PatternAction;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void render (float* left, float* right, int numSamples);
    void readStrip (int machineType);
    void advanceStep();
    void syncToHost (double ppq);
    float stepSeconds() const { return 60.0f / bpmValue / 4.0f; }

    Snapshot snapshot() const;
    void restore (const Snapshot&);
    techno::GenerateContext context() const;
    void chooseProgression();
    void resetParameters();
    void restoreProjectXml (const juce::XmlElement&);
    static void addRecent (const juce::File&);
    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override { dirty = true; }

    std::vector<std::unique_ptr<techno::Machine>> machines;
    std::array<Strip, techno::numMachines> strips {};

    std::atomic<float>* bpm, *swing, *keyParam, *scaleParam, *bars;
    std::atomic<float>* echoTime, *echoFeedback, *echoLevel, *reverbSize, *reverbLevel, *pumpParam, *djFilterParam, *comp, *disto, *volume;

    float sampleRate = 44100.0f, bpmValue = 128.0f, muteCoef = 0.0f;
    double samplesToNextStep = 0.0;
    int stepIndex = -1, lengthSteps = 16;
    bool wasPlaying = false;

    techno::PingPongDelay delay;
    techno::Pump pump;
    techno::FdnReverb reverb;
    techno::DjFilter masterFilter;
    techno::GlueCompressor compressor;
    techno::PeakLimiter limiter;
    float feedbackValue = 0, echoLevelValue = 0, reverbLevelValue = 0, pumpDepth = 0, distoValue = 0, volumeValue = 1;
    float lastReverbSize = -1.0f;
    juce::Random random;

    int genreIndex = 0;
    std::vector<int> progression { 0 };
    std::optional<techno::TrackData> clipboard;
    int clipboardKind = -1;
    juce::String name;
    juce::File file;
    std::atomic<bool> dirty { false };

    void handleMidi (const juce::MidiBuffer&);
    int quantizedStep() const;
    void recordNoteOn (int machineType, int note, float velocity);
    void recordNoteOff (int machineType, int note);
    int recordedSemitone (int machineType, int note) const;
    std::atomic<int> midiTarget { -1 };
    std::atomic<bool> recordNotes { false }, recordAutomation { false };
    std::map<juce::String, std::pair<int, int>> automationIndex;   // parameter id -> (machine, target)
    int recordStart[128] {};

    struct PreviewNote { int type = 0, note = 0; bool on = false; };
    std::array<PreviewNote, 64> previewQueue {};
    juce::AbstractFifo previewFifo { 64 };

    techno::SampleBank samples;

    juce::TimeSliceThread writerThread { "Tech Machine recorder" };
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };
    juce::File lastRecording;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TechMachineProcessor)
};
