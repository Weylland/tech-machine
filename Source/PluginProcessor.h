#pragma once

#include "core/Examples.h"
#include "core/PatternLibrary.h"
#include "core/Presets.h"
#include "core/Styles.h"
#include "dsp/Effects.h"
#include "dsp/Reverb.h"
#include "machines/Machines.h"

#include <juce_audio_formats/juce_audio_formats.h>
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
    bool acceptsMidi() const override { return false; }
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

    using Snapshot = std::array<techno::TrackData, techno::numMachines>;
    class PatternAction;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void render (float* left, float* right, int numSamples);
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

    juce::TimeSliceThread writerThread { "Tech Machine recorder" };
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };
    juce::File lastRecording;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TechMachineProcessor)
};
