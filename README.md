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
- A logical six-dot practice entry with individual dot toggling.
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
practice [A-Z]   enter Practice Mode
dot 1..6         toggle a practice dot
1..6             shortcut for dot 1..6
confirm          assess the entry
clear            clear entry and attempts
show             display the answer
next / previous  change target letter
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
