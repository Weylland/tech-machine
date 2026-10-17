#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace techno
{
enum class Language { French, English };

Language language();
void setLanguage (Language);

// Every text shown to the user is written in French in the code and goes through tr():
// in English it is looked up in the translation table (falls back to French if missing).
juce::String tr (const char* french);

// Texts asked for in English but missing from the table (used by the tests).
juce::StringArray missingTranslations();

// Application settings shared by the standalone app and the plugin (language, recent projects).
juce::PropertiesFile& settings();
} // namespace techno
