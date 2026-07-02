#include "Wifi_manager.h"
#include <DNSServer.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

namespace
{
    Preferences prefs;
    WebServer server(80);
    DNSServer dnsServer;
    const char* kSetupSsid = "RADAR_SETUP";
    const char* kSetupPassword = "radar123";
    const IPAddress kApIp(192, 168, 4, 1);
    const IPAddress kApNetmask(255, 255, 255, 0);
}

String wifiSSID = "";
String wifiPassword = "";

void Wifi_manager::saveCredentials(String ssid, String password)
{
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", password);
    prefs.end();

    wifiSSID = ssid;
    wifiPassword = password;
    Serial.println("WiFi credentials saved.");
}

void Wifi_manager::clearCredentials()
{
    prefs.begin("wifi", false);
    prefs.clear();
    prefs.end();
    wifiSSID = "";
    wifiPassword = "";
    Serial.println("Credentials cleared.");
}

bool Wifi_manager::loadCredentials()
{
    prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", "");
    String password = prefs.getString("password", "");
    prefs.end();

    wifiSSID = ssid;
    wifiPassword = password;

    if (ssid.length() > 0 && password.length() > 0)
    {
        Serial.println("Loaded WiFi credentials for:");
        Serial.println(ssid);
    }
    else
    {
        Serial.println("No WiFi credentials found.");
    }
    return ssid.length() > 0 && password.length() > 0;
}

bool Wifi_manager::connectWifi()
{
    if (!loadCredentials())
    {
        Serial.println("Failed to load WiFi credentials.");
        return false;
    }

    Serial.print("Attempting to connect to: ");
    Serial.println(wifiSSID);

    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(false);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi connected successfully.");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }

    Serial.println("Failed to connect to WiFi.");
    return false;
}

void Wifi_manager::handleSetupRoot()
{
    sendPortalPage("");
}

void Wifi_manager::handleSetupSubmit()
{
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.length() == 0)
    {
        sendPortalPage("Please enter a Wi-Fi network name.");
        return;
    }

    saveCredentials(ssid, password);
    sendPortalPage("Connecting to your Wi-Fi network...");

    if (connectWifi())
    {
        sendPortalPage("Connected successfully. You can close this page.");
        setupPortalActive = false;
        WiFi.softAPdisconnect(true);
        server.stop();
        dnsServer.stop();
        return;
    }

    sendPortalPage("Connection failed. Please check the Wi-Fi name and password.");
}

void Wifi_manager::sendPortalPage(String message)
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Radar Wi-Fi Setup</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 24px; color: #1f2937; }
    .card { max-width: 420px; margin: 0 auto; padding: 20px; border: 1px solid #d1d5db; border-radius: 12px; box-shadow: 0 2px 8px rgba(0,0,0,0.08); }
    input { width: 100%; padding: 10px; margin-top: 8px; margin-bottom: 12px; border: 1px solid #cbd5e1; border-radius: 8px; box-sizing: border-box; }
    button { width: 100%; padding: 10px; background: #2563eb; color: white; border: none; border-radius: 8px; font-size: 16px; }
    .message { margin-top: 14px; color: %COLOR%; font-weight: bold; }
  </style>
</head>
<body>
  <div class="card">
    <h2>Connect Radar to Wi-Fi</h2>
    <p>Enter your home or office Wi-Fi details. The ESP32 will save them and connect automatically.</p>
    <form method="post" action="/">
      <label for="ssid">Wi-Fi Name</label>
      <input id="ssid" name="ssid" placeholder="MyWiFi" required>
      <label for="password">Password</label>
      <input id="password" name="password" type="password" placeholder="Password">
      <button type="submit">Connect</button>
    </form>
    <div class="message">%MESSAGE%</div>
  </div>
</body>
</html>
)rawliteral";

    String color = "#2563eb";
    if (message.indexOf("Connected") >= 0)
    {
        color = "#15803d";
    }
    else if (message.length() > 0)
    {
        color = "#b91c1c";
    }

    html.replace("%MESSAGE%", message.length() > 0 ? message : "Enter your Wi-Fi details to continue.");
    html.replace("%COLOR%", color);
    server.send(200, "text/html", html);
}

void Wifi_manager::startSetupPortal()
{
    Serial.println("Starting setup portal...");
    setupPortalActive = true;

    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(false);
    WiFi.softAPConfig(kApIp, kApIp, kApNetmask);
    WiFi.softAP(kSetupSsid, kSetupPassword);

    server.on("/", HTTP_GET, [this]() { handleSetupRoot(); });
    server.on("/", HTTP_POST, [this]() { handleSetupSubmit(); });
    server.on("/save", HTTP_POST, [this]() { handleSetupSubmit(); });
    server.onNotFound([this]() { handleSetupRoot(); });
    server.begin();

    dnsServer.start(53, "*", WiFi.softAPIP());

    Serial.print("Portal IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Open the setup page from your phone and connect to the RADAR_SETUP network.");

    while (setupPortalActive)
    {
        dnsServer.processNextRequest();
        server.handleClient();
        delay(2);
    }

    server.stop();
    dnsServer.stop();
}

