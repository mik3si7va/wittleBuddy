# Wittle Buddy 🤖

Wittle is a modular 2WD experimental mobile robot built from scratch to learn embedded systems, robotics, sensing, navigation, communications, mapping and autonomous behaviour—with room for a little personality along the way.

The initial embedded controller is an **Arduino Mega 2560**, programmed in object-oriented C++ using the Arduino framework and PlatformIO Core. This repository is the starting foundation, and will grow as hardware decisions and experiments become concrete.

## Current status

**Implemented:** a PlatformIO Mega firmware target, a lightweight experiment launcher, and a reusable `UltrasonicSensor` class. The active ultrasonic experiment measures three HC-SR04 sensors and controls red/yellow/green LEDs using the nearest distance. The five subsystem classes remain skeletons with empty `begin()` and `update()` methods and are not instantiated. There are no automated tests yet.

**Planned:** motor and encoder support, odometry and IMU integration, distance sensing, battery monitoring, communication with higher-level controllers, and a behaviour state machine.

**Experimental / future ideas:** 360-degree LiDAR integration, mapping, autonomous navigation, ESP32 cooperation and expressive behaviours. These are exploration directions, not working capabilities or settled designs.

## Architecture

| System | Planned responsibility |
| --- | --- |
| `DriveSystem` | DC motors and wheel encoders |
| `Navigation` | Odometry, IMU and pose/movement estimation |
| `Perception` | Ultrasonic, ToF and 360-degree LiDAR sensing |
| `PowerSystem` | Battery voltage monitoring and power state |
| `Communication` | Higher-level controllers, telemetry and commands |

`main.cpp` stays small. Future application orchestration and a behaviour state machine will coordinate these systems. The active ultrasonic experiment has defined pin assignments; broader robot wiring, protocols and timing contracts remain TBD. See [architecture](docs/architecture.md), [hardware inventory](docs/hardware.md) and [roadmap](docs/roadmap.md).

## Repository structure

```text
wittleBuddy/
├── README.md
├── platformio.ini
├── .vscode/extensions.json       # Optional PlatformIO IDE recommendation
├── docs/
│   ├── architecture.md
│   ├── hardware.md
│   └── roadmap.md
├── include/                     # UltrasonicSensor.h and subsystem interfaces
├── src/                         # Launcher, UltrasonicSensor.cpp and subsystem skeletons
├── test/                        # Reserved for PlatformIO tests
└── experiments/                 # Implementations; selected experiment is built
    ├── motor/
    ├── encoder/
    ├── ultrasonic/
    ├── tof/
    ├── servo/
    └── lidar/
```

PlatformIO makes `include/` available for headers. The root configuration sets `src_dir = .` and explicitly selects these implementation files with `build_src_filter`:

```ini
build_src_filter =
    +<src/main.cpp>
    +<src/UltrasonicSensor.cpp>
    +<experiments/ultrasonic/exp001.cpp>
```

This compiles the experiment in place, without copying it into `src/` or creating another project or environment. Other experiments and the subsystem skeletons are not compiled.

## Running experiments

`src/main.cpp` owns the Arduino `setup()` and `loop()` functions. It includes the experiment header and delegates to `ultrasonicExperimentSetup()` and `ultrasonicExperimentLoop()`. The implementation stays in `experiments/ultrasonic/exp001.cpp`.

The experiment creates `left`, `front`, and `right` sensor objects. Each stores its own TRIG/ECHO pins; `begin()` configures them and `measureDistance()` returns centimeters. The experiment compares all three readings and selects RED at <= 10 cm, YELLOW at <= 20 cm, or GREEN otherwise. See the [ultrasonic experiment README](experiments/ultrasonic/README.md) for pins and measurement details.

To activate a future experiment, keep its implementation and header under its own `experiments/` directory, update the header and calls in `src/main.cpp`, and replace the experiment path in `build_src_filter`. Explicitly include any reusable implementation files it needs. Keep Arduino entry points in `main.cpp`; no selector or registry is required.

## Development setup

Use Python 3 and [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/). A user-local installation keeps Python packages separate from Fedora's system Python:

```sh
python3 -m venv "$HOME/.platformio/penv"
"$HOME/.platformio/penv/bin/python" -m pip install platformio
export PATH="$HOME/.platformio/penv/bin:$PATH"
pio --version
```

On the initial development machine, Core is installed at that location, with `pio` and `platformio` links in `~/.local/bin` already on PATH. No system package or permission changes were needed. If another machine lacks Python or venv support, resolve its prerequisites before proceeding; do not run PlatformIO with sudo.

For VS Code, open this repository and optionally install the recommended **PlatformIO IDE** extension (`platformio.platformio-ide`). The CLI works independently in VS Code's terminal. Generated PlatformIO build and editor files are ignored by Git.

The board configuration follows the [PlatformIO Mega 2560 definition](https://docs.platformio.org/en/stable/boards/atmelavr/megaatmega2560.html). The Atmel AVR platform is pinned to version 5.3.0. PlatformIO downloads the compiler and Arduino framework on the first build; the uploader is fetched when needed. These downloads require internet access.

## Build, detect, upload and monitor

Run from the repository root:

```sh
pio run -e mega2560
pio device list
```

On the initial machine, USB ID `2341:0042` identifies the Mega 2560 R3 at `/dev/ttyACM0`. Device names can change after reconnecting; check the list before using the following commands.

**Upload only when you choose to replace the board's current firmware:**

```sh
pio run -e mega2560 -t upload --upload-port /dev/ttyACM0
```

For firmware that uses Serial, monitor output with:

```sh
pio device monitor -e mega2560 --port /dev/ttyACM0 --baud 115200
```

Exit the monitor with Ctrl+C and close it before uploading. The current ultrasonic experiment does not initialize Serial or print output; observe its LEDs. The startup message in the historical validation record below belongs to the earlier minimal firmware.

For serial access, inspect `id` and `ls -l /dev/ttyACM0`. On the initial Fedora machine the user is already in `dialout`, and the device is read/write for that group. No extra udev rules were needed for this connected board. Review any future permission change explicitly rather than granting world-writable access.

## Current build validation

The sensor-class refactor passed `pio run -e mega2560`: flash usage was 2,348 / 253,952 bytes and RAM usage was 21 / 8,192 bytes. This verified compilation and linking; no firmware upload was performed during the refactor.

## Initial validation (historical)

On 2026-09-05, `pio run -e mega2560` succeeded on Fedora 44 with PlatformIO Core 6.1.19, Atmel AVR 5.3.0, AVR GCC 7.3.0 and Arduino AVR framework package 5.4.0. Flash usage was 1,856 / 253,952 bytes; RAM usage was 188 / 8,192 bytes. AVRDUDE 6.3 was installed separately through PlatformIO and its help command ran successfully. PlatformIO device enumeration and the monitor CLI were available.

The connected Arduino Mega 2560 at `/dev/ttyACM0` was successfully flashed using AVRDUDE. All 1,856 bytes were written and verified. After reset, serial communication at 115200 baud was verified with the exact startup output:

```text
Wittle Buddy starting up!
```

The received bytes were `b'Wittle Buddy starting up!\r\n'`. This confirms the minimal firmware runs and serial communication works; robot subsystems remain unimplemented. Serial access permissions were checked, and no system packages, group memberships or udev rules were changed.
