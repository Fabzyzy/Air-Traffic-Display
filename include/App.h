#pragma once

#include "Screen.h"
#include <Preferences.h>

enum class AppState
{
    MAIN_MENU,
    SETTINGS,
    DISPLAY_SETTINGS,
    RADAR_DISPLAY,
    PLANE_DETAILS,
    WIFI_SETTINGS,
    WIFI_CURRENT_CONNECTION,
    WIFI_SAVED_CONNECTIONS,
    WIFI_PORTAL,
    WIFI_CONNECTING,
    WIFI_RESULT
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

private:
    AppState currentState = AppState::MAIN_MENU;
    AppState navigationStack[8] = {};
    int navigationDepth = 0;
    bool screenDirty = true;
    bool aircraftUpdatesEnabled = false;
    unsigned long lastAircraftRequestMs = 0;
    unsigned long resultShownMs = 0;
    bool resultSuccess = false;
    int detectionRadiusKm = Config::kDefaultDetectionRadiusKm;
    uint16_t radarColor = DisplayColors::kGreen;
    Preferences displayPrefs;

    MainMenuScreen mainMenuScreen;
    SettingsScreen settingsScreen;
    DisplaySettingsScreen displaySettingsScreen;
    RadarScreen radarScreen;
    PlaneDetailsScreen planeDetailsScreen;
    WifiScreen wifiScreen;
    Screen* activeScreen = nullptr;

    void setState(AppState newState, bool withPush = true);
    void goBack();
    void drawCurrentScreen();
    void beginAircraftUpdates();
    void stopAircraftUpdates();
    void updateAircraftData();
    void applyTheme();
    void loadDisplaySettings();
    void saveDisplaySettings();
    void refreshWifiStatus();
    void refreshSavedNetworks();
    void leaveTransientWifi();
};
