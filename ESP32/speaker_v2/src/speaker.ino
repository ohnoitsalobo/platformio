#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>

#define NUMBER_OF_LEDS 66

// #define debug 1

HardwareSerial Serial_1(2);

bool music = 1;

void setup(){

    pinMode(2, OUTPUT);
    
    // Serial.begin(115200);
    Serial_1.begin(115200);

    setupWiFi();
    
    fftSetup();
    
    ledSetup();
    
    // Serial_1.print("\nCPU is running at ");
    // Serial_1.print(getCpuFrequencyMhz());
    // Serial_1.print(" MHz\n\n");
    
}

void loop(){
#ifdef debug
    Serial_1.println("Starting loop");
#endif

    wifiLoop();
    
    if(music)
        fftLoop();

    ledLoop();
    
#ifdef debug
    Serial_1.println("Ending loop");
#endif
}
