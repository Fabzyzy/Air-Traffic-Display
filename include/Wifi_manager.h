#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <WiFiManager.h>
#include "Config.h"

class Wifi_manager
{
public:
    Wifi_manager();

    bool connectWifi(const String& ssid = "", const String& password = "");
    bool startSavedConnect(const String& ssid);
    bool pollConnecting(bool& success);
    void cancelConnecting();
    bool isConnecting() const;
    const char* connectingSsid() const;

    bool startSetupPortal();
    void processPortal();
    bool isPortalActive() const;
    bool pollPortal(bool& success);
    void stopPortal();

    void markPortalSaved();
    void saveNetwork(const String& ssid, const String& password = "");
    void rememberCurrentNetwork();
    void loadSavedNetworks(String (&networks)[Config::kMaxSavedNetworks], int& count) const;
    bool getSavedPassword(const String& ssid, String& password) const;
    void clearCredentials();

private:
    WiFiManager wifiManager_;
    Preferences prefs_;
    bool portalActive_ = false;
    bool portalSaved_ = false;
    bool connecting_ = false;
    unsigned long connectStartMs_ = 0;
    unsigned long portalSavedMs_ = 0;
    char connectingSsid_[33] = "";

    void loadHistoryLocked(String (&ssids)[Config::kMaxSavedNetworks], String (&passwords)[Config::kMaxSavedNetworks], int& count) const;
    void storeHistory(const String* ssids, const String* passwords, int count);
};
