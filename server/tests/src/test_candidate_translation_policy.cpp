#include "ipc/candidate_translation_policy.h"
#include "tests/includes/test_framework.h"

TEST_CASE(ctrl_enter_alone_commits_the_secondary_candidate)
{
    using FanyImeIpc::IsTranslationCommitKey;
    REQUIRE(IsTranslationCommitKey(0x0D, FanyImeIpc::kModifierControl));
    // The UI-less flag lives outside the modifier mask and must not disarm the shortcut.
    REQUIRE(IsTranslationCommitKey(0x0D, FanyImeIpc::kModifierControl | FanyImeIpc::kModifierUiLess));

    REQUIRE(!IsTranslationCommitKey(0x0D, 0));
    REQUIRE(!IsTranslationCommitKey(0x0D, FanyImeIpc::kModifierControl | FanyImeIpc::kModifierShift));
    REQUIRE(!IsTranslationCommitKey(0x0D, FanyImeIpc::kModifierControl | FanyImeIpc::kModifierAlt));
    REQUIRE(!IsTranslationCommitKey(0x20, FanyImeIpc::kModifierControl));
    REQUIRE(!IsTranslationCommitKey('J', FanyImeIpc::kModifierControl));
}

TEST_CASE(translation_gloss_splits_on_both_semicolon_widths)
{
    using FanyImeIpc::SplitTranslationGloss;
    // en->zh glosses join senses with the fullwidth semicolon.
    const auto apple = SplitTranslationGloss("苹果；家伙；[计]苹果公司");
    REQUIRE_EQ(apple.size(), static_cast<size_t>(3));
    REQUIRE_EQ(apple[0], std::string("苹果"));
    REQUIRE_EQ(apple[1], std::string("家伙"));
    REQUIRE_EQ(apple[2], std::string("[计]苹果公司"));

    // zh->en glosses use "; " and the trailing space must not survive.
    const auto chuan = SplitTranslationGloss("string; trail; chuan");
    REQUIRE_EQ(chuan.size(), static_cast<size_t>(3));
    REQUIRE_EQ(chuan[0], std::string("string"));
    REQUIRE_EQ(chuan[1], std::string("trail"));
    REQUIRE_EQ(chuan[2], std::string("chuan"));
}

TEST_CASE(translation_gloss_drops_part_of_speech_labels)
{
    using FanyImeIpc::SplitTranslationGloss;
    // Machine-translated zh->en glosses label each sense; every sense is kept.
    const auto laugh = SplitTranslationGloss("adj. spreading; n. sound of laughter; v. giggle; idiom. burst out");
    REQUIRE_EQ(laugh.size(), static_cast<size_t>(4));
    REQUIRE_EQ(laugh[0], std::string("spreading"));
    REQUIRE_EQ(laugh[1], std::string("sound of laughter"));
    REQUIRE_EQ(laugh[2], std::string("giggle"));
    REQUIRE_EQ(laugh[3], std::string("burst out"));

    const auto place = SplitTranslationGloss("proper n. Wanshan District");
    REQUIRE_EQ(place.size(), static_cast<size_t>(1));
    REQUIRE_EQ(place[0], std::string("Wanshan District"));

    // Unlabelled senses, and words that merely start like a label, are untouched.
    const auto stoop = SplitTranslationGloss("v. stoop; crouch");
    REQUIRE_EQ(stoop[1], std::string("crouch"));
    const auto peculiar = SplitTranslationGloss("very peculiar");
    REQUIRE_EQ(peculiar[0], std::string("very peculiar"));
    const auto bare = SplitTranslationGloss("n.");
    REQUIRE_EQ(bare[0], std::string("n."));
}

TEST_CASE(translation_gloss_keeps_a_single_sense_whole)
{
    using FanyImeIpc::SplitTranslationGloss;
    // A cloud gloss is one sentence: commas inside it are not sense separators.
    const auto cloud = SplitTranslationGloss("今天天气不错，我们出去走走");
    REQUIRE_EQ(cloud.size(), static_cast<size_t>(1));
    REQUIRE_EQ(cloud[0], std::string("今天天气不错，我们出去走走"));

    REQUIRE(SplitTranslationGloss("").empty());
    REQUIRE(SplitTranslationGloss("   ").empty());
    // Empty senses are dropped rather than becoming blank candidates.
    REQUIRE_EQ(SplitTranslationGloss("；；").size(), static_cast<size_t>(0));
    const auto ragged = SplitTranslationGloss("；apple； ；pear；");
    REQUIRE_EQ(ragged.size(), static_cast<size_t>(2));
    REQUIRE_EQ(ragged[0], std::string("apple"));
    REQUIRE_EQ(ragged[1], std::string("pear"));
}
