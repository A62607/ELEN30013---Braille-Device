#include "BraillePatterns.h"

// happens at compil time:
//  IF we're compiling for an AVR Arduino
//     include avr/pgmspace.h
//  ENDIF
#if defined(ARDUINO_ARCH_AVR)
#include <avr/pgmspace.h> //Arduino Uno have seperate memory space
#endif

// bit:       5 4 3 2 1 0
// Braille:   6 5 4 3 2 1

// name space, function call become braille::patternForLetter() instead of patternForLetter()

namespace braille
{
  // private namespace
  namespace
  {

#if defined(ARDUINO_ARCH_AVR)
    // keep lookup table in program flash rather than runtime SRAM
    const uint8_t kLetterPatterns[kLetterCount] PROGMEM = {
#else
    const uint8_t kLetterPatterns[kLetterCount] = {
#endif
        kDot1,                                 // A: 1
        kDot1 | kDot2,                         // B: 12 = 000011, kdot 1 or 2
        kDot1 | kDot4,                         // C: 14
        kDot1 | kDot4 | kDot5,                 // D: 145
        kDot1 | kDot5,                         // E: 15
        kDot1 | kDot2 | kDot4,                 // F: 124
        kDot1 | kDot2 | kDot4 | kDot5,         // G: 1245
        kDot1 | kDot2 | kDot5,                 // H: 125
        kDot2 | kDot4,                         // I: 24
        kDot2 | kDot4 | kDot5,                 // J: 245
        kDot1 | kDot3,                         // K: 13
        kDot1 | kDot2 | kDot3,                 // L: 123
        kDot1 | kDot3 | kDot4,                 // M: 134
        kDot1 | kDot3 | kDot4 | kDot5,         // N: 1345
        kDot1 | kDot3 | kDot5,                 // O: 135
        kDot1 | kDot2 | kDot3 | kDot4,         // P: 1234
        kDot1 | kDot2 | kDot3 | kDot4 | kDot5, // Q: 12345
        kDot1 | kDot2 | kDot3 | kDot5,         // R: 1235
        kDot2 | kDot3 | kDot4,                 // S: 234
        kDot2 | kDot3 | kDot4 | kDot5,         // T: 2345
        kDot1 | kDot3 | kDot6,                 // U: 136
        kDot1 | kDot2 | kDot3 | kDot6,         // V: 1236
        kDot2 | kDot4 | kDot5 | kDot6,         // W: 2456
        kDot1 | kDot3 | kDot4 | kDot6,         // X: 1346
        kDot1 | kDot3 | kDot4 | kDot5 | kDot6, // Y: 13456
        kDot1 | kDot3 | kDot5 | kDot6,         // Z: 1356
    };

  } // namespace

  // return braille pattern stored at that index, e.g index 0 should return braile pattern for A
  uint8_t patternForIndex(uint8_t index)
  {
    // check if the index is valiad, return invalid rather than access unknown memory space
    if (index >= kLetterCount)
    {
      return kInvalidPattern;
    }

#if defined(ARDUINO_ARCH_AVR)
    // read 1 byte from this location
    return pgm_read_byte(&kLetterPatterns[index]);
#else
    return kLetterPatterns[index];
#endif
  }

  // for a given latter, return its braille pattern
  uint8_t patternForLetter(char letter)
  {
    // convert lower case to upper case, in ASCII, A = 65, a = 97, Z = 90, z = 122
    if (letter >= 'a' && letter <= 'z')
    {
      // letter - 'a' + 'A' gives an integer value, cast it into char
      letter = static_cast<char>(letter - 'a' + 'A');
    }

    if (letter < 'A' || letter > 'Z')
    {
      return kInvalidPattern;
    }

    return patternForIndex(static_cast<uint8_t>(letter - 'A'));
  }

  // checks: for this Braille pattern, should this particular dot be raised?
  bool isDotRaised(uint8_t pattern, uint8_t dotNumber)
  {
    // validity check
    if (dotNumber < 1 || dotNumber > 6 || pattern == kInvalidPattern)
    {
      return false;
    }

    // uint8_t{1} = 00000001, << is left shift operator
    const uint8_t dotMask = uint8_t{1} << (dotNumber - 1);

    // firmware utility check
    return (pattern & dotMask) != 0; // bitwise AND, return non-zero value if that dot should be raised
  }

} // namespace braille
