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

## Current software prototype

The prototype currently provides:

- Grade 1 English Braille patterns for A-Z.
- Learning and Practice modes.
- Target-letter selection and wrap-around navigation.
- Random initial and Next targets in Practice Mode.
- A logical six-dot practice entry in which dots can only be raised.
- Correct/incorrect assessment.
- Clear and manually controlled Show Answer operations.
- A non-blocking Serial command interface.

The state and learning rules are independent from Arduino hardware in
`BrailleTrainer`. They can be tested on a computer now and connected to
buttons, sensors, servos, and audio later.

## Running the Arduino prototype

Open `firmware/interactive_braille/interactive_braille.ino` in the Arduino
IDE, select an Arduino Uno, upload it, and open the Serial Monitor at 9600 baud.
Set the line ending to **Newline** or **Both NL & CR**.

Available commands:

```text
A-Z              select target letter
learn [A-Z]      enter Learning Mode
practice [A-Z]   enter Practice; random if letter omitted
dot 1..6         raise a practice dot
1..6             shortcut for dot 1..6
confirm          assess the entry
clear            clear entry
show             display the answer
next / previous  change target; Next random in Practice
status           display current state
alphabet         list all patterns
help             show commands
```

Example practice session for the letter C:

```text
practice C
1
4
confirm
```

The logical cell uses `O` for a raised dot and `.` for a lowered dot:

```text
O O
. .
. .
```

## Hardware configuration

Hardware values and pin assignments are kept in
`firmware/interactive_braille/HardwareConfig.h`. The current configuration is:

- FSR inputs: A0-A5, pressed at an ADC value of 300 or higher for 50 ms.
- Servo signals: digital pins 2-7.
- 74HC165 load, clock, and data: digital pins 8, 9, and 10.
- Functional-button debounce: 30 ms.
- Servo positions: 10 degrees lowered and 150 degrees raised.

Normal input only raises dots. Pressing Clear lowers all six servos and clears
the logical entry; no other action lowers an individual dot. Clear the cell
before moving to a different target so previously raised physical dots do not
remain in the next pattern.

The 74HC165 input is sampled and debounced, but the bit-to-button mapping and
active level still need to be matched to the final wiring before the physical
buttons can invoke Learning, Practice, Confirm, Clear, Show Answer, or Next.

## Running the host-side tests

From the repository root:

```bash
c++ -std=c++11 -Wall -Wextra -Werror \
  tests/test_braille_patterns.cpp \
  firmware/interactive_braille/BraillePatterns.cpp \
  -o /tmp/test_braille_patterns

/tmp/test_braille_patterns

c++ -std=c++11 -Wall -Wextra -Werror \
  tests/test_braille_trainer.cpp \
  firmware/interactive_braille/BraillePatterns.cpp \
  firmware/interactive_braille/BrailleTrainer.cpp \
  -o /tmp/test_braille_trainer

/tmp/test_braille_trainer
```
