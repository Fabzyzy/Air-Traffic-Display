#include "DisplayManager.h"
#include "RadarMath.h"
#include <cmath>

DisplayManager::DisplayManager(Adafruit_GC9A01A& display)
    : display_(display)
{
}

void DisplayManager::begin()
{
    display_.begin();
    display_.setRotation(0);
    display_.fillScreen(0x0000);
    display_.setTextColor(0xFFFF);
}

void DisplayManager::clear()
{
    display_.fillScreen(0x0000);
}

void DisplayManager::drawTextCentered(int y, const String& text)
{
    display_.setTextColor(0xFFFF);
    display_.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    display_.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
    int16_t x = (Config::kDisplayWidth - w) / 2;
    display_.setCursor(x, y);
    display_.print(text);
}

void DisplayManager::drawMainMenu(int selectedIndex)
{
    clear();
    display_.setTextSize(2);
    display_.setTextColor(0x07E0);
    drawTextCentered(20, "Main Menu");

    const char* options[2] = { "Radar Display", "WiFi Settings" };
    for (int i = 0; i < 2; ++i)
    {
        int y = 80 + i * 40;
        bool selected = (i == selectedIndex);
        if (selected)
        {
            display_.fillRect(10, y - 2, Config::kDisplayWidth - 20, 30, 0x07E0);
            display_.setTextColor(0x0000);
        }
        else
        {
            display_.setTextColor(0xFFFF);
            display_.drawRect(10, y - 2, Config::kDisplayWidth - 20, 30, 0x07E0);
        }

        display_.setCursor(20, y);
        display_.print(options[i]);
    }
}

void DisplayManager::drawRadarBackground()
{
    display_.fillScreen(0x0000);
    display_.drawCircle(Config::kDisplayWidth / 2, Config::kDisplayHeight / 2, Config::kRadarRadiusPx, 0x07E0);
    display_.drawCircle(Config::kDisplayWidth / 2, Config::kDisplayHeight / 2, Config::kRadarRadiusPx / 2, 0x03E0);
    display_.drawCircle(Config::kDisplayWidth / 2, Config::kDisplayHeight / 2, Config::kRadarRadiusPx / 4, 0x03E0);
    display_.drawLine(Config::kDisplayWidth / 2, Config::kDisplayHeight / 2 - Config::kRadarRadiusPx, Config::kDisplayWidth / 2, Config::kDisplayHeight / 2 + Config::kRadarRadiusPx, 0x03E0);
    display_.drawLine(Config::kDisplayWidth / 2 - Config::kRadarRadiusPx, Config::kDisplayHeight / 2, Config::kDisplayWidth / 2 + Config::kRadarRadiusPx, Config::kDisplayHeight / 2, 0x03E0);
    drawCardinals();
}

void DisplayManager::drawRangeCircles()
{
    display_.drawCircle(Config::kDisplayWidth / 2, Config::kDisplayHeight / 2, Config::kRadarRadiusPx / 3, 0x03E0);
    display_.drawCircle(Config::kDisplayWidth / 2, Config::kDisplayHeight / 2, (Config::kRadarRadiusPx * 2) / 3, 0x03E0);
}

void DisplayManager::drawCardinals()
{
    display_.setTextSize(1);
    display_.setTextColor(0x07E0);
    int cx = Config::kDisplayWidth / 2;
    int cy = Config::kDisplayHeight / 2;
    display_.setCursor(cx - 4, cy - Config::kRadarRadiusPx - 12);
    display_.print("N");
    display_.setCursor(cx + Config::kRadarRadiusPx + 2, cy - 4);
    display_.print("E");
    display_.setCursor(cx - 4, cy + Config::kRadarRadiusPx + 2);
    display_.print("S");
    display_.setCursor(cx - Config::kRadarRadiusPx - 10, cy - 4);
    display_.print("W");
}

void DisplayManager::drawAircraftIcon(float x, float y, bool selected)
{
    uint16_t color = selected ? 0xF800 : 0xFFE0;
    display_.fillCircle((int16_t)x, (int16_t)y, selected ? 4 : 2, color);
    if (selected)
    {
        display_.drawCircle((int16_t)x, (int16_t)y, 7, color);
    }
}

void DisplayManager::drawRadarScreen(const std::vector<Aircraft>& aircrafts, size_t selectedIndex)
{
    drawRadarBackground();
    drawRangeCircles();

    int cx = Config::kDisplayWidth / 2;
    int cy = Config::kDisplayHeight / 2;
    display_.setTextSize(1);
    display_.setTextColor(0x07E0);
    display_.setCursor(5, 5);
    display_.print("Radar Display");

    for (size_t i = 0; i < aircrafts.size(); ++i)
    {
        const Aircraft& aircraft = aircrafts[i];
        if (aircraft.distanceKm > Config::kRadarRangeKm)
        {
            continue;
        }

        float scale = aircraft.distanceKm / Config::kRadarRangeKm;
        float angle = RadarMath::degreesToRadians(aircraft.bearingDeg);
        float px = cx + sin(angle) * scale * Config::kRadarRadiusPx;
        float py = cy - cos(angle) * scale * Config::kRadarRadiusPx;
        drawAircraftIcon(px, py, (i == selectedIndex));
    }

    const Aircraft* selectedAircraft = nullptr;
    if (selectedIndex < aircrafts.size())
    {
        selectedAircraft = &aircrafts[selectedIndex];
    }

    if (selectedAircraft)
    {
        display_.setCursor(5, Config::kDisplayHeight - 25);
        display_.setTextColor(0x07E0);
        display_.print("Selected: ");
        display_.print(selectedAircraft->callsign.length() ? selectedAircraft->callsign : "Unknown");
    }
}

void DisplayManager::drawAircraftDetails(const Aircraft& aircraft)
{
    clear();
    display_.setTextSize(1);
    display_.setTextColor(0x07E0);
    display_.setCursor(5, 5);
    display_.print("Plane Details");

    int y = 20;
    auto drawLine = [&](const String& label, const String& value)
    {
        display_.setCursor(5, y);
        display_.print(label);
        display_.print(value);
        y += 14;
    };

    drawLine("Callsign: ", aircraft.callsign.length() ? aircraft.callsign : "Unknown");
    drawLine("ICAO Hex: ", aircraft.hex.length() ? aircraft.hex : "Unknown");
    drawLine("Lat: ", aircraft.latitude ? String(aircraft.latitude, 6) : "Unknown");
    drawLine("Lon: ", aircraft.longitude ? String(aircraft.longitude, 6) : "Unknown");
    drawLine("Alt: ", aircraft.altitude ? String(aircraft.altitude) + " ft" : "Unknown");
    drawLine("GS: ", aircraft.groundSpeed ? String(aircraft.groundSpeed, 1) + " kt" : "Unknown");
    drawLine("Hdg: ", aircraft.heading ? String(aircraft.heading, 0) + "°" : "Unknown");
    drawLine("V/S: ", aircraft.verticalSpeed ? String(aircraft.verticalSpeed, 0) + " ft/m" : "Unknown");
    drawLine("Squawk: ", aircraft.squawk.length() ? aircraft.squawk : "Unknown");
    drawLine("Reg: ", aircraft.registration.length() ? aircraft.registration : "Unknown");
    drawLine("Type: ", aircraft.aircraftType.length() ? aircraft.aircraftType : "Unknown");
    drawLine("From: ", aircraft.origin.length() ? aircraft.origin : "Unknown");
    drawLine("To: ", aircraft.destination.length() ? aircraft.destination : "Unknown");
    drawLine("Last seen: ", aircraft.lastSeen ? String(aircraft.lastSeen, 1) + " s" : "Unknown");
    drawLine("Dist: ", String(aircraft.distanceKm, 1) + " km");
    drawLine("Brg: ", String(aircraft.bearingDeg, 0) + "°");
}

void DisplayManager::drawWifiMenu(int selectedIndex)
{
    clear();
    drawTextCentered(20, "WiFi Settings");
    const char* options[2] = { "Current Connection", "Change Connection" };
    for (int i = 0; i < 2; ++i)
    {
        int y = 80 + i * 40;
        bool selected = (i == selectedIndex);
        if (selected)
        {
            display_.fillRect(10, y - 2, Config::kDisplayWidth - 20, 30, 0x07E0);
            display_.setTextColor(0x0000);
        }
        else
        {
            display_.setTextColor(0xFFFF);
            display_.drawRect(10, y - 2, Config::kDisplayWidth - 20, 30, 0x07E0);
        }

        display_.setCursor(20, y);
        display_.print(options[i]);
    }
}

void DisplayManager::drawWifiStatus(const String& ssid, const String& status, const String& ip, const String& rssi, const String& gateway, const String& subnet)
{
    clear();
    display_.setTextSize(1);
    display_.setTextColor(0x07E0);
    display_.setCursor(5, 5);
    display_.print("WiFi Status");

    int y = 22;
    auto drawLine = [&](const String& label, const String& value)
    {
        display_.setCursor(5, y);
        display_.print(label);
        display_.print(value);
        y += 14;
    };

    drawLine("SSID: ", ssid.length() ? ssid : "Unknown");
    drawLine("Status: ", status.length() ? status : "Unknown");
    drawLine("IP: ", ip.length() ? ip : "Unknown");
    drawLine("RSSI: ", rssi.length() ? rssi : "Unknown");
    drawLine("Gateway: ", gateway.length() ? gateway : "Unknown");
    drawLine("Subnet: ", subnet.length() ? subnet : "Unknown");
}
