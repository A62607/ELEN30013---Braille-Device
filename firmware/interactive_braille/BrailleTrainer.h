#pragma once

#include <stdint.h>

namespace braille
{

  enum class Mode : uint8_t
  {
    Learning,
    Practice,
  };

  enum class ConfirmResult : uint8_t
  {
    Unavailable,
    Correct,
    Incorrect,
  };

  // Hardware-independent state and rules for the two learning activities.
  class BrailleTrainer
  {
  public:
    BrailleTrainer();

    Mode mode() const;
    void setMode(Mode newMode);

    char targetLetter() const;
    uint8_t targetPattern() const;
    bool setTargetLetter(char letter);
    void nextTarget();
    void previousTarget();

    uint8_t entryPattern() const;
    uint8_t displayedPattern() const;
    bool toggleDot(uint8_t dotNumber);
    void clearEntry();

    ConfirmResult confirm();
    void showAnswer();
    bool answerVisible() const;

  private:
    void resetPracticeSession();

    Mode mode_;
    uint8_t targetIndex_;
    uint8_t entryPattern_;
    bool answerVisible_;
  };

} // namespace braille
