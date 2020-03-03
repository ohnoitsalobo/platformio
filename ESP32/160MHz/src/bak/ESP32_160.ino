
void setup(){
    Serial.begin(115200);
    
    setupWiFi();
    
    setupOTA();
    
    LEDsetup();

}

void loop(){
    
    wifiLoop();
    
    yield();
}