# Motor experiments

## exp003: Xbox to ESP32 to Mega UART motor control

**2026-09-06: first successful wireless drive.** The complete controller → Bluetooth → ESP32 → UART → Mega → motor driver → motors chain worked on hardware. [Watch the first drive](../../docs/videos/001_mileStone001.mp4); see the [milestone and power story](../../README.md#milestone-log). The Mega remains the main controller and owns motor actuation and its receive failsafe.

`src/main.cpp` delegates to the existing `exp003.cpp`. Motor A is the left
channel and Motor B is the right channel. Positive speeds use IN1 LOW / IN2 HIGH
for both motors; negative speeds reverse those levels. Zero disables PWM.
Startup holds STBY LOW while configuring outputs, stops both motors, then enables STBY.

Both motors initially ran backwards in physical testing. The experimentally confirmed **forward** orientation is AIN1 LOW / AIN2 HIGH for Motor A and BIN1 LOW / BIN2 HIGH for Motor B. PWM is removed before setting direction and then set to the command magnitude. A zero command sets both direction pins LOW and PWM to zero; stopping does not lower STBY.

| Driver pin | Mega pin / connection |
| --- | --- |
| PWMA | 5 |
| AIN1 | 22 |
| AIN2 | 23 |
| STBY | 24 |
| BIN1 | 25 |
| BIN2 | 26 |
| PWMB | 6 |
| VCC | Mega 5V |
| GND | Mega GND and motor supply negative |
| VM | 4.5 V battery pack positive (3 × 1.5 V cells), tested supply |
| AO1 / AO2 | Left motor |
| BO1 / BO2 | Right motor |

ESP32 GPIO17 TX2 connects to Mega pin 19 RX1. Connect ESP32 GND, Mega GND,
driver GND and motor-supply negative together. Leave Mega TX1 disconnected.
Disconnect the previous ultrasonic wiring from pins 22–26.

UART uses 115200 baud, 8N1, one-way ASCII `left,right\n`, each value -255..255.
Malformed, overlong, non-ASCII and out-of-range packets stop the motors and do
not refresh the watchdog. Rejection happens when the terminating newline arrives; an incomplete or discarded line without a newline leaves the 400 ms watchdog to stop an active command. The buffer holds at most 15 characters before the terminator, and each loop reads at most 32 bytes so incoming garbage cannot starve the watchdog. Only complete valid lines refresh the 400 ms timeout.
CRLF, whitespace and plus signs are not accepted. USB Serial at 115200 prints `[BOOT]`, `[RX]`, `[REJECT]` and `[FAILSAFE]`. Routine command/rejection snapshots are throttled to 250 ms, with an immediate command log on startup/resumption; timeout transitions are printed once. No blocking loop delays are used.

Example packets (each ends in a single newline):

```text
120,120
255,255
-100,-100
180,80
-255,255
0,0
```

Positive means forward, negative reverse and zero stop, in the exact accepted range -255..255. The first value controls Motor A / left; the second controls Motor B / right.

The ESP32 currently mixes the **right stick**, not the left stick named in the session description: negative RY is forward, positive RX turns right, and a 50-count deadzone precedes proportional mixing. It sends every 50 ms, sends stop on detected disconnect/unusable input, and stops stale input after 300 ms at the next timer tick. The Mega independently stops after 400 ms without a complete valid command. These are implemented behaviors; the successful drive does not establish that every fault case was physically tested.

For the untethered drive, a USB power bank supplied portable control power and the separate 4.5 V pack supplied VM. Connect Mega 5 V to VCC and all grounds together. Missing VM power prevented movement during bring-up. The failed 9 V/Mega-regulator and shared motor-supply attempts are recorded in the [power debugging notes](../../README.md#power-debugging-a-small-power-bank-big-moment); the power bank is temporary.

Keep the wheels raised for the first test. From the repository root:

```bash
pio run -e mega2560
pio run -e mega2560 -t upload --upload-port /dev/ttyACM0
pio device monitor -e mega2560 --port /dev/ttyACM0 --baud 115200
```

Substitute the Mega USB port reported by `pio device list`. Close the monitor
before uploading; Ctrl+C exits. USB Serial is debug only; commands use Serial1.
See the [ESP32 instructions](../esp32_xbox_controller/README.md#experiment-003-uart-motor-control).

Bench checks: confirm startup stop, right-stick forward/reverse/turning, then
controller disconnect and ESP32/UART power loss. Both motors must stop on input
loss. Compilation alone does not verify physical wiring or motor operation.
