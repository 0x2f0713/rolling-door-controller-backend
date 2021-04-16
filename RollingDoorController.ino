#include "FS.h"
#include "src/Controller/Controller.h"
#include "src/WifiConfig/WifiConfig.h"
#include "src/WebServer/WebServer.h"

Controller dieu_khien = Controller(5, 4, 14, 12);
WifiConfig wificonfig;
WebServer webserver;
void setup()
{
    Serial.begin(115200);
    SPIFFS.begin();

    wificonfig.autoConnect("TECHDOOR");
    webserver.setupWeb();
    webserver.startWeb();
}
void loop()
{
    webserver.handleClient();
}
