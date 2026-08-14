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

void App::loadDisplaySettings()
{
    displayPrefs.begin("display", true);
    detectionRadiusKm = displayPrefs.getInt("radius", Config::kDefaultDetectionRadiusKm);
    const int colorIndex = displayPrefs.getInt("color", 0);
    displayPrefs.end();
    detectionRadiusKm = constrain(detectionRadiusKm, Config::kMinDetectionRadiusKm, Config::kMaxDetectionRadiusKm);
    radarColor = Ui::colorAt(colorIndex);
    applyTheme();
}

void App::saveDisplaySettings()
{
    displayPrefs.begin("display", false);
    displayPrefs.putInt("radius", detectionRadiusKm);
    displayPrefs.putInt("color", Ui::colorIndex(radarColor));
    displayPrefs.end();
}

void App::applyTheme()
{
    Ui::setPrimary(radarColor);
    displaySettingsScreen.setRadius(detectionRadiusKm);
    displaySettingsScreen.setColor(radarColor);
    radarScreen.setPrimaryColor(radarColor);
    radarScreen.setRadiusKm(detectionRadiusKm);
    radarScreen.setCenter(Config::kRadarCenterLatitude, Config::kRadarCenterLongitude);
}

void App::begin()
{
    loadDisplaySettings();
    navigationDepth = 0;
    setState(AppState::MAIN_MENU, false);
}

void App::drawCurrentScreen()
{
    if (!screenDirty || activeScreen == nullptr)
    {
        return;
    }
    activeScreen->draw();
    screenDirty = false;
}

void App::leaveTransientWifi()
{
    if (currentState == AppState::WIFI_PORTAL)
    {
        wifi.stopPortal();
    }
    if (currentState == AppState::WIFI_CONNECTING)
    {
        wifi.cancelConnecting();
    }
}

void App::goBack()
{
    leaveTransientWifi();
    if (navigationDepth <= 0)
    {
        setState(AppState::MAIN_MENU, false);
        return;
    }
    const AppState previous = navigationStack[--navigationDepth];
    setState(previous, false);
}

void App::refreshWifiStatus()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        const String rssi = String(WiFi.RSSI()) + " dBm";
        wifiScreen.setConnectionStatus(true, WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), rssi.c_str());
    }
    else
    {
        wifiScreen.setConnectionStatus(false, "", "", "");
    }
}

void App::refreshSavedNetworks()
{
    String networks[Config::kMaxSavedNetworks];
    int count = 0;
    wifi.loadSavedNetworks(networks, count);
    wifiScreen.setSavedNetworks(networks, count);
}

void App::setState(AppState newState, bool withPush)
{
    if (withPush && currentState != newState && navigationDepth < 8)
    {
        navigationStack[navigationDepth++] = currentState;
    }

    if (currentState == AppState::WIFI_PORTAL && newState != AppState::WIFI_PORTAL)
    {
        wifi.stopPortal();
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
            radarScreen.setRadiusKm(detectionRadiusKm);
            radarScreen.setWifiConnected(WiFi.status() == WL_CONNECTED);
            radarScreen.resetSweep();
            beginAircraftUpdates();
            Serial.println("[MENU] Radar Display");
            break;

        case AppState::PLANE_DETAILS:
            activeScreen = &planeDetailsScreen;
            stopAircraftUpdates();
            Serial.println("[MENU] Plane Details");
            break;

        case AppState::WIFI_SETTINGS:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(WifiScreen::Mode::Menu);
            wifiScreen.setSelection(0);
            stopAircraftUpdates();
            Serial.println("[MENU] WiFi Settings");
            break;

        case AppState::WIFI_CURRENT_CONNECTION:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(WifiScreen::Mode::Current);
            refreshWifiStatus();
            stopAircraftUpdates();
            Serial.println("[MENU] Current Connection");
            break;

        case AppState::WIFI_SAVED_CONNECTIONS:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(WifiScreen::Mode::Saved);
            refreshSavedNetworks();
            wifiScreen.setSelection(0);
            stopAircraftUpdates();
            Serial.println("[MENU] Saved Connections");
            break;

        case AppState::WIFI_PORTAL:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(WifiScreen::Mode::Portal);
            stopAircraftUpdates();
            Serial.println("[WIFI] Portal open");
            wifi.startSetupPortal();
            break;

        case AppState::WIFI_CONNECTING:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(WifiScreen::Mode::Connecting);
            stopAircraftUpdates();
            break;

        case AppState::WIFI_RESULT:
            activeScreen = &wifiScreen;
            wifiScreen.setMode(WifiScreen::Mode::Result);
            resultShownMs = millis();
            stopAircraftUpdates();
            break;
    }

    drawCurrentScreen();
}

void App::update()
{
    if (currentState == AppState::WIFI_PORTAL)
    {
        bool success = false;
        if (wifi.pollPortal(success))
        {
            resultSuccess = success;
            const String ssid = WiFi.SSID();
            wifiScreen.setStatusText(success ? "Connected" : "Connection Failed", "SSID:",
                                     ssid.length() > 0 ? ssid.c_str() : "");
            setState(AppState::WIFI_RESULT, false);
            return;
        }
    }

    if (currentState == AppState::WIFI_CONNECTING)
    {
        bool success = false;
        if (wifi.pollConnecting(success))
        {
            resultSuccess = success;
            wifiScreen.setStatusText(success ? "Connected" : "Connection Failed", "SSID:", wifi.connectingSsid());
            setState(AppState::WIFI_RESULT, false);
            return;
        }
    }

    if (currentState == AppState::WIFI_RESULT && resultSuccess &&
        millis() - resultShownMs >= Config::kSuccessHintMs)
    {
        goBack();
        return;
    }

    updateAircraftData();

    if (activeScreen != nullptr && activeScreen->update())
    {
        screenDirty = true;
    }
    drawCurrentScreen();
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
            if (displaySettingsScreen.isEditing())
            {
                displaySettingsScreen.adjust(1);
            }
            else
            {
                displaySettingsScreen.moveNext();
            }
            break;
        case AppState::WIFI_SETTINGS:
        case AppState::WIFI_SAVED_CONNECTIONS:
            wifiScreen.moveNext();
            break;
        case AppState::RADAR_DISPLAY:
            radarScreen.nextAircraft();
            break;
        case AppState::PLANE_DETAILS:
            planeDetailsScreen.scroll(1);
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
            if (displaySettingsScreen.isEditing())
            {
                displaySettingsScreen.adjust(-1);
            }
            else
            {
                displaySettingsScreen.movePrevious();
            }
            break;
        case AppState::WIFI_SETTINGS:
        case AppState::WIFI_SAVED_CONNECTIONS:
            wifiScreen.movePrevious();
            break;
        case AppState::RADAR_DISPLAY:
            radarScreen.previousAircraft();
            break;
        case AppState::PLANE_DETAILS:
            planeDetailsScreen.scroll(-1);
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
                setState(AppState::DISPLAY_SETTINGS);
            }
            else if (settingsScreen.getSelection() == 1)
            {
                setState(AppState::WIFI_SETTINGS);
            }
            else
            {
                goBack();
            }
            return;

        case AppState::DISPLAY_SETTINGS:
            if (displaySettingsScreen.isEditing())
            {
                detectionRadiusKm = displaySettingsScreen.getRadius();
                radarColor = displaySettingsScreen.getColor();
                applyTheme();
                saveDisplaySettings();
                displaySettingsScreen.confirmEdit();
                screenDirty = true;
                return;
            }
            if (displaySettingsScreen.getSelection() == 2)
            {
                goBack();
                return;
            }
            displaySettingsScreen.beginEdit();
            screenDirty = true;
            return;

        case AppState::WIFI_SETTINGS:
            if (wifiScreen.getSelection() == 0)
            {
                setState(AppState::WIFI_CURRENT_CONNECTION);
            }
            else if (wifiScreen.getSelection() == 1)
            {
                setState(AppState::WIFI_SAVED_CONNECTIONS);
            }
            else if (wifiScreen.getSelection() == 2)
            {
                setState(AppState::WIFI_PORTAL);
            }
            else
            {
                goBack();
            }
            return;

        case AppState::WIFI_SAVED_CONNECTIONS:
            if (wifiScreen.isNewConnectionSelected() || wifiScreen.getSavedNetworkCount() == 0)
            {
                setState(AppState::WIFI_PORTAL);
                return;
            }
            {
                const String ssid = wifiScreen.getSavedNetwork(wifiScreen.getSelection());
                wifiScreen.setConnectionStatus(false, ssid.c_str(), "", "");
                if (wifi.startSavedConnect(ssid))
                {
                    setState(AppState::WIFI_CONNECTING);
                }
                else
                {
                    resultSuccess = false;
                    wifiScreen.setStatusText("Connection Failed", "SSID:", ssid.c_str());
                    setState(AppState::WIFI_RESULT, false);
                }
            }
            return;

        case AppState::WIFI_CURRENT_CONNECTION:
        case AppState::WIFI_RESULT:
            goBack();
            return;

        case AppState::RADAR_DISPLAY:
            if (radarScreen.hasSelection() && radarScreen.getSelectedAircraft() != nullptr)
            {
                planeDetailsScreen.setAircraft(*radarScreen.getSelectedAircraft(),
                                               Config::kRadarCenterLatitude,
                                               Config::kRadarCenterLongitude);
                setState(AppState::PLANE_DETAILS);
            }
            return;

        default:
            return;
    }
}

void App::handleLongPress()
{
    if (currentState == AppState::DISPLAY_SETTINGS && displaySettingsScreen.isEditing())
    {
        displaySettingsScreen.cancelEdit();
        screenDirty = true;
        return;
    }

    if (currentState == AppState::MAIN_MENU)
    {
        return;
    }

    goBack();
}

void App::beginAircraftUpdates()
{
    aircraftUpdatesEnabled = true;
    lastAircraftRequestMs = 0;
    aircraftFetcher.setLocation(Config::kRadarCenterLatitude, Config::kRadarCenterLongitude, detectionRadiusKm);
    updateAircraftData();
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

    radarScreen.setWifiConnected(WiFi.status() == WL_CONNECTED);

    bool fetchSuccess = false;
    if (aircraftFetcher.consumeFetchResult(fetchSuccess))
    {
        const int count = aircraftFetcher.getAircraftCount();
        radarScreen.setAircraft(aircraftFetcher.getAircrafts(), count);
        Serial.print("[RADAR] Aircraft update: ");
        Serial.print(radarScreen.getContactCount());
        Serial.println(" contacts");
        if (count > 0 && radarScreen.getContactCount() == 0)
        {
            Serial.print("[AIRCRAFT] API returned ");
            Serial.print(count);
            Serial.println(", displayable 0");
        }
        screenDirty = true;
    }

    if (WiFi.status() != WL_CONNECTED || aircraftFetcher.isFetchInProgress())
    {
        return;
    }

    const unsigned long now = millis();
    if (lastAircraftRequestMs != 0 && now - lastAircraftRequestMs < Config::kAircraftRefreshMs)
    {
        return;
    }

    lastAircraftRequestMs = now;
    aircraftFetcher.setLocation(Config::kRadarCenterLatitude, Config::kRadarCenterLongitude, detectionRadiusKm);
    aircraftFetcher.requestFetch();
}
