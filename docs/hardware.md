# Hardware inventory

Wittle's intended form is a modular two-wheel-drive robot. The chassis is assembled and the first wireless Xbox-controlled drive succeeded on 2026-09-06. The initial controller choice, USB enumeration, firmware upload and serial startup output have also been confirmed here. Inventory entries below do not imply that components are installed or supported in firmware.

| Item | Status / quantity | Model / specification | Connection / pins | Notes |
| --- | --- | --- | --- | --- |
| Primary controller | Confirmed, 1 detected | Arduino Mega 2560 R3 | USB debug; Serial1 RX19; motor pins below | Initial Arduino framework target |
| USB connection | Upload and serial output verified | USB ID 2341:0042 | `/dev/ttyACM0` during validation | AVRDUDE wrote and verified 1,856 bytes; startup message received at 115200 baud after reset |
| Chassis / wheels | Assembled 2WD | Two acrylic decks, drive wheels and two swivel casters | Two geared motors on lower chassis | Dimensions and wheel geometry TBD |
| DC motors | Both tested, 2 | Geared DC motors; ratings/gearing TBD | Left: AO1/AO2; right: BO1/BO2 | Forward orientation confirmed experimentally |
| Motor driver | Dual driver tested | Exact model not recorded here | PWMA=5, AIN1=22, AIN2=23, STBY=24, BIN1=25, BIN2=26, PWMB=6 | VCC=Mega 5 V; VM=separate 4.5 V pack; common GND |
| Wheel encoders | Planned, quantity TBD | TBD | TBD | Resolution and electrical interface TBD |
| IMU | Planned | TBD | TBD | Calibration and mounting TBD |
| Ultrasonic sensors | Planned, quantity TBD | TBD | TBD | Placement TBD |
| ToF sensors | Planned, quantity TBD | TBD | TBD | Placement TBD |
| 360-degree LiDAR | Future exploration | TBD | TBD | Power and processing needs TBD |
| Motor battery | Tested | 4.5 V pack, 3 × 1.5 V cells | Positive to VM; negative to common GND | Chemistry/capacity not recorded; final rechargeable system planned |
| Voltage measurement | Planned | TBD | TBD | Circuit and scaling TBD |
| Control power | Temporary prototype, tested | USB power bank | Portable USB power for Mega/ESP32 controls | Final DC-DC regulation, protection and current budget TBD |
| Wireless helper | Tested with Xbox over Bluetooth | ESP32 project targets ESP32-WROOM / DOIT ESP32 DEVKIT V1 | GPIO17 UART2 TX → Mega RX1 pin 19; common GND; 115200 baud | Mega remains main controller; one-way motor-command link |
| Servo | Experiment placeholder only | TBD | TBD | No robot role assigned |

## Assignment policy

Confirmed motor wiring and software orientation are recorded in the [motor experiment](../experiments/motor/README.md): forward is AIN1/BIN1 LOW and AIN2/BIN2 HIGH. The earlier [ultrasonic experiment](../experiments/ultrasonic/README.md) has its own conflicting pin assignments; disconnect that wiring before using exp003. Broader robot wiring and sensor addresses remain open. Subsystem classes remain empty, while experiment motor and ultrasonic code performs real operations.

The small 9 V battery through the Mega regulator and the shared 4.5 V motor-supply attempt did not reliably power the ESP32/controller setup. Both were abandoned in favour of temporary USB power-bank control power. See the [power debugging story](../README.md#power-debugging-a-small-power-bank-big-moment); a final rechargeable battery/DC-DC architecture is still future work.

## Development host observation

Initial inspection: Fedora Linux 44, user already in `dialout`; `/dev/ttyACM0` is owned by `root:dialout` with mode `0660`, and the current user has read/write access. No permissions or udev rules were changed. Device paths and access must be rechecked on another host or after reconnecting.
