#include "Key/CapsLockPunctuationPolicy.h"

int main()
{
    // Caps Lock OFF keeps the user's punctuation/full-width behavior.
    if (ShouldPassThroughCapsLockPunctuation(false, false, false, true))
        return 1;
    // Caps Lock ON passes an idle Chinese punctuation key to the host.
    if (!ShouldPassThroughCapsLockPunctuation(true, false, false, true))
        return 2;
    // A composition or candidate list must retain its existing handling.
    if (ShouldPassThroughCapsLockPunctuation(true, false, true, true))
        return 3;
    // Japanese punctuation and long-vowel handling remain unchanged.
    if (ShouldPassThroughCapsLockPunctuation(true, true, false, true))
        return 4;
    // Letters, digits, Space and control keys use their existing classifiers.
    if (ShouldPassThroughCapsLockPunctuation(true, false, false, false))
        return 5;
    return 0;
}
