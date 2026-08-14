#pragma once
#include <Arduino.h>
#include <Preferences.h>

class Wifi_manager
{
public:
    Wifi_manager();
    bool connectWifi(const String &ssid = "", const String &password = "");
    bool startSetupPortal();
    void clearCredentials();

    void saveNetwork(const String& ssid);
    void loadSavedNetworks(String (&networks)[5], int& count) const;
    bool connectSavedNetwork(const String& ssid, const String& password = "");
};
