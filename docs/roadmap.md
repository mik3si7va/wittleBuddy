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

## First wireless drive — completed, 2026-09-06

- Soldered and debugged motor-driver wiring, identified missing VM power, tested both motors and corrected forward direction in software.
- Proved Xbox → Bluetooth → ESP32 → UART → Mega → motor driver → motors end-to-end.
- Replaced inadequate prototype power attempts with temporary USB power-bank control power and a separate 4.5 V motor pack.
- Recorded [the first wireless drive](videos/001_mileStone001.mp4). See the [full milestone log](../README.md#milestone-log).

## Hardware decisions and isolated experiments — ongoing

- Inventory available components and confirm motor, driver and power requirements.
- Extend confirmed motor/UART and earlier ultrasonic pin assignments as further electrical and timing requirements are checked.
- Investigate motors, encoders, ultrasonic, ToF, servo and LiDAR independently where relevant.
- Record measurements and findings before integrating code.

## Subsystem integration — planned

- Integrate working experiment motor control into DriveSystem and add encoder feedback, followed by odometry and IMU integration.
- Add perception and power monitoring using confirmed hardware.
- Integrate the working ESP32 UART controller link into subsystem communication contracts.
- Develop the final rechargeable battery and appropriate DC-DC regulation beyond temporary power-bank control power.
- Add tests for established logic and repeatable hardware checks for drivers.

## Behaviour coordination — planned

- Define a behaviour state machine, transitions, fault handling and update timing.
- Integrate subsystems incrementally with measurable acceptance criteria.

## Exploration — future ideas

Mapping, autonomous navigation, 360-degree LiDAR processing, further ESP32 cooperation and expressive behaviour remain future investigations. Feasibility and division of work between controllers need experimental evidence.
