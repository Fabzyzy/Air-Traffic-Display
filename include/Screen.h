#pragma once

#include "AircraftData.h"
#include "Config.h"
#include "Menu.h"
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

class Screen
{
public:
    virtual ~Screen() = default;
    virtual void draw() = 0;
    virtual bool update() { return false; }
};

class MainMenuScreen : public Screen
{
public:
    MainMenuScreen();
    void draw() override;
    bool update() override;

    void setSelection(int selection);
    int getSelection() const;
    void moveNext();
    void movePrevious();

private:
    Menu menu;
};

class SettingsScreen : public Screen
{
public:
    SettingsScreen();
    void draw() override;
    bool update() override;

    void setSelection(int selection);
    int getSelection() const;
    void moveNext();
    void movePrevious();

private:
    Menu menu;
};

class DisplaySettingsScreen : public Screen
{
public:
    DisplaySettingsScreen();
    void draw() override;
    bool update() override;

    void setSelection(int selection);
    int getSelection() const;
    void moveNext();
    void movePrevious();
    void setRadius(int radiusKm);
    int getRadius() const;
    void setColor(uint16_t color);
    uint16_t getColor() const;

private:
    Menu menu;
    int radiusKm = Config::kMinDetectionRadiusKm;
    uint16_t currentColor = DisplayColors::kGreen;
    int selectedField = 0;
};

class RadarScreen : public Screen
{
public:
    RadarScreen();
    void draw() override;
    bool update() override;

    void setAircraft(const Aircraft* aircrafts, int count);
    void setSelectedHex(const String& hex);
    String getSelectedHex() const;
    void setCenter(float latitude, float longitude);
    void nextAircraft();
    void previousAircraft();
    void setPrimaryColor(uint16_t color);

private:
    const Aircraft* aircrafts = nullptr;
    int aircraftCount = 0;
    String selectedHex;
    float centerLatitude = 51.5072f;
    float centerLongitude = -0.1276f;
    uint16_t primaryColor = DisplayColors::kGreen;

    void drawAircraft(const Aircraft& aircraft, bool selected) const;
    int mapLongitudeToX(float longitude) const;
    int mapLatitudeToY(float latitude) const;
    void drawSelectionCursor(int x, int y) const;
    void drawStatusMessage(const char* message) const;
};

class WifiScreen : public Screen
{
public:
    WifiScreen();
    void draw() override;
    bool update() override;

    void setSelection(int selection);
    int getSelection() const;
    void moveNext();
    void movePrevious();
    void setMode(int mode);
    void setConnectionStatus(const char* status, const char* ssid, const char* ip, const char* rssi, const char* gateway, const char* subnet, const char* mac);
    void scrollUp();
    void scrollDown();
    void setScrollOffset(int offset);
    void setStatusText(const char* text);
    void setSavedNetworks(const String* networks, int count);

private:
    Menu menu;
    int mode = 0;
    int scrollOffset = 0;
    char status[24] = "Disconnected";
    char ssid[32] = "None";
    char ipAddress[24] = "N/A";
    char rssiValue[16] = "N/A";
    char gateway[24] = "N/A";
    char subnet[24] = "N/A";
    char macAddress[24] = "N/A";
    char statusText[32] = "";
    String savedNetworks[5];
    int savedNetworkCount = 0;

    void drawConnectionPage() const;
    void drawSavedConnectionPage() const;
    void drawSetupPage() const;
    void drawStatusPage(const char* title, const char* line1, const char* line2) const;
};

extern Adafruit_GC9A01A tft;
void drawVintageRadarSweep();
