#pragma once

namespace RadarMath {
    float degreesToRadians(float degrees);
    float radiansToDegrees(float radians);
    float computeDistanceKm(float latitude1, float longitude1, float latitude2, float longitude2);
    float computeBearingDeg(float latitude1, float longitude1, float latitude2, float longitude2);
}
