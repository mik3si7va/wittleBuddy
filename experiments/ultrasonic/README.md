# Ultrasonic experiments

## exp001: three sensors and traffic-light LEDs

This earlier experiment remains available; the root Mega 2560 build currently selects motor exp003. Its pins overlap motor-driver wiring, so disconnect that wiring before restoring this experiment. It measures three HC-SR04 sensors sequentially, finds the minimum distance, and lights one LED:

| Minimum distance | LED |
| --- | --- |
| <= 10 cm | RED |
| > 10 cm and <= 20 cm | YELLOW |
| > 20 cm | GREEN |

All three LEDs start LOW during setup. Each loop measures left, front, then right before updating the LEDs. The experiment does not print serial output.

## Pin assignments

| Sensor | TRIG | ECHO |
| --- | --- | --- |
| left | 22 | 23 |
| front | 24 | 25 |
| right | 26 | 27 |

| LED | Output pin |
| --- | --- |
| RED | 28 |
| YELLOW | 29 |
| GREEN | 30 |

These are the digital pin assignments in `exp001.cpp`; the LED logic uses HIGH for on and LOW for off.

## Files and responsibilities

```text
include/UltrasonicSensor.h        Class declaration
src/UltrasonicSensor.cpp          Sensor setup and measurement
src/main.cpp                     Arduino setup()/loop() launcher
experiments/ultrasonic/exp001.h   Experiment function declarations
experiments/ultrasonic/exp001.cpp Sensor objects, comparison and LEDs
platformio.ini                   Explicit build source selection
```

The experiment creates its sensors with:

```cpp
UltrasonicSensor left(TRIG1, ECHO1);
UltrasonicSensor front(TRIG2, ECHO2);
UltrasonicSensor right(TRIG3, ECHO3);
```

The constructor stores the two pins without accessing hardware. Each object has its own private `trigPin` and `echoPin` members, so the same methods operate on different pins for each sensor.

`begin()` configures TRIG as OUTPUT and ECHO as INPUT, then sets TRIG LOW. The experiment calls it once on each object during setup.

`measureDistance()` holds TRIG LOW for 2 microseconds, HIGH for 10 microseconds, then LOW again. It measures the ECHO HIGH pulse using `pulseIn()` with its default timeout and returns `pulse / 58` as a float. This preserves the original integer division: fractional centimeters are discarded before conversion to float. Measurement is blocking. A zero pulse result produces 0 cm and therefore selects RED when included in the minimum; there is no separate invalid-reading handling yet.

The sensor class only handles the sensor. The experiment owns the objects and keeps the minimum-distance calculation, thresholds, and all LED control.

## Launcher and build

To select this experiment, configure `src/main.cpp` to include `exp001.h` and call `ultrasonicExperimentSetup()` from Arduino `setup()` and `ultrasonicExperimentLoop()` from Arduino `loop()`. The experiment does not define Arduino entry points or require including a `.cpp` file.

The root `platformio.ini` sets `src_dir = .`. To restore this experiment, use this source filter (the current filter selects motor exp003):

```ini
build_src_filter =
    +<src/main.cpp>
    +<src/UltrasonicSensor.cpp>
    +<experiments/ultrasonic/exp001.cpp>
```

Run from the repository root:

```sh
pio run -e mega2560
```

The refactor passed this build with 2,348 bytes of flash and 21 bytes of RAM used. No upload was performed during the refactor; this result verifies compilation and linking, not hardware behavior after the refactor.

With that selection, other experiments remain excluded. To change the active experiment, update the launcher header/calls and source filter as described in the [root README](../../README.md#running-experiments). Keep one root project and the existing Mega environment.
