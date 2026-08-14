#include "App.h"
#include <Arduino.h>
#include <WiFi.h>
#include "AircraftData.h"
#include "Wifi_manager.h"

extern Wifi_manager wifi;
extern AircraftDataFetcher aircraftFetcher;

App::App() : activeScreen(&mainMenuScreen)
{
}

void App::begin()
{
    navigationDepth = 0;
    setState(AppState::MAIN_MENU, false);
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

void App::pushState(AppState state)
{
    if (navigationDepth < 7)
    {
        navigationStack[navigationDepth++] = state;
    }
}

void App::popState()
{
    if (navigationDepth > 0)
    {
        currentState = navigationStack[--navigationDepth];
        screenDirty = true;
    }
    else
    {
        currentState = AppState::MAIN_MENU;
        screenDirty = true;
    }
}

void App::setState(AppState newState, bool withPush)
{
    if (withPush && currentState != newState && navigationDepth < 7)
    {
        navigationStack[navigationDepth++] = currentState;
    }

    currentState = newState;
    screenDirty = true;

    switch (currentState)
    {
        case AppState::MAIN_MENU:
            activeScreen = &mainMenuScreen;
            mainMenuScreen.setSelection(0);
            stopAircraftUpdates();
            Serial.println("[MENU] Main Menu");
            break;

        case AppState::SETTINGS:
            activeScreen = &settingsScreen;
            settingsScreen.setSelection(0);
            stopAircraftUpdates();
            Serial.println("[MENU] Settings");
            break;

        case AppState::DISPLAY_SETTINGS:
            activeScreen = &displaySettingsScreen;
            displaySettingsScreen.setRadius(detectionRadiusKm);
            displaySettingsScreen.setColor(radarColor);
            displaySettingsScreen.setSelection(0);
            stopAircraftUpdates();
            Serial.println("[MENU] Display Settings");
            break;

        case AppState::RADAR_DISPLAY:
            activeScreen = &radarScreen;
            radarScreen.setPrimaryColor(radarColor);
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

        case AppState::WIFI_CURRENT_CONNECTION:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(1);
            wifiScreen.setScrollOffset(0);
            {
                String statusText = WiFi.isConnected() ? "Connected" : "Disconnected";
                String ssidText = WiFi.SSID();
                if (ssidText.length() == 0)
                {
                    ssidText = "None";
                }
                String ipText = WiFi.localIP().toString();
                if (!WiFi.isConnected())
                {
                    ipText = "N/A";
                }
                String rssiText = WiFi.isConnected() ? String(WiFi.RSSI()) + " dBm" : "N/A";
                String gatewayText = WiFi.gatewayIP().toString();
                String subnetText = WiFi.subnetMask().toString();
                String macText = WiFi.macAddress();
                wifiScreen.setConnectionStatus(statusText.c_str(), ssidText.c_str(), ipText.c_str(), rssiText.c_str(), gatewayText.c_str(), subnetText.c_str(), macText.c_str());
            }
            stopAircraftUpdates();
            Serial.println("[WIFI] Current Connection");
            break;

        case AppState::WIFI_SAVED_CONNECTIONS:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(3);
            wifiScreen.setSelection(0);
            stopAircraftUpdates();
            Serial.println("[WIFI] Saved Connections");
            break;

        case AppState::WIFI_NEW_CONNECTION:
        case AppState::WIFI_CHANGE_CONNECTION:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(2);
            wifiScreen.setScrollOffset(0);
            Serial.println("[WIFI] Configuration started");
            if (wifi.startSetupPortal())
            {
                Serial.println("[WIFI] Connected");
                wifiScreen.setMode(1);
                String statusText = "Connected";
                String ssidText = WiFi.SSID();
                String ipText = WiFi.localIP().toString();
                String rssiText = String(WiFi.RSSI()) + " dBm";
                String gatewayText = WiFi.gatewayIP().toString();
                String subnetText = WiFi.subnetMask().toString();
                String macText = WiFi.macAddress();
                wifiScreen.setConnectionStatus(statusText.c_str(), ssidText.c_str(), ipText.c_str(), rssiText.c_str(), gatewayText.c_str(), subnetText.c_str(), macText.c_str());
            }
            else
            {
                Serial.println("[WIFI] Configuration failed");
                wifiScreen.setMode(2);
            }
            break;

        case AppState::PLANE_DETAILS:
            activeScreen = &radarScreen;
            stopAircraftUpdates();
            Serial.println("[RADAR] Plane details view");
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
    if (currentState == AppState::RADAR_DISPLAY || currentState == AppState::PLANE_DETAILS)
    {
        radarScreen.setAircraft(aircrafts, count);
        screenDirty = true;
    }
}

void App::nextPage()
{
    switch (currentState)
    {
        case AppState::MAIN_MENU:
            mainMenuScreen.moveNext();
            break;
        case AppState::SETTINGS:
            settingsScreen.moveNext();
            break;
        case AppState::DISPLAY_SETTINGS:
            displaySettingsScreen.moveNext();
            break;
        case AppState::WIFI_SETTINGS:
            wifiScreen.moveNext();
            break;
        case AppState::WIFI_CURRENT_CONNECTION:
            wifiScreen.scrollDown();
            break;
        case AppState::RADAR_DISPLAY:
        case AppState::PLANE_DETAILS:
            radarScreen.nextAircraft();
            break;
        default:
            return;
    }
    screenDirty = true;
}

void App::previousPage()
{
    switch (currentState)
    {
        case AppState::MAIN_MENU:
            mainMenuScreen.movePrevious();
            break;
        case AppState::SETTINGS:
            settingsScreen.movePrevious();
            break;
        case AppState::DISPLAY_SETTINGS:
            displaySettingsScreen.movePrevious();
            break;
        case AppState::WIFI_SETTINGS:
            wifiScreen.movePrevious();
            break;
        case AppState::WIFI_CURRENT_CONNECTION:
            wifiScreen.scrollUp();
            break;
        case AppState::RADAR_DISPLAY:
        case AppState::PLANE_DETAILS:
            radarScreen.previousAircraft();
            break;
        default:
            return;
    }
    screenDirty = true;
}

void App::buttonPressed()
{
    switch (currentState)
    {
        case AppState::MAIN_MENU:
            if (mainMenuScreen.getSelection() == 0)
            {
                setState(AppState::RADAR_DISPLAY);
            }
            else
            {
                setState(AppState::SETTINGS);
            }
            return;

        case AppState::SETTINGS:
            if (settingsScreen.getSelection() == 0)
            {
                setState(AppState::RADAR_DISPLAY);
            }
            else if (settingsScreen.getSelection() == 1)
            {
                setState(AppState::DISPLAY_SETTINGS);
            }
            else if (settingsScreen.getSelection() == 2)
            {
                setState(AppState::WIFI_SETTINGS);
            }
            else
            {
                setState(AppState::MAIN_MENU);
            }
            return;

        case AppState::DISPLAY_SETTINGS:
            if (displaySettingsScreen.getSelection() == 0)
            {
                setDetectionRadius(detectionRadiusKm + Config::kRadiusStepKm);
            }
            else if (displaySettingsScreen.getSelection() == 1)
            {
                setDetectionRadius(detectionRadiusKm - Config::kRadiusStepKm);
            }
            else if (displaySettingsScreen.getSelection() == 2)
            {
                setRadarColor(radarColor == DisplayColors::kGreen ? DisplayColors::kAmber : DisplayColors::kGreen);
            }
            else
            {
                setState(AppState::SETTINGS);
            }
            return;

        case AppState::WIFI_SETTINGS:
            if (wifiScreen.getSelection() == 0)
            {
                setState(AppState::WIFI_CURRENT_CONNECTION);
            }
            else if (wifiScreen.getSelection() == 1)
            {
                setState(AppState::WIFI_CHANGE_CONNECTION);
            }
            else if (wifiScreen.getSelection() == 2)
            {
                setState(AppState::SETTINGS);
            }
            else
            {
                setState(AppState::MAIN_MENU);
            }
            return;

        case AppState::WIFI_CURRENT_CONNECTION:
        case AppState::WIFI_NEW_CONNECTION:
        case AppState::WIFI_CHANGE_CONNECTION:
        case AppState::WIFI_SAVED_CONNECTIONS:
            setState(AppState::WIFI_SETTINGS);
            return;

        case AppState::RADAR_DISPLAY:
        case AppState::PLANE_DETAILS:
            setState(AppState::MAIN_MENU);
            return;

        default:
            setState(AppState::MAIN_MENU);
            return;
    }
}

void App::handleLongPress()
{
    if (navigationDepth > 0)
    {
        popState();
        return;
    }

    setState(AppState::MAIN_MENU, false);
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
    if (lastAircraftUpdateMs != 0 && now - lastAircraftUpdateMs < Config::kAircraftRefreshMs)
    {
        return;
    }

    lastAircraftUpdateMs = now;
    if (aircraftFetcher.fetchAndPrintAircrafts())
    {
        setAircraftData(aircraftFetcher.getAircrafts(), aircraftFetcher.getAircraftCount());
    }
}

void App::setDetectionRadius(int radiusKm)
{
    detectionRadiusKm = constrain(radiusKm, Config::kMinDetectionRadiusKm, Config::kMaxDetectionRadiusKm);
    displaySettingsScreen.setRadius(detectionRadiusKm);
    radarScreen.setPrimaryColor(radarColor);
    screenDirty = true;
}

int App::getDetectionRadius() const
{
    return detectionRadiusKm;
}

void App::setRadarColor(uint16_t color)
{
    radarColor = color;
    displaySettingsScreen.setColor(radarColor);
    radarScreen.setPrimaryColor(radarColor);
    screenDirty = true;
}

uint16_t App::getRadarColor() const
{
    return radarColor;
}

