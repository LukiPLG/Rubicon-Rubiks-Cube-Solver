#include "MotorDriver.h"

// Motors M1..M6, left to right in Schemat Elektryczny połączeń.png.
// PUL-, DIR-, reversed
const MotorDriver MOTORS[] = {
    {2, 8, false},   // M1
    {3, 9, false},   // M2
    {4, 10, false},  // M3
    {5, 11, false},  // M4
    {6, 12, false},  // M5
    {7, 13, false},  // M6
};
const uint8_t MOTOR_COUNT = sizeof(MOTORS) / sizeof(MOTORS[0]);

struct MotorPair {
  uint8_t firstMotor;   // Motor number 1..6; 0 means unassigned.
  uint8_t secondMotor;  // Motor number 1..6; 0 means unassigned.
};

const MotorPair pairs[] = {
    {0, 0},  // TODO: Fill in the first pair of opposite-side motor numbers.
    {0, 0},  // TODO: Fill in the second pair of opposite-side motor numbers.
    {0, 0},  // TODO: Fill in the third pair of opposite-side motor numbers.
};
const uint8_t PAIR_COUNT = sizeof(pairs) / sizeof(pairs[0]);

// Explicit declarations keep Arduino's generated prototypes after MotorPair.
bool isConfiguredPair(const MotorPair &pair);
bool testPairMove(const MotorPair &pair, int firstAngle, int secondAngle);
bool testMotorPair(const MotorPair &pair);
void runPairTests(const MotorPair motorPairs[], uint8_t pairCount);

const unsigned long STARTUP_DELAY_MS = 2500;
const unsigned long BETWEEN_MOVES_MS = 500;
const bool RUN_ON_STARTUP = true;

bool testMove(uint8_t motorIndex, int angleDegrees) {
  if (motorIndex >= MOTOR_COUNT) {
    return false;
  }
  Serial.print(F("M"));
  Serial.print(motorIndex + 1);
  Serial.print(F(": "));
  Serial.print(angleDegrees);
  Serial.print(F(" degrees, "));

  const unsigned long startedAt = millis();
  if (!moveMotor(MOTORS[motorIndex], angleDegrees)) {
    Serial.println(F("rejected; check angle and STEPS"));
    return false;
  }
  Serial.print(millis() - startedAt);
  Serial.println(F(" ms"));
  delay(BETWEEN_MOVES_MS);
  return true;
}

bool testSingleMotor(uint8_t motorIndex) {
  // Each block returns to its starting orientation: +90/-90, then 180/180.
  return testMove(motorIndex, 90) && testMove(motorIndex, -90) &&
         testMove(motorIndex, 180) && testMove(motorIndex, 180);
}

bool isConfiguredPair(const MotorPair &pair) {
  return pair.firstMotor >= 1 && pair.firstMotor <= MOTOR_COUNT &&
         pair.secondMotor >= 1 && pair.secondMotor <= MOTOR_COUNT &&
         pair.firstMotor != pair.secondMotor;
}

bool testPairMove(const MotorPair &pair, int firstAngle, int secondAngle) {
  if (!isConfiguredPair(pair)) {
    return false;
  }
  Serial.print(F("Pair M"));
  Serial.print(pair.firstMotor);
  Serial.print(F("/M"));
  Serial.print(pair.secondMotor);
  Serial.print(F(": "));
  Serial.print(firstAngle);
  Serial.print(F("/"));
  Serial.print(secondAngle);
  Serial.print(F(" degrees, "));

  const unsigned long startedAt = millis();
  if (!moveMotorPair(MOTORS[pair.firstMotor - 1], firstAngle,
                     MOTORS[pair.secondMotor - 1], secondAngle)) {
    Serial.println(F("rejected; check angles, STEPS and driver pins"));
    return false;
  }
  Serial.print(millis() - startedAt);
  Serial.println(F(" ms"));
  delay(BETWEEN_MOVES_MS);
  return true;
}

bool testMotorPair(const MotorPair &pair) {
  // Each pair of commands restores both faces to their starting orientations.
  return testPairMove(pair, 90, 90) && testPairMove(pair, -90, -90) &&
         testPairMove(pair, 90, -90) && testPairMove(pair, -90, 90) &&
         testPairMove(pair, 180, 180) && testPairMove(pair, 180, 180) &&
         testPairMove(pair, 90, 180) && testPairMove(pair, -90, 180);
}

void runPairTests(const MotorPair motorPairs[], uint8_t pairCount) {
  Serial.println(F("Opposite motor pairs: coordinated rotations"));
  for (uint8_t pairIndex = 0; pairIndex < pairCount; ++pairIndex) {
    if (!isConfiguredPair(motorPairs[pairIndex])) {
      Serial.print(F("Skipping pair "));
      Serial.print(pairIndex + 1);
      Serial.println(F(": TODO fill in two distinct opposite motors (1..6)"));
      continue;
    }
    if (!testMotorPair(motorPairs[pairIndex])) {
      return;
    }
  }
}

void runMotorTests() {
  Serial.println(F("Individual motors: +90, -90, 180, 180"));
  for (uint8_t motor = 0; motor < MOTOR_COUNT; ++motor) {
    if (!testSingleMotor(motor)) {
      return;
    }
  }

  Serial.println(F("All six: +90 in order, then -90 in reverse order"));
  for (uint8_t motor = 0; motor < MOTOR_COUNT; ++motor) {
    if (!testMove(motor, 90)) {
      return;
    }
  }
  for (uint8_t motor = MOTOR_COUNT; motor > 0; --motor) {
    if (!testMove(motor - 1, -90)) {
      return;
    }
  }

  Serial.println(F("Mixed moves, then their inverse in reverse order"));
  const int mixedAngles[] = {90, -90, 180, -90, 90, 180};
  static_assert(sizeof(mixedAngles) / sizeof(mixedAngles[0]) == MOTOR_COUNT,
                "Provide one mixed move per motor");
  for (uint8_t motor = 0; motor < MOTOR_COUNT; ++motor) {
    if (!testMove(motor, mixedAngles[motor])) {
      return;
    }
  }
  for (uint8_t motor = MOTOR_COUNT; motor > 0; --motor) {
    const int angle = mixedAngles[motor - 1];
    // A second +180 also undoes a half turn on a cube face.
    if (!testMove(motor - 1, angle == 180 ? 180 : -angle)) {
      return;
    }
  }
  runPairTests(pairs, PAIR_COUNT);
  Serial.println(F("Test complete. Motors are idle."));
}

void printHelp() {
  Serial.println(F("t: test all; p: test pairs; 1..6: test one motor; ?: help"));
}

void setup() {
  Serial.begin(115200);
  for (uint8_t motor = 0; motor < MOTOR_COUNT; ++motor) {
    beginMotor(MOTORS[motor]);
  }
  printHelp();
  delay(STARTUP_DELAY_MS);
  if (RUN_ON_STARTUP) {
    runMotorTests();
  }
}

void loop() {
  if (Serial.available() == 0) {
    return;
  }
  const int command = Serial.read();
  if (command == 't' || command == 'T') {
    runMotorTests();
  } else if (command == 'p' || command == 'P') {
    runPairTests(pairs, PAIR_COUNT);
  } else if (command >= '1' && command <= '6') {
    testSingleMotor(command - '1');
  } else if (command == '?') {
    printHelp();
  }
}
