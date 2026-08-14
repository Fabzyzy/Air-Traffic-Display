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
    const Aircraft* getAircrafts() const;
    int getAircraftCount() const;
    bool hasValidAircraftData() const;
    bool hasStaleData() const;
    unsigned long getLastSuccessfulUpdateMs() const;

private:
    static constexpr int kMaxAircraft = 32;

    float latitude_ = 51.5072f;
    float longitude_ = -0.1276f;
    int radiusKm_ = 100;
    Aircraft aircrafts_[kMaxAircraft];
    int aircraftCount_ = 0;
    bool aircraftDataValid_ = false;
    unsigned long lastSuccessfulUpdateMs_ = 0;
};
