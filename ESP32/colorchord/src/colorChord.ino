#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>

#include "ccconfig.h"

#define samples  512 // must ALWAYS be a power of 2
#define samplingFrequency 40000 // 25000
unsigned int sampling_period_us;
unsigned long microseconds;

void setup(){

    Serial.begin(115200); pinMode(2, OUTPUT);

    setupWiFi();
    
    ledSetup();
    
    InitColorChord();
    
    // sampling_period_us = round(1000000*(1.0/samplingFrequency));
}

void loop(){
    
    wifiLoop();
    
    // microseconds = micros();
    // for(int i=0; i<samples; i++){
        PushSample32(analogRead(39));
        // while(micros() - microseconds < sampling_period_us){  }
        // microseconds += sampling_period_us;
    // }
    
    HandleFrameInfo();
    
    UpdateAllSameLEDs();
    
    ledLoop();

}
