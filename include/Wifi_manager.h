#pragma once
#include <Arduino.h>

class Wifi_manager
{
public:
    Wifi_manager();
    bool connectWifi(const String &ssid = "", const String &password = "");
    bool startSetupPortal();
    void clearCredentials();
};
