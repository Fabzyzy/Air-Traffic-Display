#pragma once

#include <Arduino.h>
#include "Config.h"

struct Aircraft
{
    String callsign;
    String hex;
    String registration;
    String type;
    String squawk;
    String origin;
    String destination;
    float latitude = 0.0f;
    float longitude = 0.0f;
    int altitude = 0;
    float speed = 0.0f;
    int heading = 0;
    int verticalSpeed = 0;
    bool hasVerticalSpeed = false;
    bool hasPosition = false;
};

float geoDistanceKm(float lat1, float lon1, float lat2, float lon2);
float geoBearingDeg(float lat1, float lon1, float lat2, float lon2);

class AircraftDataFetcher
{
public:
    void setLocation(float latitude, float longitude, int radiusKm);
    bool requestFetch();
    bool isFetchInProgress() const;
    bool consumeFetchResult(bool& success);
    const Aircraft* getAircrafts() const;
    int getAircraftCount() const;
    bool hasValidAircraftData() const;
    unsigned long getLastSuccessfulUpdateMs() const;

    void fetchTaskBody();

private:
    float latitude_ = Config::kRadarCenterLatitude;
    float longitude_ = Config::kRadarCenterLongitude;
    int radiusKm_ = Config::kDefaultDetectionRadiusKm;

    Aircraft published_[Config::kMaxAircraft];
    int publishedCount_ = 0;
    bool publishedValid_ = false;

    Aircraft staging_[Config::kMaxAircraft];
    int stagingCount_ = 0;
    bool stagingValid_ = false;
    bool stagingSuccess_ = false;

    volatile bool fetchInProgress_ = false;
    volatile bool fetchComplete_ = false;
    unsigned long lastSuccessfulUpdateMs_ = 0;

    bool fetchIntoStaging();
};
