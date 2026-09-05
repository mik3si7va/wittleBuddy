# Hardware inventory

Wittle's intended form is a modular two-wheel-drive robot. Only the initial controller choice and its USB enumeration have been confirmed here. Inventory entries below do not imply that components are installed or supported in firmware.

| Item | Status / quantity | Model / specification | Connection / pins | Notes |
| --- | --- | --- | --- | --- |
| Primary controller | Confirmed, 1 detected | Arduino Mega 2560 R3 | USB serial; application pins TBD | Initial Arduino framework target |
| USB connection | Device enumerated | USB ID 2341:0042 | `/dev/ttyACM0` at initial inspection | Upload and startup output not yet verified |
| Chassis / wheels | Planned 2WD layout | TBD | TBD | Dimensions and wheel geometry TBD |
| DC motors | Planned, 2 | TBD | TBD | Ratings and gearing TBD |
| Motor driver(s) | TBD | TBD | TBD | Select after motor requirements |
| Wheel encoders | Planned, quantity TBD | TBD | TBD | Resolution and electrical interface TBD |
| IMU | Planned | TBD | TBD | Calibration and mounting TBD |
| Ultrasonic sensors | Planned, quantity TBD | TBD | TBD | Placement TBD |
| ToF sensors | Planned, quantity TBD | TBD | TBD | Placement TBD |
| 360-degree LiDAR | Future exploration | TBD | TBD | Power and processing needs TBD |
| Battery | TBD | TBD | TBD | Chemistry, voltage and capacity TBD |
| Voltage measurement | Planned | TBD | TBD | Circuit and scaling TBD |
| Power regulation / distribution | TBD | TBD | TBD | Protection and current budget TBD |
| Higher-level controller | Future possibility | ESP32 candidate; exact board TBD | TBD | Protocol and electrical interface TBD |
| Servo | Experiment placeholder only | TBD | TBD | No robot role assigned |

## Assignment policy

There are no application pin assignments, sensor addresses, bus choices or robot wiring instructions yet. Record confirmed electrical requirements and interfaces before adding hardware implementation. Motor, sensor and power-system code currently performs no operations.

## Development host observation

Initial inspection: Fedora Linux 44, user already in `dialout`; `/dev/ttyACM0` is owned by `root:dialout` with mode `0660`, and the current user has read/write access. No permissions or udev rules were changed. Device paths and access must be rechecked on another host or after reconnecting.
