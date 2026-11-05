#ifndef RUBICON_MOTOR_DRIVER_H
#define RUBICON_MOTOR_DRIVER_H

#include <Arduino.h>

// One DM556, wired with PUL+ and DIR+ at +5 V as in the schematic.
struct MotorDriver {
  uint8_t pulsePin;             // Arduino pin connected to PUL-.
  uint8_t directionPin;         // Arduino pin connected to DIR-.
  bool reverseDirection;       // Set per motor after checking its mounting.
};

void beginMotor(const MotorDriver &driver);

// Blocking, RELATIVE rotation in degrees: +90, -90, +180 or -180.
// Positive uses DIR=LOW, as in Arduino_Sample, unless reverseDirection is true.
// Returns false without changing the outputs for unsupported angles or an
// invalid step setting (zero, or not divisible by four).
bool moveMotor(const MotorDriver &driver, int angleDegrees);

// Run two motors in one coordinated move, with independent signed angles.
// Both moves use the same ramp and finish on the final pulse tick. Rejects
// invalid angles or overlapping driver pins before changing either output.
// The caller must select motors driving opposite cube faces.
bool moveMotorPair(const MotorDriver &first, int firstAngleDegrees,
                   const MotorDriver &second, int secondAngleDegrees);

#endif
