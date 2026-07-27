#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <vector>
#include "Config.h"
#include "Aircraft.h"

class DisplayManager
{
public:
    DisplayManager(Adafruit_GC9A01A& display);

    void begin();
    void clear();
    void drawMainMenu(int selectedIndex);
    void drawRadarScreen(const std::vector<Aircraft>& aircrafts, size_t selectedIndex);
    void drawAircraftDetails(const Aircraft& aircraft);
    void drawWifiMenu(int selectedIndex);
    void drawWifiStatus(const String& ssid, const String& status, const String& ip, const String& rssi, const String& gateway, const String& subnet);
    void drawTextCentered(int y, const String& text);

private:
    Adafruit_GC9A01A& display_;
    void drawRadarBackground();
    void drawAircraftIcon(float x, float y, bool selected);
    void drawRangeCircles();
    void drawCardinals();
};
