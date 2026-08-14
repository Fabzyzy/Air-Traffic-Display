#pragma once

#include <Arduino.h>

namespace Config
{
    constexpr uint8_t kEncoderClkPin = 25;
    constexpr uint8_t kEncoderDtPin = 26;
    constexpr uint8_t kEncoderSwPin = 32;

    constexpr unsigned long kEncoderDebounceMs = 3;
    constexpr unsigned long kEncoderLongPressMs = 3000;

    constexpr uint16_t kDefaultRadarColor = 0x07E0;
    constexpr int kMinDetectionRadiusKm = 25;
    constexpr int kMaxDetectionRadiusKm = 200;
    constexpr int kRadiusStepKm = 25;
    constexpr unsigned long kAircraftRefreshMs = 5000;
    constexpr unsigned long kSuccessHintMs = 1200;
}

namespace DisplayColors
{
    constexpr uint16_t kGreen = 0x07E0;
    constexpr uint16_t kBlue = 0x001F;
    constexpr uint16_t kAmber = 0xFBE0;
    constexpr uint16_t kWhite = 0xFFFF;
    constexpr uint16_t kRed = 0xF800;
}
