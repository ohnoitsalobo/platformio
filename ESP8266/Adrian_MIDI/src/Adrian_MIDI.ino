
void setup(){
    wifiStuff();

    OTAsetup();

    LEDsetup();
    
    blynkSetup();
}

void loop(){
    wifiLoop();

    LEDloop();
        
    blynkLoop();
    
    yield();

}

