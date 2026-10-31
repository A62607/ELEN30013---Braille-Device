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

  // FSR configuration.
  constexpr unsigned long kFsrPressDurationMs = 50;

  constexpr uint16_t kFsrPressThresholds[6] = {
  250,  // A0
  250,  // A1
  250,  // A2
  150,  // A3
  250,  // A4
  150   // A5
};

  // Re-arm below the press threshold. Add hysteresis here after calibration
  // if a noisy sensor repeatedly crosses 300.
  constexpr uint16_t kFsrReleaseThreshold = 200;

  // Functional-button debounce.
  constexpr unsigned long kButtonDebounceMs = 30;

  // Individual angles for dots/servos 1–6
  constexpr uint8_t kServoRaisedAngles[kDotCount] =
  {
    70,  // Servo 1
    50,  // Servo 2
    60,  // Servo 3
    70,  // Servo 4
    70,  // Servo 5
    50   // Servo 6
  };

  constexpr uint8_t kServoLoweredAngles[kDotCount] =
  {
    180, // Servo 1
    160, // Servo 2
    160, // Servo 3
    160, // Servo 4
    180, // Servo 5
    160  // Servo 6
  };

  // 74HC165 interface to the Arduino Uno.
  constexpr uint8_t kButtonLoadPin = 2;
  constexpr uint8_t kButtonClockPin = 3;
  constexpr uint8_t kButtonDataPin = 4;

  // DFPlayer Mini serial interface.
  // Arduino RX receives from DFPlayer TX.
  // Arduino TX sends to DFPlayer RX through a 1 kOhm series resistor.
  constexpr uint8_t kDfPlayerRxPin = 11;
  constexpr uint8_t kDfPlayerTxPin = 12;
  constexpr uint8_t kDfPlayerVolume = 25; // Valid range: 0-30.


  // Desired physical button arrangement:
  //
  // 1 = Mode
  // 2 = Clear
  // 3 = Previous
  // 4 = Show Answer
  // 5 = Confirm
  // 6 = Next
  constexpr uint8_t kModeButtonMask     = 0b01000000;  // Position 1
  constexpr uint8_t kClearButtonMask    = 0b00100000;  // Position 2
  constexpr uint8_t kPreviousButtonMask = 0b00000100;  // Position 3
  constexpr uint8_t kShowButtonMask     = 0b00000010;  // Position 4
  constexpr uint8_t kConfirmButtonMask  = 0b00010000;  // Position 5
  constexpr uint8_t kNextButtonMask     = 0b00001000;  // Position 6

} // namespace hardware
