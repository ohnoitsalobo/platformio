// #define DEBUGGER 1

void setup(){
#ifdef DEBUGGER
    Serial.begin(115200);
#endif
    setupLED();

    setupWiFi();
    
    setupMIDI();
    
    // setupBlynk();
#ifdef DEBUGGER
    delay(5000);
#endif
}

void loop(){
    runLED();
    
    wifiLoop();
    
    runMIDI();
    
    // runBlynk();
}