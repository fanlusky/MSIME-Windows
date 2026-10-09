#pragma once

// Caps Lock temporarily bypasses Chinese punctuation without changing the user's
// punctuation/full-width compartments. Active input retains its commit/navigation rules.
inline bool ShouldPassThroughCapsLockPunctuation(bool capsLockEnabled, bool japaneseMode, bool inputInProgress,
                                                 bool punctuationKey)
{
    return capsLockEnabled && !japaneseMode && !inputInProgress && punctuationKey;
}
