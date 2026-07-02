#pragma once
#include <Arduino.h>

class Wifi_manager
{
public:
    bool loadCredentials();
    bool connectWifi();
    void startSetupPortal();
    void saveCredentials(String ssid, String password);
    void clearCredentials();

private:
    bool setupPortalActive;
    void handleSetupRoot();
    void handleSetupSubmit();
    void sendPortalPage(String message);
};