#include <Arduino.h>
#include <Servo.h>

#include "BraillePatterns.h"
#include "BrailleTrainer.h"
#include "HardwareConfig.h"

namespace
{
  constexpr unsigned long kServoStepDelayMs = 80;

  braille::BrailleTrainer trainer;
  Servo dotServos[hardware::kDotCount];
  uint8_t raisedHardwarePattern = 0;

  bool buttonSampleInitialized = false;
  uint8_t lastRawButtonState = 0xFF;
  uint8_t debouncedButtonState = 0xFF;
  unsigned long buttonStateChangedAt = 0;

  unsigned long fsrPressStartedAt[hardware::kDotCount] = {};
  bool fsrTimingPress[hardware::kDotCount] = {};
  bool fsrPressLatched[hardware::kDotCount] = {};

  void printDotNumbers(uint8_t pattern)
  {
    bool printedDot = false;
    for (uint8_t dot = 0; dot < hardware::kDotCount; ++dot)
    {
      if (!braille::isDotRaised(pattern, dot + 1))
      {
        continue;
      }

      if (printedDot)
      {
        Serial.print(',');
      }

      Serial.print(dot + 1);
      printedDot = true;
    }

    Serial.println();
  }

  void printRaisedDots(uint8_t pattern)
  {
    Serial.print(F("Raised dots: "));
    printDotNumbers(pattern);
  }

  void printPracticeTargetForTest()
  {
    Serial.print(F("Practice test target: "));
    Serial.println(trainer.targetLetter());
    Serial.print(F("Expected dots: "));
    printDotNumbers(trainer.targetPattern());
  }

  void raisePattern(uint8_t pattern)
  {
    const uint8_t newlyRaised =
        static_cast<uint8_t>(pattern & ~raisedHardwarePattern);

    for (uint8_t dot = 0; dot < hardware::kDotCount; ++dot)
    {
      if (braille::isDotRaised(newlyRaised, dot + 1))
      {
        dotServos[dot].write(hardware::kServoRaisedAngle);
        delay(kServoStepDelayMs);
      }
    }

    raisedHardwarePattern =
        static_cast<uint8_t>(raisedHardwarePattern | pattern);
  }

  void lowerAllDots()
  {
    // Called only by Clear, startup, and an explicit mode change.
    for (uint8_t dot = 0; dot < hardware::kDotCount; ++dot)
    {
      dotServos[dot].write(hardware::kServoLoweredAngle);
      delay(kServoStepDelayMs);
    }

    raisedHardwarePattern = 0;
  }

  void showCurrentLetter()
  {
    const char letter = trainer.targetLetter();
    const uint8_t pattern = trainer.displayedPattern();

    Serial.print(F("Displaying letter "));
    Serial.println(letter);
    printRaisedDots(pattern);

    raisePattern(pattern);
  }

  bool requireClearBeforeChange()
  {
    if (raisedHardwarePattern == 0)
    {
      return false;
    }

    Serial.println(F("Press Clear before changing mode or target."));
    return true;
  }

  char randomDifferentLetter()
  {
    const uint8_t current =
        static_cast<uint8_t>(trainer.targetLetter() - 'A');
    uint8_t next = static_cast<uint8_t>(random(braille::kLetterCount - 1));
    if (next >= current)
    {
      ++next;
    }
    return static_cast<char>('A' + next);
  }

  void resetFsrDetection()
  {
    for (uint8_t dot = 0; dot < hardware::kDotCount; ++dot)
    {
      fsrPressStartedAt[dot] = 0;
      fsrTimingPress[dot] = false;
      fsrPressLatched[dot] = false;
    }
  }

  void changeMode()
  {
    Serial.println(F("Button: Mode"));

    // Changing mode is the second explicit operation, after Clear, that is
    // allowed to lower every physical dot.
    lowerAllDots();
    resetFsrDetection();

    if (trainer.mode() == braille::Mode::Learning)
    {
      trainer.setMode(braille::Mode::Practice);
      trainer.setTargetLetter(randomDifferentLetter());
      Serial.println(F("Mode: Practice"));
      printPracticeTargetForTest();
      Serial.println(F("Target remains physically hidden."));
      return;
    }

    trainer.setMode(braille::Mode::Learning);
    trainer.setTargetLetter('A');
    Serial.println(F("Mode: Learning; reset to A."));
    showCurrentLetter();
  }

  void confirmEntry()
  {
    Serial.println(F("Button: Confirm"));
    const braille::ConfirmResult result = trainer.confirm();

    if (result == braille::ConfirmResult::Correct)
    {
      Serial.println(F("Result: correct."));
    }
    else if (result == braille::ConfirmResult::Incorrect)
    {
      Serial.println(F("Result: incorrect."));
    }
    else
    {
      Serial.println(F("Result: unavailable in the current state."));
    }
  }

  void clearCell()
  {
    Serial.println(F("Button: Clear"));
    trainer.clearEntry();
    lowerAllDots();
    resetFsrDetection();
    Serial.println(F("All dots lowered to 150 degrees."));
  }

  void showAnswer()
  {
    Serial.println(F("Button: Show Answer"));
    trainer.showAnswer();
    showCurrentLetter();
  }

  void selectNextTarget()
  {
    Serial.println(F("Button: Next"));
    if (requireClearBeforeChange())
    {
      return;
    }

    if (trainer.mode() == braille::Mode::Practice)
    {
      trainer.setTargetLetter(randomDifferentLetter());
      printPracticeTargetForTest();
      Serial.println(F("Target remains physically hidden."));
    }
    else
    {
      trainer.nextTarget();
      showCurrentLetter();
    }
  }

  void selectPreviousTarget()
  {
    Serial.println(F("Button: Previous"));
    if (requireClearBeforeChange())
    {
      return;
    }

    trainer.previousTarget();
    if (trainer.mode() == braille::Mode::Learning)
    {
      showCurrentLetter();
    }
    else
    {
      printPracticeTargetForTest();
      Serial.println(F("Target remains physically hidden."));
    }
  }

  uint8_t readButtonRegister()
  {
    digitalWrite(hardware::kButtonLoadPin, LOW);
    delayMicroseconds(5);
    digitalWrite(hardware::kButtonLoadPin, HIGH);
    delayMicroseconds(5);

    return shiftIn(hardware::kButtonDataPin,
                   hardware::kButtonClockPin,
                   MSBFIRST);
  }

  void handleButtonPresses(uint8_t previousState, uint8_t currentState)
  {
    // Each input is pulled high and becomes low while its button is pressed.
    const uint8_t pressed =
        static_cast<uint8_t>(previousState & ~currentState);

    if ((pressed & hardware::kShowButtonMask) != 0)
    {
      showAnswer();
    }
    if ((pressed & hardware::kPreviousButtonMask) != 0)
    {
      selectPreviousTarget();
    }
    if ((pressed & hardware::kNextButtonMask) != 0)
    {
      selectNextTarget();
    }
    if ((pressed & hardware::kConfirmButtonMask) != 0)
    {
      confirmEntry();
    }
    if ((pressed & hardware::kClearButtonMask) != 0)
    {
      clearCell();
    }
    if ((pressed & hardware::kModeButtonMask) != 0)
    {
      changeMode();
    }
  }

  void processButtons()
  {
    const unsigned long now = millis();
    const uint8_t rawState = readButtonRegister();

    if (!buttonSampleInitialized)
    {
      buttonSampleInitialized = true;
      lastRawButtonState = rawState;
      debouncedButtonState = rawState;
      buttonStateChangedAt = now;
      return;
    }

    if (rawState != lastRawButtonState)
    {
      lastRawButtonState = rawState;
      buttonStateChangedAt = now;
    }
    else if (rawState != debouncedButtonState &&
             now - buttonStateChangedAt >= hardware::kButtonDebounceMs)
    {
      const uint8_t previousState = debouncedButtonState;
      debouncedButtonState = rawState;
      handleButtonPresses(previousState, debouncedButtonState);
    }
  }

  void processFsrInputs()
  {
    if (trainer.mode() != braille::Mode::Practice ||
        trainer.answerVisible())
    {
      return;
    }

    const unsigned long now = millis();
    for (uint8_t dot = 0; dot < hardware::kDotCount; ++dot)
    {
      const uint16_t reading = analogRead(hardware::kFsrPins[dot]);

      if (fsrPressLatched[dot])
      {
        if (reading < hardware::kFsrReleaseThresholds[dot])
        {
          fsrPressLatched[dot] = false;
          fsrTimingPress[dot] = false;
        }
        continue;
      }

      if (reading >= hardware::kFsrPressThresholds[dot])
      {
        if (!fsrTimingPress[dot])
        {
          fsrTimingPress[dot] = true;
          fsrPressStartedAt[dot] = now;
        }
        else if (now - fsrPressStartedAt[dot] >=
                 hardware::kFsrPressDurationMs)
        {
          fsrTimingPress[dot] = false;
          fsrPressLatched[dot] = true;

          if (trainer.raiseDot(dot + 1))
          {
            Serial.print(F("FSR input: dot "));
            Serial.println(dot + 1);
            printRaisedDots(trainer.entryPattern());
            raisePattern(trainer.entryPattern());
          }
        }
      }
      else
      {
        fsrTimingPress[dot] = false;
      }
    }
  }

} // namespace

void setup()
{
  Serial.begin(9600);
  randomSeed(micros());

  trainer.setMode(braille::Mode::Learning);
  trainer.setTargetLetter('A');

  pinMode(hardware::kButtonLoadPin, OUTPUT);
  pinMode(hardware::kButtonClockPin, OUTPUT);
  pinMode(hardware::kButtonDataPin, INPUT);
  digitalWrite(hardware::kButtonLoadPin, HIGH);
  digitalWrite(hardware::kButtonClockPin, LOW);

  for (uint8_t dot = 0; dot < hardware::kDotCount; ++dot)
  {
    pinMode(hardware::kFsrPins[dot], INPUT);

    // Set the required starting angle before attachment to reduce startup
    // movement towards the Servo library's default position.
    dotServos[dot].write(hardware::kServoLoweredAngle);
    dotServos[dot].attach(hardware::kServoPins[dot]);
    delay(kServoStepDelayMs);
  }

  lowerAllDots();

  Serial.println(F("Braille interaction hardware test"));
  Serial.println(F("74HC165: D2=LOAD, D3=CLOCK, D4=QH DATA."));
  Serial.println(F("A=Show, B=Previous, C=Next, D=Confirm,"));
  Serial.println(F("E=Clear, F=Mode. Buttons are active-low."));
  Serial.println(F("FSR dots 1-6: A0-A5; servo signals: D5-D10."));
  Serial.println(F("Use a 10k pull-up on each input; tie unused G/H high."));

  showCurrentLetter();
}

void loop()
{
  processButtons();
  processFsrInputs();
}
