#include "headers.h"

bool music = 0;

void setup(){

    Serial.begin(115200); pinMode(2, OUTPUT);

    setupWiFi();
    
    fftSetup();
    
    ledSetup();
    
}

void loop(){

    wifiLoop();
    
    if(music)
        fftLoop();

    ledLoop();
    
}
