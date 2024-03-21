#include "headers.h"

void setup() {
    _serial_.begin(115200);
    // _serial_.printf("\n\nChange cpu freq : %d\n",system_update_cpu_freq(160));
    _serial_.println("Booting");
    
    setupWiFi();
    
    setupOTA();
    
    setupVirtuino();
    
    setupPins();
    
    setupSteppers();
    
    _serial_.printf("Current freq : %u\n", ESP.getCpuFreqMHz());
}

void loop() {
    runWiFi();
    
    runSteppers();
    
    switch(winder_state){
        case IDLE:
            
        break;
        case WINDING:
            
        break;
        case PAUSE:
            
        break;
        default:
            
        break;
    }
    
}
