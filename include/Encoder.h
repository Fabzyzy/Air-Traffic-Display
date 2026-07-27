#pragma once
#include <Arduino.h>

class Encoder
{
public:
    void begin();
    void update();

private:
    int lastCLK = HIGH;
};