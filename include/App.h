#pragma once

#include "Screen.h"

enum class AppState
{
    MAIN_MENU,
    SETTINGS,
    DISPLAY_SETTINGS,
    RADAR_DISPLAY,
    WIFI_SETTINGS,
    WIFI_CURRENT_CONNECTION,
    WIFI_SAVED_CONNECTIONS,
    WIFI_NEW_CONNECTION,
    WIFI_CHANGE_CONNECTION,
    PLANE_DETAILS
};

class App
{
public:
    App();
    void begin();
    void update();
    void nextPage();
    void previousPage();
    void buttonPressed();
    void handleLongPress();
    void setAircraftData(const Aircraft* aircrafts, int count);

    void setDetectionRadius(int radiusKm);
    int getDetectionRadius() const;
    void setRadarColor(uint16_t color);
    uint16_t getRadarColor() const;

private:
    AppState currentState = AppState::MAIN_MENU;
    AppState navigationStack[8];
    int navigationDepth = 0;
    bool screenDirty = true;
    bool aircraftUpdatesEnabled = false;
    unsigned long lastAircraftUpdateMs = 0;
    int detectionRadiusKm = 50;
    uint16_t radarColor = DisplayColors::kGreen;

    MainMenuScreen mainMenuScreen;
    SettingsScreen settingsScreen;
    DisplaySettingsScreen displaySettingsScreen;
    RadarScreen radarScreen;
    WifiScreen wifiScreen;
    Screen* activeScreen = nullptr;

    void setState(AppState newState, bool withPush = true);
    void drawCurrentScreen();
    void beginAircraftUpdates();
    void stopAircraftUpdates();
    void updateAircraftData();
    void pushState(AppState state);
    void popState();
};

