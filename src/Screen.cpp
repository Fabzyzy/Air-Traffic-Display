#include "Screen.h"
#include <math.h>
#include <WiFi.h>

namespace
{
    constexpr const char* kUnknown = "Unknown";

    static const char* kMainMenuLabels[] = {"Radar Display", "Settings"};
    static const char* kSettingsLabels[] = {"Display Settings", "WiFi Settings", "Back"};
    static const char* kDisplayLabels[] = {"Detection Radius", "Color", "Back"};
    static const char* kWifiMenuLabels[] = {"Current Connection", "Saved Connections", "New Connection", "Back"};

    const char* orUnknown(const String& value)
    {
        return value.length() > 0 ? value.c_str() : kUnknown;
    }

    float angleDiff(float a, float b)
    {
        float d = fabsf(a - b);
        if (d > 180.0f)
        {
            d = 360.0f - d;
        }
        return d;
    }
}

MainMenuScreen::MainMenuScreen()
{
    menu.setItems(kMainMenuLabels, 2);
}

void MainMenuScreen::setSelection(int selection) { menu.setSelection(selection); }
int MainMenuScreen::getSelection() const { return menu.getSelection(); }
void MainMenuScreen::moveNext() { menu.moveNext(); }
void MainMenuScreen::movePrevious() { menu.movePrevious(); }

void MainMenuScreen::draw()
{
    menu.drawList("Main Menu", 2);
}

SettingsScreen::SettingsScreen()
{
    menu.setItems(kSettingsLabels, 3);
}

void SettingsScreen::setSelection(int selection) { menu.setSelection(selection); }
int SettingsScreen::getSelection() const { return menu.getSelection(); }
void SettingsScreen::moveNext() { menu.moveNext(); }
void SettingsScreen::movePrevious() { menu.movePrevious(); }

void SettingsScreen::draw()
{
    menu.drawList("Settings", 3);
}

DisplaySettingsScreen::DisplaySettingsScreen()
{
    menu.setItems(kDisplayLabels, 3);
}

void DisplaySettingsScreen::setSelection(int selection)
{
    menu.setSelection(selection);
    editing = false;
}

int DisplaySettingsScreen::getSelection() const { return menu.getSelection(); }

void DisplaySettingsScreen::moveNext()
{
    if (!editing)
    {
        menu.moveNext();
    }
}

void DisplaySettingsScreen::movePrevious()
{
    if (!editing)
    {
        menu.movePrevious();
    }
}

void DisplaySettingsScreen::setRadius(int radius)
{
    radiusKm = constrain(radius, Config::kMinDetectionRadiusKm, Config::kMaxDetectionRadiusKm);
}

int DisplaySettingsScreen::getRadius() const { return radiusKm; }

void DisplaySettingsScreen::setColor(uint16_t color)
{
    currentColor = color;
    Ui::setPrimary(color);
}

uint16_t DisplaySettingsScreen::getColor() const { return currentColor; }
bool DisplaySettingsScreen::isEditing() const { return editing; }

void DisplaySettingsScreen::beginEdit()
{
    if (menu.getSelection() != 2)
    {
        editing = true;
    }
}

void DisplaySettingsScreen::confirmEdit() { editing = false; }
void DisplaySettingsScreen::cancelEdit() { editing = false; }

void DisplaySettingsScreen::adjust(int direction)
{
    if (menu.getSelection() == 0)
    {
        setRadius(radiusKm + direction * Config::kRadiusStepKm);
    }
    else if (menu.getSelection() == 1)
    {
        int index = Ui::colorIndex(currentColor) + direction;
        if (index < 0)
        {
            index = DisplayColors::kCount - 1;
        }
        setColor(Ui::colorAt(index));
    }
}

void DisplaySettingsScreen::draw()
{
    Ui::fillBackground();
    Ui::drawTitle("Display");

    char radiusText[16];
    snprintf(radiusText, sizeof(radiusText), "%d km", radiusKm);

    const int selected = menu.getSelection();
    const uint16_t radiusLabel = selected == 0 ? Ui::primary() : DisplayColors::kText;
    const uint16_t radiusValue = (selected == 0 && editing) ? Ui::primary() : DisplayColors::kDimText;
    const uint16_t colorLabel = selected == 1 ? Ui::primary() : DisplayColors::kText;
    const uint16_t colorValue = (selected == 1 && editing) ? Ui::primary() : DisplayColors::kDimText;
    const uint16_t backColor = selected == 2 ? Ui::primary() : DisplayColors::kText;

    Ui::drawCentered("Detection Radius", 72, radiusLabel, 1);
    Ui::drawCentered(radiusText, 90, radiusValue, 2);
    Ui::drawCentered("Color", 122, colorLabel, 1);
    Ui::drawCentered(Ui::colorName(currentColor), 140, colorValue, 2);
    Ui::drawCentered("Back", 176, backColor, 1);

    if (editing)
    {
        Ui::drawHint("Rotate to change  Press to save");
    }
}

RadarScreen::RadarScreen()
{
    resetSweep();
}

void RadarScreen::resetSweep()
{
    sweepStartMs = millis();
    lastFrameMs = 0;
}

void RadarScreen::setPrimaryColor(uint16_t color)
{
    primaryColor = color;
    Ui::setPrimary(color);
}

void RadarScreen::setWifiConnected(bool connected)
{
    wifiConnected = connected;
}

void RadarScreen::setRadiusKm(int radius)
{
    radiusKm = constrain(radius, Config::kMinDetectionRadiusKm, Config::kMaxDetectionRadiusKm);
}

void RadarScreen::setCenter(float latitude, float longitude)
{
    centerLatitude = latitude;
    centerLongitude = longitude;
}

void RadarScreen::setSelectedHex(const String& hex)
{
    selectedHex = hex;
}

String RadarScreen::getSelectedHex() const
{
    return selectedHex;
}

bool RadarScreen::hasSelection() const
{
    return findSelectedIndex() >= 0;
}

int RadarScreen::getContactCount() const
{
    return aircraftCount;
}

const Aircraft* RadarScreen::getSelectedAircraft() const
{
    const int index = findSelectedIndex();
    return index >= 0 ? &contacts[index] : nullptr;
}

void RadarScreen::setAircraft(const Aircraft* aircraftsData, int count)
{
    aircraftCount = 0;
    if (aircraftsData != nullptr)
    {
        for (int i = 0; i < count && aircraftCount < Config::kMaxAircraft; ++i)
        {
            float distanceKm = 0.0f;
            float bearingDeg = 0.0f;
            int x = 0;
            int y = 0;
            mapToRadar(aircraftsData[i].latitude, aircraftsData[i].longitude, x, y, distanceKm, bearingDeg);
            if (distanceKm <= static_cast<float>(radiusKm) + 0.5f)
            {
                contacts[aircraftCount] = aircraftsData[i];
                glowUntil[aircraftCount] = 0;
                ++aircraftCount;
            }
        }
    }

    if (aircraftCount <= 0)
    {
        selectedHex = "";
        return;
    }

    if (findSelectedIndex() >= 0)
    {
        return;
    }

    const int nearest = nearestIndex();
    selectedHex = nearest >= 0 ? contacts[nearest].hex : contacts[0].hex;
}

int RadarScreen::findSelectedIndex() const
{
    if (selectedHex.length() == 0)
    {
        return -1;
    }
    for (int i = 0; i < aircraftCount; ++i)
    {
        if (contacts[i].hex == selectedHex)
        {
            return i;
        }
    }
    return -1;
}

int RadarScreen::nearestIndex() const
{
    int best = -1;
    float bestDistance = 1.0e9f;
    for (int i = 0; i < aircraftCount; ++i)
    {
        const float distance = geoDistanceKm(centerLatitude, centerLongitude, contacts[i].latitude, contacts[i].longitude);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = i;
        }
    }
    return best;
}

void RadarScreen::nextAircraft()
{
    if (aircraftCount <= 0)
    {
        return;
    }
    const int current = findSelectedIndex();
    const int next = current < 0 ? 0 : (current + 1) % aircraftCount;
    selectedHex = contacts[next].hex;
}

void RadarScreen::previousAircraft()
{
    if (aircraftCount <= 0)
    {
        return;
    }
    const int current = findSelectedIndex();
    const int prev = current < 0 ? 0 : (current + aircraftCount - 1) % aircraftCount;
    selectedHex = contacts[prev].hex;
}

void RadarScreen::mapToRadar(float latitude, float longitude, int& x, int& y, float& distanceKm, float& bearingDeg) const
{
    distanceKm = geoDistanceKm(centerLatitude, centerLongitude, latitude, longitude);
    bearingDeg = geoBearingDeg(centerLatitude, centerLongitude, latitude, longitude);
    const float scale = static_cast<float>(Config::kRadarRadiusPx) / static_cast<float>(radiusKm);
    const float rad = bearingDeg * 0.01745329252f;
    const float px = distanceKm * scale;
    x = Config::kCenterX + static_cast<int>(sinf(rad) * px);
    y = Config::kCenterY - static_cast<int>(cosf(rad) * px);
}

float RadarScreen::sweepDegrees() const
{
    const unsigned long elapsed = millis() - sweepStartMs;
    return fmodf(static_cast<float>(elapsed % Config::kSweepPeriodMs) * 360.0f / static_cast<float>(Config::kSweepPeriodMs), 360.0f);
}

void RadarScreen::drawHeadingMark(int x, int y, int heading, uint16_t color, int size) const
{
    float deg = static_cast<float>(((heading % 360) + 360) % 360);
    const float rad = deg * 0.01745329252f;
    const int x1 = x + static_cast<int>(sinf(rad) * size);
    const int y1 = y - static_cast<int>(cosf(rad) * size);
    const int x2 = x + static_cast<int>(sinf(rad + 2.4f) * size * 0.55f);
    const int y2 = y - static_cast<int>(cosf(rad + 2.4f) * size * 0.55f);
    const int x3 = x + static_cast<int>(sinf(rad - 2.4f) * size * 0.55f);
    const int y3 = y - static_cast<int>(cosf(rad - 2.4f) * size * 0.55f);
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);
}

void RadarScreen::drawSelectionBrackets(int x, int y, uint16_t color) const
{
    tft.drawFastVLine(x - 10, y - 8, 6, color);
    tft.drawFastHLine(x - 10, y - 8, 6, color);
    tft.drawFastVLine(x + 10, y - 8, 6, color);
    tft.drawFastHLine(x + 5, y - 8, 6, color);
    tft.drawFastVLine(x - 10, y + 3, 6, color);
    tft.drawFastHLine(x - 10, y + 8, 6, color);
    tft.drawFastVLine(x + 10, y + 3, 6, color);
    tft.drawFastHLine(x + 5, y + 8, 6, color);
}

void RadarScreen::drawAircraftContact(const Aircraft& aircraft, int index, bool selected) const
{
    float distanceKm = 0.0f;
    float bearingDeg = 0.0f;
    int x = 0;
    int y = 0;
    mapToRadar(aircraft.latitude, aircraft.longitude, x, y, distanceKm, bearingDeg);

    const int dx = x - Config::kCenterX;
    const int dy = y - Config::kCenterY;
    if ((dx * dx + dy * dy) > (Config::kRadarRadiusPx * Config::kRadarRadiusPx))
    {
        return;
    }

    const bool glowing = millis() < glowUntil[index];
    uint16_t color = primaryColor;
    int size = 7;
    if (glowing)
    {
        color = Ui::dim(primaryColor, 255);
        size = 9;
    }
    else if (!selected)
    {
        color = Ui::dim(primaryColor, 140);
    }

    drawHeadingMark(x, y, aircraft.heading, color, size);
    if (selected)
    {
        drawSelectionBrackets(x, y, DisplayColors::kText);
    }
}

bool RadarScreen::update()
{
    const unsigned long now = millis();
    if (lastFrameMs != 0 && now - lastFrameMs < Config::kRadarFrameMs)
    {
        return false;
    }
    lastFrameMs = now;

    const float sweep = sweepDegrees();
    for (int i = 0; i < aircraftCount; ++i)
    {
        const float bearing = geoBearingDeg(centerLatitude, centerLongitude, contacts[i].latitude, contacts[i].longitude);
        if (angleDiff(sweep, bearing) < 6.0f)
        {
            glowUntil[i] = now + Config::kContactGlowMs;
        }
    }
    return true;
}

void RadarScreen::draw()
{
    Ui::fillBackground();

    const uint16_t ring = Ui::dim(primaryColor, 90);
    const uint16_t ringBright = Ui::dim(primaryColor, 160);
    tft.drawCircle(Config::kCenterX, Config::kCenterY, Config::kRadarRadiusPx, ringBright);
    tft.drawCircle(Config::kCenterX, Config::kCenterY, Config::kRadarRadiusPx * 2 / 3, ring);
    tft.drawCircle(Config::kCenterX, Config::kCenterY, Config::kRadarRadiusPx / 3, ring);
    tft.fillCircle(Config::kCenterX, Config::kCenterY, 2, primaryColor);

    tft.setTextSize(1);
    tft.setTextColor(DisplayColors::kText);
    tft.setCursor(114, 14);
    tft.print("N");
    tft.setCursor(220, 116);
    tft.print("E");
    tft.setCursor(114, 222);
    tft.print("S");
    tft.setCursor(10, 116);
    tft.print("W");

    const float sweep = sweepDegrees();
    for (int i = 12; i >= 0; --i)
    {
        const float trail = sweep - static_cast<float>(i) * 2.2f;
        const float rad = trail * 0.01745329252f;
        const int x = Config::kCenterX + static_cast<int>(sinf(rad) * Config::kRadarRadiusPx);
        const int y = Config::kCenterY - static_cast<int>(cosf(rad) * Config::kRadarRadiusPx);
        const uint8_t brightness = static_cast<uint8_t>(255 - i * 16);
        tft.drawLine(Config::kCenterX, Config::kCenterY, x, y, Ui::dim(primaryColor, brightness));
    }

    if (!wifiConnected)
    {
        Ui::drawCentered("RADAR", 88, primaryColor, 1);
        Ui::drawCentered("NO WIFI CONNECTION", 110, DisplayColors::kText, 1);
        Ui::drawCentered("Connect WiFi", 132, DisplayColors::kDimText, 1);
        Ui::drawCentered("to view aircraft", 146, DisplayColors::kDimText, 1);
        return;
    }

    for (int i = 0; i < aircraftCount; ++i)
    {
        drawAircraftContact(contacts[i], i, contacts[i].hex == selectedHex);
    }

    if (aircraftCount <= 0)
    {
        Ui::drawCentered("No contacts", 210, DisplayColors::kDimText, 1);
    }
}

void PlaneDetailsScreen::setAircraft(const Aircraft& value, float centerLat, float centerLon)
{
    aircraft = value;
    centerLatitude = centerLat;
    centerLongitude = centerLon;
    valid = true;
    scrollOffset = 0;
}

void PlaneDetailsScreen::clear()
{
    valid = false;
    scrollOffset = 0;
}

bool PlaneDetailsScreen::hasAircraft() const
{
    return valid;
}

void PlaneDetailsScreen::scroll(int direction)
{
    scrollOffset = constrain(scrollOffset + direction, 0, 10);
}

void PlaneDetailsScreen::draw()
{
    Ui::fillBackground();
    Ui::drawTitle("Plane Details");

    if (!valid)
    {
        Ui::drawCentered("No aircraft", 110, DisplayColors::kDimText, 1);
        return;
    }

    const float distance = geoDistanceKm(centerLatitude, centerLongitude, aircraft.latitude, aircraft.longitude);
    const float bearing = geoBearingDeg(centerLatitude, centerLongitude, aircraft.latitude, aircraft.longitude);

    char latText[24];
    char lonText[24];
    char altText[24];
    char spdText[24];
    char hdgText[24];
    char vsText[24];
    char distText[24];
    char brgText[24];
    snprintf(latText, sizeof(latText), "%.4f", aircraft.latitude);
    snprintf(lonText, sizeof(lonText), "%.4f", aircraft.longitude);
    snprintf(altText, sizeof(altText), "%d ft", aircraft.altitude);
    snprintf(spdText, sizeof(spdText), "%.0f kt", aircraft.speed);
    snprintf(hdgText, sizeof(hdgText), "%d deg", aircraft.heading);
    if (aircraft.hasVerticalSpeed)
    {
        snprintf(vsText, sizeof(vsText), "%d fpm", aircraft.verticalSpeed);
    }
    else
    {
        strncpy(vsText, kUnknown, sizeof(vsText) - 1);
        vsText[sizeof(vsText) - 1] = '\0';
    }
    snprintf(distText, sizeof(distText), "%.1f km", distance);
    snprintf(brgText, sizeof(brgText), "%.0f deg", bearing);

    const char* labels[] = {
        "Flight", "Type", "From", "To", "ICAO", "Reg",
        "Lat", "Lon", "Alt", "Speed", "Hdg", "V/S",
        "Squawk", "Dist", "Brg"
    };
    const char* values[] = {
        orUnknown(aircraft.callsign),
        orUnknown(aircraft.type),
        orUnknown(aircraft.origin),
        orUnknown(aircraft.destination),
        orUnknown(aircraft.hex),
        orUnknown(aircraft.registration),
        latText, lonText, altText, spdText, hdgText, vsText,
        orUnknown(aircraft.squawk),
        distText, brgText
    };

    const int total = 15;
    const int visible = 7;
    const int maxOffset = max(0, total - visible);
    scrollOffset = constrain(scrollOffset, 0, maxOffset);

    int y = 62;
    for (int i = 0; i < visible; ++i)
    {
        const int index = scrollOffset + i;
        char line[40];
        snprintf(line, sizeof(line), "%s: %s", labels[index], values[index]);
        Ui::drawCentered(line, y, DisplayColors::kText, 1);
        y += 16;
    }
}

WifiScreen::WifiScreen()
{
    menu.setItems(kWifiMenuLabels, 4);
}

void WifiScreen::setSelection(int selection) { menu.setSelection(selection); }
int WifiScreen::getSelection() const { return menu.getSelection(); }
void WifiScreen::moveNext() { menu.moveNext(); }
void WifiScreen::movePrevious() { menu.movePrevious(); }
void WifiScreen::setMode(Mode newMode) { mode = newMode; }
WifiScreen::Mode WifiScreen::getMode() const { return mode; }

void WifiScreen::setConnectionStatus(bool isConnected, const char* newSsid, const char* newIp, const char* newRssi)
{
    connected = isConnected;
    strncpy(ssid, newSsid != nullptr ? newSsid : "", sizeof(ssid) - 1);
    strncpy(ipAddress, newIp != nullptr ? newIp : "", sizeof(ipAddress) - 1);
    strncpy(rssiValue, newRssi != nullptr ? newRssi : "", sizeof(rssiValue) - 1);
    ssid[sizeof(ssid) - 1] = '\0';
    ipAddress[sizeof(ipAddress) - 1] = '\0';
    rssiValue[sizeof(rssiValue) - 1] = '\0';
}

void WifiScreen::rebuildSavedMenu()
{
    for (int i = 0; i < savedNetworkCount; ++i)
    {
        menu.setItem(i, savedNetworks[i].c_str());
    }
    menu.setItem(savedNetworkCount, "New Connection");
    menu.setCount(savedNetworkCount + 1);
    menu.setSelection(min(menu.getSelection(), savedNetworkCount));
}

void WifiScreen::setSavedNetworks(const String* networks, int count)
{
    savedNetworkCount = constrain(count, 0, Config::kMaxSavedNetworks);
    for (int i = 0; i < savedNetworkCount; ++i)
    {
        savedNetworks[i] = networks[i];
    }
    rebuildSavedMenu();
}

int WifiScreen::getSavedNetworkCount() const { return savedNetworkCount; }

String WifiScreen::getSavedNetwork(int index) const
{
    if (index < 0 || index >= savedNetworkCount)
    {
        return String();
    }
    return savedNetworks[index];
}

bool WifiScreen::isNewConnectionSelected() const
{
    return menu.getSelection() == savedNetworkCount;
}

void WifiScreen::setStatusText(const char* title, const char* line1, const char* line2)
{
    strncpy(resultTitle, title != nullptr ? title : "", sizeof(resultTitle) - 1);
    strncpy(resultLine1, line1 != nullptr ? line1 : "", sizeof(resultLine1) - 1);
    strncpy(resultLine2, line2 != nullptr ? line2 : "", sizeof(resultLine2) - 1);
    resultTitle[sizeof(resultTitle) - 1] = '\0';
    resultLine1[sizeof(resultLine1) - 1] = '\0';
    resultLine2[sizeof(resultLine2) - 1] = '\0';
}

void WifiScreen::drawCurrentPage() const
{
    Ui::fillBackground();
    Ui::drawTitle("WiFi Status");
    if (!connected)
    {
        Ui::drawCentered("NOT CONNECTED", 110, Ui::primary(), 1);
        Ui::drawHint("Hold to go back");
        return;
    }

    Ui::drawCentered("Connected", 68, Ui::primary(), 1);
    Ui::drawCentered("SSID:", 90, DisplayColors::kDimText, 1);
    Ui::drawCentered(ssid, 104, DisplayColors::kText, 1);
    Ui::drawCentered("IP:", 124, DisplayColors::kDimText, 1);
    Ui::drawCentered(ipAddress, 138, DisplayColors::kText, 1);
    Ui::drawCentered("RSSI:", 158, DisplayColors::kDimText, 1);
    Ui::drawCentered(rssiValue, 172, DisplayColors::kText, 1);
}

void WifiScreen::drawPortalPage() const
{
    Ui::fillBackground();
    Ui::drawTitle("WiFi Setup");
    Ui::drawCentered("Portal is open", 88, Ui::primary(), 1);
    Ui::drawCentered("Connect via phone WiFi", 110, DisplayColors::kText, 1);
    Ui::drawCentered("RADAR_SETUP", 136, DisplayColors::kText, 1);
    Ui::drawHint("Hold to close portal");
}

void WifiScreen::drawConnectingPage() const
{
    Ui::fillBackground();
    Ui::drawTitle("Connecting...");
    Ui::drawCentered("SSID:", 100, DisplayColors::kDimText, 1);
    Ui::drawCentered(ssid, 118, DisplayColors::kText, 1);
}

void WifiScreen::drawResultPage() const
{
    Ui::fillBackground();
    Ui::drawTitle(resultTitle);
    Ui::drawCentered(resultLine1, 100, Ui::primary(), 1);
    Ui::drawCentered(resultLine2, 122, DisplayColors::kText, 1);
    Ui::drawHint("Hold to go back");
}

void WifiScreen::draw()
{
    switch (mode)
    {
        case Mode::Current:
            drawCurrentPage();
            break;
        case Mode::Saved:
            if (savedNetworkCount == 0)
            {
                Ui::fillBackground();
                Ui::drawTitle("Saved Networks");
                Ui::drawCentered("No saved networks", 100, DisplayColors::kDimText, 1);
                Ui::drawCentered("New Connection", 140, menu.getSelection() == 0 ? Ui::primary() : DisplayColors::kText, 1);
            }
            else
            {
                menu.drawList("Saved Networks", 6);
            }
            break;
        case Mode::Portal:
            drawPortalPage();
            break;
        case Mode::Connecting:
            drawConnectingPage();
            break;
        case Mode::Result:
            drawResultPage();
            break;
        case Mode::Menu:
        default:
            menu.drawList("WiFi Settings", 4);
            break;
    }
}
