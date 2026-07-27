#pragma once

#include "DisplayManager.h"
#include "Encoder.h"

enum class AppState;

class MenuScreen
{
public:
    MenuScreen(DisplayManager& display);

    void enter();
    void onRotate(int8_t direction);
    AppState onButton() const;
    void onLongPress() const;
    int getSelectedIndex() const;

private:
    DisplayManager& display_;
    int selectedIndex_ = 0;
    const char* options_[2] = { "Radar Display", "WiFi Settings" };
};
