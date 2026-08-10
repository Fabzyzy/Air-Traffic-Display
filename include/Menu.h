#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

class Menu
{
public:
    Menu();

    void setItems(const char* const* items, int count);
    void setSelection(int selection);
    int getSelection() const;
    void moveNext();
    void movePrevious();

    void drawList(const char* title, int titleY, int topY, int lineHeight, int visibleItems,
                  uint16_t textColor, uint16_t highlightColor) const;

private:
    const char* const* items_ = nullptr;
    int itemCount_ = 0;
    int selectedIndex_ = 0;
    int scrollOffset_ = 0;

    int clampSelection(int value) const;
};

extern Adafruit_GC9A01A tft;
