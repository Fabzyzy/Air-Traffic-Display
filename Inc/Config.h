#pragma once
#include <stdint.h>

namespace Config {
    constexpr uint32_t kLongPressMs = 3000;
    constexpr uint32_t kAircraftFetchIntervalMs = 10000;
    constexpr uint32_t kRadarRefreshMs = 1000;

    constexpr float kRadarCenterLatitude = 51.5072f;
    constexpr float kRadarCenterLongitude = -0.1276f;
    constexpr int kRadarRangeKm = 100;

    constexpr uint8_t kTftCs = 5;
    constexpr uint8_t kTftDc = 27;
    constexpr uint8_t kTftRst = 33;

    constexpr uint8_t kEncoderClk = 25;
    constexpr uint8_t kEncoderDt = 26;
    constexpr uint8_t kEncoderSw = 32;

    constexpr int kDisplayWidth = 240;
    constexpr int kDisplayHeight = 240;
    constexpr int kRadarRadiusPx = 100;
}
