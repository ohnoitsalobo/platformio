char blynkAuth[] = "95y13VELKuhBALusysknhF2u_kyyYOEI";
IPAddress blynkServer(192,168,0,200);
int blynkPort = 8080;

void blynkSetup(){
    Blynk.config(blynkAuth, blynkServer, blynkPort);
}

void blynkLoop(){
    // EVERY_N_MILLISECONDS(20){
        Blynk.run();
    // }
}

BLYNK_WRITE(V0){ // MUSIC
    if(param.asInt()>0){
        music  = 1;
        _auto  = 0;
        manual = 0;
        _setBrightness = 255;
    }
}
BLYNK_WRITE(V1){ // AUTO
    if(param.asInt()>0){
        music  = 0;
        _auto  = 1;
        manual = 0;
        FastLED.setBrightness(30);
    }
}
BLYNK_WRITE(V2){ // MANUAL
    if(param.asInt()>0){
        music  = 0;
        _auto  = 0;
        manual = 1;
        _setBrightness = 255;
    }
}
BLYNK_WRITE(V3){ // brightness
    int x = param.asInt();
    x = (x*x)/255.0;
    _setBrightness = x;
}
BLYNK_WRITE(V4){ // next pattern
    if(param.asInt()>0)
        nextPattern();
}
BLYNK_WRITE(V5){ // previous pattern
    if(param.asInt()>0)
        previousPattern();
}
BLYNK_WRITE(V6){ // red
    int x = param.asInt();
    x = (x*x)/255.0;
    manualColor.r = x;
    setManualcolor();
}
BLYNK_WRITE(V7){ // green
    int x = param.asInt();
    x = (x*x)/255.0;
    manualColor.g = x;
    setManualcolor();
}
BLYNK_WRITE(V8){ // blue
    int x = param.asInt();
    x = (x*x)/255.0;
    manualColor.b = x;
    setManualcolor();
}
BLYNK_WRITE(V9){ // hue
    int x = param.asInt();
    manualHSV.h = x;
    manualColor = manualHSV;
    setManualcolor();
}
BLYNK_WRITE(V10){ // saturation
    int x = param.asInt();
    manualHSV.s = x;
    manualColor = manualHSV;
    setManualcolor();
}

void setManualcolor(){
    manualColor_L = manualColor;
    manualColor_R = manualColor;
}
