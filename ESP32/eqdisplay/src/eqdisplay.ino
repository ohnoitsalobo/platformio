#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>

#define NUMBER_OF_LEDS 144

bool music = 1;

void setup(){

    Serial.begin(115200); pinMode(2, OUTPUT);

    setupWiFi();
    
    fftSetup();
    
    ledSetup();
    
}

void loop(){

    wifiLoop();

    ledLoop();
    
}
