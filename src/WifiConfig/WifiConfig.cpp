/**************************************************************
   WifiConfig is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license
 **************************************************************/

#include "WifiConfig.h"

#define UP 5
#define DOWN 4
#define STOP 14
#define LOCK 12

WifiConfig::WifiConfig()
{
}

WifiConfig::~WifiConfig()
{
}


void WifiConfig::setupConfigPortal()
{
    DEBUG_WM(F("Configuring access point... "));
    DEBUG_WM(_apName);
    if (_apPassword != NULL)
    {
        if (strlen(_apPassword) < 8 || strlen(_apPassword) > 63)
        {
            // fail passphrase to short or long!
            DEBUG_WM(F("Invalid AccessPoint password. Ignoring"));
            _apPassword = NULL;
        }
        DEBUG_WM(_apPassword);
    }

    //optional soft ip config
    if (_ap_static_ip)
    {
        DEBUG_WM(F("Custom AP IP/GW/Subnet"));
        WiFi.softAPConfig(_ap_static_ip, _ap_static_gw, _ap_static_sn);
    }

    if (_apPassword != NULL)
    {
        WiFi.softAP(_apName, _apPassword); //password option
    }
    else
    {
        WiFi.softAP(_apName);
    }

    delay(500); // Without delay I've seen the IP address blank
    DEBUG_WM(F("AP IP address: "));
    DEBUG_WM(WiFi.softAPIP());
}

boolean WifiConfig::autoConnect()
{
    String ssid = "ESP" + String(ESP.getChipId());
    return autoConnect(ssid.c_str(), NULL);
}

boolean WifiConfig::autoConnect(char const *apName, char const *apPassword)
{
    DEBUG_WM(F(""));
    DEBUG_WM(F("AutoConnect"));

    // attempt to connect; should it fail, fall back to AP
    WiFi.mode(WIFI_STA);

    if (connectWifi("", "") == WL_CONNECTED)
    {
        DEBUG_WM(F("IP Address:"));
        DEBUG_WM(WiFi.localIP());
        //connected
        return true;
    }

    return startConfigPortal(apName, apPassword);
}

boolean WifiConfig::configPortalHasTimeout()
{
    if (_configPortalTimeout == 0 || wifi_softap_get_station_num() > 0)
    {
        _configPortalStart = millis(); // kludge, bump configportal start time to skew timeouts
        return false;
    }
    return (millis() > _configPortalStart + _configPortalTimeout);
}

boolean WifiConfig::startConfigPortal()
{
    String ssid = "ESP" + String(ESP.getChipId());
    return startConfigPortal(ssid.c_str(), NULL);
}

boolean WifiConfig::startConfigPortal(char const *apName, char const *apPassword)
{

    if (!WiFi.isConnected())
    {
        WiFi.persistent(false);
        // disconnect sta, start ap
        WiFi.disconnect(); //  this alone is not enough to stop the autoconnecter
        WiFi.mode(WIFI_AP);
        WiFi.persistent(true);
    }
    else
    {
        //setup AP
        WiFi.mode(WIFI_AP_STA);
        DEBUG_WM(F("SET AP STA"));
    }

    _apName = apName;
    _apPassword = apPassword;

    //notify we entered AP mode
    if (_apcallback != NULL)
    {
        _apcallback(this);
    }

    connect = false;
    setupConfigPortal();

    // while (1)
    // {

    //     // check if timeout
    //     if (configPortalHasTimeout())
    //         break;

    //     //DNS
    //     dnsServer->processNextRequest();
    //     //HTTP
    //     server->handleClient();

    //     if (connect)
    //     {
    //         delay(1000);
    //         connect = false;

    //         // if saving with no ssid filled in, reconnect to ssid
    //         // will not exit cp
    //         if (_ssid == "")
    //         {
    //             DEBUG_WM(F("No ssid, skipping wifi"));
    //         }
    //         else
    //         {
    //             DEBUG_WM(F("Connecting to new AP"));
    //             if (connectWifi(_ssid, _pass) != WL_CONNECTED)
    //             {
    //                 delay(2000);
    //                 // using user-provided  _ssid, _pass in place of system-stored ssid and pass
    //                 DEBUG_WM(F("Failed to connect."));
    //             }
    //             else
    //             {
    //                 //connected
    //                 WiFi.mode(WIFI_STA);
    //                 //notify that configuration has changed and any optional parameters should be saved
    //                 if (_savecallback != NULL)
    //                 {
    //                     //todo: check if any custom parameters actually exist, and check if they really changed maybe
    //                     _savecallback();
    //                 }
    //                 break;
    //             }
    //         }
    //         if (_shouldBreakAfterConfig)
    //         {
    //             //flag set to exit after config after trying to connect
    //             //notify that configuration has changed and any optional parameters should be saved
    //             if (_savecallback != NULL)
    //             {
    //                 //todo: check if any custom parameters actually exist, and check if they really changed maybe
    //                 _savecallback();
    //             }
    //             WiFi.mode(WIFI_STA); // turn off ap
    //             // reconnect to ssid
    //             // int res = WiFi.begin();
    //             // attempt connect for 10 seconds
    //             DEBUG_WM(F("Waiting for sta (10 secs) ......."));
    //             for (size_t i = 0; i < 100; i++)
    //             {
    //                 if (WiFi.status() == WL_CONNECTED)
    //                     break;
    //                 DEBUG_WM(".");
    //                 // Serial.println(WiFi.status());
    //                 delay(100);
    //             }
    //             delay(1000);
    //             break;
    //         }
    //     }
    //     yield();
    // }

    // server.reset();
    // dnsServer.reset();

    return WiFi.status() == WL_CONNECTED;
}

int WifiConfig::connectWifi(String ssid, String pass)
{
    DEBUG_WM(F("Connecting as wifi client..."));

    // check if we've got static_ip settings, if we do, use those.
    if (_sta_static_ip)
    {
        DEBUG_WM(F("Custom STA IP/GW/Subnet"));
        WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
        DEBUG_WM(WiFi.localIP());
    }
    //fix for auto connect racing issue
    if (WiFi.status() == WL_CONNECTED && (WiFi.SSID() == ssid))
    {
        DEBUG_WM(F("Already connected. Bailing out."));
        return WL_CONNECTED;
    }

    DEBUG_WM(F("Status:"));
    DEBUG_WM(WiFi.status());

    wl_status_t res;
    //check if we have ssid and pass and force those, if not, try with last saved values
    if (ssid != "")
    {
        //trying to fix connection in progress hanging
        ETS_UART_INTR_DISABLE();
        wifi_station_disconnect();
        ETS_UART_INTR_ENABLE();
        res = WiFi.begin(ssid.c_str(), pass.c_str(), 0, NULL, true);
        if (res != WL_CONNECTED)
        {
            DEBUG_WM(F("[ERROR] WiFi.begin res:"));
            DEBUG_WM(res);
        }
    }
    else
    {
        if (WiFi.SSID() != "")
        {
            DEBUG_WM(F("Using last saved values, should be faster"));
            //trying to fix connection in progress hanging
            ETS_UART_INTR_DISABLE();
            wifi_station_disconnect();
            ETS_UART_INTR_ENABLE();
            res = WiFi.begin();
        }
        else
        {
            DEBUG_WM(F("No saved credentials"));
        }
    }

    int connRes = waitForConnectResult();
    DEBUG_WM("Connection result: ");
    DEBUG_WM(connRes);
//not connected, WPS enabled, no pass - first attempt
#ifdef NO_EXTRA_4K_HEAP
    if (_tryWPS && connRes != WL_CONNECTED && pass == "")
    {
        startWPS();
        //should be connected at the end of WPS
        connRes = waitForConnectResult();
    }
#endif
    return connRes;
}

uint8_t WifiConfig::waitForConnectResult()
{
    if (_connectTimeout == 0)
    {
        return WiFi.waitForConnectResult();
    }
    else
    {
        DEBUG_WM(F("Waiting for connection result with time out"));
        unsigned long start = millis();
        boolean keepConnecting = true;
        uint8_t status;
        while (keepConnecting)
        {
            status = WiFi.status();
            if (millis() > start + _connectTimeout)
            {
                keepConnecting = false;
                DEBUG_WM(F("Connection timed out"));
            }
            if (status == WL_CONNECTED)
            {
                keepConnecting = false;
            }
            delay(100);
        }
        return status;
    }
}

void WifiConfig::startWPS()
{
    DEBUG_WM(F("START WPS"));
    WiFi.beginWPSConfig();
    DEBUG_WM(F("END WPS"));
}

String WifiConfig::getConfigPortalSSID()
{
    return _apName;
}

void WifiConfig::resetSettings()
{
    DEBUG_WM(F("settings invalidated"));
    DEBUG_WM(F("THIS MAY CAUSE AP NOT TO START UP PROPERLY. YOU NEED TO COMMENT IT OUT AFTER ERASING THE DATA."));
    WiFi.disconnect(true);
    //delay(200);
}
void WifiConfig::setTimeout(unsigned long seconds)
{
    setConfigPortalTimeout(seconds);
}

void WifiConfig::setConfigPortalTimeout(unsigned long seconds)
{
    _configPortalTimeout = seconds * 1000;
}

void WifiConfig::setConnectTimeout(unsigned long seconds)
{
    _connectTimeout = seconds * 1000;
}

void WifiConfig::setDebugOutput(boolean debug)
{
    _debug = debug;
}

void WifiConfig::setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn)
{
    _ap_static_ip = ip;
    _ap_static_gw = gw;
    _ap_static_sn = sn;
}

void WifiConfig::setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn)
{
    _sta_static_ip = ip;
    _sta_static_gw = gw;
    _sta_static_sn = sn;
}

void WifiConfig::setMinimumSignalQuality(int quality)
{
    _minimumQuality = quality;
}

void WifiConfig::setBreakAfterConfig(boolean shouldBreak)
{
    _shouldBreakAfterConfig = shouldBreak;
}

/** Handle root or redirect to captive portal */
void WifiConfig::handleRoot()
{
    DEBUG_WM(F("Handle root"));
    if (captivePortal())
    { // If caprive portal redirect instead of displaying the page.
        return;
    }

    String page;
    page.replace("{v}", "Options");

    page += String(F("<h1>"));
    page += _apName;
    page += String(F("</h1>"));
    page += String(F("<h3>WifiConfig</h3>"));

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);
}


/** Handle the WLAN save form and redirect to WLAN config page again */
void WifiConfig::handleWifiSave()
{
    DEBUG_WM(F("WiFi save"));

    //SAVE/connect here
    _ssid = server->arg("s").c_str();
    _pass = server->arg("p").c_str();

    if (server->arg("ip") != "")
    {
        DEBUG_WM(F("static ip"));
        DEBUG_WM(server->arg("ip"));
        //_sta_static_ip.fromString(server->arg("ip"));
        String ip = server->arg("ip");
        optionalIPFromString(&_sta_static_ip, ip.c_str());
    }
    if (server->arg("gw") != "")
    {
        DEBUG_WM(F("static gateway"));
        DEBUG_WM(server->arg("gw"));
        String gw = server->arg("gw");
        optionalIPFromString(&_sta_static_gw, gw.c_str());
    }
    if (server->arg("sn") != "")
    {
        DEBUG_WM(F("static netmask"));
        DEBUG_WM(server->arg("sn"));
        String sn = server->arg("sn");
        optionalIPFromString(&_sta_static_sn, sn.c_str());
    }

    DEBUG_WM(F("Sent wifi save page"));

    connect = true; //signal ready to connect/reset
}

/** Handle the info page */
void WifiConfig::handleInfo()
{
    DEBUG_WM(F("Info"));

    String page;
    page.replace("{v}", "Info");

    page += F("<dl>");
    page += F("<dt>Chip ID</dt><dd>");
    page += ESP.getChipId();
    page += F("</dd>");
    page += F("<dt>Flash Chip ID</dt><dd>");
    page += ESP.getFlashChipId();
    page += F("</dd>");
    page += F("<dt>IDE Flash Size</dt><dd>");
    page += ESP.getFlashChipSize();
    page += F(" bytes</dd>");
    page += F("<dt>Real Flash Size</dt><dd>");
    page += ESP.getFlashChipRealSize();
    page += F(" bytes</dd>");
    page += F("<dt>Soft AP IP</dt><dd>");
    page += WiFi.softAPIP().toString();
    page += F("</dd>");
    page += F("<dt>Soft AP MAC</dt><dd>");
    page += WiFi.softAPmacAddress();
    page += F("</dd>");
    page += F("<dt>Station MAC</dt><dd>");
    page += WiFi.macAddress();
    page += F("</dd>");
    page += F("</dl>");

    server->sendHeader("Content-Length", String(page.length()));
    server->send(200, "text/html", page);

    DEBUG_WM(F("Sent info page"));
}

/** Handle the reset page */
void WifiConfig::handleReset()
{

    DEBUG_WM(F("Sent reset page"));
    delay(5000);
    ESP.reset();
    delay(2000);
}

void WifiConfig::handleNotFound()
{
    if (captivePortal())
    { // If captive portal redirect instead of displaying the error page.
        return;
    }
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server->uri();
    message += "\nMethod: ";
    message += (server->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server->args();
    message += "\n";

    for (uint8_t i = 0; i < server->args(); i++)
    {
        message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
    }
    server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server->sendHeader("Pragma", "no-cache");
    server->sendHeader("Expires", "-1");
    server->sendHeader("Content-Length", String(message.length()));
    server->send(404, "text/plain", message);
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean WifiConfig::captivePortal()
{
    if (!isIp(server->hostHeader()))
    {
        DEBUG_WM(F("Request redirected to captive portal"));
        server->sendHeader("Location", String("http://") + toStringIp(server->client().localIP()), true);
        server->send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        server->client().stop();             // Stop is needed because we sent no content length
        return true;
    }
    return false;
}

//start up config portal callback
void WifiConfig::setAPCallback(void (*func)(WifiConfig *myWifiConfig))
{
    _apcallback = func;
}

//start up save config callback
void WifiConfig::setSaveConfigCallback(void (*func)(void))
{
    _savecallback = func;
}

//sets a custom element to add to head, like a new style tag
void WifiConfig::setCustomHeadElement(const char *element)
{
    _customHeadElement = element;
}

//if this is true, remove duplicated Access Points - defaut true
void WifiConfig::setRemoveDuplicateAPs(boolean removeDuplicates)
{
    _removeDuplicateAPs = removeDuplicates;
}

template <typename Generic>
void WifiConfig::DEBUG_WM(Generic text)
{
    if (_debug)
    {
        Serial.print("*WM: ");
        Serial.println(text);
    }
}

int WifiConfig::getRSSIasQuality(int RSSI)
{
    int quality = 0;

    if (RSSI <= -100)
    {
        quality = 0;
    }
    else if (RSSI >= -50)
    {
        quality = 100;
    }
    else
    {
        quality = 2 * (RSSI + 100);
    }
    return quality;
}

/** Is this an IP? */
boolean WifiConfig::isIp(String str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9'))
        {
            return false;
        }
    }
    return true;
}

/** IP to String? */
String WifiConfig::toStringIp(IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++)
    {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}
