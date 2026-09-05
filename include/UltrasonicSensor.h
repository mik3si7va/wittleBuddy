#pragma once

class UltrasonicSensor
{
public:
    UltrasonicSensor(int trigPin, int echoPin);

    void begin();
    float measureDistance();

private:
    int trigPin;
    int echoPin;
};
