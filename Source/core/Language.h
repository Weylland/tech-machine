#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace techno
{
enum class Language { French, English, Spanish, German, Portuguese };
inline constexpr int numLanguages = 5;
juce::String languageCode (Language);   // "fr", "en", "es", "de", "pt"
juce::String languageName (Language);   // in the language itself: FRANÇAIS, ENGLISH, ESPAÑOL, DEUTSCH, PORTUGUÊS

Language language();
void setLanguage (Language);

// Every text shown to the user is written in French in the code and goes through tr():
// in the other languages it is looked up in their table (then in English, then French, if missing).
juce::String tr (const char* french);

// Texts asked for in another language but missing from its table (used by the tests).
juce::StringArray missingTranslations();

// Application settings shared by the standalone app and the plugin (language, recent projects).
juce::PropertiesFile& settings();
} // namespace techno
