#include "Screen.h"
#include <Arduino.h>

namespace
{
    constexpr uint16_t kBgColor = 0x0000;
    constexpr uint16_t kTextColor = 0xFFFF;
    constexpr uint16_t kHighlightColor = 0x07E0;

    void drawHeader(const char* title)
    {
        tft.fillScreen(kBgColor);
        tft.setTextWrap(false);
        tft.setTextSize(2);
        tft.setTextColor(kTextColor);
        tft.setCursor(10, 10);
        tft.print(title);
    }
}

MainMenuScreen::MainMenuScreen()
{
    selectedIndex = 0;
}

void MainMenuScreen::setSelection(int selection)
{
    selectedIndex = selection;
}

void MainMenuScreen::draw()
{
    drawHeader("Main Menu");

    tft.setTextSize(2);
    tft.setTextColor(kTextColor);
    tft.setCursor(20, 60);
    tft.print("Radar Display");

    tft.setCursor(20, 100);
    tft.print("WiFi Settings");

    tft.setTextColor(kHighlightColor);
    const int y = selectedIndex == 0 ? 60 : 100;
    tft.setCursor(10, y);
    tft.print("> ");
}

bool MainMenuScreen::update()
{
    return false;
}

void RadarScreen::draw()
{
    drawHeader("RADAR DISPLAY");
    tft.setTextSize(1);
    tft.setTextColor(kTextColor);
    tft.setCursor(20, 80);
    tft.println("(Radar graphics placeholder)");
}

bool RadarScreen::update()
{
    return false;
}

void WifiScreen::draw()
{
    drawHeader("WiFi Settings");
    tft.setTextSize(1);
    tft.setTextColor(kTextColor);
    tft.setCursor(20, 70);
    tft.println("Current Connection");
    tft.setCursor(20, 95);
    tft.println("Change Connection");
}

bool WifiScreen::update()
{
    return false;
}

void drawVintageRadarSweep()
{
    const int cx = 120;
    const int cy = 120;
    const int radius = 100;

    tft.fillScreen(0x0000);

    tft.drawCircle(cx, cy, radius, 0x07E0);
    tft.drawCircle(cx, cy, radius / 2, 0x0520);
    tft.drawCircle(cx, cy, radius / 4, 0x0310);
    tft.drawLine(cx, 20, cx, 220, 0x0520);
    tft.drawLine(20, cy, 220, cy, 0x0520);

    for (int i = 0; i <= radius; i += 20)
    {
        tft.drawCircle(cx, cy, i, 0x0340);
    }

    for (int i = 0; i < 360; i += 45)
    {
        float rad = i * 0.017453f;
        int x = cx + cos(rad) * radius;
        int y = cy + sin(rad) * radius;
        tft.drawLine(cx, cy, x, y, 0x0320);
    }

    float rad = 0.0f;
    int x = cx + cos(rad) * radius;
    int y = cy + sin(rad) * radius;
    tft.drawLine(cx, cy, x, y, 0xFFFF);

    tft.setTextWrap(false);
    tft.setTextSize(1);
    tft.setTextColor(0x07E0);
    tft.setCursor(10, 10);
    tft.print("Vintage Radar");
}
