#include "Screen.h"
#include "AircraftData.h"
#include <Arduino.h>
#include <WiFi.h>

namespace
{
    constexpr uint16_t kBgColor = 0x0000;
    constexpr uint16_t kTextColor = 0xFFFF;
    constexpr uint16_t kHighlightColor = 0x07E0;
    constexpr int kCenterX = 120;
    constexpr int kCenterY = 120;
    constexpr int kRadius = 90;
    constexpr uint16_t kCompassColor = 0xFFFF;
    constexpr uint16_t kSelectionColor = 0xF81F;

    constexpr int kMainMenuItems = 2;
    constexpr int kSettingsItems = 4;
    constexpr int kDisplaySettingsItems = 4;
    constexpr int kWifiMenuItems = 3;

    static const char* kMainMenuLabels[kMainMenuItems] = {"Radar Display", "Settings"};
    static const char* kSettingsLabels[kSettingsItems] = {"Radar", "Display", "WiFi", "Back"};
    static const char* kDisplayLabels[kDisplaySettingsItems] = {"Radius +", "Radius -", "Color", "Back"};
    static const char* kWifiMenuLabels[kWifiMenuItems] = {"Current Connection", "Change Connection", "Back"};
}

MainMenuScreen::MainMenuScreen()
{
    menu.setItems(kMainMenuLabels, kMainMenuItems);
}

void MainMenuScreen::setSelection(int selection)
{
    menu.setSelection(selection);
}

int MainMenuScreen::getSelection() const
{
    return menu.getSelection();
}

void MainMenuScreen::moveNext()
{
    menu.moveNext();
}

void MainMenuScreen::movePrevious()
{
    menu.movePrevious();
}

void MainMenuScreen::draw()
{
    menu.drawList("Main Menu", 18, 58, 24, kMainMenuItems, kTextColor, kHighlightColor);
}

bool MainMenuScreen::update()
{
    return false;
}

SettingsScreen::SettingsScreen()
{
    menu.setItems(kSettingsLabels, kSettingsItems);
}

void SettingsScreen::setSelection(int selection)
{
    menu.setSelection(selection);
}

int SettingsScreen::getSelection() const
{
    return menu.getSelection();
}

void SettingsScreen::moveNext()
{
    menu.moveNext();
}

void SettingsScreen::movePrevious()
{
    menu.movePrevious();
}

void SettingsScreen::draw()
{
    menu.drawList("Settings", 18, 58, 24, kSettingsItems, kTextColor, kHighlightColor);
}

bool SettingsScreen::update()
{
    return false;
}

DisplaySettingsScreen::DisplaySettingsScreen()
{
    menu.setItems(kDisplayLabels, kDisplaySettingsItems);
    radiusKm = Config::kMinDetectionRadiusKm;
    currentColor = DisplayColors::kGreen;
    selectedField = 0;
}

void DisplaySettingsScreen::setSelection(int selection)
{
    menu.setSelection(selection);
    selectedField = selection;
}

int DisplaySettingsScreen::getSelection() const
{
    return menu.getSelection();
}

void DisplaySettingsScreen::moveNext()
{
    menu.moveNext();
    selectedField = menu.getSelection();
}

void DisplaySettingsScreen::movePrevious()
{
    menu.movePrevious();
    selectedField = menu.getSelection();
}

void DisplaySettingsScreen::setRadius(int radius)
{
    radiusKm = constrain(radius, Config::kMinDetectionRadiusKm, Config::kMaxDetectionRadiusKm);
}

int DisplaySettingsScreen::getRadius() const
{
    return radiusKm;
}

void DisplaySettingsScreen::setColor(uint16_t color)
{
    currentColor = color;
}

uint16_t DisplaySettingsScreen::getColor() const
{
    return currentColor;
}

void DisplaySettingsScreen::draw()
{
    tft.fillScreen(kBgColor);
    tft.setTextWrap(false);
    tft.setTextSize(2);
    tft.setTextColor(kTextColor);
    tft.setCursor(18, 18);
    tft.print("Display");

    tft.setTextSize(1);
    tft.setTextColor(kTextColor);
    tft.setCursor(18, 52);
    tft.print("Radius:");
    tft.setCursor(100, 52);
    tft.print(radiusKm);
    tft.print(" km");

    tft.setCursor(18, 72);
    tft.print("Color:");
    tft.setCursor(100, 72);
    tft.print(currentColor == DisplayColors::kGreen ? "GREEN" : "AMBER");

    menu.drawList("Options", 18, 96, 20, kDisplaySettingsItems, kTextColor, kHighlightColor);
}

bool DisplaySettingsScreen::update()
{
    return false;
}

RadarScreen::RadarScreen()
{
    selectedHex = "";
    primaryColor = DisplayColors::kGreen;
}

void RadarScreen::setPrimaryColor(uint16_t color)
{
    primaryColor = color;
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
    const uint16_t color = selected ? kSelectionColor : primaryColor;
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
    if (selected)
    {
        drawSelectionCursor(x, y);
    }
}

void RadarScreen::drawStatusMessage(const char* message) const
{
    tft.setTextWrap(false);
    tft.setTextSize(1);
    tft.setTextColor(kTextColor);
    tft.setCursor(18, 210);
    tft.print(message);
}

void RadarScreen::draw()
{
    tft.fillScreen(kBgColor);
    tft.drawCircle(kCenterX, kCenterY, kRadius, primaryColor);
    tft.drawCircle(kCenterX, kCenterY, kRadius / 2, primaryColor);
    tft.drawLine(kCenterX, kCenterY - kRadius, kCenterX, kCenterY + kRadius, primaryColor);
    tft.drawLine(kCenterX - kRadius, kCenterY, kCenterX + kRadius, kCenterY, primaryColor);

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

    if (aircraftCount <= 0)
    {
        drawStatusMessage("No aircraft in range");
    }
}

bool RadarScreen::update()
{
    return false;
}

WifiScreen::WifiScreen()
{
    menu.setItems(kWifiMenuLabels, kWifiMenuItems);
    mode = 0;
}

void WifiScreen::setSelection(int selection)
{
    menu.setSelection(selection);
}

int WifiScreen::getSelection() const
{
    return menu.getSelection();
}

void WifiScreen::moveNext()
{
    menu.moveNext();
}

void WifiScreen::movePrevious()
{
    menu.movePrevious();
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
    status[sizeof(status) - 1] = '\0';
    ssid[sizeof(ssid) - 1] = '\0';
    ipAddress[sizeof(ipAddress) - 1] = '\0';
    rssiValue[sizeof(rssiValue) - 1] = '\0';
    gateway[sizeof(gateway) - 1] = '\0';
    subnet[sizeof(subnet) - 1] = '\0';
    macAddress[sizeof(macAddress) - 1] = '\0';
    scrollOffset = 0;
}

void WifiScreen::scrollUp()
{
    scrollOffset = max(0, scrollOffset - 1);
}

void WifiScreen::scrollDown()
{
    scrollOffset = min(6, scrollOffset + 1);
}

void WifiScreen::setScrollOffset(int offset)
{
    scrollOffset = offset;
}

void WifiScreen::setStatusText(const char* text)
{
    strncpy(statusText, text, sizeof(statusText) - 1);
    statusText[sizeof(statusText) - 1] = '\0';
}

void WifiScreen::setSavedNetworks(const String* networks, int count)
{
    savedNetworkCount = max(0, min(count, 5));
    for (int i = 0; i < savedNetworkCount; ++i)
    {
        savedNetworks[i] = networks[i];
    }
}

void WifiScreen::drawConnectionPage() const
{
    tft.fillScreen(kBgColor);
    tft.setTextWrap(false);
    tft.setTextSize(2);
    tft.setTextColor(kTextColor);
    tft.setCursor(14, 18);
    tft.print("Current WiFi");

    tft.setTextSize(1);
    const bool connected = strcmp(status, "Connected") == 0 || strcmp(status, "connected") == 0;

    if (!connected)
    {
        tft.setTextColor(kHighlightColor);
        tft.setCursor(24, 58);
        tft.print("NOT CONNECTED");
        tft.setTextColor(kTextColor);
        tft.setCursor(20, 84);
        tft.print("No active");
        tft.setCursor(22, 98);
        tft.print("WiFi connection");
        return;
    }

    const char* lines[] = {"Connected", "SSID:", ssid, "IP:", ipAddress, "RSSI:", rssiValue};
    const int visibleLines = 7;
    int y = 52;

    for (int i = 0; i < visibleLines; ++i)
    {
        if (lines[i] == nullptr)
        {
            continue;
        }

        tft.setCursor(18, y);
        tft.print(lines[i]);
        y += 14;
    }
}

void WifiScreen::drawSavedConnectionPage() const
{
    tft.fillScreen(kBgColor);
    tft.setTextWrap(false);
    tft.setTextSize(2);
    tft.setTextColor(kTextColor);
    tft.setCursor(12, 18);
    tft.print("Saved WiFi");

    tft.setTextSize(1);
    int y = 52;
    const int maxVisible = min(5, savedNetworkCount);
    for (int i = 0; i < maxVisible; ++i)
    {
        if (i == 0)
        {
            tft.setTextColor(kHighlightColor);
        }
        else
        {
            tft.setTextColor(kTextColor);
        }
        tft.setCursor(18, y);
        tft.print(savedNetworks[i]);
        y += 18;
    }

    if (savedNetworkCount == 0)
    {
        tft.setTextColor(kTextColor);
        tft.setCursor(20, 90);
        tft.print("No saved networks");
    }
}

void WifiScreen::drawSetupPage() const
{
    tft.fillScreen(kBgColor);
    tft.setTextWrap(false);
    tft.setTextSize(2);
    tft.setTextColor(kTextColor);
    tft.setCursor(20, 28);
    tft.print("WiFi Setup");
    tft.setTextSize(1);
    tft.setTextColor(kHighlightColor);
    tft.setCursor(28, 70);
    tft.print("Starting...");
    tft.setTextColor(kTextColor);
    tft.setCursor(18, 90);
    tft.print("Connect phone");
    tft.setCursor(18, 104);
    tft.print("to setup network");
    tft.setCursor(24, 128);
    tft.print("RADAR_SETUP");
}

void WifiScreen::drawStatusPage(const char* title, const char* line1, const char* line2) const
{
    tft.fillScreen(kBgColor);
    tft.setTextWrap(false);
    tft.setTextSize(2);
    tft.setTextColor(kTextColor);
    tft.setCursor(18, 18);
    tft.print(title);
    tft.setTextSize(1);
    tft.setTextColor(kHighlightColor);
    tft.setCursor(20, 64);
    tft.print(line1);
    tft.setTextColor(kTextColor);
    tft.setCursor(18, 90);
    tft.print(line2);
}

void WifiScreen::draw()
{
    if (mode == 1)
    {
        drawConnectionPage();
        return;
    }

    if (mode == 2)
    {
        drawSetupPage();
        return;
    }

    if (mode == 3)
    {
        drawSavedConnectionPage();
        return;
    }

    menu.drawList("WiFi Settings", 18, 58, 24, kWifiMenuItems, kTextColor, kHighlightColor);
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
        const float rad = i * 0.017453f;
        const int x = cx + int(cos(rad) * radius);
        const int y = cy + int(sin(rad) * radius);
        tft.drawLine(cx, cy, x, y, 0x0520);
    }

    tft.setTextWrap(false);
    tft.setTextSize(1);
    tft.setTextColor(0x07E0);
    tft.setCursor(10, 10);
    tft.print("Vintage Radar");
}
