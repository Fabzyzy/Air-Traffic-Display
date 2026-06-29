#include "App.h"
#include <Arduino.h>

const char* App::getPageName()
{
    switch(currentPage)
    {
        case Page::RADAR:
            return "Radar";
        case Page::MENU:
            return "Menu";
        case Page::INFO:
            return "Info";
    }
    return "Unknown";
}

void App::nextPage()
{
    switch(currentPage)
    {
        case Page::RADAR:
            currentPage = Page::MENU;
            break;
        case Page::MENU:
            currentPage = Page::INFO;
            break;
        case Page::INFO:
            currentPage = Page::RADAR;
            break;
    }
    Serial.print("Button turned into ");
    Serial.println(getPageName());
}

void App::previousPage()
{
    switch(currentPage)
    {
        case Page::RADAR:
            currentPage = Page::INFO;
            break;
        case Page::MENU:
            currentPage = Page::RADAR;
            break;
        case Page::INFO:
            currentPage = Page::MENU;
            break;
    }
    Serial.print("Button turned into ");
    Serial.println(getPageName());
}

void App::buttonPressed()
{
    Serial.print("Button pressed on ");
    Serial.println(getPageName());
}

