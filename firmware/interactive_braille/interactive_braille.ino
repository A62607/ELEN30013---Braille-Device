#include <Arduino.h>
#include <string.h>

#include "BraillePatterns.h"
#include "BrailleTrainer.h"

namespace {

constexpr uint8_t kInputBufferSize = 48;

braille::BrailleTrainer trainer;
char inputBuffer[kInputBufferSize];
uint8_t inputLength = 0;
bool inputOverflow = false;

void printSixBits(uint8_t pattern) {
  for (int8_t bit = 5; bit >= 0; --bit) {
    Serial.print((pattern >> bit) & 0x01);
  }
}

void printRaisedDots(uint8_t pattern) {
  bool found = false;
  for (uint8_t dot = 1; dot <= 6; ++dot) {
    if (braille::isDotRaised(pattern, dot)) {
      if (found) {
        Serial.print(',');
      }
      Serial.print(dot);
      found = true;
    }
  }
  if (!found) {
    Serial.print(F("none"));
  }
}

void printCellRow(uint8_t pattern, uint8_t leftDot, uint8_t rightDot) {
  Serial.print(braille::isDotRaised(pattern, leftDot) ? 'O' : '.');
  Serial.print(' ');
  Serial.println(braille::isDotRaised(pattern, rightDot) ? 'O' : '.');
}

void printCell(uint8_t pattern) {
  printCellRow(pattern, 1, 4);
  printCellRow(pattern, 2, 5);
  printCellRow(pattern, 3, 6);
}

void printState() {
  const uint8_t pattern = trainer.displayedPattern();

  Serial.print(F("Mode: "));
  Serial.println(trainer.mode() == braille::Mode::Learning ? F("Learning")
                                                           : F("Practice"));
  Serial.print(F("Target: "));
  Serial.println(trainer.targetLetter());

  if (trainer.mode() == braille::Mode::Practice) {
    Serial.print(F("View: "));
    Serial.println(trainer.answerVisible() ? F("answer") : F("entry"));
    Serial.print(F("Incorrect attempts: "));
    Serial.println(trainer.incorrectAttempts());
  }

  Serial.print(F("Bits: "));
  printSixBits(pattern);
  Serial.print(F("  Raised dots: "));
  printRaisedDots(pattern);
  Serial.println();
  printCell(pattern);
}

void printAlphabet() {
  Serial.println(F("Letter  Bits    Raised dots"));
  for (uint8_t index = 0; index < braille::kLetterCount; ++index) {
    const uint8_t pattern = braille::patternForIndex(index);
    Serial.print(static_cast<char>('A' + index));
    Serial.print(F("       "));
    printSixBits(pattern);
    Serial.print(F("  "));
    printRaisedDots(pattern);
    Serial.println();
  }
}

void printHelp() {
  Serial.println(F("Commands:"));
  Serial.println(F("  A-Z              select target letter"));
  Serial.println(F("  learn [A-Z]      enter Learning Mode"));
  Serial.println(F("  practice [A-Z]   enter Practice Mode"));
  Serial.println(F("  dot 1..6 or 1..6 toggle a practice dot"));
  Serial.println(F("  confirm          assess the entry"));
  Serial.println(F("  clear            clear entry and attempts"));
  Serial.println(F("  show             display the answer"));
  Serial.println(F("  next / previous  change target letter"));
  Serial.println(F("  status           display current state"));
  Serial.println(F("  alphabet         list all patterns"));
  Serial.println(F("  help             show commands"));
}

void lowercaseAscii(char* text) {
  while (*text != '\0') {
    if (*text >= 'A' && *text <= 'Z') {
      *text = static_cast<char>(*text - 'A' + 'a');
    }
    ++text;
  }
}

bool validLetter(const char* text) {
  return text != nullptr && text[0] != '\0' && text[1] == '\0' &&
         braille::patternForLetter(text[0]) != braille::kInvalidPattern;
}

void handleMode(braille::Mode mode, const char* letter) {
  if (letter != nullptr && !validLetter(letter)) {
    Serial.println(F("Error: expected one letter from A to Z."));
    return;
  }

  trainer.setMode(mode);
  if (letter != nullptr) {
    trainer.setTargetLetter(letter[0]);
  }
  printState();
}

void handleDot(const char* text) {
  if (text == nullptr || text[0] < '1' || text[0] > '6' ||
      text[1] != '\0') {
    Serial.println(F("Error: dot number must be 1 to 6."));
    return;
  }
  if (trainer.mode() != braille::Mode::Practice) {
    Serial.println(F("Error: dots can only change in Practice Mode."));
    return;
  }
  if (trainer.answerVisible()) {
    Serial.println(F("Error: clear the answer before entering dots."));
    return;
  }

  trainer.toggleDot(static_cast<uint8_t>(text[0] - '0'));
  printState();
}

void handleConfirm() {
  switch (trainer.confirm()) {
    case braille::ConfirmResult::Correct:
      Serial.println(F("Correct."));
      break;
    case braille::ConfirmResult::Incorrect:
      Serial.println(F("Incorrect. Try again."));
      break;
    case braille::ConfirmResult::AnswerRevealed:
      Serial.println(F("Incorrect three times. Showing the answer."));
      break;
    case braille::ConfirmResult::Unavailable:
      Serial.println(F("Error: Confirm requires an editable Practice entry."));
      return;
  }
  printState();
}

void processCommand(char* line) {
  char* command = strtok(line, " \t");
  char* argument = strtok(nullptr, " \t");
  if (command == nullptr) {
    return;
  }
  lowercaseAscii(command);

  if (command[1] == '\0' && command[0] >= 'a' && command[0] <= 'z') {
    trainer.setTargetLetter(command[0]);
    printState();
  } else if (command[1] == '\0' && command[0] >= '1' && command[0] <= '6') {
    handleDot(command);
  } else if (strcmp(command, "learn") == 0) {
    handleMode(braille::Mode::Learning, argument);
  } else if (strcmp(command, "practice") == 0) {
    handleMode(braille::Mode::Practice, argument);
  } else if (strcmp(command, "dot") == 0) {
    handleDot(argument);
  } else if (strcmp(command, "confirm") == 0) {
    handleConfirm();
  } else if (strcmp(command, "clear") == 0) {
    trainer.clearEntry();
    Serial.println(F("Entry and attempt count cleared."));
    printState();
  } else if (strcmp(command, "show") == 0) {
    trainer.showAnswer();
    Serial.println(F("Showing answer."));
    printState();
  } else if (strcmp(command, "next") == 0) {
    trainer.nextTarget();
    printState();
  } else if (strcmp(command, "previous") == 0 || strcmp(command, "prev") == 0) {
    trainer.previousTarget();
    printState();
  } else if (strcmp(command, "status") == 0) {
    printState();
  } else if (strcmp(command, "alphabet") == 0) {
    printAlphabet();
  } else if (strcmp(command, "help") == 0) {
    printHelp();
  } else {
    Serial.println(F("Unknown command. Enter 'help'."));
  }
}

void processSerialInput() {
  while (Serial.available() > 0) {
    const char received = static_cast<char>(Serial.read());
    if (received == '\r') {
      continue;
    }
    if (received == '\n') {
      if (inputOverflow) {
        Serial.println(F("Error: command is too long."));
      } else {
        inputBuffer[inputLength] = '\0';
        processCommand(inputBuffer);
      }
      inputLength = 0;
      inputOverflow = false;
      Serial.print(F("> "));
    } else if (inputLength < kInputBufferSize - 1) {
      inputBuffer[inputLength++] = received;
    } else {
      inputOverflow = true;
    }
  }
}

}  // namespace

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    // The Arduino Uno continues immediately.
  }

  Serial.println(F("Interactive Braille software prototype"));
  Serial.println(F("Raised dot = O, lowered dot = ."));
  printHelp();
  Serial.println();
  printState();
  Serial.print(F("> "));
}

void loop() {
  processSerialInput();
}
