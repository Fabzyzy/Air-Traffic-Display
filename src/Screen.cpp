#include "Screen.h"
#include "AircraftData.h"
#include <Arduino.h>
#include <WiFi.h>

namespace
{
    constexpr uint16_t kBgColor = 0x0000;
    constexpr uint16_t kTextColor = 0xFFFF;
    constexpr uint16_t kHighlightColor = 0x07E0;

    void drawHeader(const char* title)
    {
        tft.fillScreen(kBgColor);
        tft.setTextWrap(false);
        tft.setTextSize(2);
        tft.setTextColor(kTextColor);
        tft.setCursor(10, 10);
        tft.print(title);
    }
}

MainMenuScreen::MainMenuScreen()
{
    selectedIndex = 0;
}

void MainMenuScreen::setSelection(int selection)
{
    selectedIndex = selection;
}

void MainMenuScreen::draw()
{
    drawHeader("Main Menu");

    tft.setTextSize(2);
    tft.setTextColor(kTextColor);
    tft.setCursor(20, 60);
    tft.print("Radar Display");

    tft.setCursor(20, 100);
    tft.print("WiFi Settings");

    tft.setTextColor(kHighlightColor);
    const int y = selectedIndex == 0 ? 60 : 100;
    tft.setCursor(10, y);
    tft.print("> ");
}

bool MainMenuScreen::update()
{
    return false;
}

namespace
{
    constexpr int kCenterX = 120;
    constexpr int kCenterY = 120;
    constexpr int kRadius = 90;
    constexpr uint16_t kRadarColor = 0x07E0;
    constexpr uint16_t kCompassColor = 0xFFFF;
    constexpr uint16_t kSelectionColor = 0xF81F;

    int clampToDisplay(int value)
    {
        return max(5, min(235, value));
    }
}

RadarScreen::RadarScreen()
{
    selectedHex = "";
}

void RadarScreen::setAircraft(const Aircraft* aircraftsData, int count)
{
    aircrafts = aircraftsData;
    aircraftCount = count;

    if (aircraftCount <= 0)
    {
        selectedHex = "";
        return;
    }

    if (selectedHex.length() == 0)
    {
        selectedHex = aircrafts[0].hex;
        return;
    }

    for (int i = 0; i < aircraftCount; ++i)
    {
        if (aircrafts[i].hex == selectedHex)
        {
            return;
        }
    }

    selectedHex = aircrafts[0].hex;
}

void RadarScreen::setSelectedHex(const String& hex)
{
    selectedHex = hex;
}

String RadarScreen::getSelectedHex() const
{
    return selectedHex;
}

void RadarScreen::setCenter(float latitude, float longitude)
{
    centerLatitude = latitude;
    centerLongitude = longitude;
}

void RadarScreen::nextAircraft()
{
    if (aircraftCount <= 0)
    {
        return;
    }

    if (selectedHex.length() == 0)
    {
        selectedHex = aircrafts[0].hex;
        return;
    }

    for (int i = 0; i < aircraftCount; ++i)
    {
        if (aircrafts[i].hex == selectedHex)
        {
            const int nextIndex = (i + 1) % aircraftCount;
            selectedHex = aircrafts[nextIndex].hex;
            return;
        }
    }

    selectedHex = aircrafts[0].hex;
}

void RadarScreen::previousAircraft()
{
    if (aircraftCount <= 0)
    {
        return;
    }

    if (selectedHex.length() == 0)
    {
        selectedHex = aircrafts[0].hex;
        return;
    }

    for (int i = 0; i < aircraftCount; ++i)
    {
        if (aircrafts[i].hex == selectedHex)
        {
            const int prevIndex = (i + aircraftCount - 1) % aircraftCount;
            selectedHex = aircrafts[prevIndex].hex;
            return;
        }
    }

    selectedHex = aircrafts[0].hex;
}

int RadarScreen::mapLongitudeToX(float longitude) const
{
    const float delta = longitude - centerLongitude;
    const float scale = kRadius / 1.5f;
    return kCenterX + int(delta * scale);
}

int RadarScreen::mapLatitudeToY(float latitude) const
{
    const float delta = latitude - centerLatitude;
    const float scale = kRadius / 1.5f;
    return kCenterY - int(delta * scale);
}

void RadarScreen::drawSelectionCursor(int x, int y) const
{
    tft.drawLine(x - 8, y - 8, x - 4, y - 8, kSelectionColor);
    tft.drawLine(x - 8, y - 8, x - 8, y - 4, kSelectionColor);
    tft.drawLine(x + 8, y - 8, x + 4, y - 8, kSelectionColor);
    tft.drawLine(x + 8, y - 8, x + 8, y - 4, kSelectionColor);
    tft.drawLine(x - 8, y + 8, x - 4, y + 8, kSelectionColor);
    tft.drawLine(x - 8, y + 8, x - 8, y + 4, kSelectionColor);
    tft.drawLine(x + 8, y + 8, x + 4, y + 8, kSelectionColor);
    tft.drawLine(x + 8, y + 8, x + 8, y + 4, kSelectionColor);
}

void RadarScreen::drawAircraft(const Aircraft& aircraft, bool selected) const
{
    const int x = mapLongitudeToX(aircraft.longitude);
    const int y = mapLatitudeToY(aircraft.latitude);

    if (x < 10 || x > 230 || y < 10 || y > 230)
    {
        return;
    }

    const int radius = 4;
    const uint16_t color = selected ? kSelectionColor : kRadarColor;
    tft.fillCircle(x, y, radius, color);

    int heading = aircraft.heading % 360;
    if (heading < 0)
    {
        heading += 360;
    }
    const float headingRad = heading * 0.017453f;
    const int dx = int(sin(headingRad) * 7.0f);
    const int dy = int(-cos(headingRad) * 7.0f);

    tft.drawLine(x, y, x + dx, y + dy, color);
    tft.drawLine(x + dx, y + dy, x + dx / 2, y + dy / 2, color);
    tft.drawLine(x + dx, y + dy, x - dx / 2, y - dy / 2, color);

    if (selected)
    {
        drawSelectionCursor(x, y);
    }
}

void RadarScreen::draw()
{
    tft.fillScreen(kBgColor);
    tft.drawCircle(kCenterX, kCenterY, kRadius, kRadarColor);
    tft.drawCircle(kCenterX, kCenterY, kRadius / 2, kRadarColor);
    tft.drawLine(kCenterX, kCenterY - kRadius, kCenterX, kCenterY + kRadius, kRadarColor);
    tft.drawLine(kCenterX - kRadius, kCenterY, kCenterX + kRadius, kCenterY, kRadarColor);

    tft.setTextWrap(false);
    tft.setTextSize(1);
    tft.setTextColor(kCompassColor);
    tft.setCursor(111, 8);
    tft.print("N");
    tft.setCursor(220, 115);
    tft.print("E");
    tft.setCursor(111, 224);
    tft.print("S");
    tft.setCursor(8, 115);
    tft.print("W");

    if (aircrafts != nullptr)
    {
        for (int i = 0; i < aircraftCount; ++i)
        {
            const bool selected = aircrafts[i].hex == selectedHex;
            drawAircraft(aircrafts[i], selected);
        }
    }
}

bool RadarScreen::update()
{
    return false;
}

WifiScreen::WifiScreen()
{
    selectedIndex = 0;
    mode = 0;
}

void WifiScreen::setSelection(int selection)
{
    selectedIndex = selection;
}

void WifiScreen::setMode(int newMode)
{
    mode = newMode;
}

void WifiScreen::setConnectionStatus(const char* newStatus, const char* newSsid, const char* newIp, const char* newRssi, const char* newGateway, const char* newSubnet, const char* newMac)
{
    strncpy(status, newStatus, sizeof(status) - 1);
    strncpy(ssid, newSsid, sizeof(ssid) - 1);
    strncpy(ipAddress, newIp, sizeof(ipAddress) - 1);
    strncpy(rssiValue, newRssi, sizeof(rssiValue) - 1);
    strncpy(gateway, newGateway, sizeof(gateway) - 1);
    strncpy(subnet, newSubnet, sizeof(subnet) - 1);
    strncpy(macAddress, newMac, sizeof(macAddress) - 1);
}

void WifiScreen::draw()
{
    drawHeader("WiFi Settings");

    if (mode == 1)
    {
        tft.setTextSize(1);
        tft.setTextColor(kTextColor);
        tft.setCursor(10, 45);
        tft.print("SSID: ");
        tft.println(ssid);

        tft.setCursor(10, 58);
        tft.print("Status: ");
        tft.println(status);

        tft.setCursor(10, 71);
        tft.print("IP: ");
        tft.println(ipAddress);

        tft.setCursor(10, 84);
        tft.print("RSSI: ");
        tft.println(rssiValue);

        tft.setCursor(10, 97);
        tft.print("Gateway: ");
        tft.println(gateway);

        tft.setCursor(10, 110);
        tft.print("Subnet: ");
        tft.println(subnet);

        tft.setCursor(10, 123);
        tft.print("MAC: ");
        tft.println(macAddress);
        return;
    }

    tft.setTextSize(1);
    tft.setTextColor(kTextColor);

    const char* items[] = {"Current Connection", "Change Connection", "Back"};
    const int yPositions[] = {70, 95, 120};

    for (int i = 0; i < 3; ++i)
    {
        const int y = yPositions[i];
        tft.setCursor(20, y);
        if (selectedIndex == i)
        {
            tft.setTextColor(kHighlightColor);
            tft.print("> ");
            tft.setTextColor(kTextColor);
        }
        else
        {
            tft.print("  ");
        }
        tft.println(items[i]);
    }
}

bool WifiScreen::update()
{
    return false;
}

void drawVintageRadarSweep()
{
    const int cx = 120;
    const int cy = 120;
    const int radius = 100;

    tft.fillScreen(0x0000);

    tft.drawCircle(cx, cy, radius, 0x07E0);
    tft.drawCircle(cx, cy, radius / 2, 0x0520);
    tft.drawCircle(cx, cy, radius / 4, 0x0310);
    tft.drawLine(cx, 20, cx, 220, 0x0520);
    tft.drawLine(20, cy, 220, cy, 0x0520);

    for (int i = 0; i <= radius; i += 20)
    {
        tft.drawCircle(cx, cy, i, 0x0340);
    }

    for (int i = 0; i < 360; i += 45)
    {
        float rad = i * 0.017453f;
        int x = cx + cos(rad) * radius;
        int y = cy + sin(rad) * radius;
        tft.drawLine(cx, cy, x, y, 0x0320);
    }

    float rad = 0.0f;
    int x = cx + cos(rad) * radius;
    int y = cy + sin(rad) * radius;
    tft.drawLine(cx, cy, x, y, 0xFFFF);

    tft.setTextWrap(false);
    tft.setTextSize(1);
    tft.setTextColor(0x07E0);
    tft.setCursor(10, 10);
    tft.print("Vintage Radar");
}
