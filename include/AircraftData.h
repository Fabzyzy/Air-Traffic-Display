#pragma once

#include <Arduino.h>

struct Aircraft
{
    String callsign;
    float latitude;
    float longitude;
    int altitude;
    float speed;
    int heading;
    String hex;
};

class AircraftDataFetcher
{
public:
    bool fetchAndPrintAircrafts();
    void setLocation(float latitude, float longitude, int radiusKm);

private:
    float latitude_ = 51.5072f;
    float longitude_ = -0.1276f;
    int radiusKm_ = 100;
};
