#include <Arduino.h>
#include <string.h>

#include "BraillePatterns.h"
#include "BrailleTrainer.h"

namespace
{

  constexpr uint8_t kInputBufferSize = 48;

  braille::BrailleTrainer trainer;
  char inputBuffer[kInputBufferSize];
  uint8_t inputLength = 0;
  bool inputOverflow = false;
  bool randomSeeded = false;
  uint8_t lastHardwarePattern = braille::kInvalidPattern;

  // Hardware adapter points. Replace these no-op bodies after the pin map,
  // servo angles, FSR thresholds, button wiring, and audio files are fixed.
  void hardwareBegin()
  {
    // Initialize FSR inputs, the 74HC165, servos, audio, and haptic output.
  }

  void hardwareApplyDisplayedPattern(uint8_t pattern)
  {
    // Command each dot actuator from bits 0-5 of pattern.
    (void)pattern;
  }

  void hardwareOnConfirmResult(braille::ConfirmResult result)
  {
    // Map Correct/Incorrect/Unavailable to audio or haptic feedback.
    (void)result;
  }

  void hardwareUpdate()
  {
    // Advance any non-blocking servo, audio, or haptic activity.
  }

  void seedRandomOnce()
  {
    if (randomSeeded)
    {
      return;
    }

    // Practice begins after a human input, so micros() varies between runs.
    randomSeed(micros());
    randomSeeded = true;
  }

  char randomPracticeLetter()
  {
    seedRandomOnce();
    const uint8_t index =
        static_cast<uint8_t>(random(braille::kLetterCount));
    return static_cast<char>('A' + index);
  }

  char randomDifferentPracticeLetter()
  {
    seedRandomOnce();

    const uint8_t current =
        static_cast<uint8_t>(trainer.targetLetter() - 'A');
    uint8_t next =
        static_cast<uint8_t>(random(braille::kLetterCount - 1));

    if (next >= current)
    {
      ++next;
    }

    return static_cast<char>('A' + next);
  }

  void printSixBits(uint8_t pattern)
  {
    for (int8_t bit = 5; bit >= 0; --bit)
    {
      Serial.print((pattern >> bit) & 0x01);
    }
  }

  void printRaisedDots(uint8_t pattern)
  {
    bool found = false;
    for (uint8_t dot = 1; dot <= 6; ++dot)
    {
      if (braille::isDotRaised(pattern, dot))
      {
        if (found)
        {
          Serial.print(',');
        }
        Serial.print(dot);
        found = true;
      }
    }
    if (!found)
    {
      Serial.print(F("none"));
    }
  }

  void printCellRow(uint8_t pattern, uint8_t leftDot, uint8_t rightDot)
  {
    Serial.print(braille::isDotRaised(pattern, leftDot) ? 'O' : '.');
    Serial.print(' ');
    Serial.println(braille::isDotRaised(pattern, rightDot) ? 'O' : '.');
  }

  void printCell(uint8_t pattern)
  {
    printCellRow(pattern, 1, 4);
    printCellRow(pattern, 2, 5);
    printCellRow(pattern, 3, 6);
  }

  void printState()
  {
    const uint8_t pattern = trainer.displayedPattern();

    Serial.print(F("Mode: "));
    Serial.println(trainer.mode() == braille::Mode::Learning ? F("Learning")
                                                             : F("Practice"));
    Serial.print(F("Target: "));
    Serial.println(trainer.targetLetter());

    if (trainer.mode() == braille::Mode::Practice)
    {
      Serial.print(F("View: "));
      Serial.println(trainer.answerVisible() ? F("answer") : F("entry"));
    }

    Serial.print(F("Bits: "));
    printSixBits(pattern);
    Serial.print(F("  Raised dots: "));
    printRaisedDots(pattern);
    Serial.println();
    printCell(pattern);
  }

  void syncHardwareOutputs()
  {
    const uint8_t pattern = trainer.displayedPattern();
    if (pattern == lastHardwarePattern)
    {
      return;
    }

    hardwareApplyDisplayedPattern(pattern);
    lastHardwarePattern = pattern;
  }

  void publishState()
  {
    syncHardwareOutputs();
    printState();
  }

  void printAlphabet()
  {
    Serial.println(F("Letter  Bits    Raised dots"));
    for (uint8_t index = 0; index < braille::kLetterCount; ++index)
    {
      const uint8_t pattern = braille::patternForIndex(index);
      Serial.print(static_cast<char>('A' + index));
      Serial.print(F("       "));
      printSixBits(pattern);
      Serial.print(F("  "));
      printRaisedDots(pattern);
      Serial.println();
    }
  }

  void printHelp()
  {
    Serial.println(F("Commands:"));
    Serial.println(F("  A-Z              select target letter"));
    Serial.println(F("  learn [A-Z]      enter Learning Mode"));
    Serial.println(F("  practice [A-Z]   Practice; random if letter omitted"));
    Serial.println(F("  dot 1..6 or 1..6 toggle a practice dot"));
    Serial.println(F("  confirm          assess the entry"));
    Serial.println(F("  clear            clear entry"));
    Serial.println(F("  show             display the answer"));
    Serial.println(F("  next / previous  change target; Next random in Practice"));
    Serial.println(F("  status           display current state"));
    Serial.println(F("  alphabet         list all patterns"));
    Serial.println(F("  help             show commands"));
  }

  void lowercaseAscii(char *text)
  {
    while (*text != '\0')
    {
      if (*text >= 'A' && *text <= 'Z')
      {
        *text = static_cast<char>(*text - 'A' + 'a');
      }
      ++text;
    }
  }

  bool validLetter(const char *text)
  {
    return text != nullptr && text[0] != '\0' && text[1] == '\0' &&
           braille::patternForLetter(text[0]) != braille::kInvalidPattern;
  }

  bool performSetMode(braille::Mode mode, char targetLetter)
  {
    if (mode == braille::Mode::Practice && targetLetter == '\0')
    {
      targetLetter = randomPracticeLetter();
    }

    if (targetLetter != '\0' &&
        braille::patternForLetter(targetLetter) == braille::kInvalidPattern)
    {
      return false;
    }

    trainer.setMode(mode);
    if (targetLetter != '\0')
    {
      trainer.setTargetLetter(targetLetter);
    }
    publishState();
    return true;
  }

  bool performSelectTarget(char letter)
  {
    if (!trainer.setTargetLetter(letter))
    {
      return false;
    }

    publishState();
    return true;
  }

  bool performToggleDot(uint8_t dotNumber)
  {
    if (!trainer.toggleDot(dotNumber))
    {
      return false;
    }

    publishState();
    return true;
  }

  void performClear()
  {
    trainer.clearEntry();
    Serial.println(F("Entry cleared."));
    publishState();
  }

  void performShowAnswer()
  {
    trainer.showAnswer();
    Serial.println(F("Showing answer."));
    publishState();
  }

  void performNextTarget()
  {
    if (trainer.mode() == braille::Mode::Practice)
    {
      trainer.setTargetLetter(randomDifferentPracticeLetter());
    }
    else
    {
      trainer.nextTarget();
    }
    publishState();
  }

  void performPreviousTarget()
  {
    trainer.previousTarget();
    publishState();
  }

  void handleMode(braille::Mode mode, const char *letter)
  {
    if (letter != nullptr && !validLetter(letter))
    {
      Serial.println(F("Error: expected one letter from A to Z."));
      return;
    }

    performSetMode(mode, letter == nullptr ? '\0' : letter[0]);
  }

  void handleDot(const char *text)
  {
    if (text == nullptr || text[0] < '1' || text[0] > '6' ||
        text[1] != '\0')
    {
      Serial.println(F("Error: dot number must be 1 to 6."));
      return;
    }
    if (trainer.mode() != braille::Mode::Practice)
    {
      Serial.println(F("Error: dots can only change in Practice Mode."));
      return;
    }
    if (trainer.answerVisible())
    {
      Serial.println(F("Error: clear the answer before entering dots."));
      return;
    }

    performToggleDot(static_cast<uint8_t>(text[0] - '0'));
  }

  void handleConfirm()
  {
    const braille::ConfirmResult result = trainer.confirm();
    hardwareOnConfirmResult(result);

    switch (result)
    {
    case braille::ConfirmResult::Correct:
      Serial.println(F("Correct."));
      break;
    case braille::ConfirmResult::Incorrect:
      Serial.println(F("Incorrect. Try again."));
      break;
    case braille::ConfirmResult::Unavailable:
      Serial.println(F("Error: Confirm requires an editable Practice entry."));
      return;
    }
    publishState();
  }

  void processCommand(char *line)
  {
    char *command = strtok(line, " \t");
    char *argument = strtok(nullptr, " \t");
    if (command == nullptr)
    {
      return;
    }
    lowercaseAscii(command);

    if (command[1] == '\0' && command[0] >= 'a' && command[0] <= 'z')
    {
      performSelectTarget(command[0]);
    }
    else if (command[1] == '\0' && command[0] >= '1' && command[0] <= '6')
    {
      handleDot(command);
    }
    else if (strcmp(command, "learn") == 0)
    {
      handleMode(braille::Mode::Learning, argument);
    }
    else if (strcmp(command, "practice") == 0)
    {
      handleMode(braille::Mode::Practice, argument);
    }
    else if (strcmp(command, "dot") == 0)
    {
      handleDot(argument);
    }
    else if (strcmp(command, "confirm") == 0)
    {
      handleConfirm();
    }
    else if (strcmp(command, "clear") == 0)
    {
      performClear();
    }
    else if (strcmp(command, "show") == 0)
    {
      performShowAnswer();
    }
    else if (strcmp(command, "next") == 0)
    {
      performNextTarget();
    }
    else if (strcmp(command, "previous") == 0 || strcmp(command, "prev") == 0)
    {
      performPreviousTarget();
    }
    else if (strcmp(command, "status") == 0)
    {
      printState();
    }
    else if (strcmp(command, "alphabet") == 0)
    {
      printAlphabet();
    }
    else if (strcmp(command, "help") == 0)
    {
      printHelp();
    }
    else
    {
      Serial.println(F("Unknown command. Enter 'help'."));
    }
  }

  void processHardwareInput()
  {
  }

  void processSerialInput()
  {
    while (Serial.available() > 0)
    {
      const char received = static_cast<char>(Serial.read());
      if (received == '\r')
      {
        continue;
      }
      if (received == '\n')
      {
        if (inputOverflow)
        {
          Serial.println(F("Error: command is too long."));
        }
        else
        {
          inputBuffer[inputLength] = '\0';
          processCommand(inputBuffer);
        }
        inputLength = 0;
        inputOverflow = false;
        Serial.print(F("> "));
      }
      else if (inputLength < kInputBufferSize - 1)
      {
        inputBuffer[inputLength++] = received;
      }
      else
      {
        inputOverflow = true;
      }
    }
  }

} // namespace

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    // The Arduino Uno continues immediately.
  }

  hardwareBegin();

  Serial.println(F("Interactive Braille software prototype"));
  Serial.println(F("Raised dot = O, lowered dot = ."));
  printHelp();
  Serial.println();
  publishState();
  Serial.print(F("> "));
}

void loop()
{
  processSerialInput();
  processHardwareInput();
  syncHardwareOutputs();
  hardwareUpdate();
}
