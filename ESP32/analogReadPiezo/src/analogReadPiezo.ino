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
    pinMode (2, OUTPUT);
    // pinMode (34, INPUT);
}

const int potPin = 34;
int potValue = 0;

void loop(){

    wifiLoop();
    potValue = analogRead(potPin);
    Serial.println(potValue);
    digitalWrite(2, (potValue > 3072) ? 1 : 0);
    delay(5);
}
