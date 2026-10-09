#pragma once

#include <stdint.h>

namespace braille
{

    // Dot numbering follows the standard six-dot Braille cell:
    //
    //   1 4
    //   2 5
    //   3 6
    //
    // Bits 0-5 correspond directly to dots 1-6.
    constexpr uint8_t kDot1 = uint8_t{1} << 0;
    constexpr uint8_t kDot2 = uint8_t{1} << 1;
    constexpr uint8_t kDot3 = uint8_t{1} << 2;
    constexpr uint8_t kDot4 = uint8_t{1} << 3;
    constexpr uint8_t kDot5 = uint8_t{1} << 4;
    constexpr uint8_t kDot6 = uint8_t{1} << 5;

    constexpr uint8_t kAllDotsMask = 0b00111111;
    constexpr uint8_t kInvalidPattern = 0b11111111; // lower all 6 dots
    constexpr uint8_t kLetterCount = 26;

    // Returns the six-dot pattern for A-Z or a-z. Unsupported characters return
    // kInvalidPattern.
    uint8_t patternForLetter(char letter);

    // Returns the pattern for index 0-25 (A-Z). An invalid index returns
    // kInvalidPattern.
    uint8_t patternForIndex(uint8_t index);

    // Returns true when dotNumber (1-6) is raised in pattern.
    bool isDotRaised(uint8_t pattern, uint8_t dotNumber);

} // namespace braille
