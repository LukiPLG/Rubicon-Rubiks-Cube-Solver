# Six-motor Arduino test

Open `Arduino_Motor_Test.ino` in Arduino IDE, select **Arduino Uno**, and
upload. No additional libraries are needed. Open Serial Monitor at **115200 baud**.

**The test starts automatically 2.5 seconds after power-up/reset**, runs once,
then waits. Opening Serial Monitor can reset an Uno and start the test again.
Set `RUN_ON_STARTUP` to `false` for manual operation. Send `t` to run the full
test, `p` to test configured opposite pairs, `1` through `6` to test one motor,
or `?` for help. Serial commands are
processed between tests; motor moves are blocking.

## Wiring and calibration

Motor numbers follow the drivers left to right in
[the repository schematic](../../../Schemat%20Elektryczny%20po%C5%82%C4%85cze%C5%84.png).
They are not cube face labels; assign faces after checking the physical assembly.

| Motor | Arduino PUL- | Arduino DIR- |
| --- | --- | --- |
| M1 | D2 | D8 |
| M2 | D3 | D9 |
| M3 | D4 | D10 |
| M4 | D5 | D11 |
| M5 | D6 | D12 |
| M6 | D7 | D13 |

The schematic connects DM556 PUL+ and DIR+ to +5 V. The code keeps PUL HIGH
between pulses and drives it LOW for 5 microseconds, matching
`Arduino_Sample.ino`. It preserves the sample's 10 microsecond direction setup,
500 to 350 microsecond step delays, and 15-step acceleration/braking ramp.

Each `MotorDriver` contains its two pins and a direction reversal flag. The shared
`STEPS` constant in `MotorDriver.cpp` configures pulses per revolution for all
six motors. The default **400 pulses/revolution must match the
physical microstep DIP switch setting on each DM556**: 90 degrees then needs
100 pulses and 180 degrees needs 200 pulses. For a direct motor-to-face coupling,
these are also the cube face angles. Adjust pulses per face revolution if the
mechanism uses gearing. Check each motor's direction and set its
`reverseDirection` flag as needed. Positive angles initially use DIR LOW, as in
the sample; clockwise depends on the mounting and viewing side.

## Driver API

The reusable pulse-generation code is in `MotorDriver.cpp`; the test sequences
and six driver structs are in `Arduino_Motor_Test.ino`.

```cpp
MotorDriver driver = {2, 8, false};
beginMotor(driver);          // Call once in setup().
moveMotor(driver, 90);       // Relative +90 degree rotation.
moveMotor(driver, -90);      // Relative -90 degree rotation.
moveMotor(driver, 180);      // Relative +180 degree rotation.
```

`moveMotor(const MotorDriver &driver, int angleDegrees)` also accepts `-180`.
It rejects other angles and step settings that are zero or not divisible by four,
returning `false` without changing outputs. Calls finish after sending all pulses.
There is no encoder or homing sensor, so angles are relative commands rather
than measured absolute positions; successful return means the pulses were sent.

## Test sequence

1. Each motor M1 through M6: `+90, -90, +180, +180`.
2. M1 through M6: `+90` each, then M6 through M1: `-90` each.
3. M1 through M6: `+90, -90, +180, -90, +90, +180`, then undo these moves
   in reverse motor order (a half turn is undone with another `+180`).
4. Run the opposite-pair tests described below, skipping unassigned entries.

The first three stages move one motor at a time, with a 500 ms pause after each
move. Serial output
identifies the motor, commanded angle and elapsed move time. Each sequence
restores the starting cube state if all moves complete without missed steps.
The printed completion message describes command completion, not a sensor check.

## Opposite motor pairs

`MotorPair` is a separate structure in `Arduino_Motor_Test.ino`. Its `pairs`
table contains three `{0, 0}` entries, each marked **TODO**. Fill each entry with
the two **motor numbers 1 through 6** that drive opposite cube faces in the
physical assembly. Use each motor once across the three pairs. These are motor
numbers (M1 is `1`), rather than array indices. The repository does not establish
which motor numbers are opposite, so no pair is preassigned.

`runPairTests(pairs, PAIR_COUNT)` runs at the end of the full test and can also
be triggered by sending `p`. It prints a skip message for unassigned, out-of-range
or self-paired entries. Each configured pair runs these simultaneous angles:

| First motor | Second motor |
| --- | --- |
| +90 | +90 |
| -90 | -90 |
| +90 | -90 |
| -90 | +90 |
| +180 | +180 |
| +180 | +180 |
| +90 | +180 |
| -90 | +180 |

Every two rows restore both faces to their starting orientations. There is a
500 ms pause after each coordinated move.

The reusable driver procedure is
`moveMotorPair(firstDriver, firstAngle, secondDriver, secondAngle)`. Each angle
can independently be `90`, `-90`, `180` or `-180`. It interleaves both pulse
trains over one common acceleration/braking ramp; for a 90/180 move, the
quarter-turn motor receives every second pulse tick and both motors finish on
the final tick. PUL pins are written consecutively, so their edges have the small
skew of `digitalWrite` calls, while the motor movements overlap. Invalid angles
and overlapping driver pins are rejected before either motor is driven.

## Build and software checks

From the repository root, with Arduino CLI installed:

```sh
arduino-cli core update-index
arduino-cli core install arduino:avr
arduino-cli compile --fqbn arduino:avr:uno Assets/Kod/Arduino_Motor_Test
```

The host checks run the real driver and test sketch against simulated Arduino
I/O to check pulse counts, direction, timing, pin mapping, invalid requests,
the complete sequence, concurrent pair pulses, and skipping TODO pair entries
without moving hardware:

```sh
sh tests/motor_driver/run.sh
```

Actual direction, microstep settings, missed steps and mechanical motion must be
checked on the assembled hardware.
