#include "AircraftData.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define DEBUG_GENERAL 1
#define DEBUG_HTTP 1
#define DEBUG_AIRCRAFT 0

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

    String safeString(const JsonVariant& value)
    {
        if (value.isNull() || !value.is<const char*>())
        {
            return String();
        }
        return String(value.as<const char*>());
    }
}

void AircraftDataFetcher::setLocation(float latitude, float longitude, int radiusKm)
{
    latitude_ = latitude;
    longitude_ = longitude;
    radiusKm_ = radiusKm;
}

bool AircraftDataFetcher::fetchAndPrintAircrafts()
{
    aircraftDataValid_ = false;
    aircraftCount_ = 0;

    if (WiFi.status() != WL_CONNECTED)
    {
#if DEBUG_GENERAL || DEBUG_HTTP
        Serial.println("[AIRCRAFT] Wi-Fi not connected, skipping ADS-B fetch.");
#endif
        return false;
    }

    Serial.println("[AIRCRAFT] Requesting data...");

    HTTPClient http;
    String url = buildUrl(latitude_, longitude_, radiusKm_);
    http.begin(url);
    http.setTimeout(10000);

    int httpCode = http.GET();
    if (httpCode <= 0)
    {
#if DEBUG_HTTP
        Serial.print("[AIRCRAFT] HTTP FAILED");
        Serial.print(" (code ");
        Serial.print(httpCode);
        Serial.print(")");
        if (http.errorToString(httpCode).length() > 0)
        {
            Serial.print(": ");
            Serial.print(http.errorToString(httpCode));
        }
        Serial.println();
#endif
        http.end();
        return false;
    }

    if (httpCode != HTTP_CODE_OK)
    {
#if DEBUG_HTTP
        Serial.print("[HTTP] Returned code: ");
        Serial.println(httpCode);
#endif
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    Serial.print("[AIRCRAFT] HTTP ");
    Serial.print(httpCode);
    Serial.print(", ");
    Serial.print(payload.length());
    Serial.println(" bytes");

    if (payload.length() == 0)
    {
#if DEBUG_HTTP
        Serial.println("[HTTP] Response payload is empty; treating as unavailable aircraft feed");
#endif
        http.end();
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
#if DEBUG_HTTP
        Serial.print("[HTTP] JSON parse failed: ");
        Serial.println(error.c_str());
#endif
        return false;
    }

    JsonArray aircrafts = doc["ac"].as<JsonArray>();
    aircraftCount_ = 0;
#if DEBUG_AIRCRAFT
    Serial.println("[AIRCRAFT] Aircraft data received");
    Serial.print("[AIRCRAFT] Count: ");
    Serial.println(aircrafts.size());
#else
    Serial.print("[AIRCRAFT] Updated (");
    Serial.print(aircrafts.size());
    Serial.println(" aircraft)");
#endif

    for (JsonVariant value : aircrafts)
    {
        if (aircraftCount_ >= kMaxAircraft)
        {
            break;
        }

        JsonObject aircraft = value.as<JsonObject>();


        Aircraft entry;
        entry.callsign = safeString(aircraft["flight"]);
        entry.latitude = aircraft["lat"].as<float>();
        entry.longitude = aircraft["lon"].as<float>();
        entry.altitude = aircraft["alt_baro"].as<int>();
        entry.speed = aircraft["gs"].as<float>();
        entry.heading = aircraft["track"].as<int>();
        entry.hex = safeString(aircraft["hex"]);
        aircrafts_[aircraftCount_++] = entry;

#if DEBUG_AIRCRAFT
        Serial.print("Callsign: ");
        Serial.println(entry.callsign.length() > 0 ? entry.callsign : "<none>");
        Serial.print("  Latitude: ");
        Serial.println(entry.latitude, 6);
        Serial.print("  Longitude: ");
        Serial.println(entry.longitude, 6);
        Serial.print("  Altitude: ");
        Serial.println(entry.altitude);
        Serial.print("  Speed: ");
        Serial.println(entry.speed);
        Serial.print("  Heading: ");
        Serial.println(entry.heading);
        Serial.print("  Hex: ");
        Serial.println(entry.hex);
        Serial.println();
#endif
    }


    if (aircraftCount_ == 0)
    {
        Serial.print("[AIRCRAFT] API returned ");
        Serial.print(aircrafts.size());
        Serial.print(" but ");
        Serial.print(aircraftCount_);
        Serial.println(" remain after filtering");
        return false;
    }

    aircraftCount_ = aircraftCount_;
    aircraftDataValid_ = true;
    lastSuccessfulUpdateMs_ = millis();
    Serial.print("[AIRCRAFT] Parsed ");
    Serial.print(aircraftCount_);
    Serial.println(" aircraft");
    return true;
}

const Aircraft* AircraftDataFetcher::getAircrafts() const
{
    return aircraftDataValid_ && aircraftCount_ > 0 ? aircrafts_ : nullptr;
}

int AircraftDataFetcher::getAircraftCount() const
{
    return aircraftDataValid_ ? aircraftCount_ : 0;
}

bool AircraftDataFetcher::hasValidAircraftData() const
{
    return aircraftDataValid_ && aircraftCount_ > 0;
}

bool AircraftDataFetcher::hasStaleData() const
{
    return aircraftDataValid_ && millis() - lastSuccessfulUpdateMs_ > 30000;
}

unsigned long AircraftDataFetcher::getLastSuccessfulUpdateMs() const
{
    return lastSuccessfulUpdateMs_;
}
