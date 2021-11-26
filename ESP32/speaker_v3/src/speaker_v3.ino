#define _serial_ Serial_1 // Serial_1
// #define _debug 1
HardwareSerial Serial_1(2);


#include "headers.h"

void setup(){
    pinMode(2, OUTPUT);
    _serial_.begin(115200);

    wifiSetup();
    
    fftSetup();
    
    ledSetup();
    
    MIDIsetup();

    dualCoreInit();
    
    setupVirtuino();
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