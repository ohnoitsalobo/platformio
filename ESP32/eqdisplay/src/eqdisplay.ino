#define _serial_ Serial
// #define debug 1
#include "headers.h"

// HardwareSerial Serial_1(2);

void setup(){

    pinMode(2, OUTPUT);
    Serial.begin(115200);

    setupWiFi();
    
    fftSetup();
    
    ledSetup();
    
    MIDIsetup();

    TelnetStream.begin();
    
    // _serial_.print("\nCPU is running at ");
    // _serial_.print(getCpuFrequencyMhz());
    // _serial_.print(" MHz\n\n");
    
    dualCoreInit();
}

void loop(){
#ifdef debug
    _serial_.println("Starting loop");
#endif
    
    wifiLoop();
    
    MIDIloop();

    ledLoop();
    
#ifdef debug
    _serial_.println("Ending loop");
#endif
}

