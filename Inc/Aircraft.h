#pragma once

#include <Arduino.h>

struct Aircraft
{
    String callsign;
    String hex;
    String registration;
    String aircraftType;
    String origin;
    String destination;
    String squawk;

    float latitude = 0.0f;
    float longitude = 0.0f;
    int altitude = 0;
    float groundSpeed = 0.0f;
    float heading = 0.0f;
    float verticalSpeed = 0.0f;
    float lastSeen = 0.0f;
    float distanceKm = 0.0f;
    float bearingDeg = 0.0f;
};
