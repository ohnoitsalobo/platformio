#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define LED_PINS    13
#define NUM_LEDS    NUMBER_OF_LEDS
#define BRIGHTNESS  255*225/255

CRGBArray<NUM_LEDS> leds;                              // LED array containing all LEDs
CRGBSet LEFT  (leds (0,            NUM_LEDS/2-1)   );  // < subset containing only left  LEDs
CRGBSet L1    (leds (0,            NUM_LEDS/4-1)   );  // < subset containing only left  side of left  LEDs
CRGBSet L2    (leds (NUM_LEDS/4,   NUM_LEDS/2-1)   );  // < subset containing only right side of left  LEDs
CRGBSet RIGHT (leds (NUM_LEDS/2,   NUM_LEDS)       );  // < subset containing only right LEDs
CRGBSet R1    (leds (NUM_LEDS/2,   3*NUM_LEDS/4-1) );  // < subset containing only left  side of right LEDs
CRGBSet R2    (leds (3*NUM_LEDS/4, NUM_LEDS)       );  // < subset containing only right side of right LEDs

CRGBPalette16 currentPalette, randomPalette1;
CRGBPalette16 targetPalette, randomPalette2;
TBlendType    currentBlending;
uint8_t maxChanges = 24;        // Value for blending between palettes.

typedef void (*SimplePatternList[])();
// SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, dot_beat, juggle, bpm, inoise8_mover};
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, dot_beat, bpm, inoise8_mover};
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

bool manual = 0, _auto = 0;
CRGB manualColor = 0x000000;
CHSV manualHSV (0, 255, 255);
uint8_t gHue = 0, gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns

String eqBroadcast = "";
uint8_t eq[2][samples/2-2];

void ledSetup(){
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 750);
    
    for(int i = 0; i < samples/2-2; i++){
        eq[0][i] = 0;
        eq[1][i] = 0;
    }
}

void ledLoop(){
#ifdef debug
    Serial_1.println("Starting ledLoop");
#endif
    if(music){
        audio_spectrum();
        // audioLight();
        EVERY_N_MILLISECONDS(20){
            if(webSocketConn()){
                eqBroadcast = "E";
                for(uint8_t i = 0; i < samples/2-2; i++){
                    eqBroadcast += ",";
                    eqBroadcast += String(eq[0][i]);
                    if(eq[0][i] != 0) eq[0][i] /= 5.0;
                }
                for(uint8_t i = 0; i < samples/2-2; i++){
                    eqBroadcast += ",";
                    eqBroadcast += String(eq[1][i]);
                    if(eq[1][i] != 0) eq[1][i] /= 5.0;
                }
                wsBroadcast();
                eqBroadcast = "";
            }
        }
    }
    else if(_auto){
        EVERY_N_SECONDS(20){
            targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
            randomPalette1 = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
            randomPalette2 = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
        }
        EVERY_N_MILLISECONDS( 29 ) {
            gHue1++;
        }
        EVERY_N_MILLISECONDS( 23 ) {
            gHue2--;
        }
        gPatterns[gCurrentPatternNumber]();
    }
    else if(manual){
        
    }
    FastLED.show();
#ifdef debug
    Serial_1.println("Ending ledLoop");
#endif
}

void audio_spectrum(){
#ifdef debug
    Serial_1.println("Starting audio_spectrum");
#endif

    nscale8(leds, NUM_LEDS, 10); // smaller = faster fade
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
            R2[p] = R1[pos];
        }
        eq[0][i-2] = v;
        temp2 = spectrum[2][i]/MAX;
        s = 255 - (temp2*30.0);
        v = temp2*255.0;
        tempRGB2 = CHSV(h, s, v);
        if(tempRGB2 > L1[pos]){
            L1[pos] = tempRGB2;
            L2[p] = L1[pos];
        }
        eq[1][i-2] = v;
        yield();
    }
#ifdef debug
    Serial_1.println("Ending audio_spectrum");
#endif
}

void audioLight(){
#ifdef debug
    Serial_1.println("Starting audioLight");
#endif
    
#ifdef debug
    Serial_1.println("Ending audioLight");
#endif
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern(){
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns );
}

void rainbow() {
    // FastLED's built-in rainbow generator
    fill_rainbow( LEFT , NUM_LEDS/2, gHue2);
    fill_rainbow( RIGHT, NUM_LEDS/2, gHue1);
}

void rainbowWithGlitter() {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter();
}

void addGlitter() {
    if( random8() < 80) {
        leds[ random16(NUM_LEDS) ] += CRGB::White;
    }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
    fadeToBlackBy( leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV( random8(255), 255, 255);
    FastLED.show();
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 10);
    int pos1 = beatsin16(11, 0, NUM_LEDS/2-1);
    int pos2 = beatsin16(13, 0, NUM_LEDS/2-1);
    int pos3 = beatsin16( 9, 0, NUM_LEDS/2-1);
    int pos4 = beatsin16(15, 0, NUM_LEDS/2-1);
    LEFT [pos1] = ColorFromPalette(randomPalette1, pos1, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.
    RIGHT[pos2] = ColorFromPalette(randomPalette2, pos2, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.
    LEFT [pos3] += CHSV( gHue2, 255, 255);
    RIGHT[pos4] += CHSV( gHue1, 255, 255);
    FastLED.show();
}

void dot_beat() {
    uint8_t fadeval = 190;                                        // Trail behind the LED's. Lower => faster fade.
    uint8_t BPM = 30;


    uint8_t inner = beatsin8(BPM, NUM_LEDS/4, NUM_LEDS/4*3);    // Move 1/4 to 3/4
    uint8_t outer = beatsin8(BPM, 0, NUM_LEDS-1);               // Move entire length
    uint8_t middle = beatsin8(BPM, NUM_LEDS/3, NUM_LEDS/3*2);   // Move 1/3 to 2/3

    leds[outer]  = CHSV( gHue1    , 200, 255); // CRGB::Aqua;
    leds[middle] = CHSV( gHue1+96 , 200, 255); // CRGB::Purple;
    leds[inner]  = CHSV( gHue1+160, 200, 255); // CRGB::Blue;

    nscale8(leds,NUM_LEDS,fadeval);                             // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);
} // dot_beat()

void juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 15);
    byte dothue1 = 0, dothue2 = 0;
    for( int i = 0; i < 8; i++) {
        RIGHT[beatsin16(i+7,0,NUM_LEDS/2)] |= CHSV(dothue1, 200, 255);
        LEFT [beatsin16(i+5,0,NUM_LEDS/2)] |= CHSV(dothue2, 200, 255);
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

static int16_t dist1 = 0, dist2 = 0;    // A moving location for our noise generator.
uint16_t xscale = 30;           // Wouldn't recommend changing this on the fly, or the animation will be really blocky.
uint16_t yscale = 30;           // Wouldn't recommend changing this on the fly, or the animation will be really blocky.

void inoise8_mover() {
    uint8_t locn = 0, pixlen = 0;
    
    locn = inoise8(xscale, dist1+yscale) % 255;          // Get a new pixel location from moving noise.
    pixlen = map(locn,0,255,0,NUM_LEDS/2);                // Map that to the length of the strand.
    LEFT[pixlen] = ColorFromPalette(currentPalette, pixlen, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.

    locn = inoise8(xscale, dist2+yscale) % 255;          // Get a new pixel location from moving noise.
    pixlen = map(locn,0,255,0,NUM_LEDS/2);                // Map that to the length of the strand.
    RIGHT[pixlen] = ColorFromPalette(randomPalette2, pixlen, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.

    dist1 += beatsin8(10,1,4);                                                // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.                                             
    dist2 += beatsin8(13,1,4);                                                // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.                                             
    FastLED.show();
    fadeToBlackBy(leds, NUM_LEDS, 10);     
}