#include "WebServer.h"
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);
WebServer::WebServer()
{
}

void WebServer::setupWeb()
{
    server.on("/", std::bind(&WebServer::handleRoot, this));
    server.on("/scanWifi", std::bind(&WebServer::handleScanWifi, this));
    server.on("/setConfig", std::bind(&WebServer::handleSetConfig, this));
    server.onNotFound(std::bind(&WebServer::handleNotFound, this));
}
void WebServer::startWeb()
{
    server.begin();
}
void WebServer::handleClient()
{
    server.handleClient();
}

void WebServer::handleRoot()
{
    server.sendHeader("Location", "/index.html", true); //Redirect to our html web page
    server.send(302, "text/plain", "");
}
void WebServer::handleScanWifi()
{
    int n = WiFi.scanNetworks();
    if (n == 0)
    {
        Serial.println("No networks found.");
    }
    else
    {
        //sort networks
        int indices[n];
        for (int i = 0; i < n; i++)
        {
            indices[i] = i;
        }

        // RSSI SORT

        // old sort
        for (int i = 0; i < n; i++)
        {
            for (int j = i + 1; j < n; j++)
            {
                if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i]))
                {
                    std::swap(indices[i], indices[j]);
                }
            }
        }

        String cssid;
        for (int i = 0; i < n; i++)
        {
            if (indices[i] == -1)
                continue;
            cssid = WiFi.SSID(indices[i]);
            for (int j = i + 1; j < n; j++)
            {
                if (cssid == WiFi.SSID(indices[j]))
                {
                    indices[j] = -1; // set dup aps to index -1
                }
            }
        }

        //display networks in page
        DynamicJsonDocument doc(1024);
        String JsonData;
        for (int i = 0; i < n; i++)
        {
            if (indices[i] == -1)
                continue; // skip dups

            Serial.println(WiFi.SSID(indices[i]).c_str());
            Serial.println(WiFi.encryptionType(indices[i]));
            Serial.println(WiFi.channel(indices[i]));

            doc[i]["id"] = i + 1;
            doc[i]["ssid"] = WiFi.SSID(indices[i]);
            doc[i]["signal"] = getRSSIasQuality(WiFi.RSSI(indices[i]));
            doc[i]["mac_address"] = WiFi.BSSIDstr(indices[i]);
            doc[i]["security"] = WiFi.encryptionType(indices[i]) == ENC_TYPE_NONE ? false : true;
        }
        serializeJson(doc, JsonData);
        server.send(200, "application/json", JsonData);
    }
}

void WebServer::handleSetConfig()
{
    if (server.hasArg("plain") == false)
    { //Check if body received

        server.send(500, "application/json", "{\"status\": \"Failed\"}");
        return;
    }
    server.send(200, "application/json", "{\"status\": \"OK\"}");
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    const char* ssid = doc["wifi"]["ssid"];
    const char* password = doc["wifi"]["password"];
    WiFi.begin(ssid, password, 0, NULL, true);
    delay(3000);
    ESP.reset();
}

void WebServer::handleNotFound()
{
    if (loadFromSpiffs(server.uri()))
        return;
    else
        loadFromSpiffs("/index.html");
    String message = "File Not Detected\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    Serial.println(message);
}
bool WebServer::loadFromSpiffs(String path)
{
    String dataType = "text/plain";
    if (path.endsWith("/"))
        path += "index.htm";

    if (path.endsWith(".src"))
        path = path.substring(0, path.lastIndexOf("."));
    else if (path.endsWith(".html"))
        dataType = "text/html";
    else if (path.endsWith(".htm"))
        dataType = "text/html";
    else if (path.endsWith(".css"))
        dataType = "text/css";
    else if (path.endsWith(".js"))
        dataType = "application/javascript";
    else if (path.endsWith(".png"))
        dataType = "image/png";
    else if (path.endsWith(".gif"))
        dataType = "image/gif";
    else if (path.endsWith(".jpg"))
        dataType = "image/jpeg";
    else if (path.endsWith(".ico"))
        dataType = "image/x-icon";
    else if (path.endsWith(".xml"))
        dataType = "text/xml";
    else if (path.endsWith(".pdf"))
        dataType = "application/pdf";
    else if (path.endsWith(".zip"))
        dataType = "application/zip";
    else if (path.endsWith(".woff2"))
        dataType = "font/woff2";
    File dataFile = SPIFFS.open(("/web" + path).c_str(), "r");
    if (!dataFile)
    {
        return false;
    }
    if (server.hasArg("download"))
        dataType = "application/octet-stream";
    if (server.streamFile(dataFile, dataType) != dataFile.size())
    {
    }

    dataFile.close();
    return true;
}

int WebServer::getRSSIasQuality(int RSSI)
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
