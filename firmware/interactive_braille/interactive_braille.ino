#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <Servo.h>
#include <SoftwareSerial.h>

#include "BraillePatterns.h"
#include "BrailleTrainer.h"
#include "HardwareConfig.h"

namespace
{
  constexpr unsigned long kServoStepDelayMs = 20;

  // MP3 track numbers in the /MP3 folder.
  constexpr uint16_t kTrackWelcome = 1;
  constexpr uint16_t kTrackLearningMode = 2;
  constexpr uint16_t kTrackPracticeMode = 3;
  constexpr uint16_t kTrackEntryCleared = 4;
  constexpr uint16_t kTrackCorrect = 5;
  constexpr uint16_t kTrackIncorrect = 6;
  constexpr uint16_t kTrackAnswerShown = 7;
  constexpr uint16_t kTrackClearBeforeChange = 8;
  constexpr uint16_t kTrackShowUnavailable = 9;
  constexpr uint16_t kTrackConfirmUnavailable = 10;
  constexpr uint16_t kFirstLearningLetterTrack = 11; // A=11, Z=36
  constexpr uint16_t kFirstPracticeLetterTrack = 37; // A=37, Z=62

  SoftwareSerial dfPlayerSerial(hardware::kDfPlayerRxPin,
                                hardware::kDfPlayerTxPin);
  DFRobotDFPlayerMini dfPlayer;
  bool audioReady = false;
  uint16_t pendingAudioTrack = 0;
  unsigned long pendingAudioAt = 0;
  enum class StartupAudioState
  {
    WaitingForLearningMode,
    WaitingForLetterA,
    Finished
  };

  StartupAudioState startupAudioState =
      StartupAudioState::WaitingForLearningMode;

  unsigned long startupAudioAt = 0;

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

  uint8_t currentLetterIndex()
  {
    return static_cast<uint8_t>(trainer.targetLetter() - 'A');
  }

  void playAudioTrack(uint16_t track)
  {
    pendingAudioTrack = 0;
    if (audioReady)
    {
      dfPlayer.playMp3Folder(track);
    }
  }

  void scheduleAudioTrack(uint16_t track, unsigned long delayMs)
  {
    if (!audioReady)
    {
      return;
    }

    pendingAudioTrack = track;
    pendingAudioAt = millis() + delayMs;
  }

  void serviceAudio()
  {
    if (pendingAudioTrack != 0 &&
        static_cast<long>(millis() - pendingAudioAt) >= 0)
    {
      const uint16_t track = pendingAudioTrack;
      pendingAudioTrack = 0;
      dfPlayer.playMp3Folder(track);
    }
  }

  void serviceStartupAudio()
  {
    if (!audioReady ||
        startupAudioState == StartupAudioState::Finished ||
        static_cast<long>(millis() - startupAudioAt) < 0)
    {
      return;
    }

    if (startupAudioState ==
        StartupAudioState::WaitingForLearningMode)
    {
      playAudioTrack(kTrackLearningMode); // 0002.mp3
      startupAudioState = StartupAudioState::WaitingForLetterA;
      startupAudioAt = millis() + 1300;
    }
    else if (startupAudioState ==
             StartupAudioState::WaitingForLetterA)
    {
      playAudioTrack(kFirstLearningLetterTrack); // 0011.mp3
      startupAudioState = StartupAudioState::Finished;
    }
  }

  void playLearningLetter()
  {
    playAudioTrack(kFirstLearningLetterTrack + currentLetterIndex());
  }

  void playPracticeTarget()
  {
    playAudioTrack(kFirstPracticeLetterTrack + currentLetterIndex());
  }

  void printRaisedDots(uint8_t pattern)
  {
    Serial.print(F("Raised dots: "));

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

  void raisePattern(uint8_t pattern)
  {
    const uint8_t newlyRaised =
        static_cast<uint8_t>(pattern & ~raisedHardwarePattern);

    for (uint8_t dot = 0; dot < hardware::kDotCount; ++dot)
    {
      if (braille::isDotRaised(newlyRaised, dot + 1))
      {
        dotServos[dot].write(hardware::kServoRaisedAngles[dot]);
        delay(kServoStepDelayMs);
      }
    }

    raisedHardwarePattern =
        static_cast<uint8_t>(raisedHardwarePattern | pattern);
  }

  void lowerAllDots()
  {
    for (uint8_t dot = 0; dot < hardware::kDotCount; ++dot)
    {
      if (braille::isDotRaised(raisedHardwarePattern, dot + 1))
      {
        dotServos[dot].write(hardware::kServoLoweredAngles[dot]);
        delay(kServoStepDelayMs);
      }
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
    playAudioTrack(kTrackClearBeforeChange);
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

  void announcePracticeTarget()
  {
    Serial.print(F("Practice target letter: "));
    Serial.println(trainer.targetLetter());
  }

  void changeMode()
  {
    Serial.println(F("Button: Mode"));

    lowerAllDots();
    resetFsrDetection();

    if (trainer.mode() == braille::Mode::Learning)
    {
      trainer.setMode(braille::Mode::Practice);

      // Create a different random sequence each time Practice mode begins.
      unsigned long newSeed = micros();

      for (uint8_t i = 0; i < hardware::kDotCount; ++i)
      {
        newSeed ^= static_cast<unsigned long>(
                       analogRead(hardware::kFsrPins[i]))
                   << (i + 1);
      }

      randomSeed(newSeed);
      trainer.setTargetLetter(randomDifferentLetter());

      Serial.println(F("Mode: Practice"));
      announcePracticeTarget();
      Serial.println(F("Pattern is hidden."));

      playAudioTrack(kTrackPracticeMode);
      scheduleAudioTrack(kFirstPracticeLetterTrack + currentLetterIndex(), 1300);
      return;
    }

    trainer.setMode(braille::Mode::Learning);
    trainer.setTargetLetter('A');
    Serial.println(F("Mode: Learning; reset to A."));
    showCurrentLetter();

    playAudioTrack(kTrackLearningMode);
    scheduleAudioTrack(kFirstLearningLetterTrack + currentLetterIndex(), 1300);
  }

  void confirmEntry()
  {
    Serial.println(F("Button: Confirm"));
    const braille::ConfirmResult result = trainer.confirm();

    if (result == braille::ConfirmResult::Correct)
    {
      Serial.println(F("Result: correct."));
      playAudioTrack(kTrackCorrect);
    }
    else if (result == braille::ConfirmResult::Incorrect)
    {
      Serial.println(F("Result: incorrect."));
      playAudioTrack(kTrackIncorrect);
    }
    else
    {
      Serial.println(F("Result: unavailable in the current state."));
      playAudioTrack(kTrackConfirmUnavailable);
    }
  }

  void clearCell()
  {
    Serial.println(F("Button: Clear"));
    trainer.clearEntry();
    lowerAllDots();
    resetFsrDetection();
    Serial.println(F("All dots lowered to 150 degrees."));
    playAudioTrack(kTrackEntryCleared);
  }

  void showAnswer()
  {
    Serial.println(F("Button: Show Answer"));

    if (trainer.mode() != braille::Mode::Practice)
    {
      Serial.println(F("Show Answer is only available in Practice mode."));
      playAudioTrack(kTrackShowUnavailable);
      return;
    }

    // Remove any entered dots before revealing the exact target pattern.
    lowerAllDots();
    resetFsrDetection();
    trainer.showAnswer();
    showCurrentLetter();
    playAudioTrack(kTrackAnswerShown);
  }

  void selectNextTarget()
  {
    Serial.println(F("Button: Next"));

    if (trainer.mode() == braille::Mode::Practice)
    {
      if (requireClearBeforeChange())
      {
        return;
      }

      trainer.setTargetLetter(randomDifferentLetter());

      Serial.println(F("Next random practice target selected."));
      announcePracticeTarget();
      Serial.println(F("Pattern is hidden."));
      playPracticeTarget();
    }
    else
    {
      // Learning mode: change immediately without pressing Clear
      lowerAllDots();
      resetFsrDetection();
      trainer.nextTarget();
      showCurrentLetter();
      playLearningLetter();
    }
  }

  void selectPreviousTarget()
  {
    Serial.println(F("Button: Previous"));

    if (trainer.mode() == braille::Mode::Practice)
    {
      if (requireClearBeforeChange())
      {
        return;
      }

      trainer.previousTarget();
      announcePracticeTarget();
      Serial.println(F("Previous practice target selected; pattern hidden."));
      playPracticeTarget();
    }
    else
    {
      // Learning mode: change immediately without pressing Clear
      lowerAllDots();
      resetFsrDetection();
      trainer.previousTarget();
      showCurrentLetter();
      playLearningLetter();
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
    // The installed buttons read low while idle and high while pressed.
    const uint8_t pressed =
        static_cast<uint8_t>(~previousState & currentState);

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
        if (reading < hardware::kFsrReleaseThreshold)
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

  dfPlayerSerial.begin(9600);
  if (dfPlayer.begin(dfPlayerSerial, true, true))
  {
    audioReady = true;
    dfPlayer.volume(hardware::kDfPlayerVolume);
    Serial.println(F("DFPlayer Mini ready."));
  }
  else
  {
    Serial.println(F("DFPlayer Mini unavailable; continuing without audio."));
  }

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
    dotServos[dot].write(hardware::kServoLoweredAngles[dot]);
    dotServos[dot].attach(hardware::kServoPins[dot]);
    delay(kServoStepDelayMs);
  }

  lowerAllDots();

  Serial.println(F("Braille interaction hardware test"));
  Serial.println(F("74HC165: D2=LOAD, D3=CLOCK, D4=QH DATA."));
  Serial.println(F("Buttons: 1=Mode, 2=Clear, 3=Previous,"));
  Serial.println(F("4=Show Answer, 5=Confirm, 6=Next. Active-high."));
  Serial.println(F("FSR dots 1-6: A0-A5; servo signals: D5-D10."));
  Serial.println(F("DFPlayer: D11=RX, D12=TX through 1k resistor."));
  Serial.println(F("Use a 10k pull-up on each input; tie unused G/H high."));

  showCurrentLetter();

  // Startup audio:
  // 0001: Welcome
  // 0002: Learning mode
  // 0011: Letter A
  playAudioTrack(kTrackWelcome);

  startupAudioState = StartupAudioState::WaitingForLearningMode;
  startupAudioAt = millis() + 2500;
}

void loop()
{
  processButtons();
  processFsrInputs();
  serviceAudio();
  serviceStartupAudio();
}
