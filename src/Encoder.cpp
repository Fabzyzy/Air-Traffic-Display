#include "Encoder.h"
#include "App.h"
#include <Arduino.h>


// Change according to wiring
#define CLK_PIN 25
#define DT_PIN 26
#define SW_PIN 32

extern App app;

void Encoder::begin()
{
    pinMode(CLK_PIN, INPUT_PULLUP);
    pinMode(DT_PIN, INPUT_PULLUP);
    pinMode(SW_PIN, INPUT_PULLUP);

    lastCLK = digitalRead(CLK_PIN);
}


void Encoder::update()
{
    // Read rotation
    int currentCLK = digitalRead(CLK_PIN);

    if (currentCLK != lastCLK && currentCLK == LOW)
    {
        int currentDT = digitalRead(DT_PIN);
        if (currentDT != currentCLK)
        {
            app.nextPage();
        }
        else
        {
            app.previousPage();
        }
    }
    lastCLK = currentCLK;

    // Read button press
    static bool lastButtonState = HIGH;

    bool buttonState = digitalRead(SW_PIN);

    if (lastButtonState == HIGH && buttonState == LOW)
    {
        app.buttonPressed();
    }
    lastButtonState = buttonState;
}
