#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

class Screen
{
public:
    virtual ~Screen() = default;
    virtual void draw() = 0;
    virtual bool update() { return false; }
};

class MainMenuScreen : public Screen
{
public:
    MainMenuScreen();
    void draw() override;
    bool update() override;

    void setSelection(int selection);

private:
    int selectedIndex = 0;
};

class RadarScreen : public Screen
{
public:
    void draw() override;
    bool update() override;
};

class WifiScreen : public Screen
{
public:
    void draw() override;
    bool update() override;
};

extern Adafruit_GC9A01A tft;
void drawVintageRadarSweep();
