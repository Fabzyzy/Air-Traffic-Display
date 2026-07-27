#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "Encoder.h"
#include "App.h"
#include <WiFi.h>
#include "Wifi_manager.h"
#include "AircraftData.h"

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

int sweepAngle = 0;
unsigned long lastSweepMs = 0;

void drawVintageRadarSweep()
{
  const int cx = 120;
  const int cy = 120;
  const int radius = 100;

  tft.fillScreen(0x0000);

  tft.drawCircle(cx, cy, radius, 0x07E0);
  tft.drawCircle(cx, cy, radius / 2, 0x0520);
  tft.drawCircle(cx, cy, radius / 4, 0x0310);
  tft.drawLine(cx, 20, cx, 220, 0x0520);
  tft.drawLine(20, cy, 220, cy, 0x0520);

  for (int i = 0; i <= radius; i += 20)
  {
    tft.drawCircle(cx, cy, i, 0x0340);
  }

  for (int i = 0; i < 360; i += 20)
  {
    float rad = i * 0.017453f;
    int x = cx + cos(rad) * radius;
    int y = cy + sin(rad) * radius;
    tft.drawLine(cx, cy, x, y, 0x0320);
  }

  float rad = sweepAngle * 0.017453f;
  int x = cx + cos(rad) * radius;
  int y = cy + sin(rad) * radius;
  tft.drawLine(cx, cy, x, y, 0xFFFF);

  tft.setTextWrap(false);
  tft.setTextSize(1);
  tft.setTextColor(0x07E0);
  tft.setCursor(10, 10);
  tft.print("Vintage Radar");
}

void runDisplayTest()
{
  unsigned long now = millis();
  if (now - lastSweepMs < 30)
  {
    return;
  }

  lastSweepMs = now;
  sweepAngle = (sweepAngle + 3) % 360;
  drawVintageRadarSweep();
}

void setup() {
  Serial.begin(115200);
  delay(5000);

  SPI.begin(18, -1, 23, 5);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setCursor(20, 90);
  tft.print("Booting...");

  encoder.begin();
  Serial.println("ESP32 Booted.");

  if (!wifi.connectWifi())
  {
    wifi.startSetupPortal();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to WiFi.");
    Serial.println("Starting Radar...");
    aircraftFetcher.setLocation(51.5072f, -0.1276f, 100);

    // Give the connection a moment to stabilize before first fetch
    delay(2000);
    aircraftFetcher.fetchAndPrintAircrafts();
  }
}

void loop() {
  encoder.update();
  runDisplayTest();

  unsigned long now = millis();
  if (now - lastAircraftFetch >= kAircraftFetchIntervalMs)
  {
    lastAircraftFetch = now;
    if (WiFi.status() == WL_CONNECTED)
    {
      aircraftFetcher.fetchAndPrintAircrafts();
    }
  }
}
