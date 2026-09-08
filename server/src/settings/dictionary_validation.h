#pragma once

#include "MetasequoiaImeEngine/quanpin/quanpin_query.h"

#include <string>

namespace SettingsDictionary::Validation
{
inline constexpr int kDefaultCodedImportWeight = 10000;

bool NormalizeFullPinyin(const std::string &input, quanpin::Segments &segments, std::string &normalized);
bool ShouldSkipImportLine(const std::string &line, bool &in_yaml_header);
bool ParseCodedImportLine(const std::string &line, std::string &word, std::string &code, int &weight,
                          std::string &message);
bool QuickPhraseFitsNamedPipe(const std::string &phrase);
} // namespace SettingsDictionary::Validation
