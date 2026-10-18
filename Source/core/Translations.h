#pragma once

#include <string>
#include <unordered_map>

namespace techno
{
// French source text -> English (Translations.cpp is generated from the texts of the interface).
const std::unordered_map<std::string, const char*>& englishTable();
} // namespace techno
