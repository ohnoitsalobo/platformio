#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>

#define kMatrixWidth    8
#define kMatrixHeight   9
#define NUMBER_OF_LEDS kMatrixWidth*kMatrixHeight

bool music = 0;

void setup(){

    Serial.begin(115200); pinMode(2, OUTPUT);

    setupWiFi();
    
    fftSetup();
    
    ledSetup();
    
}

void loop(){

    wifiLoop();
    
    if(music)
        fftLoop();

    ledLoop();
    
}
