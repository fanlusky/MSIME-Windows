#pragma once

#include <cstddef>

namespace metasequoia
{
// 长词前缀补全（#1343）的会话选项，见 quanpin/prefix_completion.h。
struct PrefixCompletionOptions
{
    bool enabled = false;
    // 首选至少要有这么多个字才补。字太少时以它开头的词太多，补出来的多半不是用户要的。
    std::size_t minimum_chars = 4;

    bool operator==(const PrefixCompletionOptions &other) const
    {
        return enabled == other.enabled && minimum_chars == other.minimum_chars;
    }
};

// 设置项的取值范围。小于 3 时 zhongguo 这类两字词也会补，候选页会被补全挤满。
inline constexpr std::size_t kPrefixCompletionMinimumCharsLowest = 3;
inline constexpr std::size_t kPrefixCompletionMinimumCharsHighest = 8;
inline constexpr std::size_t kPrefixCompletionDefaultMinimumChars = 4;
} // namespace metasequoia
