#include "Encoder.h"
#include "App.h"
#include <Arduino.h>

extern App app;

namespace
{
    bool readPinState(uint8_t pin)
    {
        return digitalRead(pin) == LOW;
    }
}

void Encoder::begin()
{
    pinMode(Config::kEncoderClkPin, INPUT_PULLUP);
    pinMode(Config::kEncoderDtPin, INPUT_PULLUP);
    pinMode(Config::kEncoderSwPin, INPUT_PULLUP);

    const unsigned long now = millis();
    clkInput.rawState = readPinState(Config::kEncoderClkPin);
    clkInput.stableState = clkInput.rawState;
    clkInput.lastTransitionMs = now;

    dtInput.rawState = readPinState(Config::kEncoderDtPin);
    dtInput.stableState = dtInput.rawState;
    dtInput.lastTransitionMs = now;

    buttonInput.rawState = readPinState(Config::kEncoderSwPin);
    buttonInput.stableState = buttonInput.rawState;
    buttonInput.lastTransitionMs = now;

    buttonLongPressTriggered = false;
    buttonPressStartMs = 0;
    eventHead = 0;
    eventCount = 0;
}

void Encoder::updateInput(DebouncedInput& input, uint8_t pin, unsigned long now)
{
    const bool rawState = readPinState(pin);

    if (rawState != input.rawState)
    {
        input.rawState = rawState;
        input.lastTransitionMs = now;
    }

    if (input.rawState != input.stableState && now - input.lastTransitionMs >= Config::kEncoderDebounceMs)
    {
        input.stableState = input.rawState;
    }
}

void Encoder::enqueueEvent(EventType event)
{
    if (eventCount >= kEventQueueSize)
    {
        return;
    }

    const size_t index = (eventHead + eventCount) % kEventQueueSize;
    eventQueue[index] = event;
    ++eventCount;
}

bool Encoder::hasEvent() const
{
    return eventCount > 0;
}

bool Encoder::dequeueEvent(EventType& event)
{
    if (eventCount == 0)
    {
        return false;
    }

    event = eventQueue[eventHead];
    eventHead = (eventHead + 1) % kEventQueueSize;
    --eventCount;
    return true;
}

void Encoder::dispatchLegacyEvent(EventType event)
{
    switch (event)
    {
        case EventType::CW:
            app.nextPage();
            break;
        case EventType::CCW:
            app.previousPage();
            break;
        case EventType::Pressed:
            app.buttonPressed();
            break;
        default:
            break;
    }
}

void Encoder::printEvent(EventType event)
{
    switch (event)
    {
        case EventType::CW:
            Serial.println("CW");
            break;
        case EventType::CCW:
            Serial.println("CCW");
            break;
        case EventType::Pressed:
            Serial.println("Button Pressed");
            break;
        case EventType::Released:
            Serial.println("Button Released");
            break;
        case EventType::LongPress:
            Serial.println("Long Press");
            break;
        default:
            break;
    }
}

void Encoder::update()
{
    const unsigned long now = millis();

    const bool previousClkState = clkInput.stableState;
    const bool previousDtState = dtInput.stableState;
    const bool previousButtonState = buttonInput.stableState;

    updateInput(clkInput, Config::kEncoderClkPin, now);
    updateInput(dtInput, Config::kEncoderDtPin, now);
    updateInput(buttonInput, Config::kEncoderSwPin, now);

    if (clkInput.stableState != previousClkState && clkInput.stableState)
    {
        const EventType event = (dtInput.stableState != clkInput.stableState) ? EventType::CW : EventType::CCW;
        enqueueEvent(event);
        printEvent(event);
        dispatchLegacyEvent(event);
    }

    if (buttonInput.stableState != previousButtonState)
    {
        if (buttonInput.stableState)
        {
            const EventType event = EventType::Pressed;
            enqueueEvent(event);
            printEvent(event);
            dispatchLegacyEvent(event);
            buttonPressStartMs = now;
            buttonLongPressTriggered = false;
        }
        else
        {
            const EventType event = EventType::Released;
            enqueueEvent(event);
            printEvent(event);
            buttonLongPressTriggered = false;
            buttonPressStartMs = 0;
        }
    }
    else if (buttonInput.stableState && !buttonLongPressTriggered && buttonPressStartMs != 0 && now - buttonPressStartMs >= Config::kEncoderLongPressMs)
    {
        const EventType event = EventType::LongPress;
        enqueueEvent(event);
        printEvent(event);
        buttonLongPressTriggered = true;
    }
}
