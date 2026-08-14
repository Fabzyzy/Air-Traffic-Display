#pragma once

#include "Config.h"
#include <Arduino.h>
#include <Adafruit_GC9A01A.h>

namespace Ui
{
    void setPrimary(uint16_t color);
    uint16_t primary();
    uint16_t dim(uint16_t color, uint8_t brightness);
    const char* colorName(uint16_t color);
    int colorIndex(uint16_t color);
    uint16_t colorAt(int index);

    void fillBackground();
    void drawCentered(const char* text, int y, uint16_t color, uint8_t size);
    void drawTitle(const char* title);
    void drawHint(const char* text);
    int textWidth(const char* text, uint8_t size);
}

extern Adafruit_GC9A01A tft;
