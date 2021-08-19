#include "headers.h"

#define led_pin 1

void setup() {
    // Serial.begin(115200);
    Serial.println("Booting");
    
    setupWifi();
    
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, 0);
}

void loop() {
    ArduinoOTA.handle();
    virtuinoRun();        // Necessary function to communicate with Virtuino. Client handler
    
    if(V[0] == 0){
        digitalWrite(led_pin, 0);
    }else{
        digitalWrite(led_pin, 1);
    }
    
    yield();
    
    // vDelay(20);     // This is an example of the recommended delay function. Remove this if you don't need
}
