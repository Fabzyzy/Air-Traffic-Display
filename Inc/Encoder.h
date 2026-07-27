#pragma once

#include <Arduino.h>
#include "Config.h"

using EncoderRotationHandler = void (*)(int8_t direction);
using EncoderButtonHandler = void (*)();

class Encoder
{
public:
    void begin();
    void update();
    void setRotationHandler(EncoderRotationHandler handler);
    void setShortPressHandler(EncoderButtonHandler handler);
    void setLongPressHandler(EncoderButtonHandler handler);

private:
    int lastCLK_ = HIGH;
    bool buttonDown_ = false;
    bool longPressSent_ = false;
    uint32_t buttonDownTime_ = 0;

    EncoderRotationHandler rotationHandler_ = nullptr;
    EncoderButtonHandler shortPressHandler_ = nullptr;
    EncoderButtonHandler longPressHandler_ = nullptr;
};
