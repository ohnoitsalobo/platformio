#define FASTLED_INTERNAL
#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define LED_PINS    13
#define NUM_LEDS    NUMBER_OF_LEDS
#define BRIGHTNESS  255*225/255

CRGBArray<NUM_LEDS> leds;                            // LED array containing all LEDs
CRGBSet RIGHT (leds ( 0*NUM_LEDS/2, 1*NUM_LEDS/2-1 ) );   // < subset containing only left  LEDs
CRGBSet LEFT  (leds ( 1*NUM_LEDS/2, 2*NUM_LEDS/2-1 ) );   // < subset containing only right LEDs
CRGBSet R1    (leds ( 0*NUM_LEDS/4, 1*NUM_LEDS/4-1 ) );   // < < subset containing only R1   LEDs
CRGBSet R2    (leds ( 1*NUM_LEDS/4, 2*NUM_LEDS/4-1 ) );   // < < subset containing only R2   LEDs
CRGBSet L1    (leds ( 2*NUM_LEDS/4, 3*NUM_LEDS/4-1 ) );   // < < subset containing only L1   LEDs
CRGBSet L2    (leds ( 3*NUM_LEDS/4, 4*NUM_LEDS/4-1 ) );   // < < subset containing only L2   LEDs

CRGBPalette16 currentPalette, randomPalette1;
CRGBPalette16 targetPalette, randomPalette2;
TBlendType    currentBlending;
uint8_t maxChanges = 24;        // Value for blending between palettes.

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, noise1, noise2, noise3, confetti, sinelon, dot_beat, juggle, bpm };
SimplePatternList gPatterns1 = { audio_spectrum, audioLight };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

bool manual = 0, _auto = 0;
uint8_t currentBrightness = BRIGHTNESS, _setBrightness = BRIGHTNESS;
CRGB manualColor = 0x000000, manualColor_L = 0x000000, manualColor_R = 0x000000;
CHSV manualHSV (0, 255, 255);
uint8_t gHue = 0, gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns

String eqBroadcast = "";
uint8_t eq[2][samples/2-2];

void ledSetup(){
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(currentBrightness);
    FastLED.setMaxPowerInVoltsAndMilliamps(5,1000);
    
    for(int i = 0; i < samples/2-2; i++){
        eq[0][i] = 0;
        eq[1][i] = 0;
    }
    
    setupNoise();
}

void ledLoop(){
    if(MIDIconnected()){
        runLED();
        FastLED.show();
    }else{
        if(music){
            gPatterns1[gCurrentPatternNumber]();
            FastLED.show();
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
            FastLED.show();
        }
        else if(manual){
            for(int i = 0; i < NUM_LEDS/2; i++){
                for(int j = 0; j < 3; j++){
                    // LEFT [i][j] = (LEFT [i][j] < manualColor_L[j]) ? LEFT [i][j]++ : (LEFT [i][j] > manualColor_L[j]) ? LEFT [i][j]-- : LEFT [i][j];
                    // RIGHT[i][j] = (RIGHT[i][j] < manualColor_R[j]) ? RIGHT[i][j]++ : (RIGHT[i][j] > manualColor_R[j]) ? RIGHT[i][j]-- : RIGHT[i][j];
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
}

void audio_spectrum(){
    fftLoop();
    eqBroadcast = "";
    nscale8(leds, NUM_LEDS, 100); // smaller = faster fade
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
        if(tempRGB1 > RIGHT[pos]){
            RIGHT[pos] = tempRGB1;
        }
        eq[0][i-2] = v;
        temp2 = spectrum[2][i]/MAX;
        s = 255 - (temp2*30.0);
        v = temp2*255.0;
        tempRGB2 = CHSV(h, s, v);
        uint8_t p = NUM_LEDS/2-1-pos;
        if(tempRGB2 > LEFT[p]){
            LEFT[p] = tempRGB2;
        }
        eq[1][i-2] = v;
        
        yield();
    }
    // uint8_t p = NUM_LEDS/2-1; uint8_t t = 200;
    // RIGHT[  1]  = RIGHT[  0].nscale8(t);
    // RIGHT[  2]  = RIGHT[  1].nscale8(t);
    // RIGHT[  4]  = RIGHT[  3].nscale8(t);
    // RIGHT[  6]  = RIGHT[  5].nscale8(t);
    // RIGHT[  8]  = RIGHT[  7].nscale8(t);
    // LEFT [p-1]  = LEFT [p-0].nscale8(t);
    // LEFT [p-2]  = LEFT [p-1].nscale8(t);
    // LEFT [p-4]  = LEFT [p-3].nscale8(t);
    // LEFT [p-6]  = LEFT [p-5].nscale8(t);
    // LEFT [p-8]  = LEFT [p-7].nscale8(t);
}

uint16_t maxR = 0, minR = 4096, maxL = 0, minL = 4096;
void audioLight(){
    EVERY_N_MILLISECONDS( 55 ) { gHue1++; }
    EVERY_N_MILLISECONDS( 57 ) { gHue2--; }
    
    uint8_t fadeval = 250;
    for(int i = 0; i < NUM_LEDS/4; i++){
        R2[NUM_LEDS/4-i] = R2[NUM_LEDS/4-1-i].nscale8(fadeval);
        R1[i] = R2[NUM_LEDS/4-i];
        L2[NUM_LEDS/4-i] = L2[NUM_LEDS/4-1-i].nscale8(fadeval);
        L1[i] = L2[NUM_LEDS/4-i];
    }
    uint16_t mid = 1800, _noise = 180;
    uint8_t _hue = 0, _sat = 255, _val = 0;
    int temp1 = abs(mid - analogRead( RightPin));
    if(temp1 > _noise){
        _val = (temp1-_noise)/float(mid) * 255;
        _hue = _val/255.0 * 65;
    }
    R2[0] = CHSV( _hue+gHue1, _sat, _val);
    R1[NUM_LEDS/4] = R2[0];
    L1[NUM_LEDS/4] = R2[0];
    L2[0] = L1[NUM_LEDS/4];
    
    // _hue = 0; _val = 0;
    // int temp2 = abs(mid - analogRead( LeftPin));
    // if(temp2 > _noise){
        // _val = (temp2-_noise)/float(mid) * 255;
        // _hue = _val/255.0 * 65;
    // }
    // L1[NUM_LEDS/4] = CHSV( _hue+gHue2, _sat, _val);
    // L2[0] = L1[NUM_LEDS/4];
    
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern(){
    if(music)
        gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns1 );
    else
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
    EVERY_N_MILLISECONDS(1000/30){
        fadeToBlackBy( leds, NUM_LEDS, 50);
        int pos = random16(NUM_LEDS/2);
        // leds[pos] += CHSV( random8(255), 255, 255);
        RIGHT[pos] += CHSV( gHue1 + random8(64), 190+random8(65), 255);
        LEFT [pos] += CHSV( gHue2 + random8(64), 190+random8(65), 255);
    }
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

uint8_t _x[NUM_LEDS/2], _y[NUM_LEDS/2]; // x/y coordinates for noise function
void setupNoise(){
    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {       // precalculate the lookup-tables:
        uint8_t angle = (i * 256) / NUM_LEDS/2;         // on which position on the circle is the led?
        _x[i] = cos8( angle );                         // corrsponding x position in the matrix
        _y[i] = sin8( angle );                         // corrsponding y position in the matrix
    }
}

int scale = 1000;                               // the "zoom factor" for the noise
void noise1() {
    
    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        uint16_t shift_x = beatsin8(3);                  // the x position of the noise field swings @ 17 bpm
        uint16_t shift_y = millis() / 100;                // the y position becomes slowly incremented

        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint8_t _noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down

        uint8_t index = _noise * 3;                        // map led color based on noise data
        uint8_t bri   = _noise;

        CRGB color = CHSV( index, 255, bri);
        LEFT[i] = color;
    }
    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        uint16_t shift_x = beatsin8(4);                  // the x position of the noise field swings @ 17 bpm
        uint16_t shift_y = millis() / 100;                // the y position becomes slowly incremented

        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint8_t _noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down

        uint8_t index = _noise * 3;                        // map led color based on noise data
        uint8_t bri   = _noise;

        CRGB color = CHSV( index, 255, bri);
        RIGHT[i] = color;
    }
}

// just moving along one axis = "lavalamp effect"
void noise2() {

    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        uint16_t shift_x = millis() / 47;                 // x as a function of time
        uint16_t shift_y = 0;

        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint8_t _noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down

        uint8_t index = _noise * 3;                        // map led color based on noise data
        uint8_t bri   = _noise;

        CRGB color = CHSV( index, 255, bri);
        LEFT[i] = color;
    }
    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        uint16_t shift_x = millis() / 51;                 // x as a function of time
        uint16_t shift_y = 0;

        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint8_t _noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down

        uint8_t index = _noise * 3;                        // map led color based on noise data
        uint8_t bri   = _noise;

        CRGB color = CHSV( index, 255, bri);
        RIGHT[i] = color;
    }
}

// no x/y shifting but scrolling along z
void noise3() {

    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        uint16_t shift_x = 0;                             // no movement along x and y
        uint16_t shift_y = 0;

        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint32_t real_z = millis() * 19;                  // increment z linear

        uint8_t _noise = inoise16(real_x, real_y, real_z) >> 8;           // get the noise data and scale it down

        uint8_t index = _noise * 3;                        // map led color based on noise data
        uint8_t bri   = _noise;

        CRGB color = CHSV( index, 255, bri);
        LEFT[i] = color;
    }
    for (uint16_t i = 0; i < NUM_LEDS/2; i++) {

        uint16_t shift_x = 0;                             // no movement along x and y
        uint16_t shift_y = 0;

        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint32_t real_z = millis() * 23;                  // increment z linear

        uint8_t _noise = inoise16(real_x, real_y, real_z) >> 8;           // get the noise data and scale it down

        uint8_t index = _noise * 3;                        // map led color based on noise data
        uint8_t bri   = _noise;

        CRGB color = CHSV( index, 255, bri);
        RIGHT[i] = color;
    }
}

//////// MIDI stuff ////////

CRGB lastPressed;             // holder for last-detected key color

void runLED(){
    EVERY_N_MILLISECONDS(50){ _hue++; gHue1++; gHue2--;}
    EVERY_N_MILLISECONDS(20){ 
        fadeToBlackBy( leds, NUM_LEDS, 10); // ( sustain ? 3 : 10) );
    }
    MIDI2LED();
    yield();
}

void MIDI2LED(){
    uint8_t _pos = MIDIdata[1]/127.0 * (NUM_LEDS-1); // map note to position
    uint8_t _col = MIDIdata[1]/127.0 * 224; // map note to color
    
    // assign color based on note position and intensity (velocity)
    leds[_pos] = CHSV(_col + _hue, 255 - (MIDIdata[2]/2.0), MIDIdata[2]/127.0 * 255);
    if(MIDIdata[2] > 0 && millis()%2 == 0)
        MIDIdata[2]--;
    lastPressed = leds[_pos]; // remember last-detected note color
    // MidiEventReceived = false;
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
    MIDIdata[0] = channel;
    MIDIdata[1] = pitch;
    MIDIdata[2] = velocity;
    // MidiEventReceived = true;
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
    MIDIdata[0] = channel;
    MIDIdata[1] = pitch;
    MIDIdata[2] = velocity;
    // MidiEventReceived = true;
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
