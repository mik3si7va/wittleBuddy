#include <Arduino.h>
#include "UltrasonicSensor.h"
#include "exp001.h"

const int TRIG1 = 22;
const int ECHO1 = 23;
const int TRIG2 = 24;
const int ECHO2 = 25;
const int TRIG3 = 26;
const int ECHO3 = 27;
const int RED = 28;
const int YELLOW = 29;
const int GREEN = 30;

UltrasonicSensor left(TRIG1, ECHO1);
UltrasonicSensor front(TRIG2, ECHO2);
UltrasonicSensor right(TRIG3, ECHO3);

void ultrasonicExperimentSetup()
{
    left.begin();
    front.begin();
    right.begin();
    pinMode(RED, OUTPUT);
    pinMode(YELLOW, OUTPUT);
    pinMode(GREEN, OUTPUT);

    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, LOW);
    digitalWrite(GREEN, LOW);
}

void ultrasonicExperimentLoop()
{
    float cms[3] = {
        left.measureDistance(),
        front.measureDistance(),
        right.measureDistance()
    };

    float cm = min(cms[0], min(cms[1], cms[2]));

    if (cm <= 10)
    {
        digitalWrite(RED, HIGH);
        digitalWrite(YELLOW, LOW);
        digitalWrite(GREEN, LOW);
    }
    else if (cm <= 20)
    {
        digitalWrite(RED, LOW);
        digitalWrite(YELLOW, HIGH);
        digitalWrite(GREEN, LOW);
    }
    else
    {
        digitalWrite(RED, LOW);
        digitalWrite(YELLOW, LOW);
        digitalWrite(GREEN, HIGH);
    }
}
