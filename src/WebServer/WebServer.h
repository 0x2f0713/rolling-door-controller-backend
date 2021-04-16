#ifndef WebServer_h
#define WebServer_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <memory>

class WebServer
{
public:
    WebServer();
    void setupWeb();
    void startWeb();
    void handleClient();
    void handleRoot();
    void handleScanWifi();
    void handleSetConfig();
    void handleNotFound();
    int getRSSIasQuality(int RSSI);

protected:
    DNSServer dnsServer;
    // WifiConfig wificonfig;
    bool loadFromSpiffs(String path);
};

#endif