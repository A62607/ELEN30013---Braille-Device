#include "BraillePatterns.h"

namespace {

void printSixBits(uint8_t pattern) {
  for (int8_t bit = 5; bit >= 0; --bit) {
    Serial.print((pattern >> bit) & 0x01);
  }
}

void printRaisedDots(uint8_t pattern) {
  for (uint8_t dot = 1; dot <= 6; ++dot) {
    if (braille::isDotRaised(pattern, dot)) {
      Serial.print(dot);
    }
  }
}

void printBrailleAlphabet() {
  Serial.println(F("Letter  Bits    Raised dots"));
  Serial.println(F("------  ------  -----------"));

  for (uint8_t index = 0; index < braille::kLetterCount; ++index) {
    const char letter = static_cast<char>('A' + index);
    const uint8_t pattern = braille::patternForLetter(letter);

    Serial.print(letter);
    Serial.print(F("       "));
    printSixBits(pattern);
    Serial.print(F("  "));
    printRaisedDots(pattern);
    Serial.println();
  }
}

}  // namespace

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    // Required by some Arduino boards; the Uno continues immediately.
  }

  Serial.println(F("Interactive Braille alphabet table"));
  Serial.println(F("Bit 0 = dot 1, ..., bit 5 = dot 6"));
  Serial.println();
  printBrailleAlphabet();
}

void loop() {
  // Pattern storage demonstration only. No repeated work is required yet.
}

