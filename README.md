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

## Repository status

The latest working-tree changes remove the standalone `firmware/BrailleLearningTest/`
sketch and its ZIP archive, and add
`firmware/interactive_braille/BrailleLearningWithAudio_final_code.ino`.
The older Serial-controlled sketch and its shared support files remain.

**The Arduino sketch folder is not currently ready to compile or upload:**

- Both `.ino` files define `setup()`, `loop()`, and overlapping globals/functions.
  Arduino builds all `.ino` files in a sketch folder together. Select one entry
  point and move the other out of that folder, or give each sketch its own folder
  with a matching primary `.ino` filename and its support files.
- The audio sketch expects settings absent from the current `HardwareConfig.h`:
  `kDfPlayerRxPin`, `kDfPlayerTxPin`, `kDfPlayerVolume`,
  `kServoRaisedAngles`, `kServoLoweredAngles`, `kFsrPressThresholds`, and
  the six button masks (`kShowButtonMask`, `kPreviousButtonMask`,
  `kNextButtonMask`, `kConfirmButtonMask`, `kClearButtonMask`, `kModeButtonMask`).
  Supply a configuration matched to the actual wiring and calibrated mechanism
  before building it.
- The audio sketch's startup messages describe servo pins D5-D10 and 74HC165
  pins D2-D4, while the remaining header assigns servos to D2-D7 and the
  74HC165 to D8-D10. Its active-high button handling also conflicts with its
  printed pull-up guidance. Treat startup messages as provisional, not a wiring
  specification.

## Audio and physical-button sketch

`BrailleLearningWithAudio_final_code.ino` adds DFPlayer Mini audio and implements
six physical-button actions. The behavior below describes the source; it has
not been verified on hardware as part of this review.

| Control | Behavior |
| --- | --- |
| Startup | Enter Learning mode, display A, and schedule welcome, mode, and A audio. |
| Mode | Lower the cell and switch modes. Practice selects a different random letter; Learning resets to A. |
| Next / Previous in Learning | Lower the old pattern and immediately display the next/previous letter, wrapping around A-Z. |
| Next in Practice | Require a cleared physical cell, then choose a different random target. |
| Previous in Practice | Require a cleared physical cell, then select the previous alphabetical target. |
| FSR press | Raise the corresponding practice dot after the configured press duration; ignored in Learning or while the answer is visible. |
| Confirm | Announce correct, incorrect, or unavailable. Does not advance the target. |
| Clear | Clear the entry, lower the cell, and reset FSR detection. |
| Show Answer | In Practice, lower the entered pattern and raise the exact answer; further FSR entry and confirmation require Clear first. |

Buttons use debounced rising edges (active-high). Servo movements are staggered
by 20 ms per changed dot. Serial at 9600 baud provides diagnostics; this sketch
does not implement the older sketch's Serial command parser. If DFPlayer
initialization fails, interaction continues without audio.

### Audio dependencies and tracks

The audio sketch includes `Servo`, `SoftwareSerial`, and `DFRobotDFPlayerMini`
in addition to the Arduino core. Provide these libraries and resolve the sketch
and configuration issues above before uploading.

Prepare the microSD card with an `/MP3` folder and four-digit filenames:

| File(s) | Audio |
| --- | --- |
| `0001.mp3` | Welcome |
| `0002.mp3` / `0003.mp3` | Learning / Practice mode |
| `0004.mp3` | Entry cleared |
| `0005.mp3` / `0006.mp3` | Correct / incorrect |
| `0007.mp3` | Answer shown |
| `0008.mp3` | Clear before changing target |
| `0009.mp3` / `0010.mp3` | Show Answer / Confirm unavailable |
| `0011.mp3`–`0036.mp3` | Learning letters A-Z |
| `0037.mp3`–`0062.mp3` | Practice targets A-Z |

Audio assets are not included in this repository. Startup announcements use
fixed delays (2500 ms, then 1300 ms); mode changes schedule their target after
1300 ms. These are timer-based, not playback-completion checks. The startup
sequence is not cancelled by button interactions, so early interaction may
still be followed by the startup Learning/A announcement.

## Original Serial-controlled prototype

The retained `interactive_braille.ino` provides:

- Grade 1 English Braille patterns for A-Z.
- Learning and Practice modes.
- Target-letter selection and wrap-around navigation.
- Random initial and Next targets in Practice Mode.
- A logical six-dot practice entry in which dots can only be raised.
- Correct/incorrect assessment.
- Clear and manually controlled Show Answer operations.
- A non-blocking Serial command interface.

The state and learning rules are independent from Arduino hardware in
`BrailleTrainer`. The shared patterns and trainer can be tested on a computer independently of
the Arduino sketches. The original sketch drives servos and reads FSRs, but its
physical-button action mapping remains a placeholder.

## Running the Arduino prototype

After isolating the original sketch from the audio `.ino` as described above,
open `interactive_braille.ino` in the Arduino IDE, select an Arduino Uno,
and verify the hardware configuration before uploading. Open the Serial Monitor
at 9600 baud.
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

## Original sketch hardware configuration

Hardware values and pin assignments are kept in
`firmware/interactive_braille/HardwareConfig.h`. The remaining header contains
the original sketch configuration, not a complete audio-sketch configuration:

- FSR inputs: A0-A5, pressed at an ADC value of 300 or higher for 50 ms.
- Servo signals: digital pins 2-7.
- 74HC165 load, clock, and data: digital pins 8, 9, and 10.
- Functional-button debounce: 30 ms.
- Servo positions: 150 degrees lowered and 20 degrees raised.

Normal input only raises dots. Pressing Clear lowers all six servos and clears
the logical entry; no other action lowers an individual dot. Clear the cell
before moving to a different target so previously raised physical dots do not
remain in the next pattern.

The 74HC165 input is sampled and debounced, but the bit-to-button mapping and
active level still need to be matched to the final wiring before the physical
buttons can invoke Learning, Practice, Confirm, Clear, Show Answer, or Next.

## Running the host-side tests

These tests cover the shared Braille patterns and trainer logic, not Arduino
sketch integration, button wiring, FSR calibration, servo movement, or audio.
Both test suites passed during the source review on 28 October 2026. An Arduino
build and physical-device test were not performed.

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
