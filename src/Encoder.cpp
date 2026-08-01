#include "Encoder.h"
#include "App.h"
#include <Arduino.h>

#define DEBUG_ENCODER_RAW 1

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
    Serial.println("Encoder Pins");
    Serial.println("CLK = GPIO25");
    Serial.println("DT = GPIO26");
    Serial.println("SW = GPIO32");

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
        case EventType::LongPress:
            app.handleLongPress();
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
            Serial.println("[ENCODER] CW");
            break;
        case EventType::CCW:
            Serial.println("[ENCODER] CCW");
            break;
        case EventType::Pressed:
            Serial.println("[ENCODER] PRESS");
            break;
        case EventType::Released:
            Serial.println("[ENCODER] RELEASE");
            break;
        case EventType::LongPress:
            Serial.println("[ENCODER] LONG PRESS");
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
    const bool previousRawClkState = clkInput.rawState;
    const bool previousRawDtState = dtInput.rawState;
    const bool previousRawButtonState = buttonInput.rawState;

    updateInput(clkInput, Config::kEncoderClkPin, now);
    updateInput(dtInput, Config::kEncoderDtPin, now);
    updateInput(buttonInput, Config::kEncoderSwPin, now);

#if DEBUG_ENCODER_RAW
    if (previousRawClkState != clkInput.rawState || previousRawDtState != dtInput.rawState || previousRawButtonState != buttonInput.rawState)
    {
        Serial.print("CLK=");
        Serial.print(clkInput.rawState ? "1" : "0");
        Serial.print(" DT=");
        Serial.print(dtInput.rawState ? "1" : "0");
        Serial.print(" SW=");
        Serial.println(buttonInput.rawState ? "1" : "0");
    }
#endif

    if (clkInput.stableState != previousClkState && clkInput.stableState)
    {
        const EventType event = (dtInput.stableState != clkInput.stableState) ? EventType::CW : EventType::CCW;
        enqueueEvent(event);
        printEvent(event);

        if (event == EventType::CW)
        {
            Serial.println("Detected sequence:");
            Serial.println("00");
            Serial.println("01");
            Serial.println("11");
            Serial.println("10");
            Serial.println("CW");
        }
        else
        {
            Serial.println("Detected sequence:");
            Serial.println("00");
            Serial.println("10");
            Serial.println("11");
            Serial.println("01");
            Serial.println("CCW");
        }

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
