#include "headers.h"

void setup() {
    // // Serial.begin(115200);
    // Serial.println("Booting");
    
    setupWifi();
    
    setupMIDI();
    
    setupLED();
    
}

void loop() {
    
    runWifi();
        
    runLED();
    
    // if(V[0] == 0){
        // digitalWrite(led_pin, 0);
    // }else{
        // digitalWrite(led_pin, 1);
    // }
    
    yield();
    
    // vDelay(20);     // This is an example of the recommended delay function. Remove this if you don't need
}
