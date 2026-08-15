#include "Wifi_manager.h"
#include <WiFi.h>

namespace
{
    const char* kSetupSsid = "RADAR_SETUP";
    const char* kSetupPassword = "radar123";
    const char* kPrefsNamespace = "wifi_hist";
    const IPAddress kApIp(192, 168, 4, 1);
    const IPAddress kApNetmask(255, 255, 255, 0);

    Wifi_manager* gWifi = nullptr;

    void portalSaveCallback()
    {
        if (gWifi != nullptr)
        {
            gWifi->markPortalSaved();
        }
    }
}

Wifi_manager::Wifi_manager()
{
    gWifi = this;
}

bool Wifi_manager::connectWifi(const String& ssid, const String& password)
{
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);

    if (ssid.length() > 0)
    {
        Serial.print("[WIFI] Connecting to ");
        Serial.println(ssid);
        WiFi.begin(ssid.c_str(), password.c_str());
        saveNetwork(ssid, password);
    }
    else if (WiFi.SSID().length() > 0)
    {
        Serial.print("[WIFI] Connecting to ");
        Serial.println(WiFi.SSID());
        WiFi.begin();
    }
    else
    {
        String networks[Config::kMaxSavedNetworks];
        int count = 0;
        loadSavedNetworks(networks, count);
        if (count > 0)
        {
            String storedPassword;
            if (getSavedPassword(networks[0], storedPassword))
            {
                Serial.print("[WIFI] Connecting to ");
                Serial.println(networks[0]);
                WiFi.begin(networks[0].c_str(), storedPassword.c_str());
            }
            else
            {
                Serial.println("[WIFI] Not connected");
                return false;
            }
        }
        else
        {
            Serial.println("[WIFI] Not connected");
            return false;
        }
    }

    const unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < Config::kWifiConnectTimeoutMs)
    {
        delay(250);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("[WIFI] Connected");
        rememberCurrentNetwork();
        return true;
    }

    Serial.println("[WIFI] Connection failed");
    return false;
}

bool Wifi_manager::startSavedConnect(const String& ssid)
{
    String password;
    if (!getSavedPassword(ssid, password) || ssid.length() == 0)
    {
        Serial.print("[WIFI] Connecting to ");
        Serial.println(ssid);
        Serial.println("[WIFI] Connection failed");
        return false;
    }

    cancelConnecting();
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    strncpy(connectingSsid_, ssid.c_str(), sizeof(connectingSsid_) - 1);
    connectingSsid_[sizeof(connectingSsid_) - 1] = '\0';
    Serial.print("[WIFI] Connecting to ");
    Serial.println(ssid);
    WiFi.disconnect(false, false);
    delay(50);
    WiFi.begin(ssid.c_str(), password.c_str());
    connecting_ = true;
    connectStartMs_ = millis();
    return true;
}

bool Wifi_manager::pollConnecting(bool& success)
{
    success = false;
    if (!connecting_)
    {
        return false;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        connecting_ = false;
        success = true;
        rememberCurrentNetwork();
        Serial.println("[WIFI] Connected");
        return true;
    }

    if (millis() - connectStartMs_ >= Config::kWifiConnectTimeoutMs)
    {
        connecting_ = false;
        WiFi.disconnect(false, false);
        Serial.println("[WIFI] Connection failed");
        return true;
    }

    return false;
}

void Wifi_manager::cancelConnecting()
{
    connecting_ = false;
    connectingSsid_[0] = '\0';
}

bool Wifi_manager::isConnecting() const
{
    return connecting_;
}

const char* Wifi_manager::connectingSsid() const
{
    return connectingSsid_;
}

bool Wifi_manager::startSetupPortal()
{
    stopPortal();
    portalSaved_ = false;
    portalSavedMs_ = 0;

    WiFi.setSleep(false);
    WiFi.mode(WIFI_AP);
    WiFi.softAPdisconnect(true);
    delay(100);
    WiFi.softAPConfig(kApIp, kApIp, kApNetmask);
    const bool apStarted = WiFi.softAP(kSetupSsid, kSetupPassword);
    if (!apStarted)
    {
        Serial.println("[WIFI] AP startup failed");
        WiFi.mode(WIFI_STA);
        return false;
    }

    WiFi.setSleep(false);
    wifiManager_.setAPStaticIPConfig(kApIp, kApIp, kApNetmask);
    wifiManager_.setCaptivePortalEnable(true);
    wifiManager_.setConfigPortalTimeout(0);
    wifiManager_.setConfigPortalBlocking(false);
    wifiManager_.setBreakAfterConfig(true);
    wifiManager_.setSaveConfigCallback(portalSaveCallback);

    const bool started = wifiManager_.startConfigPortal(kSetupSsid, kSetupPassword);
    portalActive_ = started || wifiManager_.getConfigPortalActive();
    if (portalActive_)
    {
        Serial.println("[WIFI] Portal open");
        Serial.println("[WIFI] SSID: RADAR_SETUP");
        Serial.print("[WIFI] AP IP: ");
        Serial.println(WiFi.softAPIP());
    }
    else
    {
        Serial.println("[WIFI] Connection failed");
    }
    return portalActive_;
}

void Wifi_manager::processPortal()
{
    if (portalActive_)
    {
        wifiManager_.process();
    }
}

bool Wifi_manager::isPortalActive() const
{
    return portalActive_;
}

bool Wifi_manager::pollPortal(bool& success)
{
    success = false;
    if (!portalActive_)
    {
        return false;
    }

    processPortal();

    if (portalSaved_ && WiFi.status() == WL_CONNECTED)
    {
        success = true;
        portalActive_ = false;
        wifiManager_.stopConfigPortal();
        rememberCurrentNetwork();
        Serial.println("[WIFI] Connected");
        return true;
    }

    if (portalSaved_ && millis() - portalSavedMs_ >= Config::kPortalConnectTimeoutMs)
    {
        success = false;
        portalActive_ = false;
        wifiManager_.stopConfigPortal();
        Serial.println("[WIFI] Connection failed");
        return true;
    }

    return false;
}

void Wifi_manager::stopPortal()
{
    if (portalActive_ || wifiManager_.getConfigPortalActive())
    {
        wifiManager_.stopConfigPortal();
        Serial.println("[WIFI] Portal closed");
    }
    WiFi.softAPdisconnect(true);
    portalActive_ = false;
    portalSaved_ = false;
    WiFi.mode(WIFI_STA);
}

void Wifi_manager::markPortalSaved()
{
    portalSaved_ = true;
    portalSavedMs_ = millis();
    rememberCurrentNetwork();
    Serial.println("[WIFI] Credentials saved");
}

void Wifi_manager::saveNetwork(const String& ssid, const String& password)
{
    if (ssid.length() == 0)
    {
        return;
    }

    String ssids[Config::kMaxSavedNetworks];
    String passwords[Config::kMaxSavedNetworks];
    int count = 0;
    loadHistoryLocked(ssids, passwords, count);

    int existing = -1;
    for (int i = 0; i < count; ++i)
    {
        if (ssids[i] == ssid)
        {
            existing = i;
            break;
        }
    }

    if (existing >= 0)
    {
        const String keepPass = password.length() > 0 ? password : passwords[existing];
        for (int i = existing; i > 0; --i)
        {
            ssids[i] = ssids[i - 1];
            passwords[i] = passwords[i - 1];
        }
        ssids[0] = ssid;
        passwords[0] = keepPass;
    }
    else
    {
        const int nextCount = count < Config::kMaxSavedNetworks ? count + 1 : Config::kMaxSavedNetworks;
        for (int i = nextCount - 1; i > 0; --i)
        {
            ssids[i] = ssids[i - 1];
            passwords[i] = passwords[i - 1];
        }
        ssids[0] = ssid;
        passwords[0] = password;
        count = nextCount;
    }

    storeHistory(ssids, passwords, count);
}

void Wifi_manager::rememberCurrentNetwork()
{
    if (WiFi.status() == WL_CONNECTED && WiFi.SSID().length() > 0)
    {
        saveNetwork(WiFi.SSID(), WiFi.psk());
    }
}

void Wifi_manager::loadSavedNetworks(String (&networks)[Config::kMaxSavedNetworks], int& count) const
{
    String passwords[Config::kMaxSavedNetworks];
    loadHistoryLocked(networks, passwords, count);
}

bool Wifi_manager::getSavedPassword(const String& ssid, String& password) const
{
    String ssids[Config::kMaxSavedNetworks];
    String passwords[Config::kMaxSavedNetworks];
    int count = 0;
    loadHistoryLocked(ssids, passwords, count);
    for (int i = 0; i < count; ++i)
    {
        if (ssids[i] == ssid)
        {
            password = passwords[i];
            return password.length() > 0;
        }
    }
    return false;
}

void Wifi_manager::clearCredentials()
{
    wifiManager_.resetSettings();
    prefs_.begin(kPrefsNamespace, false);
    prefs_.clear();
    prefs_.end();
    Serial.println("[WIFI] Credentials cleared");
}

void Wifi_manager::loadHistoryLocked(String (&ssids)[Config::kMaxSavedNetworks], String (&passwords)[Config::kMaxSavedNetworks], int& count) const
{
    Preferences prefs;
    prefs.begin(kPrefsNamespace, true);
    count = prefs.getInt("count", 0);
    if (count < 0)
    {
        count = 0;
    }
    if (count > Config::kMaxSavedNetworks)
    {
        count = Config::kMaxSavedNetworks;
    }

    for (int i = 0; i < count; ++i)
    {
        ssids[i] = prefs.getString(("s" + String(i)).c_str(), "");
        passwords[i] = prefs.getString(("p" + String(i)).c_str(), "");
    }
    prefs.end();
}

void Wifi_manager::storeHistory(const String* ssids, const String* passwords, int count)
{
    prefs_.begin(kPrefsNamespace, false);
    prefs_.putInt("count", count);
    for (int i = 0; i < count; ++i)
    {
        prefs_.putString(("s" + String(i)).c_str(), ssids[i]);
        prefs_.putString(("p" + String(i)).c_str(), passwords[i]);
    }
    prefs_.end();
}
