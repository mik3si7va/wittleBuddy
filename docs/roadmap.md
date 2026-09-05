# Roadmap

This is a sequence of learning milestones, not a delivery schedule. Hardware and interface choices remain open.

## Foundation — implemented

- PlatformIO configuration for Mega 2560 and Arduino.
- Minimal serial startup firmware.
- Five subsystem class skeletons, documentation and experiment directories.

## Toolchain bring-up — validation

- Completed: firmware compiles on Fedora; memory usage and tool versions are recorded in the README.
- Completed: Mega detected at `/dev/ttyACM0`; current user has read/write access.
- Completed: AVRDUDE uploaded and verified all 1,856 bytes on the Mega at `/dev/ttyACM0`; after reset, `Wittle Buddy starting up!` was received at 115200 baud.

## Hardware decisions and isolated experiments — planned

- Inventory available components and confirm motor, driver and power requirements.
- Choose wiring and pin assignments after checking electrical and timing requirements.
- Investigate motors, encoders, ultrasonic, ToF, servo and LiDAR independently where relevant.
- Record measurements and findings before integrating code.

## Subsystem integration — planned

- Implement drive and encoder feedback, followed by odometry and IMU integration.
- Add perception and power monitoring using confirmed hardware.
- Define communication contracts and implement a selected controller link.
- Add tests for established logic and repeatable hardware checks for drivers.

## Behaviour coordination — planned

- Define a behaviour state machine, transitions, fault handling and update timing.
- Integrate subsystems incrementally with measurable acceptance criteria.

## Exploration — future ideas

Mapping, autonomous navigation, 360-degree LiDAR processing, ESP32 cooperation and expressive behaviour remain future investigations. Feasibility and division of work between controllers need experimental evidence.
