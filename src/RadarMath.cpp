#include "RadarMath.h"
#include <cmath>

namespace RadarMath {

float degreesToRadians(float degrees)
{
    return degrees * 0.017453292519943295f;
}

float radiansToDegrees(float radians)
{
    return radians * 57.295779513082320876f;
}

float computeDistanceKm(float latitude1, float longitude1, float latitude2, float longitude2)
{
    const float earthRadiusKm = 6371.0f;
    float lat1Rad = degreesToRadians(latitude1);
    float lat2Rad = degreesToRadians(latitude2);
    float deltaLat = degreesToRadians(latitude2 - latitude1);
    float deltaLon = degreesToRadians(longitude2 - longitude1);

    float a = sin(deltaLat / 2.0f) * sin(deltaLat / 2.0f) + cos(lat1Rad) * cos(lat2Rad) * sin(deltaLon / 2.0f) * sin(deltaLon / 2.0f);
    float c = 2.0f * atan2(sqrt(a), sqrt(1.0f - a));
    return earthRadiusKm * c;
}

float computeBearingDeg(float latitude1, float longitude1, float latitude2, float longitude2)
{
    float lat1Rad = degreesToRadians(latitude1);
    float lat2Rad = degreesToRadians(latitude2);
    float deltaLon = degreesToRadians(longitude2 - longitude1);

    float y = sin(deltaLon) * cos(lat2Rad);
    float x = cos(lat1Rad) * sin(lat2Rad) - sin(lat1Rad) * cos(lat2Rad) * cos(deltaLon);
    float bearingRad = atan2(y, x);
    float bearingDeg = radiansToDegrees(bearingRad);
    if (bearingDeg < 0.0f)
    {
        bearingDeg += 360.0f;
    }
    return bearingDeg;
}

} // namespace RadarMath
