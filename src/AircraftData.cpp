#include "AircraftData.h"

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
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Wi-Fi not connected, skipping ADS-B fetch.");
        return false;
    }

    HTTPClient http;
    String url = buildUrl(latitude_, longitude_, radiusKm_);
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

    DynamicJsonDocument doc(20000);
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray aircrafts = doc["ac"].as<JsonArray>();
    Serial.println("--- Aircraft data ---");
    Serial.print("Found aircraft count: ");
    Serial.println(aircrafts.size());

    for (JsonVariant value : aircrafts)
    {
        JsonObject aircraft = value.as<JsonObject>();

        Aircraft entry;
        entry.callsign = safeString(aircraft["flight"]);
        entry.latitude = aircraft["lat"].as<float>();
        entry.longitude = aircraft["lon"].as<float>();
        entry.altitude = aircraft["alt_baro"].as<int>();
        entry.speed = aircraft["gs"].as<float>();
        entry.heading = aircraft["track"].as<int>();
        entry.hex = safeString(aircraft["hex"]);

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
    }

    return true;
}
