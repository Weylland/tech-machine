#pragma once

#include <string>
#include <unordered_map>

namespace techno
{
// French source text -> the other languages (Translations*.cpp are generated from the texts of the interface).
const std::unordered_map<std::string, const char*>& englishTable();
const std::unordered_map<std::string, const char*>& spanishTable();
const std::unordered_map<std::string, const char*>& germanTable();
const std::unordered_map<std::string, const char*>& portugueseTable();
} // namespace techno
