#include "BraillePatterns.h"

#if defined(ARDUINO_ARCH_AVR)
#include <avr/pgmspace.h>
#endif

namespace braille {
namespace {

// Grade 1 English Braille letters A-Z. Only the lower six bits are used.
#if defined(ARDUINO_ARCH_AVR)
const uint8_t kLetterPatterns[kLetterCount] PROGMEM = {
#else
const uint8_t kLetterPatterns[kLetterCount] = {
#endif
    kDot1,                                      // A: 1
    kDot1 | kDot2,                              // B: 12
    kDot1 | kDot4,                              // C: 14
    kDot1 | kDot4 | kDot5,                      // D: 145
    kDot1 | kDot5,                              // E: 15
    kDot1 | kDot2 | kDot4,                      // F: 124
    kDot1 | kDot2 | kDot4 | kDot5,              // G: 1245
    kDot1 | kDot2 | kDot5,                      // H: 125
    kDot2 | kDot4,                              // I: 24
    kDot2 | kDot4 | kDot5,                      // J: 245
    kDot1 | kDot3,                              // K: 13
    kDot1 | kDot2 | kDot3,                      // L: 123
    kDot1 | kDot3 | kDot4,                      // M: 134
    kDot1 | kDot3 | kDot4 | kDot5,              // N: 1345
    kDot1 | kDot3 | kDot5,                      // O: 135
    kDot1 | kDot2 | kDot3 | kDot4,              // P: 1234
    kDot1 | kDot2 | kDot3 | kDot4 | kDot5,      // Q: 12345
    kDot1 | kDot2 | kDot3 | kDot5,              // R: 1235
    kDot2 | kDot3 | kDot4,                      // S: 234
    kDot2 | kDot3 | kDot4 | kDot5,              // T: 2345
    kDot1 | kDot3 | kDot6,                      // U: 136
    kDot1 | kDot2 | kDot3 | kDot6,              // V: 1236
    kDot2 | kDot4 | kDot5 | kDot6,              // W: 2456
    kDot1 | kDot3 | kDot4 | kDot6,              // X: 1346
    kDot1 | kDot3 | kDot4 | kDot5 | kDot6,      // Y: 13456
    kDot1 | kDot3 | kDot5 | kDot6,              // Z: 1356
};

}  // namespace

uint8_t patternForIndex(uint8_t index) {
  if (index >= kLetterCount) {
    return kInvalidPattern;
  }

#if defined(ARDUINO_ARCH_AVR)
  return pgm_read_byte(&kLetterPatterns[index]);
#else
  return kLetterPatterns[index];
#endif
}

uint8_t patternForLetter(char letter) {
  if (letter >= 'a' && letter <= 'z') {
    letter = static_cast<char>(letter - 'a' + 'A');
  }

  if (letter < 'A' || letter > 'Z') {
    return kInvalidPattern;
  }

  return patternForIndex(static_cast<uint8_t>(letter - 'A'));
}

bool isDotRaised(uint8_t pattern, uint8_t dotNumber) {
  if (dotNumber < 1 || dotNumber > 6 || pattern == kInvalidPattern) {
    return false;
  }

  const uint8_t dotMask = uint8_t{1} << (dotNumber - 1);
  return (pattern & dotMask) != 0;
}

}  // namespace braille

