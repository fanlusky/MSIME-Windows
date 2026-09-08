#include "settings/dictionary_validation.h"
#include "tests/includes/test_framework.h"

#include <string>

TEST_CASE(DictionaryFullPinyinValidationRejectsAbbreviatedSyllables)
{
    quanpin::Segments segments;
    std::string normalized;

    REQUIRE(SettingsDictionary::Validation::NormalizeFullPinyin("nihao", segments, normalized));
    REQUIRE_EQ(normalized, std::string("ni'hao"));
    REQUIRE(SettingsDictionary::Validation::NormalizeFullPinyin("ni hao", segments, normalized));
    REQUIRE_EQ(normalized, std::string("ni'hao"));
    REQUIRE(SettingsDictionary::Validation::NormalizeFullPinyin("xi'an", segments, normalized));
    REQUIRE_EQ(normalized, std::string("xi'an"));
    REQUIRE(!SettingsDictionary::Validation::NormalizeFullPinyin("nh", segments, normalized));
    REQUIRE(!SettingsDictionary::Validation::NormalizeFullPinyin("ni'h", segments, normalized));
    REQUIRE(!SettingsDictionary::Validation::NormalizeFullPinyin("ni'", segments, normalized));
}

TEST_CASE(QuickPhraseValidationMatchesNamedPipeWcharCapacity)
{
    REQUIRE(SettingsDictionary::Validation::QuickPhraseFitsNamedPipe(std::string(199, 'a')));
    REQUIRE(!SettingsDictionary::Validation::QuickPhraseFitsNamedPipe(std::string(200, 'a')));

    const std::string emoji = "\xF0\x9F\x98\x80";
    REQUIRE(SettingsDictionary::Validation::QuickPhraseFitsNamedPipe(std::string(197, 'a') + emoji));
    REQUIRE(!SettingsDictionary::Validation::QuickPhraseFitsNamedPipe(std::string(198, 'a') + emoji));
}

TEST_CASE(CodedDictionaryImportRequiresTabsAndPreservesSpacesInWords)
{
    std::string word;
    std::string code;
    std::string message;
    int weight = -1;

    REQUIRE(SettingsDictionary::Validation::ParseCodedImportLine("包含 空格的词\tbao'han'kong'ge'de'ci\t123", word,
                                                                 code, weight, message));
    REQUIRE_EQ(word, std::string("包含 空格的词"));
    REQUIRE_EQ(code, std::string("bao'han'kong'ge'de'ci"));
    REQUIRE_EQ(weight, 123);

    REQUIRE(!SettingsDictionary::Validation::ParseCodedImportLine("普通词 putongci 10", word, code, weight, message));
    REQUIRE(!SettingsDictionary::Validation::ParseCodedImportLine("普通词\tputongci\t10\textra", word, code, weight,
                                                                  message));
}

TEST_CASE(CodedDictionaryImportAcceptsOptionalWeightAndRimeUserdb)
{
    std::string word;
    std::string code;
    std::string message;
    int weight = -1;

    REQUIRE(SettingsDictionary::Validation::ParseCodedImportLine("普通词\tputongci", word, code, weight, message));
    REQUIRE_EQ(word, std::string("普通词"));
    REQUIRE_EQ(code, std::string("putongci"));
    REQUIRE_EQ(weight, SettingsDictionary::Validation::kDefaultCodedImportWeight);

    REQUIRE(SettingsDictionary::Validation::ParseCodedImportLine("你好\tni hao\tc=3 d=0.12 t=12345", word, code, weight,
                                                                 message));
    REQUIRE_EQ(word, std::string("你好"));
    REQUIRE_EQ(code, std::string("ni hao"));
    REQUIRE_EQ(weight, SettingsDictionary::Validation::kDefaultCodedImportWeight);

    REQUIRE(SettingsDictionary::Validation::ParseCodedImportLine("西安\txi an\tc=1", word, code, weight, message));
    REQUIRE_EQ(word, std::string("西安"));
    REQUIRE_EQ(code, std::string("xi an"));
    REQUIRE_EQ(weight, SettingsDictionary::Validation::kDefaultCodedImportWeight);

    REQUIRE(
        !SettingsDictionary::Validation::ParseCodedImportLine("普通词\tputongci\tabc", word, code, weight, message));
}

TEST_CASE(ImportLineSkipWalksYamlFrontMatterAndComments)
{
    bool in_yaml_header = false;
    REQUIRE(SettingsDictionary::Validation::ShouldSkipImportLine("", in_yaml_header));
    REQUIRE(!in_yaml_header);
    REQUIRE(SettingsDictionary::Validation::ShouldSkipImportLine("   ", in_yaml_header));
    REQUIRE(SettingsDictionary::Validation::ShouldSkipImportLine("# Rime user dictionary", in_yaml_header));
    REQUIRE(SettingsDictionary::Validation::ShouldSkipImportLine("#@/db_name\tluna_pinyin", in_yaml_header));
    REQUIRE(!SettingsDictionary::Validation::ShouldSkipImportLine("你好\tni hao\t1", in_yaml_header));

    REQUIRE(SettingsDictionary::Validation::ShouldSkipImportLine("---", in_yaml_header));
    REQUIRE(in_yaml_header);
    REQUIRE(SettingsDictionary::Validation::ShouldSkipImportLine("name: luna_pinyin", in_yaml_header));
    REQUIRE(SettingsDictionary::Validation::ShouldSkipImportLine("sort: by_weight", in_yaml_header));
    REQUIRE(SettingsDictionary::Validation::ShouldSkipImportLine("  ---", in_yaml_header));
    REQUIRE(in_yaml_header);
    REQUIRE(SettingsDictionary::Validation::ShouldSkipImportLine("...", in_yaml_header));
    REQUIRE(!in_yaml_header);
    REQUIRE(!SettingsDictionary::Validation::ShouldSkipImportLine("你好\tni hao", in_yaml_header));
    REQUIRE(SettingsDictionary::Validation::ShouldSkipImportLine("  # comment after body", in_yaml_header));
}
