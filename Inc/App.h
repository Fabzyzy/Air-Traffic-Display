#pragma once

#include <Arduino.h>
#include "Config.h"

enum class AppState
{
    MAIN_MENU,
    RADAR_DISPLAY,
    WIFI_MENU,
    WIFI_STATUS,
    WIFI_CONNECT,
    PLANE_DETAILS
};

class App
{
public:
    void begin();

    void setState(AppState state);
    AppState getState() const;
    void handleRotate(int8_t direction);
    void handleButton();
    void handleLongPress();
    void update();

    void setMenuScreen(MenuScreen* screen);
    void setRadarScreen(RadarScreen* screen);
    void setWifiScreen(WifiScreen* screen);

private:
    AppState state_ = AppState::MAIN_MENU;
    MenuScreen* menuScreen_ = nullptr;
    RadarScreen* radarScreen_ = nullptr;
    WifiScreen* wifiScreen_ = nullptr;
};
