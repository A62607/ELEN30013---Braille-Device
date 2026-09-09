#include <assert.h>
#include <stdint.h>

#include <iostream>

#include "../firmware/interactive_braille/BraillePatterns.h"

namespace {

struct ExpectedPattern {
  char letter;
  uint8_t pattern;
};

constexpr ExpectedPattern kExpected[] = {
    {'A', 0b000001}, {'B', 0b000011}, {'C', 0b001001},
    {'D', 0b011001}, {'E', 0b010001}, {'F', 0b001011},
    {'G', 0b011011}, {'H', 0b010011}, {'I', 0b001010},
    {'J', 0b011010}, {'K', 0b000101}, {'L', 0b000111},
    {'M', 0b001101}, {'N', 0b011101}, {'O', 0b010101},
    {'P', 0b001111}, {'Q', 0b011111}, {'R', 0b010111},
    {'S', 0b001110}, {'T', 0b011110}, {'U', 0b100101},
    {'V', 0b100111}, {'W', 0b111010}, {'X', 0b101101},
    {'Y', 0b111101}, {'Z', 0b110101},
};

}  // namespace

int main() {
  static_assert(sizeof(kExpected) / sizeof(kExpected[0]) ==
                    braille::kLetterCount,
                "The test must cover all 26 letters");

  for (uint8_t index = 0; index < braille::kLetterCount; ++index) {
    const ExpectedPattern expected = kExpected[index];
    const uint8_t actual = braille::patternForLetter(expected.letter);

    assert(actual == expected.pattern);
    assert(braille::patternForIndex(index) == expected.pattern);
    assert(braille::patternForLetter(expected.letter + ('a' - 'A')) ==
           expected.pattern);
    assert((actual & ~braille::kAllDotsMask) == 0);

    for (uint8_t dot = 1; dot <= 6; ++dot) {
      const bool expectedRaised =
          (expected.pattern & (uint8_t{1} << (dot - 1))) != 0;
      assert(braille::isDotRaised(actual, dot) == expectedRaised);
    }
  }

  assert(braille::patternForLetter('?') == braille::kInvalidPattern);
  assert(braille::patternForIndex(26) == braille::kInvalidPattern);
  assert(!braille::isDotRaised(0, 0));
  assert(!braille::isDotRaised(0, 7));

  std::cout << "All 26 Braille letter patterns passed.\n";
  return 0;
}

