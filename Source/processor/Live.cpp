#include "../PluginProcessor.h"

using namespace techno;

// Scenes: which machines are cut, kept in one of 8 memories and brought back on the beat while playing live.
void TechMachineProcessor::storeScene (int scene)
{
    if (scene < 0 || scene >= numScenes) return;
    for (int t = 0; t < numMachines; ++t)
        sceneMutes[(size_t) scene][(size_t) t] = strips[(size_t) t].mute->load() > 0.5f;
    sceneStored[(size_t) scene] = true;
    currentScene = scene;
    dirty = true;
}

void TechMachineProcessor::recallScene (int scene)
{
    if (scene < 0 || scene >= numScenes || ! hasScene (scene)) return;
    cuedScene = scene;
    if (! playing.load()) applyScene (scene);
}

void TechMachineProcessor::clearScene (int scene)
{
    if (scene < 0 || scene >= numScenes) return;
    sceneStored[(size_t) scene] = false;
    if (cuedScene.load() == scene) cuedScene = -1;
    if (currentScene.load() == scene) currentScene = -1;
    dirty = true;
}

// Any thread: the audio thread calls it at the start of a bar.
void TechMachineProcessor::applyScene (int scene)
{
    cuedScene = -1;
    auto set = [] (juce::RangedAudioParameter* p, std::atomic<float>* value, bool on)
    {
        if ((value->load() > 0.5f) != on) p->setValueNotifyingHost (on ? 1.0f : 0.0f);
    };
    for (int t = 0; t < numMachines; ++t)
    {
        auto& s = strips[(size_t) t];
        set (s.muteParameter, s.mute, sceneMutes[(size_t) scene][(size_t) t].load());
        set (s.soloParameter, s.solo, false);
    }
    currentScene = scene;
}
