#pragma once

#include <Arduino.h>

namespace hardware
{

  constexpr uint8_t kDotCount = 6;

  // Six FSR voltage-divider outputs, dot 1 through dot 6.
  constexpr uint8_t kFsrPins[kDotCount] = {
      A0, A1, A2, A3, A4, A5};

  // Servo signal pins, dot 1 through dot 6. Servo.h can use ordinary digital
  // pins; hardware-PWM pins are not required.
  constexpr uint8_t kServoPins[kDotCount] = {
      5, 6, 7, 8, 9, 10};

  // FSR thresholds for dot 1 through dot 6 (A0 through A5).
  // Replace each value with the measured threshold for that individual FSR.
  constexpr uint16_t kFsrPressThresholds[kDotCount] = {
      300, 300, 300, 300, 300, 300};
  constexpr unsigned long kFsrPressDurationMs = 50;

  // Each FSR is re-armed after its reading falls below its release threshold.
  // These initially match the press thresholds. Lower individual values after
  // calibration if a noisy sensor repeatedly crosses its press threshold.
  constexpr uint16_t kFsrReleaseThresholds[kDotCount] = {
      300, 300, 300, 300, 300, 300};

  // Functional-button debounce.
  constexpr unsigned long kButtonDebounceMs = 30;

  // Servo positions.
  constexpr uint8_t kServoLoweredAngle = 150;
  constexpr uint8_t kServoRaisedAngle = 20;

  // 74HC165 interface to the Arduino Uno.
  constexpr uint8_t kButtonLoadPin = 2;
  constexpr uint8_t kButtonClockPin = 3;
  constexpr uint8_t kButtonDataPin = 4;

  // Active-low buttons on parallel inputs A-F. Inputs G and H are unused.
  constexpr uint8_t kShowButtonMask = uint8_t{1} << 0;     // A, IC pin 11
  constexpr uint8_t kPreviousButtonMask = uint8_t{1} << 1; // B, IC pin 12
  constexpr uint8_t kNextButtonMask = uint8_t{1} << 2;     // C, IC pin 13
  constexpr uint8_t kConfirmButtonMask = uint8_t{1} << 3;  // D, IC pin 14
  constexpr uint8_t kClearButtonMask = uint8_t{1} << 4;    // E, IC pin 3
  constexpr uint8_t kModeButtonMask = uint8_t{1} << 5;     // F, IC pin 4

} // namespace hardware
