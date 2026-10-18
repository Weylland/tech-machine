#include "Language.h"
#include <unordered_map>

namespace techno
{
const std::unordered_map<std::string, const char*>& englishTable();   // Translations.cpp

namespace
{
std::atomic<Language> current { Language::French };
bool loaded = false;

juce::CriticalSection& missingLock()
{
    static juce::CriticalSection lock;
    return lock;
}

juce::StringArray& missing()
{
    static juce::StringArray list;
    return list;
}
} // namespace

juce::PropertiesFile& settings()
{
    static juce::PropertiesFile props ([]
    {
        juce::PropertiesFile::Options o;
        o.applicationName = "Tech Machine";
        o.filenameSuffix = "settings";
        o.folderName = "Tech Machine";
        // Automated tests keep their own settings, away from the user's recent projects and choices.
        if (juce::SystemStats::getEnvironmentVariable ("TECHMACHINE_TEST_SETTINGS", {}).isNotEmpty()) o.folderName = "Tech Machine Tests";
        o.osxLibrarySubFolder = "Application Support";
        return o;
    }());
    return props;
}

Language language()
{
    if (! loaded)
    {
        loaded = true;
        const auto saved = settings().getValue ("language");
        if (saved == "en") current = Language::English;
        else if (saved.isEmpty() && juce::SystemStats::getUserLanguage() != "fr") current = Language::English;
    }
    return current;
}

void setLanguage (Language l)
{
    loaded = true;
    current = l;
    settings().setValue ("language", l == Language::English ? "en" : "fr");
    settings().saveIfNeeded();
}

juce::String tr (const char* french)
{
    if (language() == Language::French || french == nullptr || *french == 0)
        return juce::String::fromUTF8 (french);

    const auto& table = englishTable();
    if (auto it = table.find (french); it != table.end())
        return juce::String::fromUTF8 (it->second);

    const auto text = juce::String::fromUTF8 (french);
    const juce::ScopedLock sl (missingLock());
    missing().addIfNotAlreadyThere (text);
    return text;
}

juce::StringArray missingTranslations()
{
    const juce::ScopedLock sl (missingLock());
    return missing();
}
} // namespace techno
