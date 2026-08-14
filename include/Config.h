#pragma once

#include <Arduino.h>

namespace Config
{
    constexpr uint8_t kEncoderClkPin = 25;
    constexpr uint8_t kEncoderDtPin = 26;
    constexpr uint8_t kEncoderSwPin = 32;

    constexpr unsigned long kEncoderDebounceMs = 3;
    constexpr unsigned long kEncoderLongPressMs = 1200;

    constexpr int kScreenSize = 240;
    constexpr int kCenterX = 120;
    constexpr int kCenterY = 120;
    constexpr int kSafeInset = 28;
    constexpr int kRadarRadiusPx = 100;

    constexpr float kRadarCenterLatitude = 51.5072f;
    constexpr float kRadarCenterLongitude = -0.1276f;

    constexpr int kMinDetectionRadiusKm = 25;
    constexpr int kMaxDetectionRadiusKm = 200;
    constexpr int kDefaultDetectionRadiusKm = 50;
    constexpr int kRadiusStepKm = 25;

    constexpr unsigned long kAircraftRefreshMs = 8000;
    constexpr unsigned long kSweepPeriodMs = 4000;
    constexpr unsigned long kRadarFrameMs = 40;
    constexpr unsigned long kContactGlowMs = 450;
    constexpr unsigned long kSuccessHintMs = 1400;
    constexpr unsigned long kWifiConnectTimeoutMs = 20000;
    constexpr unsigned long kPortalConnectTimeoutMs = 25000;
    constexpr uint16_t kHttpTimeoutMs = 10000;

    constexpr int kMaxSavedNetworks = 5;
    constexpr int kMaxAircraft = 32;

    constexpr uint32_t kSerialBaud = 115200;
}

namespace DisplayColors
{
    constexpr uint16_t kBackground = 0x0000;
    constexpr uint16_t kText = 0xFFFF;
    constexpr uint16_t kDimText = 0x8410;
    constexpr uint16_t kGreen = 0x07E0;
    constexpr uint16_t kBlue = 0x001F;
    constexpr uint16_t kAmber = 0xFBE0;
    constexpr uint16_t kWhite = 0xFFFF;
    constexpr uint16_t kRed = 0xF800;

    constexpr int kCount = 5;
    constexpr uint16_t kPalette[kCount] = {kGreen, kBlue, kAmber, kWhite, kRed};
    constexpr const char* kNames[kCount] = {"Green", "Blue", "Amber", "White", "Red"};
}
