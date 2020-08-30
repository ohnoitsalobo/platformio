# 1 "C:\\Users\\Anand\\AppData\\Local\\Temp\\tmpxmhuma2z"
#include <Arduino.h>
# 1 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/AsyncServer.ino"




#define _serial_ Serial
#include "headers.h"
void setup();
void loop();
void fftSetup();
void fftLoop();
void PrintVector(double *vData, uint16_t bufferSize, int leftRight);
void ledSetup();
void ledLoop();
void audio_spectrum();
void audioLight();
void nextPattern();
void previousPattern();
void rainbow();
void rainbowWithGlitter();
void rainbow_scaling();
void addGlitter();
void confetti();
void sinelon();
void dot_beat();
void juggle();
void bpm();
void blendwave();
void setupNoise();
void noise1();
void noise2();
void noise3();
void fire();
void fireSparks();
void fireRainbow();
void ripple_blur();
void drawClock();
void interpolationTest();
USING_NAMESPACE_APPLEMIDI

void MIDIsetup();
void MIDIloop();
void OnAppleMidiConnected(const ssrc_t & ssrc, const char* name);
void OnAppleMidiDisconnected(const ssrc_t & ssrc);
void OnAppleMidiError(const ssrc_t& ssrc, int32_t err);
void MIDI2LED();
void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleNoteOff(byte channel, byte pitch, byte velocity);
void handlePitchBend(byte channel, int bend);
void handleControlChange(byte channel, byte number, byte value);
void core0_Task0( void * parameter );
void dualCoreInit();
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
void wifiSetup();
void wifiLoop();
void setupOTA();
void setupServer();
void handleSliders();
void timeSetup();
void timeLoop();
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
#line 9 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/AsyncServer.ino"
void setup(){
#ifdef debug
    _serial_.println("Starting setup");
#endif
    _serial_.begin(115200); pinMode(2, OUTPUT);
    _serial_.setDebugOutput(true);


    wifiSetup();


    fftSetup();


    ledSetup();


    MIDIsetup();



    dualCoreInit();



#ifdef debug
    _serial_.println("Ending setup");
#endif
}

void loop(){
#ifdef debug
    _serial_.println("Starting loop");
#endif


    wifiLoop();


    MIDIloop();


    ledLoop();

#ifdef debug
    _serial_.println("Ending loop");
#endif
}
# 1 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/FFT.ino"
# 29 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/FFT.ino"
#define LeftPin 36
#define RightPin 39

#define samples 512
#define samplingFrequency 25000





#define noise 1500
#define MAX 50000

unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[2][samples];
double vImag[2][samples];
double spectrum[3][samples/2];

arduinoFFT LFFT = arduinoFFT(vReal[0], vImag[0], samples, samplingFrequency);
arduinoFFT RFFT = arduinoFFT(vReal[1], vImag[1], samples, samplingFrequency);

void fftSetup(){
    sampling_period_us = round(1000000*(1.0/samplingFrequency));

    double exponent = 0.66;
    for (uint16_t i = 2; i < samples/2; i++){
        spectrum[0][i] = pow((i-2)/(samples/2.0-2), exponent) * NUM_LEDS;
        spectrum[1][i] = 0;
        spectrum[2][i] = 0;
    }
# 73 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/FFT.ino"
    for (uint16_t i = 0; i < samples; i++){
        vReal[0][i] = 0; vReal[1][i] = 0;
        vImag[0][i] = 0; vImag[1][i] = 0;
    }
}



void fftLoop(){
#ifdef debug
    _serial_.println("Starting fftLoop");
#endif


    microseconds = micros();
    for(int i=0; i<samples; i++){
        vReal[0][i] = analogRead(LeftPin);
        vImag[0][i] = 0;
#ifdef STEREO
        vReal[1][i] = analogRead(RightPin);
        vImag[1][i] = 0;
#endif
        while(micros() - microseconds < sampling_period_us){ }
        microseconds += sampling_period_us;
    }


    LFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    LFFT.Compute(FFT_FORWARD);
    LFFT.ComplexToMagnitude();

    PrintVector(vReal[0], (samples >> 1), 1);

#ifdef STEREO

    RFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    RFFT.Compute(FFT_FORWARD);
    RFFT.ComplexToMagnitude();

    PrintVector(vReal[1], (samples >> 1), 2);
#endif

#ifdef STEREO
#endif

#ifdef debug
    _serial_.println("Ending fftLoop");
#endif
}

void PrintVector(double *vData, uint16_t bufferSize, int leftRight) {
    for (uint16_t i = 2; i < bufferSize; i++){
        if(vData[i] > noise){
            spectrum[leftRight][i] = vData[i]-noise;
            if(spectrum[leftRight][i] > MAX)
                spectrum[leftRight][i] = MAX;
        }else{
            spectrum[leftRight][i] = 0;
        }
#ifndef STEREO
        spectrum[2][i] = spectrum[1][i];
#endif
        yield();
    }
}
# 1 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/LEDs.ino"
FASTLED_USING_NAMESPACE

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define LED_PINS 13
#define BRIGHTNESS 255*225/255

CRGBArray<NUM_LEDS> leds;
CRGBSet RIGHT (leds (0, NUM_LEDS/2-1) );
CRGBSet R1 (leds (0, NUM_LEDS/4-1) );
CRGBSet R2 (leds (NUM_LEDS/4, NUM_LEDS/2-1) );
CRGBSet LEFT (leds (NUM_LEDS/2, NUM_LEDS) );
CRGBSet L1 (leds (NUM_LEDS/2, 3*NUM_LEDS/4-1) );
CRGBSet L2 (leds (3*NUM_LEDS/4, NUM_LEDS) );

CRGBPalette16 currentPalette, targetPalette;
CRGBPalette16 randomPalette1, randomPalette2;
TBlendType currentBlending;
uint8_t maxChanges = 24;

uint8_t currentBrightness = BRIGHTNESS, _setBrightness = BRIGHTNESS;
CRGB manualColor = 0x000000;
CRGB manualColor_L = 0x000000;
CRGB manualColor_R = 0x000000;
CHSV manualHSV (0, 255, 255);
uint8_t gHue = 0, gHue1 = 0, gHue2 = 0;

typedef void (*SimplePatternList[])();
SimplePatternList autoPatterns = { drawClock, rainbow, rainbowWithGlitter, rainbow_scaling, fire, fireSparks, fireRainbow, noise1, noise2, noise3, blendwave, confetti, ripple_blur, sinelon, dot_beat, juggle };
SimplePatternList audioPatterns = { audio_spectrum, audioLight };
uint8_t gCurrentPatternNumber = 0;

void ledSetup(){
#ifdef debug
    _serial_.println("\tStarting ledSetup");
#endif
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(currentBrightness);
    FastLED.setDither(0);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);

    setupNoise();
    fill_solid (leds, NUM_LEDS, CRGB::Black);
#ifdef debug
    _serial_.println("\tEnding ledSetup");
#endif
}

void ledLoop(){
#ifdef debug
    _serial_.println("\tStarting ledLoop");
#endif
    if(MIDIconnected){
        MIDI2LED();
        FastLED.show();
    }else{
        if(music && gCurrentPatternNumber == 0)
            FFTenable = true;
        else if(FFTenable)
            FFTenable = false;

        if(music){
            audioPatterns[gCurrentPatternNumber]();
            FastLED.show();
        }
        else if(_auto){
            EVERY_N_MILLISECONDS( 41 ) { gHue1++; }
            EVERY_N_MILLISECONDS( 37 ) { gHue2--; }
            EVERY_N_SECONDS(20){
                targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
                randomPalette1 = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
                randomPalette2 = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
            }
            autoPatterns[gCurrentPatternNumber]();
            FastLED.show();
        }
        else if(manual){
            for(int i = 0; i < NUM_LEDS/2; i++){
                for(int j = 0; j < 3; j++){
                         if(LEFT [i][j] < manualColor_L[j]) LEFT [i][j]++;
                    else if(LEFT [i][j] > manualColor_L[j]) LEFT [i][j]--;
                         if(RIGHT[i][j] < manualColor_R[j]) RIGHT[i][j]++;
                    else if(RIGHT[i][j] > manualColor_R[j]) RIGHT[i][j]--;
                }
            }
            FastLED.show();
        }
    }
         if(currentBrightness < _setBrightness) FastLED.setBrightness(++currentBrightness);
    else if(currentBrightness > _setBrightness) FastLED.setBrightness(--currentBrightness);
#ifdef debug
    _serial_.println("\tEnding ledLoop");
#endif
}

void audio_spectrum(){
    uint8_t fadeval = 90;
    nscale8(leds, NUM_LEDS, fadeval);
    CRGB tempRGB1, tempRGB2;
    uint8_t pos = 0, h = 0, s = 0, v = 0;
    double temp1 = 0, temp2 = 0;
    for(int i = 2; i < samples/2; i++){
        pos = spectrum[0][i];
        h = pos/(NUM_LEDS/2.0)*224;
        temp1 = spectrum[1][i]/MAX;
        s = 255 - (temp1*30.0);
        v = temp1*255.0;
        tempRGB1 = CHSV(h, s, v);
        uint8_t p = NUM_LEDS/2-pos;
        if(tempRGB1 > RIGHT[pos]){
            RIGHT[pos] = tempRGB1;
        }

        temp2 = spectrum[2][i]/MAX;
        s = 255 - (temp2*30.0);
        v = temp2*255.0;
        tempRGB2 = CHSV(h, s, v);
        if(tempRGB2 > LEFT[pos]){
            LEFT[p] = tempRGB2;
        }
        yield();
    }
}

void audioLight(){
    EVERY_N_MILLISECONDS( 55 ) { gHue1++; }
    EVERY_N_MILLISECONDS( 57 ) { gHue2--; }
    EVERY_N_MILLISECONDS( 15 ) {
        uint8_t fadeval = 254;
        for(int i = 0; i < NUM_LEDS/4; i++){
            R1[i] = R1[i+1].nscale8(fadeval);
            R2[NUM_LEDS/4-i] = R1[i];
            L1[i] = R1[i+1].nscale8(fadeval);
            L2[NUM_LEDS/4-i] = L1[i];
        }
        uint16_t mid = 1800, _noise = 180;
        uint8_t _hue = 0, _sat = 255, _val = 0;
        int temp1 = abs(mid - analogRead( LeftPin));
        if(temp1 > _noise){
            _val = (temp1-_noise)/float(mid) * 255;
            _hue = _val/255.0 * 65;
        }
        L1[NUM_LEDS/4-1] = CHSV( _hue+gHue1, _sat, _val*_val/255);
        L2[0] = L1[NUM_LEDS/4-1];

#ifdef STEREO
        _hue = 0; _val = 0;
        int temp2 = abs(mid - analogRead(RightPin));
        if(temp2 > _noise){
            _val = (temp2-_noise)/float(mid) * 255;
            _hue = _val/255.0 * 65;
        }
        R1[NUM_LEDS/4-1] = CHSV( _hue+gHue1, _sat, _val*_val/255);
        R2[0] = R1[NUM_LEDS/4-1];
#else
        R1[NUM_LEDS/4-1] = L1[NUM_LEDS/4-1];
        R2[0] = R1[NUM_LEDS/4-1];
#endif
    }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern(){
    uint8_t temp = 0;
    if(music) temp = ARRAY_SIZE( audioPatterns );
    else temp = ARRAY_SIZE( autoPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % temp;
}

void previousPattern(){
    uint8_t temp = 0;
    if(music) temp = ARRAY_SIZE( audioPatterns );
    else temp = ARRAY_SIZE( autoPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + (temp-1)) % temp;
}

void rainbow() {

    fill_rainbow( RIGHT, NUM_LEDS/2, gHue1);
    fill_rainbow( LEFT , NUM_LEDS/2, gHue2);
}

void rainbowWithGlitter() {

    rainbow();
    addGlitter();
}

void rainbow_scaling(){
    for(int i = 0; i <= NUM_LEDS/4; i++){
        R1[i] = CHSV((millis()/77*i+1)%255 + gHue1, 255, 255);
        R2[NUM_LEDS/4-i] = R1[i];
        L1[i] = CHSV((millis()/73*i+1)%255 - gHue2, 255, 255);
        L2[NUM_LEDS/4-i] = L1[i];
    }
}

void addGlitter() {
    EVERY_N_MILLISECONDS(1000/30){
        if( random8() < 80) {
            leds[ random16(NUM_LEDS) ] += CRGB::White;
        }
    }
}

void confetti()
{
    EVERY_N_MILLISECONDS(1000/30){
        fadeToBlackBy( leds, NUM_LEDS, 30);
        int pos = random16(NUM_LEDS/2);

        RIGHT[pos] += CHSV( gHue1 + random8(64), 190+random8(65), 255);
        LEFT [pos] += CHSV( gHue2 + random8(64), 190+random8(65), 255);
    }
}

void sinelon()
{

    fadeToBlackBy( leds, NUM_LEDS, 5);
    int pos1 = beatsin16(11, 0, NUM_LEDS/2-1);
    int pos2 = beatsin16(13, 0, NUM_LEDS/2-1);
    int pos3 = beatsin16( 9, 0, NUM_LEDS/2-1);
    int pos4 = beatsin16(15, 0, NUM_LEDS/2-1);
    LEFT [pos1] = ColorFromPalette(randomPalette1, pos1, 255, LINEARBLEND);
    RIGHT[pos2] = ColorFromPalette(randomPalette2, pos2, 255, LINEARBLEND);
    LEFT [pos3] += CHSV( gHue2, 255, 255);
    RIGHT[pos4] += CHSV( gHue1, 255, 255);
}

void dot_beat() {
    uint8_t fadeval = 10;

    fadeToBlackBy( leds, NUM_LEDS, fadeval);

    uint8_t BPM, inner, outer, middle;

    BPM = 33;

    inner = beatsin8(BPM, NUM_LEDS/2/4, NUM_LEDS/2/4*3);
    outer = beatsin8(BPM, 0, NUM_LEDS/2-1);
    middle = beatsin8(BPM, NUM_LEDS/2/3, NUM_LEDS/2/3*2);

    LEFT[outer] = CHSV( gHue1 , 200, 255);
    LEFT[middle] = CHSV( gHue1+96 , 200, 255);
    LEFT[inner] = CHSV( gHue1+160, 200, 255);

    BPM = 31;

    inner = beatsin8(BPM, NUM_LEDS/2/4, NUM_LEDS/2/4*3);
    outer = beatsin8(BPM, 0, NUM_LEDS/2-1);
    middle = beatsin8(BPM, NUM_LEDS/2/3, NUM_LEDS/2/3*2);

    RIGHT[outer] = CHSV( gHue2 , 200, 255);
    RIGHT[middle] = CHSV( gHue2+96 , 200, 255);
    RIGHT[inner] = CHSV( gHue2+160, 200, 255);

}

void juggle() {

    fadeToBlackBy( leds, NUM_LEDS, 5);
    byte dothue1 = 0, dothue2 = 0;
    for( int i = 0; i < 6; i++) {
        RIGHT[beatsin16(i+7,0,NUM_LEDS/2-1)] |= CHSV(dothue1, 200, 255);
        LEFT [beatsin16(i+5,0,NUM_LEDS/2-1)] |= CHSV(dothue2, 200, 255);
        dothue1 += 32;
        dothue2 -= 32;
        yield();
    }
}

void bpm()
{

    uint8_t BeatsPerMinute = 62;

    CRGBPalette16 palette = RainbowColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS/2; i++) {
        RIGHT[i] = ColorFromPalette(palette, gHue1+(i*2), beat-gHue1+(i*10));
        LEFT [NUM_LEDS/2-1-i] = ColorFromPalette(palette, gHue2+(i*2), beat-gHue2+(i*10));
        yield();
    }
}

void blendwave() {
    CRGB clr1, clr2;
    uint8_t speed, loc1;

    speed = beatsin8(6,0,255);

    clr1 = blend(CHSV(beatsin8(3,0,255),255,255), CHSV(beatsin8(4,0,255),255,255), speed);
    clr2 = blend(CHSV(beatsin8(4,0,255),255,255), CHSV(beatsin8(3,0,255),255,255), speed);
    loc1 = beatsin8(13,0,NUM_LEDS/2-1);

    fill_gradient_RGB(LEFT, 0, clr2, loc1, clr1);
    fill_gradient_RGB(LEFT, loc1, clr2, NUM_LEDS/2-1, clr1);

    speed = beatsin8(7,0,255);

    clr1 = blend(CHSV(beatsin8(4,0,255),255,255), CHSV(beatsin8(5,0,255),255,255), speed);
    clr2 = blend(CHSV(beatsin8(5,0,255),255,255), CHSV(beatsin8(4,0,255),255,255), speed);
    loc1 = beatsin8(11,0,NUM_LEDS/2-1);

    fill_gradient_RGB(RIGHT, 0, clr2, loc1, clr1);
    fill_gradient_RGB(RIGHT, loc1, clr2, NUM_LEDS/2-1, clr1);
}

uint8_t _xhue[NUM_LEDS/2], _yhue[NUM_LEDS/2];
uint8_t _xsat[NUM_LEDS/2], _ysat[NUM_LEDS/2];
void setupNoise(){
    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {
        uint8_t angle = (i * 256) / NUM_LEDS/2;
        _xhue[i] = cos8( angle );
        _yhue[i] = sin8( angle );
        _xsat[i] = _yhue[i];
        _ysat[i] = _xhue[i];
    }
}

int scale = 1000;
void noise1() {
    uint8_t _noise, _hue, _sat, _val;
    uint16_t shift_x, shift_y;
    uint32_t real_x, real_y;

    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        shift_x = beatsin8(3);
        shift_y = millis() / 100;

        real_x = (_xhue[i] + shift_x) * scale;
        real_y = (_yhue[i] + shift_y) * scale;

        _noise = inoise16(real_x, real_y, 4223) >> 8;

        _hue = _noise * 3;
        _sat = 255;
        _val = _noise;

        LEFT[i] = CHSV( _hue, _sat, _val);

        shift_x = beatsin8(4);
        shift_y = millis() / 100;

        real_x = (_xhue[i] + shift_x) * scale;
        real_y = (_yhue[i] + shift_y) * scale;

        _noise = inoise16(real_x, real_y, 4223) >> 8;

        _hue = _noise * 3;
        _sat = 255;
        _val = _noise;

        RIGHT[i] = CHSV( _hue, _sat, _val);
    }
}


void noise2() {

    uint8_t _noise, _hue, _sat, _val;
    uint16_t shift_x, shift_y;
    uint32_t real_x, real_y;

    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        shift_x = millis() / 47;
        shift_y = 0;

        real_x = (_xhue[i] + shift_x) * scale;
        real_y = (_yhue[i] + shift_y) * scale;

        _noise = inoise16(real_x, real_y, 4223) >> 8;

        _hue = _noise * 3;
        _sat = 255;
        _val = _noise;

        LEFT[i] = CHSV( _hue, _sat, _val);

        shift_x = millis() / 51;
        shift_y = 0;

        real_x = (_xhue[i] + shift_x) * scale;
        real_y = (_yhue[i] + shift_y) * scale;

        _noise = inoise16(real_x, real_y, 4223) >> 8;

        _hue = _noise * 3;
        _sat = 255;
        _val = _noise;

        RIGHT[i] = CHSV( _hue, _sat, _val);
    }
}


void noise3() {

    uint8_t _noise, _hue, _sat, _val;
    uint16_t shift_x, shift_y;
    uint32_t real_x, real_y, real_z;

    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        shift_x = 0;
        shift_y = 0;

        real_x = (_xhue[i] + shift_x) * scale;
        real_y = (_yhue[i] + shift_y) * scale;

        real_z = millis() * 19;

        _noise = inoise16(real_x, real_y, real_z) >> 8;

        _hue = _noise * 3;
        _sat = 255;
        _val = _noise;

        LEFT[i] = CHSV( _hue, _sat, _val);

        shift_x = 0;
        shift_y = 0;

        real_x = (_xhue[i] + shift_x) * scale;
        real_y = (_yhue[i] + shift_y) * scale;

        real_z = millis() * 23;

        _noise = inoise16(real_x, real_y, real_z) >> 8;

        _hue = _noise * 3;
        _sat = 255;
        _val = _noise;

        RIGHT[i] = CHSV( _hue, _sat, _val);
    }
}

uint8_t fadeval = 235, frameRate = 45;
void fire(){
    EVERY_N_MILLISECONDS(1000/frameRate){
        for(int i = 0; i < NUM_LEDS/4; i++){
            R1[i] = R1[i+1].nscale8(fadeval); if(R1[i].g > 0) R1[i].g--;
            R2[NUM_LEDS/4-i] = R1[i];
            L1[i] = R1[i+1].nscale8(fadeval); if(L1[i].g > 0) L1[i].g--;
            L2[NUM_LEDS/4-i] = L1[i];
        }
        uint8_t _hue = 0, _sat = 255, _val = 0;
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        R1[NUM_LEDS/4-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        R2[0] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        L1[NUM_LEDS/4-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        L2[0] = CHSV( _hue, _sat, _val*_val/255);
    }
}

void fireSparks(){
    EVERY_N_MILLISECONDS(1000/frameRate){
        for(int i = 0; i < NUM_LEDS/4; i++){
            R1[i] = R1[i+1].nscale8(fadeval); if(R1[i].g > 0) R1[i].g--;
            R2[NUM_LEDS/4-i] = R1[i];
            L1[i] = R1[i+1].nscale8(fadeval); if(L1[i].g > 0) L1[i].g--;
            L2[NUM_LEDS/4-i] = L1[i];
        }
        uint8_t _hue = 0, _sat = 255, _val = 0;
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        R1[NUM_LEDS/4-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        R2[0] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        L1[NUM_LEDS/4-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        L2[0] = CHSV( _hue, _sat, _val*_val/255);
        EVERY_N_MILLISECONDS(1000/10){
            CRGB spark = CRGB::Yellow;
            if( random8() < 80)
                R1[NUM_LEDS/4-1-random8(NUM_LEDS/8)] = spark;
            if( random8() < 80)
                R2[random8(NUM_LEDS/8)] = spark;
            if( random8() < 80)
                L1[NUM_LEDS/4-1-random8(NUM_LEDS/8)] = spark;
            if( random8() < 80)
                L2[random8(NUM_LEDS/8)] = spark;
        }
    }
}

void fireRainbow(){
    EVERY_N_MILLISECONDS(1000/frameRate){
        for(int i = 0; i < NUM_LEDS/4; i++){
            R1[i] = R1[i+1].nscale8(fadeval);
            R2[NUM_LEDS/4-i] = R1[i];
            L1[i] = R1[i+1].nscale8(fadeval);
            L2[NUM_LEDS/4-i] = L1[i];
        }
        uint8_t _hue = 0, _sat = 255, _val = 0;
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        R1[NUM_LEDS/4-1] = CHSV( _hue+gHue1, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        R2[0] = CHSV( _hue+gHue1, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        L1[NUM_LEDS/4-1] = CHSV( _hue+gHue2, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 * 55;
        L2[0] = CHSV( _hue+gHue2, _sat, _val*_val/255);
    }
}

uint8_t blurval = 150;
void ripple_blur(){
    EVERY_N_MILLISECONDS(1000/30){
        blur1d( leds(0 , NUM_LEDS/2-1), NUM_LEDS/2, blurval);
        blur1d( leds(NUM_LEDS/2, NUM_LEDS ), NUM_LEDS/2, blurval);
    }
    EVERY_N_MILLISECONDS(30){
        if( random8() < 15) {
            uint8_t pos = random(NUM_LEDS/2);
            LEFT [pos] = CHSV(random(0, 64)+gHue1, random(250, 255), 255);
        }
        if( random8() < 15) {
            uint8_t pos = random(NUM_LEDS/2);
            RIGHT [pos] = CHSV(random(0, 64)-gHue2, random(250, 255), 255);
        }
    }
}

void drawClock(){
    if(timeStatus() == timeNotSet){
        timeLoop();
        if(WSdata.startsWith("prev"))
            previousPattern();
        else
            nextPattern();
    }else{
        nscale8( leds, NUM_LEDS, 200);
        int sec = millis()%(60*1000);
        double secPos = sec/60000.0 * NUM_LEDS/2;
        int min = getTime()%(60*60);
        double minPos = min/(60.0*60.0) * NUM_LEDS/2;
        int _hour = getTime()%(60*60*12);
        double hourPos = _hour/(60.0*60.0*12.0) * NUM_LEDS/2;
        int p = beatsin16(60, (NUM_LEDS*5)/3, (NUM_LEDS*5)/3*2);
        double pPos = p/(NUM_LEDS*5.0) * (NUM_LEDS/2-1);

        for(int i = 0; i < NUM_LEDS/2; i++){
            int _pos = (i+NUM_LEDS/4)%(NUM_LEDS/2);
            double a, b, c, y, w = 0.4;
            int bri = beatsin8(10, 100, 255);
            a = i+secPos; a = -w*a*a;
            b = i+secPos-NUM_LEDS/2.0; b = -w*b*b;
            c = i+secPos+NUM_LEDS/2.0; c = -w*c*c;
            y = pow(2, a)+pow(2, b)+pow(2, c);
            RIGHT[_pos] |= CHSV(160, 255, bri*y);
            a = i+minPos; a = -w*a*a;
            b = i+minPos-NUM_LEDS/2.0; b = -w*b*b;
            c = i+minPos+NUM_LEDS/2.0; c = -w*c*c;
            y = pow(2, a)+pow(2, b)+pow(2, c);
            RIGHT[_pos] |= CHSV( 96, 255, bri*y);
            a = i+hourPos; a = -w*a*a;
            b = i+hourPos-NUM_LEDS/2.0; b = -w*b*b;
            c = i+hourPos+NUM_LEDS/2.0; c = -w*c*c;
            y = pow(2, a)+pow(2, b)+pow(2, c);
            RIGHT[_pos] |= CHSV( 0, 255, bri*y);
            a = i-pPos; a = -2*a*a;
            y = pow(2, a);
            LEFT [i] |= CRGB(beatsin88(0.5*256, 10, 255)*y, beatsin88(0.7*256, 10, 255)*y, beatsin88(1.1*256, 10, 255)*y);
        }
    }
}



double temp = 5.0;
double width = 1.5;
double _smear = 4.0*sqrt(1.0/(width*width*width));
void interpolationTest(){
    EVERY_N_MILLISECONDS( 55 ) { gHue1++; }
    EVERY_N_MILLISECONDS( 57 ) { gHue2--; }
    nscale8( leds, NUM_LEDS, 20);

    int pos = beatsin16(1, 0, NUM_LEDS*temp);
    double a, scaledpos = pos/(NUM_LEDS*temp) * NUM_LEDS/2;
    for(int i = 0; i < NUM_LEDS/2; i++){
        a = i-scaledpos;
        a = -_smear*a*a;
        int val = 255.0*pow(2, a);
        RIGHT[i] |= CHSV(gHue1, 255, val);
        LEFT [i] |= CHSV(gHue2, 255, val);
    }
}
# 1 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/MIDI.ino"
# 14 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/MIDI.ino"
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, hostName, DEFAULT_CONTROL_PORT);

USING_NAMESPACE_APPLEMIDI

void MIDIsetup(){
    MIDI.begin(1);
    AppleMIDI.setHandleConnected(OnAppleMidiConnected);
    AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);
    AppleMIDI.setHandleError(OnAppleMidiError);

    MIDI.setHandleNoteOn(handleNoteOn);



    MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());
    MDNS.addService("http", "tcp", 80);
    IPAddress remote(192, 168, 1, 4);
    AppleMIDI.sendInvite(remote);
}

void MIDIloop(){
    MIDI.read();
}
# 45 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/MIDI.ino"
void OnAppleMidiConnected(const ssrc_t & ssrc, const char* name) {
    MIDIconnected = true;
    Serial.print(F("Connected to session "));
    Serial.println(name);
}




void OnAppleMidiDisconnected(const ssrc_t & ssrc) {
    MIDIconnected = false;
    Serial.println(F("Disconnected"));
}




void OnAppleMidiError(const ssrc_t& ssrc, int32_t err) {
    Serial.print (F("Exception "));
    Serial.print (err);
    Serial.print (F(" from ssrc 0x"));
    Serial.println(ssrc, HEX);

    switch (err){
        case Exception::NoResponseFromConnectionRequestException:
            Serial.println(F("xxx:yyy did't respond to the connection request. Check the address and port, and any firewall or router settings. (time)"));
        break;
        }
    }
# 99 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/MIDI.ino"
CRGB lastPressed;

void MIDI2LED(){
    EVERY_N_MILLISECONDS(50){ _hue++; gHue1++; gHue2--;}
    EVERY_N_MILLISECONDS(20){
        nscale8( leds, NUM_LEDS, 240);
    }




    uint8_t _pos = MIDIdata[1]/127.0 * (NUM_LEDS/2-1);
    uint8_t _col = MIDIdata[1]/127.0 * 224;

    RIGHT[_pos] = CHSV(_col + _hue, 255 - (MIDIdata[2]/2.0), MIDIdata[2]/127.0 * 255);
    LEFT [_pos] = RIGHT[_pos];
    if(MIDIdata[2] > 0 && millis()%2 == 0)
        MIDIdata[2]--;
    lastPressed = RIGHT[_pos];
    yield();
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
    MIDIdata[0] = channel;
    MIDIdata[1] = pitch;
    MIDIdata[2] = velocity;
    MidiEventReceived = true;
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
    MIDIdata[0] = channel;
    MIDIdata[1] = pitch;
    MIDIdata[2] = velocity;
    MidiEventReceived = true;
}

void handlePitchBend(byte channel, int bend) {

    fill_solid(leds, NUM_LEDS, CHSV(map(bend, -8192, 8192, 0, 224), 255, 125));
    yield();
}

void handleControlChange(byte channel, byte number, byte value){

    if( number == 1 ){
        fill_solid( leds, NUM_LEDS, 0x222222 );
    }

    if( number == 64 ){
        if( value >= 64 ){
            fill_solid( leds, NUM_LEDS, lastPressed );
            sustain = true;
        } else {
            sustain = false;
        }
    }
    yield();
}
# 1 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/dualCore.ino"
# 14 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/dualCore.ino"
TaskHandle_t Task0;


void core0_Task0( void * parameter )
{
  for (;;) {
    if(FFTenable){
        fftLoop();
        delay(1);
    }else{
        delay(100);
    }
  }
}

void dualCoreInit(){
#ifdef debug
    _serial_.println("\tStarting dualCoreInit");
#endif


    xTaskCreatePinnedToCore(
        core0_Task0,
        "core0Task0",
        1000,
        NULL,
        10,
        &Task0,
        0
    );
    delay(500);

#ifdef debug
    _serial_.println("\tEnding dualCoreInit");
#endif
}
# 1 "C:/Users/Anand/Documents/Git/platformio/ESP32/AsyncServer/src/wifi.ino"



AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
    if(type == WS_EVT_CONNECT){
        _serial_.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    } else if(type == WS_EVT_DISCONNECT){
        _serial_.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    } else if(type == WS_EVT_ERROR){
        _serial_.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    } else if(type == WS_EVT_PONG){
        _serial_.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
    } else if(type == WS_EVT_DATA){
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        WSdata = "";
        if(info->final && info->index == 0 && info->len == len){

            _serial_.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

            if(info->opcode == WS_TEXT){
                for(size_t i=0; i < info->len; i++) {
                    WSdata += (char) data[i];
                }
            } else {
                char buff[3];
                for(size_t i=0; i < info->len; i++) {
                    sprintf(buff, "%02x ", (uint8_t) data[i]);
                    WSdata += buff ;
                }
            }
            _serial_.printf("%s\n",WSdata.c_str());

            if(info->opcode == WS_TEXT){
                client->text("I got your text message");
                handleSliders();
            }
            else
                client->binary("I got your binary message");

        } else {

            if(info->index == 0){
                if(info->num == 0)
                    _serial_.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                    _serial_.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            }

            _serial_.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

            if(info->opcode == WS_TEXT){
                for(size_t i=0; i < len; i++) {
                    WSdata += (char) data[i];
                }
            } else {
                char buff[3];
                for(size_t i=0; i < len; i++) {
                    sprintf(buff, "%02x ", (uint8_t) data[i]);
                    WSdata += buff ;
                }
            }
            _serial_.printf("%s\n",WSdata.c_str());

            if((info->index + len) == info->len){
                _serial_.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                if(info->final){
                    _serial_.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                if(info->message_opcode == WS_TEXT)
                    client->text("I got your text message");
                else
                    client->binary("I got your binary message");
                }
            }
        }
    }
}



void wifiSetup(){
#ifdef debug
    _serial_.println("\tStarting wifiSetup");
#endif
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    setupOTA();

    MDNS.addService("http","tcp",80);

    SPIFFS.begin();

    setupServer();

    timeSetup();
#ifdef debug
    _serial_.println("\tEnding wifiSetup");
#endif
}

void wifiLoop(){
    if(WiFi.status() == WL_CONNECTED){
        ArduinoOTA.handle();
        EVERY_N_MILLISECONDS(500){
            ws.cleanupClients();
        }
        EVERY_N_MILLISECONDS(2000){
            _serial_.println(WiFi.localIP());
        }
        if(!digitalRead(2)){
            digitalWrite(2, HIGH);
            _serial_.println("Wifi connected!");
        }
    }

    if(WiFi.status() != WL_CONNECTED){
        EVERY_N_SECONDS(1){
            _serial_.println("Wifi disconnected");
            WiFi.begin(ssid, password);
        }
        if(digitalRead(2))
            digitalWrite(2, LOW);
    }
    yield();
}

void setupOTA(){
#ifdef debug
    _serial_.println("\t\tStarting setupOTA");
#endif
    ArduinoOTA.setPort(3232);

    ArduinoOTA.setHostname(hostName);

    ArduinoOTA
        .onStart([]() {
            digitalWrite(2, HIGH);
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else{
                type = "filesystem";
                SPIFFS.end();
            }

            events.send("Update Start", "ota");
            _serial_.println("\r\nStart updating " + type);

            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        })
        .onEnd([]() {
            digitalWrite(2, LOW);
            _serial_.println("\r\nEnd");
            events.send("Update End", "ota");
            delay(10);
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            digitalWrite(2, !digitalRead(2));
            char p[32];
            sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
            events.send(p, "ota");
            uint32_t temp = progress / (total / 100);
            _serial_.printf("Progress: %u%%\r", temp);
            if(temp<99){
                int t = map(temp, 0, 99, 0, NUM_LEDS/4);
                fill_solid( RIGHT( 0, NUM_LEDS/4), t, 0x020202);
                fill_solid( RIGHT(NUM_LEDS/2-t, NUM_LEDS/2), t, 0x020202);
                fill_solid( LEFT ( 0, NUM_LEDS/4), t, 0x020202);
                fill_solid( LEFT (NUM_LEDS/2-t, NUM_LEDS/2), t, 0x020202);
            }else if(temp >= 99){
                fill_solid (leds, NUM_LEDS, 0x020202);
            }
            FastLED.show();
        })
        .onError([](ota_error_t error) {
            fill_solid (leds, NUM_LEDS, CRGB::Red);
            FastLED.show();
            _serial_.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) { _serial_.println("Auth Failed"); events.send("Auth Failed", "ota"); }
            else if (error == OTA_BEGIN_ERROR) { _serial_.println("Begin Failed"); events.send("Begin Failed", "ota"); }
            else if (error == OTA_CONNECT_ERROR){ _serial_.println("Connect Failed"); events.send("Connect Failed", "ota"); }
            else if (error == OTA_RECEIVE_ERROR){ _serial_.println("Receive Failed"); events.send("Recieve Failed", "ota"); }
            else if (error == OTA_END_ERROR) { _serial_.println("End Failed"); events.send("End Failed", "ota"); }
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        });
    ArduinoOTA.begin();
#ifdef debug
    _serial_.println("\t\tEnding setupOTA");
#endif
}

void setupServer(){
#ifdef debug
    _serial_.println("\t\tStarting setupServer");
#endif
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    events.onConnect([](AsyncEventSourceClient *client){
        client->send("hello!",NULL,millis(),1000);
    });
    server.addHandler(&events);

    server.addHandler(new SPIFFSEditor(SPIFFS, http_username,http_password));

    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });

    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    server.onNotFound([](AsyncWebServerRequest *request){
            _serial_.printf("NOT_FOUND: ");
        if(request->method() == HTTP_GET)
            _serial_.printf("GET");
        else if(request->method() == HTTP_POST)
            _serial_.printf("POST");
        else if(request->method() == HTTP_DELETE)
            _serial_.printf("DELETE");
        else if(request->method() == HTTP_PUT)
            _serial_.printf("PUT");
        else if(request->method() == HTTP_PATCH)
            _serial_.printf("PATCH");
        else if(request->method() == HTTP_HEAD)
            _serial_.printf("HEAD");
        else if(request->method() == HTTP_OPTIONS)
            _serial_.printf("OPTIONS");
        else
            _serial_.printf("UNKNOWN");
            _serial_.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

        if(request->contentLength()){
            _serial_.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
            _serial_.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
        }

        int headers = request->headers();
        int i;
        for(i=0;i<headers;i++){
            AsyncWebHeader* h = request->getHeader(i);
            _serial_.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
        }

        int params = request->params();
        for(i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isFile()){
                _serial_.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if(p->isPost()){
                _serial_.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            } else {
                _serial_.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }

        request->send(404);
    });
        server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
        if(!index)
        _serial_.printf("UploadStart: %s\n", filename.c_str());
        _serial_.printf("%s", (const char*)data);
        if(final)
        _serial_.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
    });
    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        if(!index)
        _serial_.printf("BodyStart: %u\n", total);
        _serial_.printf("%s", (const char*)data);
        if(index + len == total)
        _serial_.printf("BodyEnd: %u\n", total);
    });
    server.begin();
#ifdef debug
    _serial_.println("\t\tEnding setupServer");
#endif
}

void handleSliders(){
    if(WSdata.startsWith("reset")){
        WiFi.disconnect();
        digitalWrite(2, LOW);
        ESP.restart();
    }
    if(WSdata.startsWith("next")){
        nextPattern();
    }
    if(WSdata.startsWith("prev")){
        previousPattern();
    }
    String temp = WSdata.substring(1, WSdata.length()-1);
    if(WSdata.startsWith("M")){
        music = temp.endsWith("0") ? true : false;
        _auto = temp.endsWith("1") ? true : false;
        manual = temp.endsWith("2") ? true : false;
        gCurrentPatternNumber = 0;
        if(_auto)
            FastLED.setBrightness(30);
        else
            _setBrightness = 255;
    }if(WSdata.startsWith("V")){
        int x = temp.toInt();
        x = (x*x)/255.0;
        _setBrightness = x;
    }
    if(manual){
         if(WSdata.startsWith("R")){
            int x = temp.toInt();
            x = (x*x)/255.0;
            manualColor.r = x;
        }else if(WSdata.startsWith("G")){
            int x = temp.toInt();
            x = (x*x)/255.0;
            manualColor.g = x;
        }else if(WSdata.startsWith("B")){
            int x = temp.toInt();
            x = (x*x)/255.0;
            manualColor.b = x;
        }else if(WSdata.startsWith("H")){
            gHue1 = temp.toInt();
            manualHSV.h = gHue1;
            manualColor = manualHSV;
        }else if(WSdata.startsWith("S")){
            manualHSV.s = temp.toInt();
            manualColor = manualHSV;
        }
        if(WSdata.endsWith("L")){
            manualColor_L = manualColor;
        }else if(WSdata.endsWith("R")){
            manualColor_R = manualColor;
        }else if(WSdata.endsWith("B")){
            manualColor_L = manualColor;
            manualColor_R = manualColor;
        }
    }
}



void timeSetup(){
    Udp.begin(localPort);
}

void timeLoop(){
 if(timeStatus() != timeSet){
        setSyncProvider(getNtpTime);
        setSyncInterval(5000);
    }
}



const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

time_t getNtpTime()
{
    IPAddress ntpServerIP;

    while (Udp.parsePacket() > 0) ;
    _serial_.println("Transmit NTP Request");

    WiFi.hostByName(ntpServerName, ntpServerIP);
    _serial_.print(ntpServerName);
    _serial_.print(": ");
    _serial_.println(ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
            _serial_.println("Receive NTP Response");
            Udp.read(packetBuffer, NTP_PACKET_SIZE);
            unsigned long secsSince1900;

            secsSince1900 = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
        }
    }
    _serial_.println("No NTP Response :-(");
    return 0;
}


void sendNTPpacket(IPAddress &address)
{

    memset(packetBuffer, 0, NTP_PACKET_SIZE);


    packetBuffer[0] = 0b11100011;
    packetBuffer[1] = 0;
    packetBuffer[2] = 6;
    packetBuffer[3] = 0xEC;

    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;


    Udp.beginPacket(address, 123);
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}