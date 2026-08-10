#include "Wifi_manager.h"
#include <WiFi.h>
#include <WiFiManager.h>

namespace
{
    const char* kSetupSsid = "RADAR_SETUP";
    const char* kSetupPassword = "radar123";
    const IPAddress kApIp(192, 168, 4, 1);
    const IPAddress kApNetmask(255, 255, 255, 0);
}

Wifi_manager::Wifi_manager()
{
}

bool Wifi_manager::connectWifi(const String &ssid, const String &password)
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(100);

    if (ssid.length() > 0)
    {
        Serial.println("[WIFI] Connecting to saved network");
        WiFi.begin(ssid.c_str(), password.c_str());
    }
    else if (WiFi.SSID().length() > 0)
    {
        Serial.println("[WIFI] Connecting with saved credentials");
        WiFi.begin();
    }
    else
    {
        Serial.println("[WIFI] No saved Wi-Fi credentials available");
        return false;
    }

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000)
    {
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("[WIFI] Connected");
        Serial.print("[WIFI] IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }

    Serial.println("[WIFI] Connect failed");
    return false;
}

bool Wifi_manager::startSetupPortal()
{
    WiFiManager wifiManager;
    wifiManager.setAPStaticIPConfig(kApIp, kApIp, kApNetmask);
    wifiManager.setConfigPortalTimeout(180);
    wifiManager.setConfigPortalBlocking(true);
    wifiManager.setBreakAfterConfig(true);

    if (!wifiManager.startConfigPortal(kSetupSsid, kSetupPassword))
    {
        Serial.println("[WIFI] Configuration failed");
        return false;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("[WIFI] Connected");
        Serial.print("[WIFI] IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }

    Serial.println("[WIFI] Configuration failed");
    return false;
}

void Wifi_manager::clearCredentials()
{
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    Serial.println("[WIFI] Credentials cleared");
}
