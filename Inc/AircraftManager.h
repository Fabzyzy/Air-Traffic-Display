#pragma once

#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>
#include "Aircraft.h"

class AircraftManager
{
public:
    AircraftManager();

    void begin(float homeLatitude, float homeLongitude, int radiusKm);
    bool fetchAircraftData();
    void updateSelection(int8_t direction);
    const std::vector<Aircraft>& getAircraftList() const;
    const Aircraft* getSelectedAircraft() const;
    size_t getSelectedIndex() const;
    bool hasAircraft() const;
    uint32_t getLastFetchTimeMs() const;

private:
    String buildRequestUrl() const;
    bool parseAircraftData(const String& payload);
    String safeString(const JsonVariant& value) const;
    float safeFloat(const JsonVariant& value) const;
    int safeInt(const JsonVariant& value) const;
    void normalizeSelection();
    void updateDistanceAndBearing();

    float homeLatitude_ = 0.0f;
    float homeLongitude_ = 0.0f;
    int radiusKm_ = 100;
    std::vector<Aircraft> aircrafts_;
    size_t selectedIndex_ = 0;
    uint32_t lastFetchMs_ = 0;
};
