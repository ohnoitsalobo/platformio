#include "headers.h"

void setup(){
    _serial_.begin(115200);
    setupWiFi();
    
    setupOTA();
    
    setupVirtuino();
    
    setupPins();
    
    setupSteppers();

    dualCoreInit();
}

void loop(){
    runWiFi();
    
    EVERY_N_SECONDS(1){
        Serial.print("Everything else running on Core ");
        Serial.println( xPortGetCoreID());
    }
}