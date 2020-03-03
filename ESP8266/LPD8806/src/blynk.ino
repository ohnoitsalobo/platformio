/*
#include <BlynkSimpleEsp8266.h>

char auth[] = "a002108cd44b4d9a97b171f8be4d68fb";

void blynkSetup(){
    Blynk.config(auth);
}

BLYNK_WRITE(0){ // brightness
    int temp = param.asInt();
    FastLED.setBrightness(int((temp*temp)/255.0));
}

BLYNK_WRITE(1){ // music / auto select
    soundresponsive = param.asInt();
    if(soundresponsive)
        FastLED.setBrightness(255);
    else
        FastLED.setBrightness(15);
    
    gCurrentPatternNumber = 0;
}

BLYNK_WRITE(2){ // next pattern
    if(param.asInt())
        nextPattern();
}

BLYNK_WRITE(3){ // L
    
}

BLYNK_WRITE(4){ // R
    
}

BLYNK_WRITE(5){ // hue
    
}

BLYNK_WRITE(6){ // sat
    
}
// */