#include <Arduino.h>
#include "UltrasonicSensor.h"

UltrasonicSensor::UltrasonicSensor(int trigPin, int echoPin)
    : trigPin(trigPin), echoPin(echoPin)
{
}

void UltrasonicSensor::begin()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    digitalWrite(trigPin, LOW);
}

float UltrasonicSensor::measureDistance()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    const unsigned long pulse = pulseIn(echoPin, HIGH);
    // Preserve the experiment's whole-centimeter conversion.
    const float cm = pulse / 58;
    return cm;
}
