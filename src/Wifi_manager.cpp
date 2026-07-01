#include "Wifi_manager.h"
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

Preferences prefs;
WebServer server(80);

String wifiSSID = "Mesther";
String wifiPassword = "MTscmbFaV!";

void Wifi_manager::saveCredentials(String ssid, String password)
{
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", password);
    prefs.end();
    Serial.println("WiFi credentials saved.");
}

void Wifi_manager::clearCredentials()
{
    prefs.begin("wifi", false);
    prefs.clear();
    prefs.end();
    Serial.println("Credentials cleared.");
}

bool Wifi_manager::loadCredentials()
{
    prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", "");
    String password = prefs.getString("password", "");
    prefs.end();

    if (ssid.length() > 0 && password.length() > 0)
    {
        Serial.println("Loaded WiFi credentials:");
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("Password: ");
        Serial.println(password);
    }
    else
    {
        Serial.println("No WiFi credentials found.");
    }
    return ssid.length() > 0 && password.length() > 0;
}

bool Wifi_manager::connectWifi()
{
    if (!loadCredentials())
    {
        Serial.println("Failed to load WiFi credentials.");
        return false;
    }

    Serial.println("Attempting to connect to: ");
    Serial.println(wifiSSID);

    WiFi.begin(
        wifiSSID.c_str(),
        wifiPassword.c_str()
    );

    unsigned long startTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) // 10 seconds timeout
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi connected successfully.");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    Serial.println("Failed to connect to WiFi.");
    return false;
}

void Wifi_manager::startSetupPortal()
{
    Serial.println("Starting setup portal...");
    WiFi.mode(WIFI_AP); // Check
    WiFi.softAP(
        "RADAR_SETUP",
        "radar123"
    );

    Serial.print("Portal IP: ");
    Serial.println(WiFi.softAPIP());
    //while (true) // wil create webpage for user to enter wifi credentials in this loop
    //{
    //    delay(100);
    //};
}

