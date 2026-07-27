#pragma once

#include "DisplayManager.h"
#include "AircraftManager.h"

class RadarScreen
{
public:
    RadarScreen(DisplayManager& display, AircraftManager& aircraftManager);

    void enter();
    void update();
    void onRotate(int8_t direction);
    void onButton();
    void onLongPress();

private:
    DisplayManager& display_;
    AircraftManager& aircraftManager_;
    uint32_t lastRefreshMs_ = 0;
};
