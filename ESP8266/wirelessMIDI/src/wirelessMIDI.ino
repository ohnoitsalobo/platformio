// #define DEBUGGER 1

String temp = "wirelessMIDI_" + String(ESP.getChipId());
const char* sessionID = temp.c_str();

void setup(){
#ifdef DEBUGGER
    Serial.begin(115200);
#endif
    setupLED();

    setupWifi();
    
    setupMIDI();
    
    // setupBlynk();
#ifdef DEBUGGER
    delay(5000);
#endif
}

void loop(){
    runLED();
    
    runWifi();
    
    runMIDI();
    
    // runBlynk();
}