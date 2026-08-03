#pragma once

#include "Screen.h"

enum class AppState
{
    MAIN_MENU,
    RADAR_DISPLAY,
    WIFI_SETTINGS,
    WIFI_CONNECTION_STATUS,
    WIFI_CHANGE_CONNECTION
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

private:
    AppState currentState = AppState::MAIN_MENU;
    int selectedIndex = 0;
    bool screenDirty = true;
    bool hasEnteredState = false;

    MainMenuScreen mainMenuScreen;
    RadarScreen radarScreen;
    WifiScreen wifiScreen;
    Screen* activeScreen = nullptr;

    void setState(AppState newState);
    void drawCurrentScreen();
};

