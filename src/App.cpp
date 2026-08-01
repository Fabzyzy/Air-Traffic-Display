#include "App.h"
#include <Arduino.h>

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
    if (currentState == newState && !hasEnteredState)
    {
        hasEnteredState = true;
    }

    currentState = newState;
    screenDirty = true;
    hasEnteredState = true;

    switch (currentState)
    {
        case AppState::MAIN_MENU:
            activeScreen = &mainMenuScreen;
            mainMenuScreen.setSelection(selectedIndex);
            Serial.println("[MENU]");
            Serial.println("Entering Main Menu");
            break;
        case AppState::RADAR_DISPLAY:
            activeScreen = &radarScreen;
            Serial.println("[MENU]");
            Serial.println("Entering Radar Display");
            break;
        case AppState::WIFI_SETTINGS:
            activeScreen = &wifiScreen;
            Serial.println("[MENU]");
            Serial.println("Entering WiFi Settings");
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

void App::nextPage()
{
    if (currentState != AppState::MAIN_MENU)
    {
        return;
    }

    selectedIndex = (selectedIndex + 1) % 2;
    mainMenuScreen.setSelection(selectedIndex);
    screenDirty = true;

    Serial.println("[MENU]");
    if (selectedIndex == 0)
    {
        Serial.println("Selection -> Radar Display");
    }
    else
    {
        Serial.println("Selection -> WiFi Settings");
    }
}

void App::previousPage()
{
    if (currentState != AppState::MAIN_MENU)
    {
        return;
    }

    selectedIndex = (selectedIndex == 0) ? 1 : 0;
    mainMenuScreen.setSelection(selectedIndex);
    screenDirty = true;

    Serial.println("[MENU]");
    if (selectedIndex == 0)
    {
        Serial.println("Selection -> Radar Display");
    }
    else
    {
        Serial.println("Selection -> WiFi Settings");
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

    if (currentState != AppState::MAIN_MENU)
    {
        Serial.println("[MENU]");
        Serial.println("Returning to Main Menu");
        setState(AppState::MAIN_MENU);
    }
}

void App::handleLongPress()
{
    if (currentState != AppState::MAIN_MENU)
    {
        Serial.println("[MENU]");
        Serial.println("Returning to Main Menu");
        setState(AppState::MAIN_MENU);
    }
}

