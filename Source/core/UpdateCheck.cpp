#include "UpdateCheck.h"
#include "Language.h"

namespace techno
{
bool isNewerVersion (const juce::String& latest, const juce::String& current)
{
    auto numbers = [] (juce::String v)
    {
        v = v.trim().trimCharactersAtStart ("vV");
        juce::Array<int> parts;
        for (auto& p : juce::StringArray::fromTokens (v.upToFirstOccurrenceOf ("-", false, false), ".", {})) parts.add (p.getIntValue());
        while (parts.size() < 3) parts.add (0);
        return parts;
    };
    const auto a = numbers (latest), b = numbers (current);
    for (int i = 0; i < 3; ++i)
        if (a[i] != b[i]) return a[i] > b[i];
    return false;
}

UpdateCheck::UpdateCheck() : juce::Thread ("Tech Machine update check") {}

UpdateCheck::~UpdateCheck() { stopThread (6000); }

bool UpdateCheck::enabled() { return settings().getBoolValue ("checkUpdates", true); }
void UpdateCheck::setEnabled (bool on) { settings().setValue ("checkUpdates", on); }

void UpdateCheck::start (std::function<void (const juce::String&, const juce::URL&)> whenNewer)
{
    if (! enabled()) return;
    const auto now = juce::Time::currentTimeMillis();
    if (now - (juce::int64) settings().getDoubleValue ("lastUpdateCheck", 0.0) < 24LL * 3600 * 1000)
    {
        // Already asked today: the answer kept then is shown again.
        const auto known = settings().getValue ("latestVersion");
        if (isNewerVersion (known, JucePlugin_VersionString)) whenNewer (known, juce::URL (settings().getValue ("latestPage")));
        return;
    }
    callback = std::move (whenNewer);
    startThread (juce::Thread::Priority::low);
}

void UpdateCheck::run()
{
    const juce::URL api ("https://api.github.com/repos/Weylland/tech-machine/releases/latest");
    auto stream = api.createInputStream (juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                                             .withConnectionTimeoutMs (5000)
                                             .withExtraHeaders ("Accept: application/vnd.github+json\r\nUser-Agent: TechMachine"));
    if (stream == nullptr || threadShouldExit()) return;
    const auto json = juce::JSON::parse (stream->readEntireStreamAsString());
    const auto tag = json.getProperty ("tag_name", {}).toString();
    const auto page = json.getProperty ("html_url", {}).toString();
    if (tag.isEmpty() || threadShouldExit()) return;

    auto cb = callback;
    juce::MessageManager::callAsync ([tag, page, cb]
    {
        settings().setValue ("lastUpdateCheck", (double) juce::Time::currentTimeMillis());
        settings().setValue ("latestVersion", tag);
        settings().setValue ("latestPage", page);
        if (isNewerVersion (tag, JucePlugin_VersionString) && cb) cb (tag, juce::URL (page));
    });
}
} // namespace techno
