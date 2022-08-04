#if __cplusplus >= 201703L
#define register // keyword 'register' is banned with c++17
#endif

#include "header.h" // all the gory libraries and global details go here

void setup(){
    Serial.begin(115200);
    Serial.print("Booting ...");
    
    setupWifi();
    
    setupMIDI();
    
    setupLeds();
    
    Serial.println(". Done!");
}

void loop(){
    runWifi();
    
    runLeds();
}