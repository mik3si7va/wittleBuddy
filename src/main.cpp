#include <Arduino.h>
#include "../experiments/ultrasonic/exp001.h"

void setup() {
    ultrasonicExperimentSetup();
}

void loop() {
    ultrasonicExperimentLoop();
}
