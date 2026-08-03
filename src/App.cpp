#include "App.h"
#include <Arduino.h>
#include <WiFi.h>
#include "Wifi_manager.h"

extern Wifi_manager wifi;

namespace
{
    constexpr int kWifiMenuItems = 3;
}

App::App()
{
    activeScreen = &mainMenuScreen;
}

void App::begin()
{
    setState(AppState::MAIN_MENU);
}

void App::drawCurrentScreen()
{
    if (!screenDirty)
    {
        return;
    }

    if (activeScreen != nullptr)
    {
        activeScreen->draw();
    }
    screenDirty = false;
}

void App::setState(AppState newState)
{
    currentState = newState;
    screenDirty = true;
    hasEnteredState = true;

    switch (currentState)
    {
        case AppState::MAIN_MENU:
            activeScreen = &mainMenuScreen;
            mainMenuScreen.setSelection(selectedIndex);
            Serial.println("[MENU] Main Menu");
            break;
        case AppState::RADAR_DISPLAY:
            activeScreen = &radarScreen;
            Serial.println("[MENU] Radar Display");
            break;
        case AppState::WIFI_SETTINGS:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(0);
            wifiScreen.setSelection(0);
            Serial.println("[MENU] WiFi Settings");
            break;
        case AppState::WIFI_CONNECTION_STATUS:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(1);
            {
                String statusText = WiFi.isConnected() ? "Connected" : "Disconnected";
                String ssidText = WiFi.SSID();
                String ipText = WiFi.localIP().toString();
                String rssiText = String(WiFi.RSSI());
                String gatewayText = WiFi.gatewayIP().toString();
                String subnetText = WiFi.subnetMask().toString();
                String macText = WiFi.macAddress();
                wifiScreen.setConnectionStatus(statusText.c_str(), ssidText.c_str(), ipText.c_str(), rssiText.c_str(), gatewayText.c_str(), subnetText.c_str(), macText.c_str());
            }
            Serial.println("[MENU] Current Connection");
            break;
        case AppState::WIFI_CHANGE_CONNECTION:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(1);
            Serial.println("[MENU] Change Connection");
            wifi.startSetupPortal();
            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("[WIFI] Connected");
            }
            setState(AppState::WIFI_SETTINGS);
            break;
    }

    drawCurrentScreen();
}

void App::update()
{
    if (activeScreen != nullptr)
    {
        activeScreen->update();
    }
    drawCurrentScreen();
}

void App::setAircraftData(const Aircraft* aircrafts, int count)
{
    radarScreen.setAircraft(aircrafts, count);
    screenDirty = true;
}

void App::nextPage()
{
    if (currentState == AppState::MAIN_MENU)
    {
        selectedIndex = (selectedIndex + 1) % 2;
        mainMenuScreen.setSelection(selectedIndex);
        screenDirty = true;
        Serial.println("[MENU] Main Menu Selection");
        return;
    }

    if (currentState == AppState::WIFI_SETTINGS)
    {
        selectedIndex = (selectedIndex + 1) % kWifiMenuItems;
        wifiScreen.setSelection(selectedIndex);
        screenDirty = true;
        Serial.println("[MENU] WiFi Selection");
        return;
    }

    if (currentState == AppState::RADAR_DISPLAY)
    {
        radarScreen.nextAircraft();
        screenDirty = true;
        Serial.println("[RADAR] Selected Aircraft");
    }
}

void App::previousPage()
{
    if (currentState == AppState::MAIN_MENU)
    {
        selectedIndex = (selectedIndex == 0) ? 1 : 0;
        mainMenuScreen.setSelection(selectedIndex);
        screenDirty = true;
        Serial.println("[MENU] Main Menu Selection");
        return;
    }

    if (currentState == AppState::WIFI_SETTINGS)
    {
        selectedIndex = (selectedIndex == 0) ? kWifiMenuItems - 1 : selectedIndex - 1;
        wifiScreen.setSelection(selectedIndex);
        screenDirty = true;
        Serial.println("[MENU] WiFi Selection");
        return;
    }

    if (currentState == AppState::RADAR_DISPLAY)
    {
        radarScreen.previousAircraft();
        screenDirty = true;
        Serial.println("[RADAR] Selected Aircraft");
    }
}

void App::buttonPressed()
{
    if (currentState == AppState::MAIN_MENU)
    {
        if (selectedIndex == 0)
        {
            setState(AppState::RADAR_DISPLAY);
        }
        else
        {
            setState(AppState::WIFI_SETTINGS);
        }
        return;
    }

    if (currentState == AppState::WIFI_SETTINGS)
    {
        if (selectedIndex == 0)
        {
            setState(AppState::WIFI_CONNECTION_STATUS);
        }
        else if (selectedIndex == 1)
        {
            setState(AppState::WIFI_CHANGE_CONNECTION);
        }
        else
        {
            setState(AppState::MAIN_MENU);
        }
        return;
    }

    if (currentState == AppState::WIFI_CONNECTION_STATUS || currentState == AppState::WIFI_CHANGE_CONNECTION)
    {
        setState(AppState::WIFI_SETTINGS);
        return;
    }

    if (currentState != AppState::MAIN_MENU)
    {
        Serial.println("[MENU] Returning to Main Menu");
        setState(AppState::MAIN_MENU);
    }
}

void App::handleLongPress()
{
    if (currentState != AppState::MAIN_MENU)
    {
        Serial.println("[MENU] Returning to Main Menu");
        setState(AppState::MAIN_MENU);
    }
}

