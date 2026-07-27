#pragma once

#include "DisplayManager.h"
#include "Wifi_manager.h"

class WifiScreen
{
public:
    WifiScreen(DisplayManager& display, Wifi_manager& wifiManager);

    void enter();
    void onRotate(int8_t direction);
    AppState onButton() const;
    void onLongPress() const;
    void updateStatus();
    void startConnect();

private:
    DisplayManager& display_;
    Wifi_manager& wifiManager_;
    int selectedIndex_ = 0;
    const char* options_[2] = { "Current Connection", "Change Connection" };
    String ssid_;
    String status_;
    String ip_;
    String rssi_;
    String gateway_;
    String subnet_;
};
