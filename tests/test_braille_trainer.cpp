#include <assert.h>

#include <iostream>

#include "../firmware/interactive_braille/BraillePatterns.h"
#include "../firmware/interactive_braille/BrailleTrainer.h"

int main() {
  braille::BrailleTrainer trainer;

  assert(trainer.mode() == braille::Mode::Learning);
  assert(trainer.targetLetter() == 'A');
  assert(trainer.displayedPattern() == braille::patternForLetter('A'));

  assert(trainer.setTargetLetter('c'));
  assert(trainer.targetLetter() == 'C');
  assert(trainer.displayedPattern() == braille::patternForLetter('C'));
  assert(!trainer.setTargetLetter('?'));
  assert(trainer.targetLetter() == 'C');

  trainer.nextTarget();
  assert(trainer.targetLetter() == 'D');
  trainer.previousTarget();
  assert(trainer.targetLetter() == 'C');
  trainer.setTargetLetter('A');
  trainer.previousTarget();
  assert(trainer.targetLetter() == 'Z');
  trainer.nextTarget();
  assert(trainer.targetLetter() == 'A');

  trainer.setTargetLetter('C');
  trainer.setMode(braille::Mode::Practice);
  assert(trainer.entryPattern() == 0);
  assert(trainer.displayedPattern() == 0);
  assert(!trainer.raiseDot(0));
  assert(!trainer.raiseDot(7));
  assert(trainer.raiseDot(1));
  assert(!trainer.raiseDot(1));
  assert(trainer.entryPattern() == braille::patternForLetter('A'));
  assert(trainer.raiseDot(4));
  assert(trainer.entryPattern() == braille::patternForLetter('C'));
  assert(trainer.confirm() == braille::ConfirmResult::Correct);

  trainer.setTargetLetter('B');
  assert(trainer.raiseDot(1));
  assert(trainer.confirm() == braille::ConfirmResult::Incorrect);
  assert(trainer.confirm() == braille::ConfirmResult::Incorrect);
  assert(trainer.confirm() == braille::ConfirmResult::Incorrect);
  assert(!trainer.answerVisible());
  assert(trainer.displayedPattern() == trainer.entryPattern());
  assert(trainer.raiseDot(2));
  assert(trainer.confirm() == braille::ConfirmResult::Correct);

  trainer.clearEntry();
  assert(!trainer.answerVisible());
  assert(trainer.entryPattern() == 0);

  trainer.showAnswer();
  assert(trainer.answerVisible());
  assert(trainer.displayedPattern() == braille::patternForLetter('B'));
  assert(!trainer.raiseDot(2));
  assert(trainer.confirm() == braille::ConfirmResult::Unavailable);

  trainer.setMode(braille::Mode::Learning);
  assert(!trainer.answerVisible());
  assert(!trainer.raiseDot(1));
  assert(trainer.confirm() == braille::ConfirmResult::Unavailable);
  assert(trainer.displayedPattern() == braille::patternForLetter('B'));

  std::cout << "Braille trainer state tests passed.\n";
  return 0;
}
