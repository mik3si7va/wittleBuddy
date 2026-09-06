#include <cstdio>
#include <cstdint>
#include <initializer_list>

extern "C" {
#include <btstack_port_esp32.h>
#include <btstack_run_loop.h>
#include <driver/gpio.h>
#include <uni.h>
#include <uni_version.h>
#include <bt/uni_bt_allowlist.h>
}

namespace {
constexpr gpio_num_t kRed = GPIO_NUM_25;
constexpr gpio_num_t kYellow = GPIO_NUM_26;
constexpr gpio_num_t kGreen = GPIO_NUM_27;
// Leave empty for normal discovery. Optional: copy the printed address here.
// This uses Bluepad32's actual persistent allowlist, not a discovery-only filter.
constexpr char kAllowedControllerAddress[] = "";
constexpr bool kForgetPairingKeysOnBoot = false;  // Temporary recovery only.
uni_hid_device_t* active = nullptr;
uni_gamepad_t latest = {};
bool inputPending = false;
bool firstInput = true;
btstack_timer_source_t statusTimer = {};
unsigned waitingTicks = 0;

void setLeds(uint16_t buttons) {
    const bool all = buttons & BUTTON_Y;
    gpio_set_level(kRed, all || (buttons & BUTTON_B));
    gpio_set_level(kYellow, all || (buttons & BUTTON_X));
    gpio_set_level(kGreen, all || (buttons & BUTTON_A));
}

void statusTick(btstack_timer_source_t* timer) {
    if (active && inputPending) {
        const auto& g = latest;
        printf("[INPUT] A=%d B=%d X=%d Y=%d D-pad=0x%02x(U=%d D=%d L=%d R=%d) "
               "LX=%ld LY=%ld RX=%ld RY=%ld LT=%ld RT=%ld\n",
               !!(g.buttons & BUTTON_A), !!(g.buttons & BUTTON_B),
               !!(g.buttons & BUTTON_X), !!(g.buttons & BUTTON_Y),
               g.dpad, !!(g.dpad & DPAD_UP), !!(g.dpad & DPAD_DOWN),
               !!(g.dpad & DPAD_LEFT), !!(g.dpad & DPAD_RIGHT),
               static_cast<long>(g.axis_x), static_cast<long>(g.axis_y),
               static_cast<long>(g.axis_rx), static_cast<long>(g.axis_ry),
               static_cast<long>(g.brake), static_cast<long>(g.throttle));
        inputPending = false;
    }
    if (!active && ++waitingTicks >= 50) {
        printf("[WAITING] Waiting for controller; enable Xbox pairing mode.\n");
        waitingTicks = 0;
    }
    btstack_run_loop_set_timer(timer, 100);
    btstack_run_loop_add_timer(timer);
}

void onInit(int, const char**) {
    printf("[BLUEPAD32] Initializing version %s\n", UNI_VERSION_STRING);
}

void onInitComplete() {
    printf("[BLUEPAD32] Initialized; Bluetooth stack ready.\n");
    if (kAllowedControllerAddress[0]) {
        bd_addr_t addr = {};
        const bd_addr_t zero = {};
        // Fail closed on a typo: no scanning or incoming connections.
        if (!sscanf_bd_addr(kAllowedControllerAddress, addr) || bd_addr_cmp(addr, zero) == 0) {
            printf("[ERROR] Invalid allowed controller address; Bluetooth acceptance disabled.\n");
            uni_bt_allow_incoming_connections(false);
            return;
        }
        uni_bt_allowlist_remove_all();
        if (!uni_bt_allowlist_add_addr(addr)) {
            printf("[ERROR] Cannot add controller to allowlist; acceptance disabled.\n");
            uni_bt_allow_incoming_connections(false);
            return;
        }
        uni_bt_allowlist_set_enabled(true);
        printf("[IDENTITY] Only accepting Bluetooth address %s\n", bd_addr_to_str(addr));
    } else {
        // Disable any allowlist left in NVS by a previous experiment build.
        uni_bt_allowlist_set_enabled(false);
        printf("[IDENTITY] Address filtering disabled; normal discovery/pairing.\n");
    }
    if (kForgetPairingKeysOnBoot)
        uni_bt_del_keys_unsafe();
    uni_bt_allow_incoming_connections(true);
    uni_bt_start_scanning_and_autoconnect_unsafe();
    printf("[WAITING] Waiting for controller.\n");
    btstack_run_loop_set_timer_handler(&statusTimer, statusTick);
    btstack_run_loop_set_timer(&statusTimer, 100);
    btstack_run_loop_add_timer(&statusTimer);
}

uni_error_t onDiscovered(bd_addr_t addr, const char* name, uint16_t cod, uint8_t rssi) {
    printf("[DISCOVERED] Bluetooth address=%s name=%s CoD=0x%04x RSSI(raw)=%u\n",
           bd_addr_to_str(addr), name && name[0] ? name : "(not supplied)", cod, rssi);
    return UNI_ERROR_SUCCESS;
}

void onConnected(uni_hid_device_t* d) {
    printf("[CONNECTED] Controller Bluetooth address=%s; preparing HID.\n", bd_addr_to_str(d->conn.btaddr));
}

uni_error_t onReady(uni_hid_device_t* d) {
    // Report klass is not initialized until the first Xbox input report.
    if (!uni_hid_device_is_gamepad(d) || (active && active != d)) {
        printf("[REJECTED] Non-gamepad or another controller already active: %s\n", bd_addr_to_str(d->conn.btaddr));
        return UNI_ERROR_IGNORE_DEVICE;
    }
    active = d;
    firstInput = true;
    inputPending = false;
    setLeds(0);
    printf("[READY] Active controller Bluetooth address=%s; waiting for input.\n", bd_addr_to_str(d->conn.btaddr));
    return UNI_ERROR_SUCCESS;
}

void onDisconnected(uni_hid_device_t* d) {
    printf("[DISCONNECTED] Controller Bluetooth address=%s\n", bd_addr_to_str(d->conn.btaddr));
    if (active == d) {
        active = nullptr;
        inputPending = false;
        setLeds(0);
        printf("[WAITING] LEDs off; waiting for controller.\n");
    }
}

void onData(uni_hid_device_t* d, uni_controller_t* ctl) {
    if (d != active || ctl->klass != UNI_CONTROLLER_CLASS_GAMEPAD)
        return;
    // Apply every report immediately; serial snapshots are limited to 10 Hz.
    setLeds(ctl->gamepad.buttons);
    const auto& g = ctl->gamepad;
    if (firstInput || g.buttons != latest.buttons || g.dpad != latest.dpad) {
        printf("[BUTTONS] A=%d B=%d X=%d Y=%d D-pad=0x%02x\n",
               !!(g.buttons & BUTTON_A), !!(g.buttons & BUTTON_B),
               !!(g.buttons & BUTTON_X), !!(g.buttons & BUTTON_Y), g.dpad);
    }
    latest = ctl->gamepad;
    inputPending = true;
    if (firstInput) {
        printf("[INPUT] First controller input received.\n");
        firstInput = false;
    }
}

const uni_property_t* getProperty(uni_property_idx_t) { return nullptr; }
void onOob(uni_platform_oob_event_t, void*) {}
}  // namespace

extern "C" void app_main() {
    setvbuf(stdout, nullptr, _IOLBF, 0);
    printf("[BOOT] ESP32 booted: Xbox controller / traffic-light experiment, UART 115200.\n");
    for (auto pin : {kRed, kYellow, kGreen}) {
        gpio_reset_pin(pin);
        gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    }
    setLeds(0);
    static uni_platform platform = {};
    platform.name = "wittle_xbox_experiment";
    platform.init = onInit;
    platform.on_init_complete = onInitComplete;
    platform.on_device_discovered = onDiscovered;
    platform.on_device_connected = onConnected;
    platform.on_device_disconnected = onDisconnected;
    platform.on_device_ready = onReady;
    platform.on_controller_data = onData;
    platform.get_property = getProperty;
    platform.on_oob_event = onOob;
    btstack_init();
    uni_platform_set_custom(&platform);
    uni_init(0, nullptr);
    // Event-driven BTstack loop dispatches input and timers; no delay() calls.
    btstack_run_loop_execute();
}
