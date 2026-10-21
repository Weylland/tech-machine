#include "Language.h"
#include "Translations.h"
#include <unordered_map>

namespace techno
{


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

juce::String languageCode (Language l)
{
    static const char* const codes[numLanguages] { "fr", "en", "es", "de", "pt" };
    return codes[(int) l];
}

juce::String languageName (Language l)
{
    static const char* const names[numLanguages] { "FRAN\xc3\x87" "AIS", "ENGLISH", "ESPA\xc3\x91OL", "DEUTSCH", "PORTUGU\xc3\x8aS" };
    return juce::String::fromUTF8 (names[(int) l]);
}

Language language()
{
    if (! loaded)
    {
        loaded = true;
        // The language chosen before, otherwise the one of the computer (English when it is none of ours).
        auto code = settings().getValue ("language");
        if (code.isEmpty()) code = juce::SystemStats::getUserLanguage().substring (0, 2).toLowerCase();
        current = Language::English;
        for (int i = 0; i < numLanguages; ++i)
            if (code == languageCode ((Language) i)) current = (Language) i;
    }
    return current;
}

void setLanguage (Language l)
{
    loaded = true;
    current = l;
    settings().setValue ("language", languageCode (l));
    settings().saveIfNeeded();
}

juce::String tr (const char* french)
{
    const auto lang = language();
    if (lang == Language::French || french == nullptr || *french == 0)
        return juce::String::fromUTF8 (french);

    auto find = [french] (const std::unordered_map<std::string, const char*>& table) -> const char*
    {
        const auto it = table.find (french);
        return it != table.end() ? it->second : nullptr;
    };
    const auto& own = lang == Language::Spanish ? spanishTable() : lang == Language::German ? germanTable()
                    : lang == Language::Portuguese ? portugueseTable() : englishTable();
    if (const char* found = find (own)) return juce::String::fromUTF8 (found);

    // Missing from this language: noted for the tests, then the English text (or the French one) stands in.
    {
        const juce::ScopedLock sl (missingLock());
        missing().addIfNotAlreadyThere (juce::String::fromUTF8 (french));
    }
    if (const char* english = find (englishTable())) return juce::String::fromUTF8 (english);
    return juce::String::fromUTF8 (french);
}

juce::StringArray missingTranslations()
{
    const juce::ScopedLock sl (missingLock());
    return missing();
}
} // namespace techno
