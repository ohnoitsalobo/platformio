FASTLED_USING_NAMESPACE

#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define LED_PINS    13
#define BRIGHTNESS  100

CRGBArray<NUM_LEDS> leds;                              // LED array containing all LEDs
CRGBSet RIGHT (leds (0,            NUM_LEDS/2-1)   );  // < subset containing only left  LEDs
CRGBSet LEFT  (leds (NUM_LEDS/2,   NUM_LEDS)       );  // < subset containing only right LEDs
CRGBSet row1  (leds ( 0,  7)); CRGBSet R1  (leds ( 0,  3)); CRGBSet L1  (leds ( 4,  7)); 
CRGBSet row2  (leds ( 8, 15)); CRGBSet L2  (leds ( 8, 11)); CRGBSet R2  (leds (12, 15)); 
CRGBSet row3  (leds (16, 23)); CRGBSet R3  (leds (16, 19)); CRGBSet L3  (leds (20, 23)); 
CRGBSet row4  (leds (24, 31)); CRGBSet L4  (leds (24, 27)); CRGBSet R4  (leds (28, 31)); 
CRGBSet row5  (leds (32, 39)); CRGBSet R5  (leds (32, 35)); CRGBSet L5  (leds (36, 39)); 
CRGBSet row6  (leds (40, 47)); CRGBSet L6  (leds (40, 43)); CRGBSet R6  (leds (44, 47)); 
CRGBSet row7  (leds (48, 55)); CRGBSet R7  (leds (48, 51)); CRGBSet L7  (leds (52, 55)); 
CRGBSet row8  (leds (56, 63)); CRGBSet L8  (leds (56, 59)); CRGBSet R8  (leds (60, 63)); 
CRGBSet row9  (leds (64, 71)); CRGBSet R9  (leds (64, 67)); CRGBSet L9  (leds (68, 71)); 
struct CRGB * rightArray[] ={ R1, R2, R3, R4, R5, R6, R7, R8, R9 }; 
struct CRGB * leftArray [] ={ L1, L2, L3, L4, L5, L6, L7, L8, L9 }; 

CRGBPalette16 currentPalette, randomPalette1;
CRGBPalette16 targetPalette, randomPalette2;
TBlendType    currentBlending;
uint8_t maxChanges = 24;        // Value for blending between palettes.

uint8_t currentBrightness = BRIGHTNESS, _setBrightness = BRIGHTNESS;
CRGB manualColor = 0x000000, manualColor_L = 0x000000, manualColor_R = 0x000000;
CHSV manualHSV (0, 255, 255);
uint8_t gHue = 0, gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns
#include "pacifica.h"

typedef void (*SimplePatternList[])();
// SimplePatternList autoPatterns = { cylon, rainbow, rainbowWithGlitter, rainbow_scaling, fire, fireSparks, fireRainbow, noise1, noise2, noise3, pacifica_loop, blendwave, confetti, ripple_blur, sinelon, dot_beat, juggle };
SimplePatternList autoPatterns = { confetti, rainbow_scaling, matrixTest, noise1};
SimplePatternList audioPatterns = { audio_spectrum, audioLight };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

cLEDMatrix<-MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> ledMatrix;

cLEDText ScrollingMsg;

/* const unsigned char TxtDemo[] = { EFFECT_SCROLL_LEFT "         LEFT SCROLL "
                                  EFFECT_SCROLL_RIGHT "         LLORCS THGIR "
                                  EFFECT_SCROLL_DOWN "         SCROLL DOWN         SCROLL DOWN         "
                                  // EFFECT_FRAME_RATE "\x04" " SCROLL DOWN         "
                                  // EFFECT_FRAME_RATE "\x00" " "
                                  EFFECT_SCROLL_UP "         SCROLL UP         SCROLL UP         "
                                  // EFFECT_FRAME_RATE "\x04" "  SCROLL UP         "
                                  // EFFECT_FRAME_RATE "\x00" " "
                                  EFFECT_CHAR_UP EFFECT_SCROLL_LEFT "         UP "
                                  EFFECT_CHAR_RIGHT "  RIGHT "
                                  EFFECT_CHAR_DOWN "  DOWN "
                                  EFFECT_CHAR_LEFT "  LEFT "
                                  EFFECT_HSV_CV "\x00\xff\xff\x40\xff\xff"
                                  EFFECT_CHAR_UP "           HSV_CV 00-40"
                                  EFFECT_HSV_CH "\x00\xff\xff\x40\xff\xff" "    HSV_CH 00-40"
                                  EFFECT_HSV_AV "\x00\xff\xff\x40\xff\xff" "    HSV_AV 00-40"
                                  EFFECT_HSV_AH "\x00\xff\xff\xff\xff\xff" "    HSV_AH 00-FF" "         "
                                  EFFECT_HSV "\x00\xff\xff" "R"
                                  EFFECT_HSV "\x20\xff\xff" "A"
                                  EFFECT_HSV "\x40\xff\xff" "I"
                                  EFFECT_HSV "\x60\xff\xff" "N"
                                  EFFECT_HSV "\xe0\xff\xff" "B"
                                  EFFECT_HSV "\xc0\xff\xff" "O"
                                  EFFECT_HSV "\xa0\xff\xff" "W"
                                  EFFECT_HSV "\x80\xff\xff" "S "
                                  EFFECT_DELAY_FRAMES "\x00\x96"
                                  EFFECT_RGB "\xff\xff\xff" };
// */
const unsigned char TxtDemo[] = { 
    // EFFECT_CHAR_LEFT
    // EFFECT_BACKGND_LEAVE
    // EFFECT_HSV "\x00\xff\x00"
    // EFFECT_HSV_AV       "\x00\xff\xff\xe0\xff\xff"
    // EFFECT_SCROLL_RIGHT " AILICEC "
    EFFECT_RGB_CV       "\x00\xff\x00\x44\ff\xaa"
    EFFECT_SCROLL_LEFT  "   LEMONGRASS   "
    EFFECT_HSV_AH       "\x00\xe0\xff\xe0\xe0\xff"
    EFFECT_SCROLL_LEFT  "   ACOUSTIC  "
    EFFECT_RGB_CV       "\x00\x00\xff\xaa\xaa\xff"
    EFFECT_SCROLL_LEFT  "   TRIO   "
};

char *wifiText = "";

void ledSetup(){
    FastLED.addLeds<LED_TYPE, LED_PINS, COLOR_ORDER>(leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    // FastLED.addLeds<LED_TYPE, LED_PINS, COLOR_ORDER>(ledMatrix[0], ledMatrix.Size());
    FastLED.setBrightness(currentBrightness);
    FastLED.setMaxPowerInMilliWatts(5*500);

    setupNoise();
    fill_solid (leds, NUM_LEDS, CRGB::Black);
    
    textSetup();
}

void ledLoop(){
#ifdef debug
    _serial_.println("Starting ledLoop");
#endif
    adjustBrightness();
    // if(text && gCurrentPatternNumber == 0)
        // FFTenable = true;
    // else if(FFTenable)
        // FFTenable = false;
    
    if(text){
        EVERY_N_MILLISECONDS( 50 ) { 
            textLoop(); 
        }
        FastLED.show();
    }
    else if(_auto){
        EVERY_N_MILLISECONDS( 41 ) { gHue1++; }
        EVERY_N_MILLISECONDS( 37 ) { gHue2--; }
        // EVERY_N_SECONDS(60){ nextPattern(); }
        EVERY_N_SECONDS(20){
            targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
            randomPalette1 = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
            randomPalette2 = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
        }
        // EVERY_N_MILLISECONDS( 20 ) { 
        autoPatterns[gCurrentPatternNumber]();
        // }
        FastLED.show();
    }
    else if(manual){
        adjustColors();
        FastLED.show();
    }

#ifdef debug
    _serial_.println("Ending ledLoop");
#endif
}

void textSetup(){
    ScrollingMsg.SetFont(MatriseFontData);
    ScrollingMsg.Init(&ledMatrix, ledMatrix.Width(), ScrollingMsg.FontHeight() + 1, 0, 1);
    // ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    // ScrollingMsg.SetTextDirection(CHAR_LEFT);
    // ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0x00);
}

void textLoop(){
    adjustBrightness();
    if (strlen(wifiText) > 0){
        if (ScrollingMsg.UpdateText() == -1)
            ScrollingMsg.SetText((unsigned char *)wifiText, strlen(wifiText));
        else{
            ledMatrix.SetLEDArray(leds);
    }
    }else{
        if (ScrollingMsg.UpdateText() == -1)
            ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
        else{
            ledMatrix.SetLEDArray(leds);
        }
    }
}

void adjustBrightness(){
         if(currentBrightness < _setBrightness) FastLED.setBrightness(++currentBrightness);
    else if(currentBrightness > _setBrightness) FastLED.setBrightness(--currentBrightness);
}
void adjustColors(){
    // for(int i = 0; i < NUM_LEDS/2; i++){
        // for(int j = 0; j < 3; j++){
                 // if(LEFT [i][j] < manualColor_L[j]) LEFT [i][j]++;
            // else if(LEFT [i][j] > manualColor_L[j]) LEFT [i][j]--;
                 // if(RIGHT[i][j] < manualColor_R[j]) RIGHT[i][j]++;
            // else if(RIGHT[i][j] > manualColor_R[j]) RIGHT[i][j]--;
        // }
    // }
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 4; j++){
            for(int k = 0; k < 3; k++){
                     if(leftArray  [i][j][k] < manualColor_L[k]) leftArray  [i][j][k]++;
                else if(leftArray  [i][j][k] > manualColor_L[k]) leftArray  [i][j][k]--;
                     if(rightArray [i][j][k] < manualColor_R[k]) rightArray [i][j][k]++;
                else if(rightArray [i][j][k] > manualColor_R[k]) rightArray [i][j][k]--;
            }
        }
    }
}

#define kMatrixSerpentineLayout true
#define kMatrixArrangedInRows   true
#define kMatrixFlipMajorAxis    false
uint16_t XY(uint8_t x, uint8_t y) {
    if (x >= MATRIX_WIDTH)  return NUM_LEDS;
    if (y >= MATRIX_HEIGHT) return NUM_LEDS;
    if (kMatrixArrangedInRows) {
        if (kMatrixFlipMajorAxis) x = (MATRIX_WIDTH - 1) - x;
        if (kMatrixSerpentineLayout && y & 1)
            return y * MATRIX_WIDTH + (MATRIX_WIDTH - 1) - x;
        return y * MATRIX_WIDTH + x;
    }
    if (kMatrixFlipMajorAxis) y = (MATRIX_HEIGHT - 1) - y;
    if (kMatrixSerpentineLayout && x & 1)
        return x * MATRIX_HEIGHT + (MATRIX_HEIGHT - 1) - y;
    return x * MATRIX_HEIGHT + y;
}

void audio_spectrum(){ // using arduinoFFT to calculate frequencies and mapping them to light spectrum
#ifdef debug
    _serial_.println("Starting audio_spectrum");
#endif
    uint8_t fadeval = 90;
    nscale8(leds, NUM_LEDS, fadeval); // smaller = faster fade
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
        if(tempRGB1 > RIGHT[p]){
            RIGHT[p] = tempRGB1;
        }

        temp2 = spectrum[2][i]/MAX;
        s = 255 - (temp2*30.0);
        v = temp2*255.0;
        tempRGB2 = CHSV(h, s, v);
        if(tempRGB2 > LEFT[pos]){
            LEFT[pos] = tempRGB2;
        }
        yield();
    }
#ifdef debug
    _serial_.println("Ending audio_spectrum");
#endif
}

void audioLight(){ // directly sampling ADC values mapped to brightness
#ifdef debug
    _serial_.println("Starting audioLight");
#endif
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
        int temp1 = abs(mid - analogRead( RightPin));
        if(temp1 > _noise){
            _val = (temp1-_noise)/float(mid) * 255;
            _hue = _val/255.0 * 65;
        }
        R1[NUM_LEDS/4-1] = CHSV( _hue+gHue1, _sat, _val*_val/255);
        R2[0] = R1[NUM_LEDS/4-1];
        
        // _hue = 0; _val = 0;
        // int temp2 = abs(mid - analogRead( LeftPin));
        // if(temp2 > _noise){
            // _val = (temp2-_noise)/float(mid) * 255;
            // _hue = _val/255.0 * 65;
        // }
        L1[NUM_LEDS/4-1] = R1[NUM_LEDS/4-1];
        L2[0] = L1[NUM_LEDS/4-1];
    }
#ifdef debug
    _serial_.println("Ending audioLight");
#endif
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern(){
    uint8_t temp = 0;
    if(text) temp = ARRAY_SIZE( audioPatterns );
    else      temp = ARRAY_SIZE( autoPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % temp;
} // advance to the next pattern

void previousPattern(){
    uint8_t temp = 0;
    if(text) temp = ARRAY_SIZE( audioPatterns );
    else      temp = ARRAY_SIZE( autoPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + (temp-1)) % temp;
} // advance to the previous pattern

void rainbow() {
    // FastLED's built-in rainbow generator
    fill_rainbow( RIGHT, NUM_LEDS/2, gHue1);
    fill_rainbow( LEFT , NUM_LEDS/2, gHue2);
} // rainbow

void rainbowWithGlitter() {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter();
} // rainbow with glitter

// void rainbow_scaling(){
    // for(int i = 0; i <= NUM_LEDS/4; i++){
        // R1[i] = CHSV((millis()/77*i+1)%255 + gHue1, 255, 255);
        // R2[NUM_LEDS/4-i] = R1[i];
        // L1[i] = CHSV((millis()/73*i+1)%255 - gHue2, 255, 255);
        // L2[NUM_LEDS/4-i] = L1[i];
    // }
// } // rainbow scaling

void rainbow_scaling(){
    long t = millis();
    for(int i = 0; i < 8; i++){
        row1[i] = CHSV((t/12*i+1)%255 + gHue1, 255, 255);
        row2[i] = CHSV((t/15*i+1)%255 + gHue1, 255, 255);
        row3[i] = CHSV((t/18*i+1)%255 + gHue1, 255, 255);
        row4[i] = CHSV((t/20*i+1)%255 + gHue1, 255, 255);
        row5[i] = CHSV((t/24*i+1)%255 + gHue1, 255, 255);
        row6[i] = CHSV((t/30*i+1)%255 + gHue1, 255, 255);
        row7[i] = CHSV((t/36*i+1)%255 + gHue1, 255, 255);
        row8[i] = CHSV((t/45*i+1)%255 + gHue1, 255, 255);
        row9[i] = CHSV((t/60*i+1)%255 + gHue1, 255, 255);
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
        fadeToBlackBy( leds, NUM_LEDS, 5);
    }
    EVERY_N_MILLISECONDS(1000/10){
        int pos = random16(NUM_LEDS);
        leds[pos] += CHSV( random8(64)+gHue1, 255, 255);
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
    // double temp = 5.0;
    // int pos1 = beatsin16(11, 0, NUM_LEDS*temp);
    // int pos2 = beatsin16(13, 0, NUM_LEDS*temp);
    // int pos3 = beatsin16( 9, 0, NUM_LEDS*temp);
    // int pos4 = beatsin16(15, 0, NUM_LEDS*temp);
    // double scaledpos1 = pos1/(NUM_LEDS*temp) * NUM_LEDS/2;
    // double scaledpos2 = pos2/(NUM_LEDS*temp) * NUM_LEDS/2;
    // double scaledpos3 = pos3/(NUM_LEDS*temp) * NUM_LEDS/2;
    // double scaledpos4 = pos4/(NUM_LEDS*temp) * NUM_LEDS/2;
    // for(int i = 0; i < NUM_LEDS/2; i++){
        // double a, b, c, w = 0.5; int val;
        // a = i-scaledpos2;
        // a = -w*a*a;
        // val = 255.0*pow(2, a);
        // RIGHT[i] = ColorFromPalette(randomPalette1, (int)scaledpos2, 255, LINEARBLEND);
        // a = i-scaledpos4;
        // a = -w*a*a;
        // val = 255.0*pow(2, a);
        // RIGHT[i] += CHSV(gHue1, 255, val);
        // a = i-scaledpos1;
        // a = -w*a*a;
        // val = 255.0*pow(2, a);
        // LEFT [i] = ColorFromPalette(randomPalette2, (int)scaledpos1, 255, LINEARBLEND);
        // a = i-scaledpos3;
        // a = -w*a*a;
        // val = 255.0*pow(2, a);
        // LEFT [i] += CHSV(gHue2, 255, val);
    // }
}

void dot_beat() {
    uint8_t fadeval = 10;       // Trail behind the LED's. Lower => faster fade.
    // nscale8(leds, NUM_LEDS, fadeval);    // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);
    fadeToBlackBy( leds, NUM_LEDS, fadeval);

    uint8_t BPM, inner, outer, middle;
    
    BPM = 33;

    inner  = beatsin8(BPM, NUM_LEDS/2/4, NUM_LEDS/2/4*3);    // Move 1/4 to 3/4
    outer  = beatsin8(BPM, 0, NUM_LEDS/2-1);               // Move entire length
    middle = beatsin8(BPM, NUM_LEDS/2/3, NUM_LEDS/2/3*2);   // Move 1/3 to 2/3

    LEFT[outer]  = CHSV( gHue1    , 200, 255);
    LEFT[middle] = CHSV( gHue1+96 , 200, 255);
    LEFT[inner]  = CHSV( gHue1+160, 200, 255);

    BPM = 31;
    
    inner  = beatsin8(BPM, NUM_LEDS/2/4, NUM_LEDS/2/4*3);    // Move 1/4 to 3/4
    outer  = beatsin8(BPM, 0, NUM_LEDS/2-1);               // Move entire length
    middle = beatsin8(BPM, NUM_LEDS/2/3, NUM_LEDS/2/3*2);   // Move 1/3 to 2/3

    RIGHT[outer]  = CHSV( gHue2    , 200, 255);
    RIGHT[middle] = CHSV( gHue2+96 , 200, 255);
    RIGHT[inner]  = CHSV( gHue2+160, 200, 255);

} // dot_beat()

void juggle() {
    // colored dots, weaving in and out of sync with each other
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

uint8_t fadeval = 235, frameRate = 45;
void fire(){ // my own simpler 'fire' code - randomly generate fire and move it up the strip while fading
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
        _hue = _val/255.0 *_val/255.0 * 55;
        R1[NUM_LEDS/4-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        R2[0] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        L1[NUM_LEDS/4-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        L2[0] = CHSV( _hue, _sat, _val*_val/255);
    }
}

void fireSparks(){ // randomly generate color and move it up the strip while fading, plus some yellow 'sparkles'
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
        _hue = _val/255.0 *_val/255.0 * 55;
        R1[NUM_LEDS/4-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        R2[0] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        L1[NUM_LEDS/4-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        L2[0] = CHSV( _hue, _sat, _val*_val/255);
        EVERY_N_MILLISECONDS(1000/10){
            CRGB spark = CRGB::Yellow;
            if( random8() < 80)
                R1[NUM_LEDS/4-1-random8(NUM_LEDS/8)] = spark;
            if( random8() < 80)
                R2[random8(NUM_LEDS/8)]              = spark;
            if( random8() < 80)
                L1[NUM_LEDS/4-1-random8(NUM_LEDS/8)] = spark;
            if( random8() < 80)
                L2[random8(NUM_LEDS/8)]              = spark;
        }
    }
}

void fireRainbow(){ // same as fire, but with color cycling
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
        _hue = _val/255.0 *_val/255.0 * 55;
        R1[NUM_LEDS/4-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        R2[0] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        L1[NUM_LEDS/4-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        L2[0] = CHSV( _hue, _sat, _val*_val/255);
    }
}

uint8_t blurval = 150;
void ripple_blur(){ // randomly drop a light somewhere and blur it using blur1d
    EVERY_N_MILLISECONDS(1000/30){
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
            RIGHT [pos] = CHSV(random(0, 64)-gHue2, random(250, 255), 255);
        }
    }
}

double temp = 5.0; // set this for the "resolution" of the bell curve temp*NUM_LEDS
double width = 15; // set this for the "width" of the bell curve (how many LEDs to light)
double _smear = 4.0*sqrt(1.0/(width*width*width)); // this calculates the necessary coefficient for a width of w
void cylon(){
    EVERY_N_MILLISECONDS( 55 ) { gHue1++; }
    EVERY_N_MILLISECONDS( 57 ) { gHue2--; }
    nscale8( leds, NUM_LEDS, 20);
    int posL, posR, val;
    posL = beatsin16(23, 0.9*width*temp, (NUM_LEDS-0.9*width)*temp);  // range of input. the 'w' prevents it from hitting the sides
    posR = beatsin16(27, 0.9*width*temp, (NUM_LEDS-0.9*width)*temp);  // range of input. the 'w' prevents it from hitting the sides
    double a;
    double scaledposL = posL/(NUM_LEDS*temp) * NUM_LEDS/2; // range scaled down to working length
    double scaledposR = posR/(NUM_LEDS*temp) * NUM_LEDS/2; // range scaled down to working length
    for(int i = 0; i < NUM_LEDS/2; i++){
        a = i-scaledposL;       // 
        a = -_smear*a*a;        // generate bell curve with coefficient calculated above for chosen width
        val = 255.0*pow(2, a);  // 
        LEFT [i] |= CHSV(gHue2, 255, val);
        a = i-scaledposR;       // 
        a = -_smear*a*a;        // generate bell curve with coefficient calculated above for chosen width
        val = 255.0*pow(2, a);  // 
        RIGHT[i] |= CHSV(gHue1, 255, val);
    }
}

void matrixTest()
{
    EVERY_N_MILLISECONDS(1000 / 30){
        uint32_t ms = millis();
        int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / MATRIX_WIDTH));
        int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / MATRIX_HEIGHT));
        DrawOneFrame( ms / 65536, yHueDelta32 / 32768, xHueDelta32 / 32768);
    }
}

void DrawOneFrame( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8)
{
  byte lineStartHue = startHue8;
  for( byte y = 0; y < MATRIX_HEIGHT; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;      
    for( byte x = 0; x < MATRIX_WIDTH; x++) {
      pixelHue += xHueDelta8;
      leds[ XY(x, y)]  = CHSV( pixelHue+gHue1, 255, 175);
    }
  }
}
