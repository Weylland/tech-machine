#pragma once

#include "core/Examples.h"
#include "core/MidiLearn.h"
#include "core/PatternLibrary.h"
#include "core/Presets.h"
#include "core/Styles.h"
#include "dsp/Effects.h"
#include "dsp/Modulation.h"
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
    bool isFollowingMidiClock() const { return clockActive.load(); }   // tempo, start and stop from another machine
    std::atomic<int> midiActivity { 0 };

    // Song: 8 parts (A to H), each holding every machine's pattern, and the list of parts to play in order.
    static constexpr int numParts = 8, maxSongEntries = 64;
    struct SongEntry { int part = 0, repeats = 1; };
    int currentPart() const { return activePart.load(); }
    int cuedPartIndex() const { return cuedPart.load(); }           // waiting for the end of the loop, -1 = none
    void selectPart (int part);                                     // at once when stopped, at the end of the loop when playing
    bool isPartEmpty (int part) const;
    void copyPart (int from, int to);
    void clearPart (int part);
    std::vector<SongEntry> song() const;
    void setSong (const std::vector<SongEntry>&);
    void setSongMode (bool on);
    bool isSongMode() const { return songMode.load(); }
    int songPosition() const { return songEntry.load(); }           // entry being played, -1 = none
    double songSeconds() const;                                     // length of the whole song

    // Automation recording: while it is on and the sequencer plays, moving a knob writes its value into the steps.
    void setRecordAutomation (bool on);
    bool isRecordingAutomation() const { return recordAutomation.load(); }
    void recordKnob (const juce::String& paramId, float value);   // message thread
    void setAutomation (int machineType, int target, int step, float value);   // noLock erases; message thread
    void clearAutomation (int machineType, int target);

    // FILL: the drums play a fill over the last bar of the loop, once, then a crash on the next downbeat.
    void armFill (bool on) { fillArmed = on; }
    bool isFillArmed() const { return fillArmed.load(); }
    bool isFillPlaying() const { return fillPlaying.load(); }

    // Scenes for playing live: which machines are cut. A scene comes in at the start of the next bar.
    static constexpr int numScenes = 8;
    void storeScene (int scene);                     // keeps the machines cut now (message thread)
    void recallScene (int scene);                    // at the next bar while playing, at once when stopped
    void clearScene (int scene);
    bool hasScene (int scene) const { return sceneStored[(size_t) scene].load(); }
    int cuedSceneIndex() const { return cuedScene.load(); }
    int lastScene() const { return currentScene.load(); }

    // Separate outputs in a host: a machine can leave on its own stereo output instead of the master.
    static constexpr int numAuxOutputs = 8;

    // Sampler pads, and the sound of SAMPLE KEYS (slot keysSlot).
    static constexpr int numPads = techno::SampleBank::numPads;
    static constexpr int keysSlot = techno::SampleBank::keysSlot;
    bool loadSamplePad (int slot, const juce::File&);
    const techno::SampleData* sampleData (int slot) const { return samples.data (slot); }
    void setSampleRange (int slot, int start, int end, int loopStart, int loopEnd);   // message thread

    // Recording from the audio input (microphone) into a slot.
    bool startSampling (int slot);                 // false without an audio input
    juce::File stopSampling();                     // writes the WAV, loads it into the slot; empty when nothing was heard
    bool isSampling() const { return sampling.load(); }
    float samplingLevel() const { return samplingPeak.load(); }
    double samplingSeconds() const { return (double) samplingCount.load() / (double) sampleRate; }
    bool hasAudioInput() const { return getTotalNumInputChannels() > 0; }
    static juce::File recordingsFolder();
    bool chopLoop (const juce::File& loop);
    void clearSamplePads();
    juce::String samplePadName (int pad) const { return samples.name (pad); }
    int sampleVersion() const { return samples.version.load(); }

    juce::RangedAudioParameter* presetParameter (int machineType, const char* key);

    // The user's own sounds, saved as files and available in every project.
    static std::vector<juce::String> soundParameters (int machineType);   // the settings that make a sound
    static juce::File userSoundsFolder (int machineType);
    static juce::StringArray userSounds (int machineType);
    juce::String saveUserSound (int machineType, const juce::String& name);   // the name used, empty on failure
    bool loadUserSound (int machineType, const juce::String& name);
    static bool deleteUserSound (int machineType, const juce::String& name);
    void resetSound (int machineType);
    void shiftMachine (int machineType, int direction);
    void setTrackTiming (int machineType, int length, int speed);   // length 0 = the whole sequence; speed: see trackSpeedFactors
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

    // The notes as a MIDI file (one track per machine; the whole song when it is on). Message thread.
    bool exportMidi (const juce::File&) const;

    // One WAV per machine of the rack, each with its own effects. Safe to call from a background thread.
    static bool exportStems (const juce::XmlElement& project, const juce::File& folder, int loops, const std::function<bool (float)>& progress);

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
        std::atomic<float>* show, *mute, *solo, *drive, *crush, *filter, *echo, *reverb, *volume, *pan, *output, *phaser, *flanger;
        techno::Phaser phaserFx;
        techno::Flanger flangerFx;
        float phaserAmount = 0, flangerAmount = 0;
        juce::RangedAudioParameter* muteParameter = nullptr, *soloParameter = nullptr;   // scenes change them on the audio thread
        int bus = 0;   // 0 = master, 1..numAuxOutputs = a separate output of the host
        techno::Crusher crusher;
        techno::DjFilter djFilter;
        float gain = 0.0f;
        bool shown = false, audible = false;
        float driveAmount = 0, crushAmount = 0, echoSend = 0, reverbSend = 0, target = 0, panLeft = 1, panRight = 1;
    };

    struct Snapshot
    {
        int part = 0;
        std::vector<techno::TrackData> tracks;   // one per machine, on the heap: it is large
    };
    class PatternAction;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void render (float* left, float* right, int numSamples);
    std::array<float*, numAuxOutputs + 1> auxLeft {}, auxRight {};
    static BusesProperties buses();
    void readStrip (int machineType);
    void advanceStep();
    void playTrackStep (int machineType, int step, long long round, int steps, double stepSamples, double nominal);
    void startStep (int machineType);
    void runOwnClocks();
    void applyScene (int scene);
    void syncToHost (double ppq);
    float stepSeconds() const { return 60.0f / bpmValue / 4.0f; }

    Snapshot snapshot() const;
    Snapshot snapshotOf (int part) const;
    void beginPartEdit (int part);
    void restore (const Snapshot&);
    techno::Track& partTrack (int part, int machineType);   // the playing tracks for the current part
    const techno::Track& partTrack (int part, int machineType) const;
    void switchPart (int part);                               // any thread
    void startOfLoop();                                       // audio thread: song and cued part
    void writeTrackXml (juce::XmlElement&, int machineType, const techno::Track&) const;
    void readTrackXml (const juce::XmlElement&, int machineType, techno::Track&);
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
    std::atomic<float>* echoType, *reverbType, *pumpMode;
    techno::KickFollower kickFollower;
    bool sidechain = false;
    int lastReverbType = 0;
    float barHz = 0.5f;   // one cycle per bar, for the phaser and the flanger

    float sampleRate = 44100.0f, bpmValue = 128.0f, muteCoef = 0.0f;
    double samplesToNextStep = 0.0, stepLength = 1.0;
    int stepIndex = -1, lengthSteps = 16;
    long long stepCount = -1, loopCount = -1;   // steps and loops since the start (from the host position in a plugin)
    bool wasPlaying = false, fillOn = false, anyOwnClock = false;
    std::atomic<bool> fillArmed { false }, fillPlaying { false };
    juce::Random stepRandom;                    // chance of the steps: the same every time the music starts
    std::array<techno::StepInfo, techno::numMachines> pendingStep {};
    std::array<double, techno::numMachines> pendingIn {};   // samples before a delayed step starts, -1 = none
    std::array<bool, techno::numMachines> ownClock {};      // tracks with their own speed

    std::array<std::array<std::atomic<bool>, techno::numMachines>, numScenes> sceneMutes {};
    std::array<std::atomic<bool>, numScenes> sceneStored {};
    std::atomic<int> cuedScene { -1 }, currentScene { -1 };

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
    void handleMidiClock (const juce::MidiBuffer&, int numSamples);
    std::atomic<bool> clockActive { false };
    double clockInterval = 0, samplesSinceClock = 1.0e9, blockStart = 0, lastClock = -1;
    std::vector<float> micInput;   // the audio input of this block (vocoder microphone, sampling)
    std::vector<float> samplingBuffer;
    std::atomic<bool> sampling { false }, samplingWrite { false };
    std::atomic<int> samplingCount { 0 }, samplingSlot { 0 };
    std::atomic<float> samplingPeak { 0.0f };
    int quantizedStep (int machineType) const;
    void recordNoteOn (int machineType, int note, float velocity);
    void recordNoteOff (int machineType, int note);
    int recordedSemitone (int machineType, int note) const;
    std::atomic<int> midiTarget { -1 };
    std::atomic<bool> recordNotes { false }, recordAutomation { false };
    std::unique_ptr<std::array<std::array<techno::Track, techno::numMachines>, numParts>> partStore;
    std::atomic<int> activePart { 0 }, cuedPart { -1 }, songLength { 0 }, songEntry { -1 };
    std::array<std::atomic<int>, maxSongEntries> songParts {}, songRepeats {};
    std::atomic<bool> songMode { false };
    int repeatsLeft = 0;
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
