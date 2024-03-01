#include "headers.h"

void setup() {
    _serial_.begin(115200);
    _serial_.println("Booting");
    
    setupWiFi();
    
    setupOTA();
    
    setupVirtuino();
    
    setupPins();
    
    setupSpeedControl();
}

void loop() {
    runWiFi();
    
    runSpeedControl();
}
