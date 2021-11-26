#define _serial_ Serial // Serial_1 for bluetooth device
// #define _debug 1

#include "headers.h"

HardwareSerial Serial_1(2);

void setup(){
    pinMode(2, OUTPUT);
    _serial_.begin(115200);

    wifiSetup();
    
    setupVirtuino();
    
    ledSetup();
    
    MIDIsetup();

    dualCoreInit();
    
    fftSetup();
    
}

void loop(){
#ifdef _debug
    _serial_.println("Starting loop");
#endif
    
    wifiLoop();

    ledLoop();
        
#ifdef _debug
    _serial_.println("Ending loop");
#endif
}