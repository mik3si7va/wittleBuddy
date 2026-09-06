# ESP32 Xbox controller experiments

Standalone PlatformIO project for the **original ESP32-WROOM / DOIT ESP32 DEVKIT V1** (4 MB flash). The root Mega build, production sources, and ultrasonic experiments are independent and unchanged. Open this directory as the PlatformIO project, or use the commands below.

**Milestone — 2026-09-06:** exp003 supplied the wireless input for [Wittle's first wireless drive](../../docs/videos/001_mileStone001.mp4). The ESP32 is the Bluetooth/UART helper; the Mega remains the brain and controls the motors. See [Experiment 003](#experiment-003-uart-motor-control) for its explicit build commands. The setup, LED mapping and validation below describe the original traffic-light experiment (`esp32_xbox`, still the default environment).

## Build and upload

### VS Code IntelliSense

Open `esp32_xbox.code-workspace` using **File > Open Workspace from File**.
This makes the experiment the editor's PlatformIO project. Opening only the
repository root uses the Mega/AVR IntelliSense configuration, which can flag
ESP32 and even standard C++ headers as missing despite a successful ESP32 build.
PlatformIO supports [project-specific VS Code workspaces](https://docs.platformio.org/en/latest/integration/ide/vscode.html).

To regenerate local editor metadata from this directory:

```sh
pio project init --ide vscode
```

If squiggles persist in the experiment workspace, run **PlatformIO: Rebuild
IntelliSense Index** from the Command Palette, then **Developer: Reload Window**.
Generated `.vscode/c_cpp_properties.json` contains local compiler/include paths
and is ignored by Git. No firmware source or compiler flags need changing.

### Commands

Prerequisites: PlatformIO Core (`pio`), Git, Python 3, and internet access for the first build. PlatformIO installs the ESP-IDF toolchain; no Arduino IDE or Arduino Bluepad32 board package is used.

From the repository root:

```sh
cd experiments/esp32_xbox_controller
pio run
pio run -t upload
pio device monitor -b 115200
```

Or upload directly from the repository root:

```sh
pio run -d experiments/esp32_xbox_controller -t upload
```

`upload_port` and `monitor_port` are both `/dev/ttyUSB0`. Change them in this project's `platformio.ini` if Linux assigns another port (`pio device list` lists ports). Close other serial monitors before uploading. If upload stays at `Connecting...`, hold BOOT while connecting, then release when writing begins. On Fedora, a permission-denied error means your user needs access to the serial device; inspect its ownership and your group membership. Exit the monitor with Ctrl+C. Press EN/reset after opening the monitor to see boot diagnostics.

Upload troubleshooting: `upload_speed = 115200` avoids switching to the board's
default 460800 baud transfer rate. If the stub starts but upload then reports
`Unable to verify flash chip connection (No serial data received)`, retry at this
speed. If it still fails, unplug/reconnect USB, try a known-good data cable and a
direct USB port, and retry with the LED module disconnected. An implausible
crystal-frequency warning during upload does not establish the actual crystal
frequency; do not change the firmware clock configuration based on that warning
alone. See [Espressif's upload troubleshooting](https://docs.espressif.com/projects/esptool/en/release-v4/esp32/troubleshooting.html).

## Wiring and mapping

Disconnect power while wiring. Use the traffic-light module's existing current limiting and 3.3 V compatible inputs; ESP32 GPIOs are not 5 V tolerant. Outputs are active HIGH, matching the existing module convention.

| Module connection | ESP32 connection | Controller mapping |
| --- | --- | --- |
| RED | GPIO 25 | B |
| YELLOW | GPIO 26 | X |
| GREEN | GPIO 27 | A |
| GND | GND | Common ground |

Y turns all three LEDs on. Simultaneous A/B/X presses light their corresponding LEDs together. Releasing all mapped face buttons turns everything off. Boot and active-controller disconnection also turn all LEDs off. Other inputs do not affect LEDs. The first ready gamepad becomes active; additional controllers are rejected until it disconnects.

## Pair and observe

1. Upload, open the monitor at 115200, and reset the ESP32 if necessary.
2. Look for `[BOOT]`, `[BLUEPAD32] Initialized`, and `[WAITING]`.
3. Turn on the Xbox controller and hold its pairing button until the Xbox light flashes rapidly. Disconnect it from another nearby host if that host reconnects automatically.
4. Look for `[DISCOVERED]`, then `[CONNECTED]` and `[READY]`. Copy the **controller Bluetooth address** from `[READY]`. Discovery names can be unavailable; a bonded reconnect may not produce a discovery callback.
5. Press buttons and move sticks/triggers. `[INPUT] First controller input received` confirms actual reports. `[BUTTONS]` prints face-button and D-pad changes, including releases. `[INPUT]` snapshots show A/B/X/Y, decoded D-pad, LX/LY, RX/RY, LT and RT.
6. Check A=green, B=red, X=yellow, Y=all, releases=off. Power off the controller and check `[DISCONNECTED]` and LEDs off once the Bluetooth stack detects the disconnect.

Stick values are Bluepad32's normalized values (nominally -512 to 511); LT=`brake` and RT=`throttle` are nominally 0 to 1023. Analog snapshots are limited to 10 Hz and may skip intermediate samples; digital changes are logged separately. LED updates happen on every gamepad report. BTstack timers provide the waiting heartbeat and snapshots without `delay()` or sleeps in controller callbacks. These are bench diagnostics, not a lossless input recorder.

Use a Bluetooth-capable Xbox Wireless Controller. Upstream documents models 1708 and 1914 and their supported firmware; older Xbox controllers without Bluetooth cannot pair this way. Both Bluetooth Classic and BLE are enabled for Xbox firmware variants. See [Bluepad32's supported Xbox controllers](https://github.com/ricardoquesada/bluepad32/blob/e9b755faabc240585da42e6d26164bb2cdd064d3/docs/supported_gamepads.md#xbox-wireless-model-1708-2-buttons).

If boot appears but initialization does not, capture the intervening stack logs. If waiting repeats without discovery, check pairing mode, controller model, power, and other hosts. If connected appears without ready/input, capture the HID/parser logs and controller firmware version. Pairing keys persist normally. For stale bonds only, temporarily set `kForgetPairingKeysOnBoot = true` in `src/main.cpp`, upload and reboot, then restore `false` and upload again before pairing normally.

## Optional controller identity restriction

Initially `kAllowedControllerAddress` in `src/main.cpp` is empty: no Xbox MAC is hard-coded and normal pairing is allowed. The experiment explicitly disables any previously persisted allowlist when this constant is empty.

After pairing successfully, verify the address printed at `[READY]` stays the same through controller power cycles and ESP32 reboots. Then, optionally, put that exact colon-separated address in `kAllowedControllerAddress`, rebuild and upload. The code uses the verified APIs `uni_bt_allowlist_remove_all()`, `uni_bt_allowlist_add_addr()` and `uni_bt_allowlist_set_enabled()` during `on_init_complete`, before scanning starts. A malformed/zero address or insertion failure leaves acceptance disabled and prints an error. Bluepad32 stores the allowlist in NVS; this experiment replaces it when a nonempty constant is configured. Set the constant back to empty and upload to disable filtering again.

The printed value is `uni_hid_device_t::conn.btaddr`, the remote Bluetooth address exposed by the pinned library. Upstream checks its allowlist during discovery and incoming Classic connections. Address filtering is only useful if this controller/firmware presents a stable address; it is not cryptographic identity verification. A BLE address that changes, or an address changed by firmware/reset, can prevent reconnection. Leave filtering off until stability is confirmed on the actual controller.

## Dependencies and files

This uses the [official Bluepad32 ESP32 raw API example](https://github.com/ricardoquesada/bluepad32/tree/e9b755faabc240585da42e6d26164bb2cdd064d3/examples/esp32), adapted to C++ and a standalone PlatformIO project. That pinned example specifies ESP-IDF 5.3. Dependencies are pinned rather than following a moving development branch:

- PlatformIO `espressif32@6.9.0`, ESP-IDF 5.3.1, board `esp32doit-devkit-v1`.
- Bluepad32 4.2.0, commit `e9b755faabc240585da42e6d26164bb2cdd064d3`.
- BTstack submodule commit `5d4d8cc7b1d35a90bbd6d5ffd2d3050b2bfc861c`.

`prepare_bluepad32.py` downloads the pinned source into local ignored `.pio/bluepad32` and runs upstream's BTstack integration script there. No global ESP-IDF installation is patched. First build downloads can take several minutes; subsequent builds reuse the cache. Deleting this project's `.pio` directory causes dependencies to be fetched again.

| File | Purpose |
| --- | --- |
| `platformio.ini` | Separate ESP32 environment and serial ports |
| `esp32_xbox.code-workspace` | Opens the experiment with its own ESP32 editor configuration |
| `src/main.cpp` | Bluepad32 callbacks, diagnostics and LED mapping |
| `src/exp003.cpp` | Right-stick differential mixing, UART motor commands and input timeout |
| `CMakeLists.txt` | ESP-IDF project and dependency component paths |
| `src/CMakeLists.txt` | Registers the C++ application component |
| `prepare_bluepad32.py` | Reproducible source download and BTstack integration |
| `sdkconfig.defaults` | Classic/BLE, UART and custom Bluepad32 platform configuration |
| `dependencies.lock` | Generated ESP-IDF component version lock |
| `partitions.csv` | 3 MB app partition in 4 MB flash, NVS and core-dump space |
| `.gitignore` | Excludes generated SDK configuration and dependency/build cache |
| `README.md` | Setup, pairing, identity and verification instructions |

## Validation

`pio run -d experiments/esp32_xbox_controller` passed with PlatformIO Core 6.1.19:
83,284 bytes RAM (25.4%) and 679,766 bytes flash (21.6% of the 3 MB app partition).
The build produced `.pio/build/esp32_xbox/firmware.bin`. Upstream code emits
unused-console-function and legacy-timer deprecation warnings; no compile or link errors remain.

Hardware session notes (user-tested):

- Upload succeeded after lowering the upload speed from 460800 to 115200.
- The Trust controller connected but timed out during SDP/HID setup; compatibility remains unresolved.
- After switching to the original Xbox controller, the user reported a successful test; Serial showed button and stick input.
- Missing-header squiggles came from the root Mega/AVR editor configuration. Generated ESP32 IntelliSense metadata and added a dedicated workspace; firmware code stayed unchanged.

Physical testing was performed by the user, not the coding agent. Long-term connection stability and optional address filtering have not been verified.

## Experiment 003: UART motor control

`src/exp003.cpp` reuses the working Bluepad32 platform callbacks, discovery,
allowlist handling and pairing-key policy from `src/main.cpp`. The original
traffic-light source and `esp32_xbox` environment remain available. Select the
new `esp32_xbox_exp003` environment to build the motor sender with ESP-IDF.
The existing configuration has an empty controller allowlist address and keeps
pairing keys on boot; exp003 preserves those settings.

**Source/session discrepancy:** the session description says left stick; [the current source](src/exp003.cpp) reads `axis_rx` and `axis_ry`, so exp003 uses the **right stick**. Buttons, triggers and the left stick do not control motors here; exp003 does not drive the traffic-light LEDs.

Right-stick RX/RY use a 50-count per-axis deadzone, rescaled to full range.
Forward is negative RY, and right turn is positive RX. Mixing is
`left = forward + turn`, `right = forward - turn`; both outputs are scaled
proportionally when either exceeds 255. Axis values outside -512..512 cause stop; +512 is clamped to +511. After the inclusive ±50 deadzone, each axis is rescaled using its negative/positive full-scale limit (512/511) with integer arithmetic. No ESP32 motor-driver pins are used.

Connect GPIO17 TX2 to Mega RX1 pin 19 and share ground. UART2 is 115200 8N1,
TX only; leave Mega TX1 disconnected. Packets are ASCII `left,right\n`, with
signed integer values -255..255. Positive is forward; zero is stop.
Startup and controller-ready events send `0,0`; the first ready gamepad is active and additional controllers are rejected until it disconnects. The 50 ms timer keeps sending zero while waiting/disconnected. There is no separate arming button.
Commands repeat every 50 ms. Detected disconnect or unusable reports send `0,0`
immediately; absence of fresh input for 300 ms stops at the next 50 ms tick.
Some controllers may suppress unchanged reports: this deliberately stops on
stale input rather than assuming that a held command remains usable.
USB diagnostics show connection/input state, raw right-stick axes, motor values
and the transmitted packet every 500 ms (`[STATE]`), plus `[BOOT]`, Bluetooth connection events and `[STOP]` reasons. Raw axes retain their last values after stopping; `usable` and transmitted motor values indicate the stop. The Mega independently validates packets and stops after 400 ms without valid commands.

For exp003 pairing-key recovery or optional address restriction, edit the constants in `src/exp003.cpp`; the earlier instructions referencing `src/main.cpp` apply to the LED environment.

The reported first drive proved the complete wireless chain with USB power-bank control power and a separate 4.5 V motor supply. It does not constitute exhaustive disconnect/timeout testing. See the [power lessons](../../README.md#power-debugging-a-small-power-bank-big-moment).

From the repository root:

```bash
pio device list
pio run -d experiments/esp32_xbox_controller -e esp32_xbox_exp003
pio run -d experiments/esp32_xbox_controller -e esp32_xbox_exp003 -t upload --upload-port /dev/ttyUSB0
pio device monitor -d experiments/esp32_xbox_controller -e esp32_xbox_exp003 --port /dev/ttyUSB0 --baud 115200
```

Replace `/dev/ttyUSB0` with the actual ESP32 USB port. Close monitors before
uploading; Ctrl+C exits. The Mega has its own separate root PlatformIO build;
see the [motor experiment](../motor/README.md). Keep wheels raised during tests.
