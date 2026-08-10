#include "App.h"
#include <Arduino.h>
#include <WiFi.h>
#include "AircraftData.h"
#include "Wifi_manager.h"

extern Wifi_manager wifi;
extern AircraftDataFetcher aircraftFetcher;

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
            mainMenuScreen.setSelection(0);
            stopAircraftUpdates();
            Serial.println("[MENU] Main Menu");
            break;
        case AppState::RADAR_DISPLAY:
            activeScreen = &radarScreen;
            beginAircraftUpdates();
            Serial.println("[RADAR] Entered Radar Display");
            break;
        case AppState::WIFI_SETTINGS:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(0);
            wifiScreen.setSelection(0);
            stopAircraftUpdates();
            Serial.println("[MENU] WiFi Settings");
            break;
        case AppState::WIFI_CONNECTION_STATUS:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(1);
            wifiScreen.setScrollOffset(0);
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
            stopAircraftUpdates();
            Serial.println("[WIFI] Current Connection");
            break;
        case AppState::WIFI_CHANGE_CONNECTION:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(1);
            wifiScreen.setScrollOffset(0);
            Serial.println("[WIFI] Opening configuration portal");
            if (wifi.startSetupPortal())
            {
                Serial.println("[WIFI] Connected");
            }
            else
            {
                Serial.println("[WIFI] Configuration failed");
            }
            setState(AppState::WIFI_SETTINGS);
            break;
    }

    drawCurrentScreen();
}

void App::update()
{
    updateAircraftData();

    if (activeScreen != nullptr)
    {
        activeScreen->update();
    }
    drawCurrentScreen();
}

void App::setAircraftData(const Aircraft* aircrafts, int count)
{
    if (currentState == AppState::RADAR_DISPLAY)
    {
        radarScreen.setAircraft(aircrafts, count);
        screenDirty = true;
    }
}

void App::nextPage()
{
    if (currentState == AppState::MAIN_MENU)
    {
        mainMenuScreen.moveNext();
        screenDirty = true;
        return;
    }

    if (currentState == AppState::WIFI_SETTINGS)
    {
        wifiScreen.moveNext();
        screenDirty = true;
        return;
    }

    if (currentState == AppState::WIFI_CONNECTION_STATUS)
    {
        wifiScreen.scrollDown();
        screenDirty = true;
        return;
    }

    if (currentState == AppState::RADAR_DISPLAY)
    {
        radarScreen.nextAircraft();
        screenDirty = true;
    }
}

void App::previousPage()
{
    if (currentState == AppState::MAIN_MENU)
    {
        mainMenuScreen.movePrevious();
        screenDirty = true;
        return;
    }

    if (currentState == AppState::WIFI_SETTINGS)
    {
        wifiScreen.movePrevious();
        screenDirty = true;
        return;
    }

    if (currentState == AppState::WIFI_CONNECTION_STATUS)
    {
        wifiScreen.scrollUp();
        screenDirty = true;
        return;
    }

    if (currentState == AppState::RADAR_DISPLAY)
    {
        radarScreen.previousAircraft();
        screenDirty = true;
    }
}

void App::buttonPressed()
{
    if (currentState == AppState::MAIN_MENU)
    {
        if (mainMenuScreen.getSelection() == 0)
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
        if (wifiScreen.getSelection() == 0)
        {
            setState(AppState::WIFI_CONNECTION_STATUS);
        }
        else if (wifiScreen.getSelection() == 1)
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

void App::beginAircraftUpdates()
{
    aircraftUpdatesEnabled = true;
    lastAircraftUpdateMs = 0;
    if (WiFi.status() == WL_CONNECTED)
    {
        aircraftFetcher.setLocation(51.5072f, -0.1276f, 100);
        updateAircraftData();
    }
}

void App::stopAircraftUpdates()
{
    aircraftUpdatesEnabled = false;
}

void App::updateAircraftData()
{
    if (!aircraftUpdatesEnabled || currentState != AppState::RADAR_DISPLAY)
    {
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    const unsigned long now = millis();
    if (lastAircraftUpdateMs != 0 && now - lastAircraftUpdateMs < kAircraftUpdateIntervalMs)
    {
        return;
    }

    lastAircraftUpdateMs = now;
    if (aircraftFetcher.fetchAndPrintAircrafts())
    {
        setAircraftData(aircraftFetcher.getAircrafts(), aircraftFetcher.getAircraftCount());
    }
}

