#pragma once

#include <Arduino.h>

namespace hardware
{

  constexpr uint8_t kDotCount = 6;

  // Only use these values if the FSRs are actually connected to A0-A5.
  constexpr uint8_t kFsrPins[kDotCount] = {
      A0, A1, A2, A3, A4, A5};

  // Replace these with the confirmed servo signal pins.
  constexpr uint8_t kServoPins[kDotCount] = {
      2, 3, 4, 5, 6, 7};

  // FSR configuration.
  constexpr uint16_t kFsrPressThreshold = 300;
  constexpr unsigned long kFsrPressDurationMs = 50;

  // Re-arm below the press threshold. Add hysteresis here after calibration
  // if a noisy sensor repeatedly crosses 300.
  constexpr uint16_t kFsrReleaseThreshold = kFsrPressThreshold;

  // Functional-button debounce.
  constexpr unsigned long kButtonDebounceMs = 30;

  // Servo positions.
  constexpr uint8_t kServoLoweredAngle = 150;
  constexpr uint8_t kServoRaisedAngle = 20;

  // 74HC165 connections—replace with confirmed pins.
  constexpr uint8_t kButtonLoadPin = 8;
  constexpr uint8_t kButtonClockPin = 9;
  constexpr uint8_t kButtonDataPin = 10;

} // namespace hardware
