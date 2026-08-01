#pragma once

#include <Arduino.h>

namespace Config
{
    constexpr uint8_t kEncoderClkPin = 25;
    constexpr uint8_t kEncoderDtPin = 26;
    constexpr uint8_t kEncoderSwPin = 32;

    constexpr unsigned long kEncoderDebounceMs = 3;
    constexpr unsigned long kEncoderLongPressMs = 3000;
}
