#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "Encoder.h"
#include "App.h"
#include <WiFi.h>
#include "Wifi_manager.h"
#include "AircraftData.h"
#include "Ui.h"

Encoder encoder;
App app;
Wifi_manager wifi;
AircraftDataFetcher aircraftFetcher;

#define TFT_CS 5
#define TFT_DC 27
#define TFT_RST 33

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
    Serial.begin(Config::kSerialBaud);
    Serial.println("[BOOT] Air Traffic Display starting");

    Serial.println("[BOOT] Initializing display");
    SPI.begin(18, -1, 23, 5);
    tft.begin();
    tft.setRotation(0);
    Ui::fillBackground();
    Ui::drawCentered("Booting...", 110, DisplayColors::kText, 2);
    Serial.println("[BOOT] Display OK");

    Serial.println("[BOOT] Initializing encoder");
    encoder.begin();
    Serial.println("[BOOT] Encoder OK");

    Serial.println("[BOOT] Initializing WiFi");
    if (wifi.connectWifi())
    {
        Serial.println("[BOOT] WiFi connected");
    }
    else
    {
        Serial.println("[WIFI] Not connected");
    }

    app.begin();
    Serial.println("[BOOT] Application ready");
}

void loop()
{
    encoder.update();
    app.update();
}
