#define _serial_ Serial
// #define debug 1
#include "headers.h"

// HardwareSerial Serial_1(2);

void setup(){

    pinMode(2, OUTPUT);
    Serial.begin(115200);

    setupWiFi();
    
    // fftSetup();
    
    ledSetup();
    
    // dualCoreInit();
}

void loop(){
#ifdef debug
    _serial_.println("Starting loop");
#endif
    
    wifiLoop();
    

    ledLoop();
    
#ifdef debug
    _serial_.println("Ending loop");
#endif
}
