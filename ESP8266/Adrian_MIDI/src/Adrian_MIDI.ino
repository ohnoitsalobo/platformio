
void setup(){
    wifiStuff();

    OTAsetup();

    LEDsetup();

}

void loop(){
    wifiLoop();

    LEDloop();

    yield();

}

