#include <cstdio>
#include <cstdint>

extern "C" {
#include <btstack_port_esp32.h>
#include <btstack_run_loop.h>
#include <driver/uart.h>
#include <esp_timer.h>
#include <uni.h>
#include <uni_version.h>
#include <bt/uni_bt_allowlist.h>
}

namespace {
// Leave empty for normal discovery. Optional: copy the printed address here.
// This uses Bluepad32's actual persistent allowlist, not a discovery-only filter.
constexpr char kAllowedControllerAddress[] = "";
constexpr bool kForgetPairingKeysOnBoot = false;  // Temporary recovery only.
uni_hid_device_t* active = nullptr;
int joystickX = 0, joystickY = 0;
int leftMotor = 0, rightMotor = 0;
int64_t lastInputUs = 0;
bool inputUsable = false;
btstack_timer_source_t statusTimer = {};
unsigned debugTicks = 0;

void transmit() {
    char packet[16];
    const int length = snprintf(packet, sizeof(packet), "%d,%d\n", leftMotor, rightMotor);
    uart_write_bytes(UART_NUM_2, packet, length);
}

void stopInput(const char* reason) {
    const bool wasUsable = inputUsable;
    inputUsable = false;
    leftMotor = rightMotor = 0;
    transmit();
    static int64_t lastStopLogUs = -500000;
    const int64_t now = esp_timer_get_time();
    if (wasUsable || now - lastStopLogUs >= 500000) {
        printf("[STOP] %s TX=0,0\n", reason);
        lastStopLogUs = now;
    }
}

int scaleAxis(int value) {
    constexpr int deadzone = 50;
    if (value >= -deadzone && value <= deadzone)
        return 0;
    const int magnitude = value < 0 ? -value : value;
    const int maximum = value < 0 ? 512 : 511;
    const int scaled = (magnitude - deadzone) * 255 / (maximum - deadzone);
    return value < 0 ? -scaled : scaled;
}

void statusTick(btstack_timer_source_t* timer) {
    if (inputUsable && esp_timer_get_time() - lastInputUs >= 300000)
        stopInput("controller input timeout");
    transmit(); // 20 Hz heartbeat, including stop while waiting/disconnected.
    if (++debugTicks >= 10) {
        printf("[STATE] connected=%d usable=%d RX=%d RY=%d left=%d right=%d TX=%d,%d\n",
               active != nullptr, inputUsable, joystickX, joystickY,
               leftMotor, rightMotor, leftMotor, rightMotor);
        debugTicks = 0;
    }
    btstack_run_loop_set_timer(timer, 50);
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
    btstack_run_loop_set_timer(&statusTimer, 50);
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
    stopInput("controller ready; waiting for input");
    printf("[READY] Active controller Bluetooth address=%s; waiting for input.\n", bd_addr_to_str(d->conn.btaddr));
    return UNI_ERROR_SUCCESS;
}

void onDisconnected(uni_hid_device_t* d) {
    printf("[DISCONNECTED] Controller Bluetooth address=%s\n", bd_addr_to_str(d->conn.btaddr));
    if (active == d) {
        active = nullptr;
        stopInput("controller disconnected");
    }
}

void onData(uni_hid_device_t* d, uni_controller_t* ctl) {
    if (d != active)
        return;
    if (ctl->klass != UNI_CONTROLLER_CLASS_GAMEPAD) {
        stopInput("unusable controller report");
        return;
    }
    const auto& g = ctl->gamepad;
    if (g.axis_rx < -512 || g.axis_rx > 512 || g.axis_ry < -512 || g.axis_ry > 512) {
        stopInput("out-of-range controller axes");
        return;
    }
    joystickX = g.axis_rx > 511 ? 511 : g.axis_rx;
    joystickY = g.axis_ry > 511 ? 511 : g.axis_ry;
    const int turn = scaleAxis(joystickX);
    const int forward = -scaleAxis(joystickY);
    int left = forward + turn;
    int right = forward - turn;
    const int absLeft = left < 0 ? -left : left;
    const int absRight = right < 0 ? -right : right;
    const int peak = absLeft > absRight ? absLeft : absRight;
    if (peak > 255) {
        left = left * 255 / peak;
        right = right * 255 / peak;
    }
    leftMotor = left;
    rightMotor = right;
    lastInputUs = esp_timer_get_time();
    inputUsable = true;
}

const uni_property_t* getProperty(uni_property_idx_t) { return nullptr; }
void onOob(uni_platform_oob_event_t, void*) {}
}  // namespace

extern "C" void app_main() {
    setvbuf(stdout, nullptr, _IOLBF, 0);
    printf("[BOOT] Experiment 003: Xbox right stick -> UART2 TX GPIO17, 115200.\n");
    uart_config_t uartConfig = {};
    uartConfig.baud_rate = 115200;
    uartConfig.data_bits = UART_DATA_8_BITS;
    uartConfig.parity = UART_PARITY_DISABLE;
    uartConfig.stop_bits = UART_STOP_BITS_1;
    uartConfig.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uartConfig.source_clk = UART_SCLK_DEFAULT;
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uartConfig));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 17, UART_PIN_NO_CHANGE,
                                UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 256, 0, 0, nullptr, 0));
    stopInput("startup");
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
