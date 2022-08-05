#define TIMES_PER_SECOND(x) EVERY_N_MILLISECONDS(1000/x)
#define TIMES_PER_MINUTE(x) EVERY_N_SECONDS(60/x)
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#include <Gaussian.h>

byte coordsX[NUM_LEDS] = { 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140, 148, 156, 165, 173, 181, 189, 197, 206, 214, 222, 230, 239, 247, 255, 255, 255, 255, 255, 255, 247, 239, 230, 222, 214, 206, 197, 189, 181, 173, 165, 156, 148, 140, 132, 123, 115, 107, 99, 90, 82, 74, 66, 58, 49, 41, 33, 25, 16, 8, 0, 0, 0, 0, 0, 0 };
byte coordsY[NUM_LEDS] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 73, 109, 146, 182, 219 };
byte angles [NUM_LEDS] = { 247, 246, 246, 245, 244, 243, 242, 240, 239, 236, 233, 229, 223, 215, 204, 191, 178, 167, 159, 154, 149, 146, 144, 142, 141, 139, 138, 137, 137, 136, 133, 130, 128, 125, 122, 119, 116, 115, 114, 113, 112, 111, 109, 106, 104, 100, 96, 90, 83, 74, 64, 54, 45, 38, 32, 27, 24, 21, 19, 17, 15, 14, 13, 12, 11, 11, 8, 5, 3, 0, 252, 250 };
byte radii  [NUM_LEDS] = { 240, 224, 209, 194, 179, 164, 149, 134, 119, 105, 91, 78, 66, 56, 50, 47, 50, 56, 66, 78, 91, 105, 119, 134, 149, 164, 179, 194, 209, 224, 237, 235, 235, 235, 237, 240, 228, 213, 198, 183, 169, 154, 140, 126, 113, 100, 89, 78, 70, 65, 63, 65, 70, 78, 89, 100, 113, 126, 140, 154, 169, 183, 198, 213, 228, 243, 255, 253, 251, 251, 251, 253 };

#include "noise_patterns.h"

CRGBSet downRight = leds( 0, 14);
CRGBSet downLeft  = leds(15, 29);
CRGBSet upLeft    = leds(36, 51);
CRGBSet upRight   = leds(52, 65);
FASTLED_USING_NAMESPACE

typedef void (*SimplePatternList[])();
// SimplePatternList gPatterns = { fire, rainbow, fireworks, confetti, ripple_blur, fire, cylon, cylon1, sinelon, juggle, bpm };
SimplePatternList gPatterns = { noise3, rainbow, fireworks, confetti, ripple_blur, fire };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

void setupLeds(){
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(currentBrightness);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
    
    fill_solid (leds, NUM_LEDS, CRGB::Black);
    FastLED.show();

}

void runLeds(){
    // EVERY_N_SECONDS(1){
        // checkMIDI();
    // }
    adjustBrightness();
    EVERY_N_MILLISECONDS(50){ gHue++; gHue1++; gHue2--;}
    
    switch(_mode){
        case _midi:
            // runMIDI();
            // displayMIDI();
            // _mode = _auto;
            for(int i = 1; i < _index; i++){
                int _hue = (i-1)/(float)_index * 255;
                int pos1 = 14-i+1; 
                int pos2 = 15+i-1;
                int pos3 = pos1+36; if (pos1 < 0) pos1+= NUM_LEDS;
                int pos4 = pos2+36; if (pos4 > NUM_LEDS) pos4-= NUM_LEDS;
                leds[pos1] = CHSV(_hue, 255, WSdata_int[i]);
                leds[pos2] = CHSV(_hue, 255, WSdata_int[i]);
                leds[pos3] = CHSV(_hue, 255, WSdata_int[i]);
                leds[pos4] = CHSV(_hue, 255, WSdata_int[i]);
            }
            fadeToBlackBy( leds, NUM_LEDS, 30);
        break;
        case _auto:
            EVERY_N_MILLISECONDS( 1000 / 60 ){  // Call the current pattern function once, updating the 'leds' array
                gPatterns[gCurrentPatternNumber]();
            }
            if(auto_advance) {
                EVERY_N_SECONDS( 40 ) { 
                    nextPattern(); 
                }
            }   // change patterns periodically
        break;
        case _manual:
            adjustColors();
        break;
        default:
        break;
    }
    adjustBrightness();
    
    FastLED.show();
    FastLED.delay(1);
    yield();
    
}

void adjustBrightness(){
         if(currentBrightness < _setBrightness) FastLED.setBrightness(++currentBrightness);
    else if(currentBrightness > _setBrightness) FastLED.setBrightness(--currentBrightness);
}
void adjustColors(){
    for(int i = 0; i < 30; i++){
        for(int j = 0; j < 3; j++){
                 if(UP [i][j] < manualColor_UP [j]) UP [i][j]++;
            else if(UP [i][j] > manualColor_UP [j]) UP [i][j]--;
                 if(DOWN[i][j] < manualColor_DOWN[j]) DOWN[i][j]++;
            else if(DOWN[i][j] > manualColor_DOWN[j]) DOWN[i][j]--;
        }
    }
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 3; j++){
                 if(LEFT [i][j] < manualColor_LEFT [j]) LEFT [i][j]++;
            else if(LEFT [i][j] > manualColor_LEFT [j]) LEFT [i][j]--;
                 if(RIGHT[i][j] < manualColor_RIGHT[j]) RIGHT[i][j]++;
            else if(RIGHT[i][j] > manualColor_RIGHT[j]) RIGHT[i][j]--;
        }
    }
}

void nextPattern(){
    currentBrightness = 0;
    uint8_t temp = 0;
    temp = ARRAY_SIZE( gPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % temp;
} // advance to the next pattern

void previousPattern(){
    currentBrightness = 0;
    uint8_t temp = 0;
    temp = ARRAY_SIZE( gPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + (temp-1)) % temp;
} // advance to the previous pattern

void rainbow() {
    uint8_t speed = 5; 
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(beat8(speed) - radii[i]/5.0, 255, 255);
    }
} // rainbow

void rainbowWithGlitter() {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter();
} // rainbow with glitter

void addGlitter() {
    EVERY_N_MILLISECONDS(1000/30){
        if( random8() < 80) {
            leds[ random16(NUM_LEDS) ] += CRGB::White;
        }
    }
}

void cylon(){
    float width = 2.0f, res = 3.0f;
    static Gaussian _cylon(0, width*width);
    // float pos = beatsin16(_BPM/2, width*5, (NUM_LEDS-width)*5);
    // _cylon.mean = pos/5.0;
    float pos = beatsin16(_BPM/2, width*res, (NUM_LEDS-1-width)*res);
    for (int i = 0; i < NUM_LEDS; i++){
        float _val;
        _cylon.mean = pos/res;
        _val = _cylon.plot(i)*(width/0.399); // divide by 0.3989 to normalize to ~1.0 
        leds[i] = CHSV(0, 255, _val*255);
    }
    yield();
}


uint8_t fadeval = 210, frameRate = 45; // 45
void fire(){ // my own simpler 'fire' code - randomly generate fire and move it up the strip while fading
    CRGBSet _left  (leds ( 30, 50) );
    CRGBSet _right (leds ( 51, 71) );
    EVERY_N_MILLISECONDS(1000/frameRate){
        uint8_t _hue = 0, _sat = 255, _val = 0;                       // 
        _val = random(100, 255);                                      // generate a random brightness value between 100-255 (never zero)
        _sat = 255 - (_val/255.0 * 60);                               // brighter = less saturated / more white-ish
        _hue = _val/255.0 *_val/255.0 * 55;                           // keep hue in the red/yellow/orange range, but nonlinear scaling
        _left[0] = CHSV( _hue, _sat, _val*_val/255);       // LEDs in the center get this new random color
        _val = random(100, 255);                                      // generate a random brightness value between 100-255 (never zero)
        _sat = 255 - (_val/255.0 * 60);                               // brighter = less saturated / more white-ish
        _hue = _val/255.0 *_val/255.0 * 55;                           // keep hue in the red/yellow/orange range, but nonlinear scaling
        _right[20] = CHSV( _hue, _sat, _val*_val/255);       // LEDs in the center get this new random color
        if( random8() < 20) {
            _left[0] = CRGB::Gold;
        }
        if( random8() < 20) {
            _right[20] = CRGB::Gold;
        }
        for(int i = 0; i < 20; i++){
            _left[20-i] = _left[20-i-1].nscale8(fadeval); // shift the color to the next LED, dim the brightness very slightly,
            if(_left[i].g > 0) _left[i].g--;        // and reduce green in particular to fade the yellow faster than the red
            _right[i] = _right[i+1].nscale8(fadeval);
            if(_right[i].g > 0) _right[i].g--;
        }
        leds[0] = _right[20];
        leds[29] = _left[0];
        for(int i = 0; i < 15; i++){
            leds[15-i] = leds[15-i-1].nscale8(fadeval); // shift the color to the next LED, dim the brightness very slightly,
            leds[15+i] = leds[15+i+1].nscale8(fadeval); // shift the color to the next LED, dim the brightness very slightly,
            if(leds[i].g > 0) leds[i].g--;        // and reduce green in particular to fade the yellow faster than the red
            if(leds[15+i].g > 0) leds[15+i].g--;        // and reduce green in particular to fade the yellow faster than the red
        }
    }
    
    // for(int i = NUM_LEDS/3; i > 0; i--){
        // leds[i] = leds[i-1].nscale8(fadeval); // shift the color to the next LED, dim the brightness very slightly,
        // if(leds[i].g > 0) leds[i].g--;        // and reduce green in particular to fade the yellow faster than the red
    // }
    yield();
}

const int sparks = 15;
FireWork _firework[sparks];
void fireworks(){
    fadeToBlackBy(leds, NUM_LEDS, 100);
    for (int i = 0; i < sparks; i++){
        _firework[i].draw();
    }
    EVERY_N_SECONDS(2){
        if(random8() < 175){
            firework_init();
        }
    }
}

void firework_init(){
    int temp = random16(0, NUM_LEDS);
    for (int i = 0; i < sparks; i++){
        _firework[i].init(temp);
    }
}

void confetti() 
{    // random colored speckles that blink in and fade smoothly
    EVERY_N_MILLISECONDS(1000/40){
    fadeToBlackBy( leds, NUM_LEDS, 20);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV( gHue + random8(64), 190+random8(65), 255);
    }
}

uint8_t blurval = 100;
void ripple_blur(){ // randomly drop a light somewhere and blur it using blur1d
    EVERY_N_MILLISECONDS(1000/30){
        blur1d(leds, NUM_LEDS, blurval);
    }
    EVERY_N_MILLISECONDS(30){
        if( random8() < 30) {
            uint8_t pos = random(30);
            leds [pos] = CHSV(random(0, 64)+gHue1, random(250, 255), 255);
        }
        if( random8() < 30) {
            uint8_t pos = random(30);
            leds [35+pos] = CHSV(random(0, 64)-gHue2, random(250, 255), 255);
        }
    }
}

double temp = 5.0; // set this for the "resolution" of the bell curve temp*NUM_LEDS
double width = 10; // set this for the "width" of the bell curve (how many LEDs to light)
double _smear = 4.0*sqrt(1.0/(width*width*width)); // this calculates the necessary coefficient for a width of w
void cylon1(){
    EVERY_N_MILLISECONDS( _BPM   ) { gHue1++; }
    EVERY_N_MILLISECONDS( _BPM-2 ) { gHue2--; }
    nscale8( leds, NUM_LEDS, 20);
    int pos, val;
    pos = beatsin16(_BPM/2  , 0.9*width*temp, (NUM_LEDS-0.9*width)*temp);  // range of input. the 'w' prevents it from hitting the sides
    double a;
    double scaledpos = pos/(NUM_LEDS*temp) * NUM_LEDS; // range scaled down to working length
    for(int i = 0; i < NUM_LEDS; i++){
        a = i-scaledpos;       // 
        a = -_smear*a*a;        // generate bell curve with coefficient calculated above for chosen width
        val = 255.0*pow(2, a);  // 
        leds [i] |= CHSV(gHue2, 255, val);
    }
    yield();
}

void juggle() {
    // colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 30);
    byte dothue = 0;
    for( int i = 0; i < 6; i++) {
        float _pos = beatsin16(i+7,0,NUM_LEDS-1);
        // leds[beatsin16(i+7,0,NUM_LEDS-1)] |= CHSV(dothue, 200, 255);
        DrawPixels(_pos, 1.5, CHSV(dothue, 200, 255));
        dothue += 32;
        yield();
    }
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    // uint8_t BeatsPerMinute = 62;
    uint8_t BeatsPerMinute = _BPM;
    // CRGBPalette16 palette = PartyColors_p;
    CRGBPalette16 palette = RainbowColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
        leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
        yield();
    }
}

void blendwave() {
    CRGB clr1, clr2;
    uint8_t speed, loc1;

    speed = beatsin8(6,0,255);

    clr1 = blend(CHSV(beatsin8(3,0,255),255,255), CHSV(beatsin8(4,0,255),255,255), speed);
    clr2 = blend(CHSV(beatsin8(4,0,255),255,255), CHSV(beatsin8(3,0,255),255,255), speed);
    loc1 = beatsin8(13,0,NUM_LEDS-1);

    fill_gradient_RGB(leds, 0, clr2, loc1, clr1);
    fill_gradient_RGB(leds, loc1, clr2, NUM_LEDS-1, clr1);
    
    yield();
} // blendwave()

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 20);
    float pos = beatsin16(_BPM/3, 0, (NUM_LEDS-1)*5.0)/5.0;
    DrawPixels(pos, 1.5, ColorFromPalette(RainbowColors_p, pos, 255, LINEARBLEND));
    // leds [pos] = ColorFromPalette(RainbowColors_p, pos, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.
    yield();
}

CRGB ColorFraction(CRGB colorIn, float fraction){
    fraction = min(1.0f, fraction);
    return CRGB(colorIn).fadeToBlackBy(255 * (1.0f - fraction));
}

void DrawPixels(float fPos, float count, CRGB color){
    // Calculate how much the first pixel will hold
    float availFirstPixel = 1.0f - (fPos - (long)(fPos));
    float amtFirstPixel = min(availFirstPixel, count);
    float remaining = min(count, FastLED.size()-fPos);
    int iPos = fPos;

    // Blend (add) in the color of the first partial pixel

    if (remaining > 0.0f){
        leds[iPos++] += ColorFraction(color, amtFirstPixel);
        remaining -= amtFirstPixel;
    }

    // Now draw any full pixels in the middle

    while (remaining > 1.0f){
        leds[iPos++] += color;
        remaining--;
    }

    // Draw tail pixel, up to a single full pixel

    if (remaining > 0.0f){
        leds[iPos] += ColorFraction(color, remaining);
    }
}