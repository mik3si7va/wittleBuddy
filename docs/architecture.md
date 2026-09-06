# Software architecture

## Current implementation

The project uses PlatformIO's Arduino framework for the Mega 2560. `src/main.cpp` owns Arduino `setup()` and `loop()` and delegates to the active motor experiment through `experiments/motor/exp003.h`.

The working control chain, proven in the first wireless drive on **2026-09-06**, is Xbox controller → Bluetooth → ESP32 → UART → Mega 2560 → GPIO/PWM → dual motor driver → two DC motors. The **Mega is the main controller / brain** and owns motor control, packet validation and the 400 ms command timeout. The ESP32 is the wireless helper: its separate ESP-IDF/Bluepad32 project mixes right-stick input and sends `left,right\n` commands at 115200 baud every 50 ms. Its 300 ms input timeout is checked on the next timer tick. See the [Mega experiment](../experiments/motor/README.md) and [ESP32 experiment](../experiments/esp32_xbox_controller/README.md#experiment-003-uart-motor-control).

The earlier ultrasonic experiment remains available. It owns three `UltrasonicSensor` objects, compares their distances, and controls traffic-light LEDs. `UltrasonicSensor` stores one TRIG/ECHO pin pair and provides `begin()` and `measureDistance()`; it has no LED or navigation responsibilities. Each subsystem has a header in `include/` and a source in `src/`; `begin()` and `update()` are empty placeholders. No subsystem is instantiated or included in the current build filter.

## Planned boundaries

| Class | Responsibility | Decisions still required |
| --- | --- | --- |
| DriveSystem | Motor actuation and encoder acquisition | Integrate tested motor wiring; encoder format, command and feedback interfaces |
| Navigation | Odometry, IMU integration, pose/movement estimation | IMU, calibration, coordinate frames and data contracts |
| Perception | Ultrasonic, ToF and 360-degree LiDAR sensing | Models, interfaces, sampling and validity representation |
| PowerSystem | Battery voltage and power state | Battery, measurement circuit, thresholds and units |
| Communication | Higher-level controller links, telemetry and commands | Integrate tested ESP32 UART link; broader telemetry and message contracts |

The small lifecycle interface is provisional. Add configuration, results and error reporting only as requirements become known. Empty methods do not indicate functioning or successfully initialized hardware.

## Future orchestration

Keep Arduino `setup()` and `loop()` focused on application orchestration. A future behaviour state machine will select modes and coordinate subsystems through explicit interfaces. States, transitions, command ownership and fault responses are TBD; none are implemented.

Prefer bounded, non-blocking updates as functionality is added. Navigation is expected to consume drive feedback, and behaviours to consume navigation, perception and power information; exact dependencies and scheduling are TBD. Keep sensor/driver details behind the subsystem boundaries and avoid shared mutable globals between systems.

Plan memory use and update timing within the Mega's resource limits. Higher-level processing may eventually move to another controller, but no allocation of mapping or autonomy work is decided.

## Experiments and validation

`experiments/` holds investigations compiled in place when explicitly selected. The root `platformio.ini` uses `src_dir = .` and `build_src_filter` to include only `src/main.cpp` and `experiments/motor/exp003.cpp`. Switching experiments means updating the launcher header/calls and the source filter within the same Mega environment.

Record wiring and reproducible observations in the experiment README. `test/` is reserved for meaningful tests as contracts are defined. The earlier sensor refactor passed compilation and linking without uploading; its experiment uses sensor readings and LED responses with no serial output. See the [ultrasonic experiment](../experiments/ultrasonic/README.md). The active motor experiment prints USB Serial diagnostics and was used in the reported first wireless drive. That result is distinct from exhaustive fault-case validation; suggested bench checks remain in the motor README.
