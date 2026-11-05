#include "MotorDriver.h"

namespace {
// Pulse polarity and ramp timings copied from Arduino_Sample.ino.
const unsigned int PULSE_US = 5;
const unsigned int DIRECTION_SETUP_US = 10;
const unsigned int START_DELAY_US = 500;
const unsigned int MIN_DELAY_US = 350;
const uint16_t RAMP_STEPS = 15;
const uint16_t STEPS = 400;

bool stepsForAngle(int angleDegrees, uint16_t &steps) {
  if (angleDegrees != 90 && angleDegrees != -90 &&
      angleDegrees != 180 && angleDegrees != -180) {
    return false;
  }
  if (STEPS == 0 || STEPS % 4 != 0) {
    return false;
  }

  const uint16_t degrees = angleDegrees < 0 ? -angleDegrees : angleDegrees;
  steps = (static_cast<uint32_t>(STEPS) * degrees) / 360;
  return true;
}

void setDirection(const MotorDriver &driver, int angleDegrees) {
  const bool directionHigh = (angleDegrees < 0) != driver.reverseDirection;
  digitalWrite(driver.directionPin, directionHigh ? HIGH : LOW);
}

bool driveMotors(const MotorDriver &first, int firstAngleDegrees,
                 const MotorDriver *second, int secondAngleDegrees) {
  uint16_t firstSteps = 0;
  uint16_t secondSteps = 0;
  if (!stepsForAngle(firstAngleDegrees, firstSteps) ||
      (second != nullptr &&
       !stepsForAngle(secondAngleDegrees, secondSteps))) {
    return false;
  }
  if (second != nullptr &&
      (first.pulsePin == first.directionPin ||
       second->pulsePin == second->directionPin ||
       first.pulsePin == second->pulsePin ||
       first.pulsePin == second->directionPin ||
       first.directionPin == second->pulsePin ||
       first.directionPin == second->directionPin)) {
    return false;
  }

  setDirection(first, firstAngleDegrees);
  if (second != nullptr) {
    setDirection(*second, secondAngleDegrees);
  }
  delayMicroseconds(DIRECTION_SETUP_US);

  const uint16_t steps = firstSteps > secondSteps ? firstSteps : secondSteps;
  uint32_t firstProgress = 0;
  uint32_t secondProgress = 0;
  for (uint16_t step = 0; step < steps; ++step) {
    // Symmetric acceleration/deceleration; also handles short moves.
    uint16_t ramp = step;
    const uint16_t remaining = steps - 1 - step;
    if (remaining < ramp) {
      ramp = remaining;
    }
    if (ramp > RAMP_STEPS) {
      ramp = RAMP_STEPS;
    }
    const unsigned int stepDelay = START_DELAY_US -
        static_cast<uint32_t>(START_DELAY_US - MIN_DELAY_US) * ramp /
            RAMP_STEPS;

    // Distribute each motor's pulses over the same move duration. A 90/180
    // pair pulses the quarter-turn motor on every second half-turn tick.
    firstProgress += firstSteps;
    secondProgress += secondSteps;
    const bool pulseFirst = firstProgress >= steps;
    const bool pulseSecond = second != nullptr && secondProgress >= steps;
    if (pulseFirst) {
      firstProgress -= steps;
      digitalWrite(first.pulsePin, LOW);
    }
    if (pulseSecond) {
      secondProgress -= steps;
      digitalWrite(second->pulsePin, LOW);
    }
    delayMicroseconds(PULSE_US);
    if (pulseFirst) {
      digitalWrite(first.pulsePin, HIGH);
    }
    if (pulseSecond) {
      digitalWrite(second->pulsePin, HIGH);
    }
    delayMicroseconds(stepDelay);
  }
  return true;
}
}

void beginMotor(const MotorDriver &driver) {
  // Set the output latches before enabling the pins; PUL idles HIGH.
  digitalWrite(driver.pulsePin, HIGH);
  digitalWrite(driver.directionPin, HIGH);
  pinMode(driver.pulsePin, OUTPUT);
  pinMode(driver.directionPin, OUTPUT);
}

bool moveMotor(const MotorDriver &driver, int angleDegrees) {
  return driveMotors(driver, angleDegrees, nullptr, 0);
}

bool moveMotorPair(const MotorDriver &first, int firstAngleDegrees,
                   const MotorDriver &second, int secondAngleDegrees) {
  return driveMotors(first, firstAngleDegrees, &second, secondAngleDegrees);
}
