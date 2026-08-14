#pragma once

#include <Arduino.h>
#include "Config.h"

class Encoder
{
public:
    enum class EventType
    {
        None,
        CW,
        CCW,
        Pressed,
        Released,
        LongPress
    };

    void begin();
    void update();
    bool hasEvent() const;
    bool dequeueEvent(EventType& event);

private:
    struct DebouncedInput
    {
        bool rawState = false;
        bool stableState = false;
        unsigned long lastTransitionMs = 0;
    };

    void updateInput(DebouncedInput& input, uint8_t pin, unsigned long now);
    void enqueueEvent(EventType event);
    void dispatchLegacyEvent(EventType event);

    DebouncedInput clkInput;
    DebouncedInput dtInput;
    DebouncedInput buttonInput;

    bool buttonLongPressTriggered = false;
    unsigned long buttonPressStartMs = 0;

    static constexpr size_t kEventQueueSize = 8;
    EventType eventQueue[kEventQueueSize] = {EventType::None};
    size_t eventHead = 0;
    size_t eventCount = 0;
};
