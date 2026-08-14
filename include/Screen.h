#pragma once

#include "AircraftData.h"
#include "Config.h"
#include "Menu.h"
#include "Ui.h"
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
    enum class Field
    {
        Radius,
        Color,
        Back
    };

    DisplaySettingsScreen();
    void draw() override;
    void setSelection(int selection);
    int getSelection() const;
    void moveNext();
    void movePrevious();
    void setRadius(int radiusKm);
    int getRadius() const;
    void setColor(uint16_t color);
    uint16_t getColor() const;
    bool isEditing() const;
    void beginEdit();
    void confirmEdit();
    void cancelEdit();
    void adjust(int direction);

private:
    Menu menu;
    int radiusKm = Config::kDefaultDetectionRadiusKm;
    uint16_t currentColor = DisplayColors::kGreen;
    bool editing = false;
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
    bool hasSelection() const;
    int getContactCount() const;
    const Aircraft* getSelectedAircraft() const;
    void setCenter(float latitude, float longitude);
    void setRadiusKm(int radiusKm);
    void setPrimaryColor(uint16_t color);
    void setWifiConnected(bool connected);
    void nextAircraft();
    void previousAircraft();
    void resetSweep();

private:
    Aircraft contacts[Config::kMaxAircraft];
    unsigned long glowUntil[Config::kMaxAircraft] = {};
    int aircraftCount = 0;
    String selectedHex;
    float centerLatitude = Config::kRadarCenterLatitude;
    float centerLongitude = Config::kRadarCenterLongitude;
    int radiusKm = Config::kDefaultDetectionRadiusKm;
    uint16_t primaryColor = DisplayColors::kGreen;
    bool wifiConnected = false;
    unsigned long lastFrameMs = 0;
    unsigned long sweepStartMs = 0;

    float sweepDegrees() const;
    void drawAircraftContact(const Aircraft& aircraft, int index, bool selected) const;
    void drawHeadingMark(int x, int y, int heading, uint16_t color, int size) const;
    void drawSelectionBrackets(int x, int y, uint16_t color) const;
    void mapToRadar(float latitude, float longitude, int& x, int& y, float& distanceKm, float& bearingDeg) const;
    int findSelectedIndex() const;
    int nearestIndex() const;
};

class PlaneDetailsScreen : public Screen
{
public:
    void draw() override;
    void setAircraft(const Aircraft& aircraft, float centerLat, float centerLon);
    void clear();
    void scroll(int direction);
    bool hasAircraft() const;

private:
    Aircraft aircraft;
    bool valid = false;
    float centerLatitude = Config::kRadarCenterLatitude;
    float centerLongitude = Config::kRadarCenterLongitude;
    int scrollOffset = 0;
};

class WifiScreen : public Screen
{
public:
    enum class Mode
    {
        Menu,
        Current,
        Saved,
        Portal,
        Connecting,
        Result
    };

    WifiScreen();
    void draw() override;

    void setSelection(int selection);
    int getSelection() const;
    void moveNext();
    void movePrevious();
    void setMode(Mode newMode);
    Mode getMode() const;
    void setConnectionStatus(bool connected, const char* ssid, const char* ip, const char* rssi);
    void setSavedNetworks(const String* networks, int count);
    int getSavedNetworkCount() const;
    String getSavedNetwork(int index) const;
    void setStatusText(const char* title, const char* line1, const char* line2);
    bool isNewConnectionSelected() const;

private:
    Menu menu;
    Mode mode = Mode::Menu;
    bool connected = false;
    char ssid[33] = "";
    char ipAddress[20] = "";
    char rssiValue[16] = "";
    char resultTitle[24] = "";
    char resultLine1[32] = "";
    char resultLine2[32] = "";
    String savedNetworks[Config::kMaxSavedNetworks];
    int savedNetworkCount = 0;

    void rebuildSavedMenu();
    void drawCurrentPage() const;
    void drawPortalPage() const;
    void drawConnectingPage() const;
    void drawResultPage() const;
};

extern Adafruit_GC9A01A tft;
