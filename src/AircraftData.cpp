#include "AircraftData.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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
        if (value.isNull())
        {
            return String();
        }
        if (value.is<const char*>())
        {
            String text = value.as<const char*>();
            text.trim();
            return text;
        }
        if (value.is<int>())
        {
            return String(value.as<int>());
        }
        return String();
    }

    int safeInt(const JsonVariant& value)
    {
        if (value.isNull() || value.is<const char*>())
        {
            return 0;
        }
        return value.as<int>();
    }

    float safeFloat(const JsonVariant& value)
    {
        if (value.isNull() || value.is<const char*>())
        {
            return 0.0f;
        }
        return value.as<float>();
    }

    void fetchTask(void* param)
    {
        auto* self = static_cast<AircraftDataFetcher*>(param);
        self->fetchTaskBody();
        vTaskDelete(nullptr);
    }
}

float geoDistanceKm(float lat1, float lon1, float lat2, float lon2)
{
    const float meanLat = (lat1 + lat2) * 0.5f * 0.01745329252f;
    const float northKm = (lat2 - lat1) * 110.574f;
    const float eastKm = (lon2 - lon1) * 111.32f * cosf(meanLat);
    return sqrtf(northKm * northKm + eastKm * eastKm);
}

float geoBearingDeg(float lat1, float lon1, float lat2, float lon2)
{
    const float meanLat = (lat1 + lat2) * 0.5f * 0.01745329252f;
    const float northKm = (lat2 - lat1) * 110.574f;
    const float eastKm = (lon2 - lon1) * 111.32f * cosf(meanLat);
    float deg = atan2f(eastKm, northKm) * 57.2957795f;
    if (deg < 0.0f)
    {
        deg += 360.0f;
    }
    return deg;
}

void AircraftDataFetcher::setLocation(float latitude, float longitude, int radiusKm)
{
    latitude_ = latitude;
    longitude_ = longitude;
    radiusKm_ = radiusKm;
}

bool AircraftDataFetcher::requestFetch()
{
    if (fetchInProgress_ || WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    fetchComplete_ = false;
    fetchInProgress_ = true;
    const BaseType_t ok = xTaskCreatePinnedToCore(fetchTask, "adsb", 12288, this, 1, nullptr, 0);
    if (ok != pdPASS)
    {
        fetchInProgress_ = false;
        Serial.println("[AIRCRAFT] HTTP failed: task create");
        return false;
    }
    return true;
}

bool AircraftDataFetcher::isFetchInProgress() const
{
    return fetchInProgress_;
}

bool AircraftDataFetcher::consumeFetchResult(bool& success)
{
    if (!fetchComplete_)
    {
        return false;
    }

    success = stagingSuccess_;
    if (stagingValid_)
    {
        publishedCount_ = stagingCount_;
        publishedValid_ = true;
        for (int i = 0; i < publishedCount_; ++i)
        {
            published_[i] = staging_[i];
        }
        lastSuccessfulUpdateMs_ = millis();
    }
    fetchComplete_ = false;
    return true;
}

const Aircraft* AircraftDataFetcher::getAircrafts() const
{
    return publishedValid_ ? published_ : nullptr;
}

int AircraftDataFetcher::getAircraftCount() const
{
    return publishedValid_ ? publishedCount_ : 0;
}

bool AircraftDataFetcher::hasValidAircraftData() const
{
    return publishedValid_;
}

unsigned long AircraftDataFetcher::getLastSuccessfulUpdateMs() const
{
    return lastSuccessfulUpdateMs_;
}

void AircraftDataFetcher::fetchTaskBody()
{
    stagingSuccess_ = fetchIntoStaging();
    fetchComplete_ = true;
    fetchInProgress_ = false;
}

bool AircraftDataFetcher::fetchIntoStaging()
{
    stagingCount_ = 0;
    stagingValid_ = false;

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[AIRCRAFT] HTTP failed: no wifi");
        return false;
    }

    HTTPClient http;
    const String url = buildUrl(latitude_, longitude_, radiusKm_);
    http.begin(url);
    http.setTimeout(Config::kHttpTimeoutMs);

    const int httpCode = http.GET();
    if (httpCode <= 0)
    {
        Serial.print("[AIRCRAFT] HTTP failed: ");
        Serial.println(httpCode);
        http.end();
        return false;
    }

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.print("[AIRCRAFT] HTTP ");
        Serial.println(httpCode);
        http.end();
        return false;
    }

    const String payload = http.getString();
    http.end();

    Serial.print("[AIRCRAFT] HTTP 200, ");
    Serial.print(payload.length());
    Serial.println(" bytes");

    if (payload.length() == 0)
    {
        Serial.println("[AIRCRAFT] Empty response");
        return false;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.print("[AIRCRAFT] JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    if (!doc["ac"].is<JsonArray>())
    {
        Serial.println("[AIRCRAFT] JSON parse failed");
        return false;
    }

    JsonArray aircrafts = doc["ac"].as<JsonArray>();
    const int apiCount = static_cast<int>(aircrafts.size());
    if (apiCount == 0)
    {
        Serial.println("[AIRCRAFT] API returned 0 aircraft");
        stagingCount_ = 0;
        stagingValid_ = true;
        return true;
    }

    int parsed = 0;
    for (JsonVariant value : aircrafts)
    {
        if (parsed >= Config::kMaxAircraft)
        {
            break;
        }

        JsonObject aircraft = value.as<JsonObject>();
        Aircraft entry;
        entry.callsign = safeString(aircraft["flight"]);
        entry.hex = safeString(aircraft["hex"]);
        entry.registration = safeString(aircraft["r"]);
        entry.type = safeString(aircraft["t"]);
        entry.squawk = safeString(aircraft["squawk"]);
        entry.origin = safeString(aircraft["orig"]);
        if (entry.origin.length() == 0)
        {
            entry.origin = safeString(aircraft["from"]);
        }
        entry.destination = safeString(aircraft["dest"]);
        if (entry.destination.length() == 0)
        {
            entry.destination = safeString(aircraft["to"]);
        }
        entry.latitude = safeFloat(aircraft["lat"]);
        entry.longitude = safeFloat(aircraft["lon"]);
        entry.altitude = safeInt(aircraft["alt_baro"]);
        entry.speed = safeFloat(aircraft["gs"]);
        entry.heading = safeInt(aircraft["track"]);
        if (!aircraft["baro_rate"].isNull() && !aircraft["baro_rate"].is<const char*>())
        {
            entry.verticalSpeed = safeInt(aircraft["baro_rate"]);
            entry.hasVerticalSpeed = true;
        }
        entry.hasPosition = !aircraft["lat"].isNull() && !aircraft["lon"].isNull();

        if (!entry.hasPosition)
        {
            continue;
        }

        staging_[parsed++] = entry;
    }

    stagingCount_ = parsed;
    stagingValid_ = true;
    Serial.print("[AIRCRAFT] Parsed ");
    Serial.print(parsed);
    Serial.println(" aircraft");

    if (apiCount > 0 && parsed == 0)
    {
        Serial.print("[AIRCRAFT] API returned ");
        Serial.print(apiCount);
        Serial.println(", displayable 0");
    }

    return true;
}
