#include "AircraftManager.h"
#include "RadarMath.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

namespace
{
    String buildUrl(float latitude, float longitude, int radiusKm)
    {
        String url = "https://api.adsb.lol/v2/point/";
        url += String(latitude, 6);
        url += "/";
        url += String(longitude, 6);
        url += "/";
        url += String(radiusKm);
        return url;
    }
}

AircraftManager::AircraftManager() = default;

void AircraftManager::begin(float homeLatitude, float homeLongitude, int radiusKm)
{
    homeLatitude_ = homeLatitude;
    homeLongitude_ = homeLongitude;
    radiusKm_ = radiusKm;
    aircrafts_.clear();
    selectedIndex_ = 0;
    lastFetchMs_ = 0;
}

bool AircraftManager::fetchAircraftData()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected, skipping aircraft fetch.");
        return false;
    }

    String url = buildRequestUrl();
    Serial.print("Fetching aircraft from: ");
    Serial.println(url);

    HTTPClient http;
    http.begin(url);
    http.setTimeout(10000);

    int httpCode = http.GET();
    if (httpCode <= 0)
    {
        Serial.print("HTTP GET failed: ");
        Serial.println(http.errorToString(httpCode));
        http.end();
        return false;
    }

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.print("HTTP GET returned code: ");
        Serial.println(httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    if (payload.length() == 0)
    {
        Serial.println("HTTP response payload is empty.");
        return false;
    }

    bool parsed = parseAircraftData(payload);
    if (parsed)
    {
        lastFetchMs_ = millis();
        normalizeSelection();
        updateDistanceAndBearing();
    }
    return parsed;
}

const std::vector<Aircraft>& AircraftManager::getAircraftList() const
{
    return aircrafts_;
}

const Aircraft* AircraftManager::getSelectedAircraft() const
{
    if (selectedIndex_ < aircrafts_.size())
    {
        return &aircrafts_[selectedIndex_];
    }
    return nullptr;
}

size_t AircraftManager::getSelectedIndex() const
{
    return selectedIndex_;
}

bool AircraftManager::hasAircraft() const
{
    return !aircrafts_.empty();
}

uint32_t AircraftManager::getLastFetchTimeMs() const
{
    return lastFetchMs_;
}

void AircraftManager::updateSelection(int8_t direction)
{
    if (aircrafts_.empty())
    {
        return;
    }

    int delta = (direction > 0) ? 1 : -1;
    int newIndex = static_cast<int>(selectedIndex_) + delta;
    if (newIndex < 0)
    {
        newIndex = static_cast<int>(aircrafts_.size()) - 1;
    }
    else if (newIndex >= static_cast<int>(aircrafts_.size()))
    {
        newIndex = 0;
    }
    selectedIndex_ = static_cast<size_t>(newIndex);
    Serial.print("Selected aircraft index: ");
    Serial.println(selectedIndex_);
}

String AircraftManager::buildRequestUrl() const
{
    return buildUrl(homeLatitude_, homeLongitude_, radiusKm_);
}

bool AircraftManager::parseAircraftData(const String& payload)
{
    StaticJsonDocument<262144> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray aircraftArray = doc["ac"].as<JsonArray>();
    if (aircraftArray.isNull())
    {
        Serial.println("No aircraft array in JSON payload.");
        return false;
    }

    aircrafts_.clear();
    for (JsonVariant value : aircraftArray)
    {
        JsonObject aircraft = value.as<JsonObject>();
        Aircraft entry;
        entry.callsign = safeString(aircraft["flight"]);
        entry.hex = safeString(aircraft["hex"]);
        entry.registration = safeString(aircraft["r"]);
        entry.aircraftType = safeString(aircraft["t"]);
        entry.origin = safeString(aircraft["from"]);
        entry.destination = safeString(aircraft["to"]);
        entry.squawk = safeString(aircraft["squawk"]);
        entry.latitude = safeFloat(aircraft["lat"]);
        entry.longitude = safeFloat(aircraft["lon"]);
        entry.altitude = safeInt(aircraft["alt_baro"]);
        entry.groundSpeed = safeFloat(aircraft["gs"]);
        entry.heading = safeFloat(aircraft["track"]);
        entry.verticalSpeed = safeFloat(aircraft["baro_rate"]);
        entry.lastSeen = safeFloat(aircraft["seen"]);

        if (entry.latitude == 0.0f && entry.longitude == 0.0f)
        {
            continue;
        }

        aircrafts_.push_back(entry);
    }

    Serial.print("Parsed aircraft count: ");
    Serial.println(aircrafts_.size());
    return true;
}

String AircraftManager::safeString(const JsonVariant& value) const
{
    if (value.isNull() || !value.is<const char*>())
    {
        return String();
    }
    return String(value.as<const char*>());
}

float AircraftManager::safeFloat(const JsonVariant& value) const
{
    if (value.isNull())
    {
        return 0.0f;
    }
    if (value.is<const char*>())
    {
        return String(value.as<const char*>()).toFloat();
    }
    return value.as<float>();
}

int AircraftManager::safeInt(const JsonVariant& value) const
{
    if (value.isNull())
    {
        return 0;
    }
    if (value.is<const char*>())
    {
        return String(value.as<const char*>()).toInt();
    }
    return value.as<int>();
}

void AircraftManager::normalizeSelection()
{
    if (aircrafts_.empty())
    {
        selectedIndex_ = 0;
        return;
    }

    if (selectedIndex_ >= aircrafts_.size())
    {
        selectedIndex_ = 0;
    }
}

void AircraftManager::updateDistanceAndBearing()
{
    for (Aircraft& aircraft : aircrafts_)
    {
        aircraft.distanceKm = RadarMath::computeDistanceKm(homeLatitude_, homeLongitude_, aircraft.latitude, aircraft.longitude);
        aircraft.bearingDeg = RadarMath::computeBearingDeg(homeLatitude_, homeLongitude_, aircraft.latitude, aircraft.longitude);
    }
}
