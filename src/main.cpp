#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "Encoder.h"
#include "App.h"
#include <WiFi.h>
#include "Wifi_manager.h"
#include "AircraftData.h"

#define DEBUG_GENERAL 1
#define DEBUG_ENCODER 0
#define DEBUG_HTTP 0
#define DEBUG_AIRCRAFT 0
#define DEBUG_WIFI 1

Encoder encoder;
App app;
Wifi_manager wifi;
AircraftDataFetcher aircraftFetcher;

#define TFT_CS 5
#define TFT_DC 27
#define TFT_RST 33

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

unsigned long lastAircraftFetch = 0;
const unsigned long kAircraftFetchIntervalMs = 10000;

void setup()
{
  Serial.begin(115200);
  Serial.println("==============================");
  Serial.println("Air Traffic Display Boot");
  Serial.println("==============================");
  Serial.println();
  Serial.println("[BOOT] Air Traffic Display Started");
  delay(5000);

#if DEBUG_GENERAL
  Serial.println("[BOOT] Initializing Display...");
#endif
  SPI.begin(18, -1, 23, 5);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setCursor(20, 90);
  tft.print("Booting...");
#if DEBUG_GENERAL
  Serial.println("[BOOT] Display OK");
#endif

#if DEBUG_GENERAL
  Serial.println("[BOOT] Initializing Encoder...");
#endif
  encoder.begin();
#if DEBUG_GENERAL
  Serial.println("[BOOT] Encoder OK");
#endif

#if DEBUG_GENERAL
  Serial.println("[BOOT] Initializing WiFi...");
#endif
  if (!wifi.connectWifi())
  {
    wifi.startSetupPortal();
  }
#if DEBUG_GENERAL
  Serial.println("[BOOT] WiFi OK");
#endif

  Serial.println("[BOOT] Entering Main Menu");
  app.begin();

  if (WiFi.status() == WL_CONNECTED)
  {
#if DEBUG_WIFI
    Serial.println("[WIFI] Connected to WiFi.");
    Serial.println("[BOOT] Starting Radar...");
#else
    Serial.println("[WIFI] Connected");
#endif
    aircraftFetcher.setLocation(51.5072f, -0.1276f, 100);

    delay(2000);
    if (aircraftFetcher.fetchAndPrintAircrafts())
    {
      app.setAircraftData(aircraftFetcher.getAircrafts(), aircraftFetcher.getAircraftCount());
    }
  }
}

void loop()
{
  encoder.update();
  app.update();

  unsigned long now = millis();
  if (now - lastAircraftFetch >= kAircraftFetchIntervalMs)
  {
    lastAircraftFetch = now;
    if (WiFi.status() == WL_CONNECTED)
    {
      if (aircraftFetcher.fetchAndPrintAircrafts())
      {
        app.setAircraftData(aircraftFetcher.getAircrafts(), aircraftFetcher.getAircraftCount());
      }
    }
  }
}
