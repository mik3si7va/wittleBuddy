# Software architecture

## Current implementation

The project uses PlatformIO's Arduino framework for the Mega 2560. `src/main.cpp` owns Arduino `setup()` and `loop()` and delegates to the active ultrasonic experiment through `exp001.h`. The experiment owns three `UltrasonicSensor` objects, compares their distances, and controls traffic-light LEDs. `UltrasonicSensor` stores one TRIG/ECHO pin pair and provides `begin()` and `measureDistance()`; it has no LED or navigation responsibilities. Each subsystem has a header in `include/` and a source in `src/`; `begin()` and `update()` are empty placeholders. No subsystem is instantiated or included in the current build filter.

## Planned boundaries

| Class | Responsibility | Decisions still required |
| --- | --- | --- |
| DriveSystem | Motor actuation and encoder acquisition | Drivers, pins, encoder format, command and feedback interfaces |
| Navigation | Odometry, IMU integration, pose/movement estimation | IMU, calibration, coordinate frames and data contracts |
| Perception | Ultrasonic, ToF and 360-degree LiDAR sensing | Models, interfaces, sampling and validity representation |
| PowerSystem | Battery voltage and power state | Battery, measurement circuit, thresholds and units |
| Communication | Higher-level controller links, telemetry and commands | Controller selection, transport, protocol and message contracts |

The small lifecycle interface is provisional. Add configuration, results and error reporting only as requirements become known. Empty methods do not indicate functioning or successfully initialized hardware.

## Future orchestration

Keep Arduino `setup()` and `loop()` focused on application orchestration. A future behaviour state machine will select modes and coordinate subsystems through explicit interfaces. States, transitions, command ownership and fault responses are TBD; none are implemented.

Prefer bounded, non-blocking updates as functionality is added. Navigation is expected to consume drive feedback, and behaviours to consume navigation, perception and power information; exact dependencies and scheduling are TBD. Keep sensor/driver details behind the subsystem boundaries and avoid shared mutable globals between systems.

Plan memory use and update timing within the Mega's resource limits. Higher-level processing may eventually move to another controller, but no allocation of mapping or autonomy work is decided.

## Experiments and validation

`experiments/` holds investigations compiled in place when explicitly selected. The root `platformio.ini` uses `src_dir = .` and `build_src_filter` to include only `src/main.cpp`, `src/UltrasonicSensor.cpp`, and `experiments/ultrasonic/exp001.cpp`. Switching experiments means updating the launcher header/calls and the source filter within the same Mega environment.

Record wiring and reproducible observations in the experiment README. `test/` is reserved for meaningful tests as contracts are defined. The current sensor refactor passed compilation and linking without uploading. Hardware validation of the active experiment uses sensor readings and LED responses; it has no serial output. See the [ultrasonic experiment](../experiments/ultrasonic/README.md).
