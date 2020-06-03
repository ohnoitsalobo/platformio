#include <FastLED.h>

FASTLED_USING_NAMESPACE

bool MidiEventReceived = false;

void setup(){

    setupLED();

    setupWiFi();
    
    setupMIDI();

}

void loop(){

    runLED();
    
    wifiLoop();
    
    runMIDI();
    
}