#define _serial_ Serial
// #define _debug 1
#include "headers.h"

// HardwareSerial Serial_1(2);

void setup(){

    pinMode(2, OUTPUT);
    _serial_.begin(115200);

    setupWiFi();
    
    fftSetup();
    
    ledSetup();
    
    MIDIsetup();

    dualCoreInit();
}

void loop(){
#ifdef _debug
    _serial_.println("Starting loop");
#endif
    
    wifiLoop();
    
    MIDIloop();

    ledLoop();
        
#ifdef _debug
    _serial_.println("Ending loop");
#endif
}
