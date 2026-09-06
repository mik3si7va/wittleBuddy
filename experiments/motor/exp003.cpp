#include <Arduino.h>
#include "exp003.h"

namespace {
const int PWMA = 5, AIN1 = 22, AIN2 = 23;
const int STBY = 24;
const int BIN1 = 25, BIN2 = 26, PWMB = 6;
const unsigned long COMMAND_TIMEOUT_MS = 400;
char packet[16];
uint8_t packetLength = 0;
bool discarding = false;
bool commandActive = false;
unsigned long lastCommandMs = 0, lastDebugMs = 0;

void setMotor(int pwm, int in1, int in2, int speed) {
    speed = constrain(speed, -255, 255);
    analogWrite(pwm, 0); // Remove drive before changing direction.
    digitalWrite(in1, speed < 0 ? HIGH : LOW);
    digitalWrite(in2, speed > 0 ? HIGH : LOW);
    analogWrite(pwm, speed < 0 ? -speed : speed);
}

// Strict optional-minus decimal parser: no whitespace, junk or overflow.
bool parseSpeed(const char*& cursor, int& speed) {
    bool negative = false;
    if (*cursor == '-') {
        negative = true;
        ++cursor;
    }
    if (*cursor < '0' || *cursor > '9')
        return false;
    int value = 0;
    while (*cursor >= '0' && *cursor <= '9') {
        value = value * 10 + (*cursor++ - '0');
        if (value > 255)
            return false;
    }
    speed = negative ? -value : value;
    return true;
}
}

void setMotorA(int speed) { setMotor(PWMA, AIN1, AIN2, speed); }
void setMotorB(int speed) { setMotor(PWMB, BIN1, BIN2, speed); }
void setMotors(int left, int right) {
    setMotorA(left);
    setMotorB(right);
}
void stopMotors() { setMotors(0, 0); }

void motorExperimentSetup() {
    pinMode(STBY, OUTPUT);
    digitalWrite(STBY, LOW);
    pinMode(PWMA, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(PWMB, OUTPUT);
    stopMotors();
    digitalWrite(STBY, HIGH);
    Serial.begin(115200);
    Serial1.begin(115200);
    Serial.println(F("[BOOT] exp003 stopped; Serial1 RX19, 115200; watchdog=400ms"));
}

void motorExperimentLoop() {
    // Bound work per iteration so incoming garbage cannot starve the watchdog.
    for (uint8_t count = 0; count < 32 && Serial1.available(); ++count) {
        const char ch = static_cast<char>(Serial1.read());
        if (ch != '\n') {
            if (ch < 32 || ch > 126 || packetLength >= sizeof(packet) - 1)
                discarding = true;
            if (!discarding)
                packet[packetLength++] = ch;
            continue;
        }
        packet[packetLength] = '\0';
        const char* cursor = packet;
        int left = 0, right = 0;
        const bool valid = !discarding && parseSpeed(cursor, left) &&
                           *cursor++ == ',' && parseSpeed(cursor, right) &&
                           *cursor == '\0';
        const unsigned long now = millis();
        if (valid) {
            setMotors(left, right);
            lastCommandMs = now;
            const bool resumed = !commandActive;
            commandActive = true;
            if (resumed || now - lastDebugMs >= 250) {
                Serial.print(F("[RX] "));
                Serial.print(packet);
                Serial.print(F(" left="));
                Serial.print(left);
                Serial.print(F(" right="));
                Serial.println(right);
                lastDebugMs = now;
            }
        } else {
            stopMotors(); // Reject the entire packet, never apply partial values.
            if (now - lastDebugMs >= 250) {
                Serial.print(F("[REJECT] malformed/out-of-range RX="));
                Serial.println(packet);
                lastDebugMs = now;
            }
        }
        packetLength = 0;
        discarding = false;
    }
    if (commandActive && millis() - lastCommandMs >= COMMAND_TIMEOUT_MS) {
        stopMotors();
        commandActive = false;
        Serial.println(F("[FAILSAFE] No valid command for 400ms; motors stopped"));
    }
}
