#include "BrailleTrainer.h"

#include "BraillePatterns.h"

namespace braille
{
  // constructor
  BrailleTrainer::BrailleTrainer()
      : mode_(Mode::Learning),
        targetIndex_(0),
        entryPattern_(0),
        answerVisible_(false) {}

  Mode BrailleTrainer::mode() const
  {
    return mode_;
  }

  // set mode
  void BrailleTrainer::setMode(Mode newMode)
  {
    mode_ = newMode;
    resetPracticeSession();
  }

  // get target letter
  char BrailleTrainer::targetLetter() const
  {
    return static_cast<char>('A' + targetIndex_);
  }

  // get targe pattern
  uint8_t BrailleTrainer::targetPattern() const
  {
    return patternForIndex(targetIndex_);
  }

  // set target letter
  bool BrailleTrainer::setTargetLetter(char letter)
  {
    if (letter >= 'a' && letter <= 'z')
    {
      letter = static_cast<char>(letter - 'a' + 'A');
    }

    if (patternForLetter(letter) == kInvalidPattern)
    {
      return false;
    }

    targetIndex_ = static_cast<uint8_t>(letter - 'A');
    resetPracticeSession();
    return true;
  }

  void BrailleTrainer::nextTarget()
  {
    targetIndex_ = static_cast<uint8_t>((targetIndex_ + 1) % kLetterCount);
    resetPracticeSession();
  }

  void BrailleTrainer::previousTarget()
  {
    targetIndex_ =
        targetIndex_ == 0 ? kLetterCount - 1 : targetIndex_ - 1;
    resetPracticeSession();
  }

  uint8_t BrailleTrainer::entryPattern() const
  {
    return entryPattern_;
  }

  uint8_t BrailleTrainer::displayedPattern() const
  {
    if (mode_ == Mode::Learning || answerVisible_)
    {
      return targetPattern();
    }

    return entryPattern_;
  }

  // updates user input pattern
  bool BrailleTrainer::raiseDot(uint8_t dotNumber)
  {
    // whether input is allowed
    if (mode_ != Mode::Practice || answerVisible_ || dotNumber < 1 ||
        dotNumber > 6)
    {
      return false;
    }

    // Create a bit representing that dot
    const uint8_t dotMask = uint8_t{1} << (dotNumber - 1);

    // whether that dot is already raised
    if ((entryPattern_ & dotMask) != 0)
    {
      return false;
    }

    entryPattern_ = static_cast<uint8_t>(entryPattern_ | dotMask);
    return true;
  }

  void BrailleTrainer::clearEntry()
  {
    resetPracticeSession();
  }

  ConfirmResult BrailleTrainer::confirm()
  {
    if (mode_ != Mode::Practice || answerVisible_)
    {
      return ConfirmResult::Unavailable;
    }

    if (entryPattern_ == targetPattern())
    {
      return ConfirmResult::Correct;
    }

    return ConfirmResult::Incorrect;
  }

  void BrailleTrainer::showAnswer()
  {
    answerVisible_ = true;
  }

  bool BrailleTrainer::answerVisible() const
  {
    return answerVisible_;
  }

  void BrailleTrainer::resetPracticeSession()
  {
    entryPattern_ = 0;
    answerVisible_ = false;
  }

} // namespace braille
