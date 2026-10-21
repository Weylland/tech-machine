#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

namespace techno
{
// True when "latest" (a tag such as "v0.8.1") is a later version than "current" ("0.8.0").
bool isNewerVersion (const juce::String& latest, const juce::String& current);

// Asks GitHub, once a day at most and in the background, whether a newer release exists.
// Only the public page of the latest release is read: nothing about the user is sent.
class UpdateCheck : private juce::Thread
{
public:
    UpdateCheck();
    ~UpdateCheck() override;

    void start (std::function<void (const juce::String& version, const juce::URL& page)> whenNewer);
    static bool enabled();
    static void setEnabled (bool);

private:
    void run() override;
    std::function<void (const juce::String&, const juce::URL&)> callback;
};
} // namespace techno
