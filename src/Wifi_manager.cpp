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
        Serial.print("Connecting to Wi-Fi network: ");
        Serial.println(ssid);
        WiFi.begin(ssid.c_str(), password.c_str());
    }
    else if (WiFi.SSID().length() > 0)
    {
        Serial.print("Connecting with saved Wi-Fi credentials: ");
        Serial.println(WiFi.SSID());
        WiFi.begin();
    }
    else
    {
        Serial.println("No saved Wi-Fi credentials available.");
        return false;
    }

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000)
    {
        delay(500);
        Serial.print('.');
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Wi-Fi connected successfully.");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }

    Serial.println("Failed to connect to Wi-Fi.");
    return false;
}

void Wifi_manager::startSetupPortal()
{
    WiFiManager wifiManager;
    wifiManager.setAPStaticIPConfig(kApIp, kApIp, kApNetmask);
    wifiManager.setConfigPortalTimeout(180);

    Serial.println("Starting Wi-Fi setup portal...");
    if (!wifiManager.autoConnect(kSetupSsid, kSetupPassword))
    {
        Serial.println("Failed to connect through Wi-Fi portal.");
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Connected to Wi-Fi through portal.");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("Wi-Fi portal exited without connection.");
    }
}

void Wifi_manager::clearCredentials()
{
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    Serial.println("Wi-Fi credentials cleared.");
}
