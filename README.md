# Interactive Braille Firmware

Arduino firmware for the ELEN30013 interactive Braille learning device.

## Braille bit convention

Each character is stored in the lower six bits of an unsigned byte. Bit 0
represents dot 1, bit 1 represents dot 2, and so on through bit 5 for dot 6.

```text
Physical cell       Stored bits

1 4                 bit 0  bit 3
2 5                 bit 1  bit 4
3 6                 bit 2  bit 5
```

For example, the letter C uses dots 1 and 4, so its stored value is
`0b001001`.

## Current milestone

- Store the Grade 1 English Braille patterns for A-Z.
- Look up patterns using uppercase or lowercase letters.
- Print the full table over the Arduino serial monitor for inspection.
- Verify all 26 entries with a host-side test.

## Running the Arduino demonstration

Open `firmware/interactive_braille/interactive_braille.ino` in the Arduino
IDE, select an Arduino Uno, upload it, and open the serial monitor at 9600 baud.

## Running the host-side test

From the repository root:

```bash
c++ -std=c++11 -Wall -Wextra -Werror \
  tests/test_braille_patterns.cpp \
  firmware/interactive_braille/BraillePatterns.cpp \
  -o /tmp/test_braille_patterns

/tmp/test_braille_patterns
```

