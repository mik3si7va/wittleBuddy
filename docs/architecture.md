# Software architecture

## Current implementation

The project uses PlatformIO's Arduino framework for the Mega 2560. `src/main.cpp` initializes Serial at 115200 baud and prints a startup message. Its loop is empty. Each subsystem has a header in `include/` and a source in `src/`; `begin()` and `update()` are empty placeholders. No subsystem is instantiated by the initial firmware.

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

`experiments/` holds isolated investigations and is excluded from the main build by location. Record confirmed wiring and reproducible observations there before integrating a driver. `test/` is reserved for meaningful tests as contracts are defined. The initial check is compilation; hardware validation requires an authorized upload and observation of the serial startup message.
