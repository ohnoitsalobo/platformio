#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>

void setup(){

    Serial.begin(115200);

    setupWiFi();
    
}

void loop(){

    wifiLoop();
    
}
