#include "Key/CapsLockPunctuationPolicy.h"
#include <cstdio>

int main()
{
    // Fixed contract table: bits are Caps Lock, Japanese, active input, punctuation.
    // Include overlapping exclusions (for example Japanese AND active input).
    // Only idle, non-Japanese punctuation with Caps Lock ON may pass through.
    constexpr bool expected[16] = {false, false, false, false, false, false, false, false,
                                   false, true,  false, false, false, false, false, false};
    int failures = 0;
    for (unsigned state = 0; state < 16; ++state)
    {
        const bool capsLock = (state & 8) != 0;
        const bool japanese = (state & 4) != 0;
        const bool inputActive = (state & 2) != 0;
        const bool punctuation = (state & 1) != 0;
        const bool actual = ShouldPassThroughCapsLockPunctuation(capsLock, japanese, inputActive, punctuation);
        if (actual != expected[state])
        {
            std::fprintf(stderr, "FAIL caps=%d japanese=%d active=%d punctuation=%d: expected=%d actual=%d\n", capsLock,
                         japanese, inputActive, punctuation, expected[state], actual);
            ++failures;
        }
    }
    if (failures != 0)
        return 1;
    std::puts("PASS: all 16 Caps Lock punctuation policy combinations");
    return 0;
}
