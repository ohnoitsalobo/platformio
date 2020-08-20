#define FASTLED_INTERNAL
#include <FastLED.h>
FASTLED_USING_NAMESPACE

#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define LED_PINS    13
#define NUM_LEDS    NUMBER_OF_LEDS
#define BRIGHTNESS  255*225/255

CRGBArray<NUM_LEDS> leds;                              // LED array containing all LEDs
CRGBSet RIGHT (leds (0,            NUM_LEDS/2-1)   );  // < subset containing only left  LEDs
CRGBSet R1    (leds (0,            NUM_LEDS/4-1)   );  // < subset containing only left  side of left  LEDs
CRGBSet R2    (leds (NUM_LEDS/4,   NUM_LEDS/2-1)   );  // < subset containing only right side of left  LEDs
CRGBSet LEFT  (leds (NUM_LEDS/2,   NUM_LEDS)       );  // < subset containing only right LEDs
CRGBSet L1    (leds (NUM_LEDS/2,   3*NUM_LEDS/4-1) );  // < subset containing only left  side of right LEDs
CRGBSet L2    (leds (3*NUM_LEDS/4, NUM_LEDS)       );  // < subset containing only right side of right LEDs

CRGBPalette16 currentPalette, randomPalette1;
CRGBPalette16 targetPalette, randomPalette2;
TBlendType    currentBlending;
uint8_t maxChanges = 24;        // Value for blending between palettes.

bool manual = 0, _auto = 0;
uint8_t currentBrightness = BRIGHTNESS, _setBrightness = BRIGHTNESS;
CRGB manualColor = 0x000000, manualColor_L = 0x000000, manualColor_R = 0x000000;
CHSV manualHSV (0, 255, 255);
uint8_t gHue = 0, gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns
#include "Pacifica.h"

typedef void (*SimplePatternList[])();
SimplePatternList autoPatterns = { rainbow, rainbowWithGlitter, rainbow_scaling, fire, fireSparks, fireRainbow, noise1, noise2, noise3, pacifica_loop, blendwave, confetti, ripple_blur, sinelon, dot_beat, juggle };
SimplePatternList audioPatterns = { audio_spectrum, audioLight };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

String eqBroadcast = "";
uint8_t eq[2][samples/2-2];

void ledSetup(){
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(currentBrightness);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
    
    for(int i = 0; i < samples/2-2; i++){
        eq[0][i] = 0;
        eq[1][i] = 0;
    }
    
    setupNoise();
}

void ledLoop(){
#ifdef debug
    _serial_.println("Starting ledLoop");
#endif
    if(MIDIconnected()){
        runLED();
    }else{
        if(music){
            audioPatterns[gCurrentPatternNumber]();
            FastLED.show();
            EVERY_N_MILLISECONDS(20){
                // if(webSocketConn()){
                    // eqBroadcast = "E";
                    // for(uint8_t i = 0; i < samples/2-2; i++){
                        // eqBroadcast += ",";
                        // eqBroadcast += String(eq[0][i]);
                        // if(eq[0][i] != 0) eq[0][i] /= 5.0;
                    // }
                    // for(uint8_t i = 0; i < samples/2-2; i++){
                        // eqBroadcast += ",";
                        // eqBroadcast += String(eq[1][i]);
                        // if(eq[1][i] != 0) eq[1][i] /= 5.0;
                    // }
                    // wsBroadcast();
                    // eqBroadcast = "";
                // }
            }
        }
        else if(_auto){
            EVERY_N_MILLISECONDS( 35 ) { gHue1++; }
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
    _serial_.println("Ending ledLoop");
#endif
}

void audio_spectrum(){ // 
#ifdef debug
    _serial_.println("Starting audio_spectrum");
#endif
    fftLoop();
    uint8_t fadeval = 90;
    nscale8(leds, NUM_LEDS, fadeval); // smaller = faster fade
    CRGB tempRGB1, tempRGB2;
    uint8_t pos = 0, h = 0, s = 0, v = 0;
    double temp1 = 0, temp2 = 0;
    for(int i = 2; i < samples/2; i++){
        pos = spectrum[0][i];
        h = pos/(NUM_LEDS/4.0)*224;
        temp1 = spectrum[1][i]/MAX;
        s = 255 - (temp1*30.0);
        v = temp1*255.0;
        tempRGB1 = CHSV(h, s, v);
        uint8_t p = NUM_LEDS/4-pos;
        if(tempRGB1 > R1[pos]){
            R1[pos] = tempRGB1;
        }else{
            // R1[pos].nscale8(fadeval);
        }
        R2[p] = R1[pos];
        eq[0][i-2] = v;

        temp2 = spectrum[2][i]/MAX;
        s = 255 - (temp2*30.0);
        v = temp2*255.0;
        tempRGB2 = CHSV(h, s, v);
        if(tempRGB2 > L1[pos]){
            L1[pos] = tempRGB2;
        }else{
            // L1[pos].nscale8(fadeval);
        }
        L2[p] = L1[pos];
        eq[1][i-2] = v;
        yield();
    }
#ifdef debug
    _serial_.println("Ending audio_spectrum");
#endif
}

void audioLight(){
#ifdef debug
    _serial_.println("Starting audioLight");
#endif
    EVERY_N_MILLISECONDS( 55 ) { gHue1++; }
    EVERY_N_MILLISECONDS( 57 ) { gHue2--; }
    EVERY_N_MILLISECONDS( 15 ) {
        uint8_t fadeval = 200;
        for(int i = 0; i < NUM_LEDS/4; i++){
            R1[NUM_LEDS/4-i] = R1[NUM_LEDS/4-1-i].nscale8(fadeval);
            R2[i] = R1[NUM_LEDS/4-i];
            L1[NUM_LEDS/4-i] = L1[NUM_LEDS/4-1-i].nscale8(fadeval);
            L2[i] = L1[NUM_LEDS/4-i];
        }
        uint16_t mid = 1800, _noise = 180;
        uint8_t _hue = 0, _sat = 255, _val = 0;
        int temp1 = abs(mid - analogRead( RightPin));
        if(temp1 > _noise){
            _val = (temp1-_noise)/float(mid) * 255;
            // _hue = _val/255.0 * 224;
            _hue = _val/255.0 * 65;
        }
        R1[0] = CHSV( _hue+gHue1, _sat, _val);
        R2[NUM_LEDS/4] = R1[0];
        
        _hue = 0; _val = 0;
        int temp2 = abs(mid - analogRead( LeftPin));
        if(temp2 > _noise){
            _val = (temp2-_noise)/float(mid) * 255;
            // _hue = _val/255.0 * 224;
            _hue = _val/255.0 * 65;
        }
        L2[NUM_LEDS/4] = CHSV( _hue+gHue2, _sat, _val);
        L1[0] = L2[NUM_LEDS/4];
    }
#ifdef debug
    _serial_.println("Ending audioLight");
#endif
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern(){
    uint8_t temp = 0;
    if(music)
        temp = ARRAY_SIZE( audioPatterns );
    else
        temp = ARRAY_SIZE( autoPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % temp;
} // advance to the next pattern

void previousPattern(){
    uint8_t temp = 0;
    if(music)
        temp = ARRAY_SIZE( audioPatterns );
    else
        temp = ARRAY_SIZE( autoPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + (temp-1)) % temp;
} // advance to the next pattern

void rainbow() {
    // FastLED's built-in rainbow generator
    fill_rainbow( LEFT , NUM_LEDS/2, gHue2);
    fill_rainbow( RIGHT, NUM_LEDS/2, gHue1);
} // rainbow

void rainbowWithGlitter() {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter();
} // rainbow with glitter

void rainbow_scaling(){
    for(int i = 0; i <= NUM_LEDS/4; i++){
        R1[i] = CHSV((millis()/77*i+1)%255 + gHue1, 255, 255);
        R2[NUM_LEDS/4-i] = R1[i];
        L1[i] = CHSV((millis()/73*i+1)%255 - gHue2, 255, 255);
        L2[NUM_LEDS/4-i] = L1[i];
    }
} // rainbow scaling

void addGlitter() {
    EVERY_N_MILLISECONDS(1000/30){
        if( random8() < 80) {
            leds[ random16(NUM_LEDS) ] += CRGB::White;
        }
    }
}

void confetti() 
{    // random colored speckles that blink in and fade smoothly
    EVERY_N_MILLISECONDS(1000/30){
        fadeToBlackBy( leds, NUM_LEDS, 30);
        int pos = random16(NUM_LEDS/2);
        // leds[pos] += CHSV( random8(255), 255, 255);
        RIGHT[pos] += CHSV( gHue1 + random8(64), 190+random8(65), 255);
        LEFT [pos] += CHSV( gHue2 + random8(64), 190+random8(65), 255);
    }
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 5);
    int pos1 = beatsin16(11, 0, NUM_LEDS/2-1);
    int pos2 = beatsin16(13, 0, NUM_LEDS/2-1);
    int pos3 = beatsin16( 9, 0, NUM_LEDS/2-1);
    int pos4 = beatsin16(15, 0, NUM_LEDS/2-1);
    LEFT [pos1] = ColorFromPalette(randomPalette1, pos1, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.
    RIGHT[pos2] = ColorFromPalette(randomPalette2, pos2, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.
    LEFT [pos3] += CHSV( gHue2, 255, 255);
    RIGHT[pos4] += CHSV( gHue1, 255, 255);
}

void dot_beat() {
    uint8_t fadeval = 10;                                        // Trail behind the LED's. Lower => faster fade.
    // nscale8(leds, NUM_LEDS, fadeval);                             // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);
    fadeToBlackBy( leds, NUM_LEDS, fadeval);

    uint8_t BPM, inner, outer, middle;
    
    BPM = 33;

    inner  = beatsin8(BPM, NUM_LEDS/2/4, NUM_LEDS/2/4*3);    // Move 1/4 to 3/4
    outer  = beatsin8(BPM, 0, NUM_LEDS/2-1);               // Move entire length
    middle = beatsin8(BPM, NUM_LEDS/2/3, NUM_LEDS/2/3*2);   // Move 1/3 to 2/3

    LEFT[outer]  = CHSV( gHue1    , 200, 255); // CRGB::Aqua;
    LEFT[middle] = CHSV( gHue1+96 , 200, 255); // CRGB::Purple;
    LEFT[inner]  = CHSV( gHue1+160, 200, 255); // CRGB::Blue;

    BPM = 31;
    
    inner  = beatsin8(BPM, NUM_LEDS/2/4, NUM_LEDS/2/4*3);    // Move 1/4 to 3/4
    outer  = beatsin8(BPM, 0, NUM_LEDS/2-1);               // Move entire length
    middle = beatsin8(BPM, NUM_LEDS/2/3, NUM_LEDS/2/3*2);   // Move 1/3 to 2/3

    RIGHT[outer]  = CHSV( gHue2    , 200, 255); // CRGB::Aqua;
    RIGHT[middle] = CHSV( gHue2+96 , 200, 255); // CRGB::Purple;
    RIGHT[inner]  = CHSV( gHue2+160, 200, 255); // CRGB::Blue;

} // dot_beat()

void juggle() {
    // eight colored dots, weaving in and out of sync with each other
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
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    // CRGBPalette16 palette = PartyColors_p;
    CRGBPalette16 palette = RainbowColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS/2; i++) { //9948
        RIGHT[i]              = ColorFromPalette(palette, gHue1+(i*2), beat-gHue1+(i*10));
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
} // blendwave()

uint8_t _xhue[NUM_LEDS/2], _yhue[NUM_LEDS/2]; // x/y coordinates for noise function
uint8_t _xsat[NUM_LEDS/2], _ysat[NUM_LEDS/2]; // x/y coordinates for noise function
void setupNoise(){
    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {       // precalculate the lookup-tables:
        uint8_t angle = (i * 256) / NUM_LEDS/2;         // on which position on the circle is the led?
        _xhue[i] = cos8( angle );                         // corrsponding x position in the matrix
        _yhue[i] = sin8( angle );                         // corrsponding y position in the matrix
        _xsat[i] = _yhue[i];                         // corrsponding x position in the matrix
        _ysat[i] = _xhue[i];                         // corrsponding y position in the matrix
    }
}

int scale = 1000;                               // the "zoom factor" for the noise
void noise1() {
    uint8_t _noise, _hue, _sat, _val;
    uint16_t shift_x, shift_y;
    uint32_t real_x, real_y;

    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        shift_x = beatsin8(3);                  // the x position of the noise field swings @ 17 bpm
        shift_y = millis() / 100;                // the y position becomes slowly incremented

        real_x = (_xhue[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        real_y = (_yhue[i] + shift_y) * scale;       // based on the precalculated positions

        _noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down
        
        _hue = _noise * 3;                        // map led color based on noise data
        _sat = 255;
        _val = _noise;

        LEFT[i] = CHSV( _hue, _sat, _val);
        
        shift_x = beatsin8(4);                  // the x position of the noise field swings @ 17 bpm
        shift_y = millis() / 100;                // the y position becomes slowly incremented

        real_x = (_xhue[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        real_y = (_yhue[i] + shift_y) * scale;       // based on the precalculated positions

        _noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down
        
        _hue = _noise * 3;                        // map led color based on noise data
        _sat = 255;
        _val = _noise;

        RIGHT[i] = CHSV( _hue, _sat, _val);
    }
}

// just moving along one axis = "lavalamp effect"
void noise2() {

    uint8_t _noise, _hue, _sat, _val;
    uint16_t shift_x, shift_y;
    uint32_t real_x, real_y;

    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        shift_x = millis() / 47;                 // x as a function of time
        shift_y = 0;

        real_x = (_xhue[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        real_y = (_yhue[i] + shift_y) * scale;       // based on the precalculated positions

        _noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down
        
        _hue = _noise * 3;                        // map led color based on noise data
        _sat = 255;
        _val = _noise;

        LEFT[i] = CHSV( _hue, _sat, _val);

        shift_x = millis() / 51;                 // x as a function of time
        shift_y = 0;

        real_x = (_xhue[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        real_y = (_yhue[i] + shift_y) * scale;       // based on the precalculated positions

        _noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down

        _hue = _noise * 3;                        // map led color based on noise data
        _sat = 255;
        _val = _noise;

        RIGHT[i] = CHSV( _hue, _sat, _val);
    }
}

// no x/y shifting but scrolling along z
void noise3() {

    uint8_t _noise, _hue, _sat, _val;
    uint16_t shift_x, shift_y;
    uint32_t real_x, real_y, real_z;

    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        shift_x = 0;                             // no movement along x and y
        shift_y = 0;

        real_x = (_xhue[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        real_y = (_yhue[i] + shift_y) * scale;       // based on the precalculated positions

        real_z = millis() * 19;                  // increment z linear

        _noise = inoise16(real_x, real_y, real_z) >> 8;           // get the noise data and scale it down

        _hue = _noise * 3;                        // map led color based on noise data
        _sat = 255;
        _val = _noise;

        LEFT[i] = CHSV( _hue, _sat, _val);

        shift_x = 0;                             // no movement along x and y
        shift_y = 0;

        real_x = (_xhue[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        real_y = (_yhue[i] + shift_y) * scale;       // based on the precalculated positions

        real_z = millis() * 23;                  // increment z linear

        _noise = inoise16(real_x, real_y, real_z) >> 8;           // get the noise data and scale it down

        _hue = _noise * 3;                        // map led color based on noise data
        _sat = 255;
        _val = _noise;

        RIGHT[i] = CHSV( _hue, _sat, _val);
    }
}

uint8_t fadeval = 200, frameRate = 45;
void fire(){
    EVERY_N_MILLISECONDS(1000/frameRate){
        for(int i = 0; i < NUM_LEDS/4; i++){
            R1[NUM_LEDS/4-i] = R1[NUM_LEDS/4-1-i].nscale8(fadeval); if(R1[NUM_LEDS/4-i].g > 0) R1[NUM_LEDS/4-i].g--;
            R2[i] = R1[NUM_LEDS/4-i];
            L1[NUM_LEDS/4-i] = L1[NUM_LEDS/4-1-i].nscale8(fadeval); if(L1[NUM_LEDS/4-i].g > 0) L1[NUM_LEDS/4-i].g--;
            L2[i] = L1[NUM_LEDS/4-i];
        }
        uint8_t _hue = 0, _sat = 255, _val = 0;
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 45);
        _hue = _val/255.0 *_val/255.0 * 55;
        R1[0] = CHSV( _hue, _sat, _val*_val/255);
        R2[NUM_LEDS/4] = R1[0];
        
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 45);
        _hue = _val/255.0 *_val/255.0 * 55;
        L2[NUM_LEDS/4] = CHSV( _hue, _sat, _val*_val/255);
        L1[0] = L2[NUM_LEDS/4];
        
    }
}

void fireSparks(){
    EVERY_N_MILLISECONDS(1000/frameRate){
        for(int i = 0; i < NUM_LEDS/4; i++){
            R1[NUM_LEDS/4-i] = R1[NUM_LEDS/4-1-i].nscale8(fadeval); if(R1[NUM_LEDS/4-i].g > 0) R1[NUM_LEDS/4-i].g--;
            R2[i] = R1[NUM_LEDS/4-i];
            L1[NUM_LEDS/4-i] = L1[NUM_LEDS/4-1-i].nscale8(fadeval); if(L1[NUM_LEDS/4-i].g > 0) L1[NUM_LEDS/4-i].g--;
            L2[i] = L1[NUM_LEDS/4-i];
        }
        uint8_t _hue = 0, _sat = 255, _val = 0;
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 45);
        _hue = _val/255.0 *_val/255.0 * 55;
        R1[0] = CHSV( _hue, _sat, _val*_val/255);
        R2[NUM_LEDS/4] = R1[0];
        
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 45);
        _hue = _val/255.0 *_val/255.0 * 55;
        L2[NUM_LEDS/4] = CHSV( _hue, _sat, _val*_val/255);
        L1[0] = L2[NUM_LEDS/4];
        EVERY_N_MILLISECONDS(1000/10){
            if( random8() < 80) {
                leds[ random16(NUM_LEDS) ] += CRGB::Yellow;
            }
        }
    }
}

void fireRainbow(){
    EVERY_N_MILLISECONDS(1000/frameRate){
        for(int i = 0; i < NUM_LEDS/4; i++){
            R1[NUM_LEDS/4-i] = R1[NUM_LEDS/4-1-i].nscale8(fadeval);
            R2[i] = R1[NUM_LEDS/4-i];
            L1[NUM_LEDS/4-i] = L1[NUM_LEDS/4-1-i].nscale8(fadeval);
            L2[i] = L1[NUM_LEDS/4-i];
        }
        uint8_t _hue = 0, _sat = 255, _val = 0;
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 45);
        _hue = _val/255.0 *_val/255.0 * 55;
        R1[0] = CHSV( _hue+gHue1, _sat, _val*_val/255);
        R2[NUM_LEDS/4] = R1[0];
        
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 45);
        _hue = _val/255.0 *_val/255.0 * 55;
        L2[NUM_LEDS/4] = CHSV( _hue+gHue2, _sat, _val*_val/255);
        L1[0] = L2[NUM_LEDS/4];
    }
}

uint8_t blurval = 100;
void ripple_blur(){
    // nscale8( leds, NUM_LEDS, 250);
    EVERY_N_MILLISECONDS(15){
        blur1d( leds(0         , NUM_LEDS/2-1), NUM_LEDS/2, blurval);
        blur1d( leds(NUM_LEDS/2, NUM_LEDS    ), NUM_LEDS/2, blurval);
    }
    EVERY_N_MILLISECONDS(30){
        if( random8() < 15) {
            uint8_t pos = random(NUM_LEDS/2);
            LEFT [pos] = CHSV(random(0, 64)+gHue1, random(250, 255), 255);
        }
        if( random8() < 15) {
            uint8_t pos = random(NUM_LEDS/2);
            RIGHT [pos] = CHSV(random(0, 64)+gHue2, random(250, 255), 255);
        }
    }
}

//////// MIDI stuff

CRGB lastPressed;             // holder for last-detected key color

void runLED(){
    EVERY_N_MILLISECONDS(50){ _hue++; gHue1++; gHue2--;}
    EVERY_N_MILLISECONDS(20){ 
        // fadeToBlackBy( leds, NUM_LEDS, 10); // ( sustain ? 3 : 10) );
        nscale8( leds, NUM_LEDS, 240); // ( sustain ? 3 : 10) );
    }
    // if(MidiEventReceived)
    MIDI2LED();
    FastLED.show();
    yield();
}

void MIDI2LED(){
    // MIDI note values 0 - 127 
    // 36-96 (for 61-key) mapped to LED 0-60
    // Serial.println(pitch);
    // int temp = map(pitch, 36, 96, 0, NUM_LEDS-1);
    
    // if(temp < 0)
        // temp = -temp;                   // if note goes above 60 or below 0
    // else if(temp > NUM_LEDS)                  //      reverse it
        // temp = NUM_LEDS - (temp%NUM_LEDS);
    
    // uint8_t _pitch = map(temp, 0, NUM_LEDS, 0, 224); // map note to color 'hue'
    uint8_t _pos = MIDIdata[1]/127.0 * (NUM_LEDS/2-1); // map note to position
    uint8_t _col = MIDIdata[1]/127.0 * 224; // map note to position
    
    // uint8_t _pos = map(temp, 0, NUM_LEDS, 0, NUM_LEDS-1);
    // assign color based on note position and intensity (velocity)
    RIGHT[_pos] = CHSV(_col + _hue, 255 - (MIDIdata[2]/2.0), MIDIdata[2]/127.0 * 255);
    LEFT [_pos] = RIGHT[_pos];
    if(MIDIdata[2] > 0 && millis()%2 == 0)
        MIDIdata[2]--;
    lastPressed = RIGHT[_pos]; // remember last-detected note color
    // MidiEventReceived = false;
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
    // fill strip with solid color based on pitch bend amount
    fill_solid(leds, NUM_LEDS, CHSV(map(bend, -8192, 8192, 0, 224), 255, 125)); // 0  8192  16383
    yield();
}

void handleControlChange(byte channel, byte number, byte value){
    // channel 1 = modulation
    if( number == 1 ){
        fill_solid( leds, NUM_LEDS, 0x222222 );
        // fill_rainbow(leds, NUM_LEDS, hue);
    }
    // channel 64 = damper / sustain pedal
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
