#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

class Menu
{
public:
    Menu();

    void setItems(const char* const* items, int count);
    void setItem(int index, const char* label);
    void setCount(int count);
    void setSelection(int selection);
    int getSelection() const;
    int getCount() const;
    void moveNext();
    void movePrevious();

    void drawList(const char* title, int visibleItems = 5) const;

private:
    static constexpr int kMaxItems = 8;
    static constexpr int kMaxLabelLen = 28;

    char storage_[kMaxItems][kMaxLabelLen] = {};
    const char* pointers_[kMaxItems] = {};
    int itemCount_ = 0;
    int selectedIndex_ = 0;

    int clampSelection(int value) const;
};

extern Adafruit_GC9A01A tft;
