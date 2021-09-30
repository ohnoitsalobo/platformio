FASTLED_USING_NAMESPACE

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, rainbow_scaling, fireSparks, confetti, ripple_blur, sinelon, /* dot_beat, */ juggle, bpm, blendwave, cylon, cylon1, noise1 };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType    currentBlending;

void setupLED(){
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(currentBrightness);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
    
    fill_solid (leds, NUM_LEDS, CRGB::Black);
    FastLED.show();

    setupNoise();
}

void runLED(){
    if(_midi){
        runMIDI();
        EVERY_N_MILLISECONDS(20){ 
            fadeToBlackBy( leds, NUM_LEDS, ( sustain ? 10 : 20) );
        }
        FastLED.show();
    }
    else if(_auto){
        EVERY_N_MILLISECONDS( 1000 / 60 ){  // Call the current pattern function once, updating the 'leds' array
            gPatterns[gCurrentPatternNumber]();
        }
        // EVERY_N_SECONDS( 30 ) { nextPattern(); }   // change patterns periodically
        EVERY_N_MILLISECONDS(100) {
            uint8_t maxChanges = 24;
            nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   // AWESOME palette blending capability.
        }
        EVERY_N_SECONDS(5) {           // Change the target palette to a random one every 5 seconds.
            targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
        }
        FastLED.show();
    }
    else if(_manual){
        adjustColors();
        FastLED.show();
    }
    adjustBrightness();
    EVERY_N_MILLISECONDS(50){ gHue++; gHue1++; gHue2--;}
    yield();
}

void adjustBrightness(){
         if(currentBrightness < _setBrightness) FastLED.setBrightness(++currentBrightness);
    else if(currentBrightness > _setBrightness) FastLED.setBrightness(--currentBrightness);
}
void adjustColors(){
    for(int i = 0; i < NUM_LEDS/2; i++){
        for(int j = 0; j < 3; j++){
                 if(LEFT [i][j] < manualColor_L[j]) LEFT [i][j]++;
            else if(LEFT [i][j] > manualColor_L[j]) LEFT [i][j]--;
                 if(RIGHT[i][j] < manualColor_R[j]) RIGHT[i][j]++;
            else if(RIGHT[i][j] > manualColor_R[j]) RIGHT[i][j]--;
        }
    }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern(){
    uint8_t temp = 0;
    temp = ARRAY_SIZE( gPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % temp;
} // advance to the next pattern

void previousPattern(){
    uint8_t temp = 0;
    temp = ARRAY_SIZE( gPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + (temp-1)) % temp;
} // advance to the previous pattern

void rainbow() {
    // FastLED's built-in rainbow generator
    fill_rainbow( leds, NUM_LEDS, gHue);
} // rainbow

void rainbowWithGlitter() {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter();
} // rainbow with glitter

void rainbow_scaling(){
    for(int i = 0; i <= NUM_LEDS/2; i++){
        LEFT [i] = CHSV((millis()/77*i+1)%255 + gHue1, 255, 255);
        int j = NUM_LEDS/2-1-i;
        RIGHT[j] = LEFT[i];
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
        int pos = random16(NUM_LEDS);
        // leds[pos] += CHSV( random8(255), 255, 255);
        leds[pos] += CHSV( gHue + random8(64), 190+random8(65), 255);
    }
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 12);
    int pos = beatsin16(11, 0, NUM_LEDS-1);
    leds [pos] = ColorFromPalette(currentPalette, pos, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.
}

void dot_beat() {
    uint8_t fadeval = 115;       // Trail behind the LED's. Lower => faster fade.
    // nscale8(leds, NUM_LEDS, fadeval);    // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);
    fadeToBlackBy( leds, NUM_LEDS, fadeval);

    uint8_t BPM, inner, outer, middle;
    
    BPM = 33;

    inner  = beatsin8(BPM, NUM_LEDS/4, NUM_LEDS/4*3);    // Move 1/4 to 3/4
    outer  = beatsin8(BPM, 0, NUM_LEDS-1);               // Move entire length
    middle = beatsin8(BPM, NUM_LEDS/3, NUM_LEDS/3*2);   // Move 1/3 to 2/3

    leds[outer]  = CHSV( gHue    , 200, 255);
    leds[middle] = CHSV( gHue+96 , 200, 255);
    leds[inner]  = CHSV( gHue+160, 200, 255);

} // dot_beat()

void juggle() {
    // colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 20);
    byte dothue = 0;
    for( int i = 0; i < 6; i++) {
        leds[beatsin16(i+7,0,NUM_LEDS-1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
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
    
} // blendwave()

uint8_t _xhue[NUM_LEDS], _yhue[NUM_LEDS]; // x/y coordinates for noise function
uint8_t _xsat[NUM_LEDS], _ysat[NUM_LEDS]; // x/y coordinates for noise function
void setupNoise(){
    for (uint16_t i = 0; i < NUM_LEDS; i++) {       // precalculate the lookup-tables:
        uint8_t angle = (i * 256) / NUM_LEDS;         // on which position on the circle is the led?
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

    for (uint16_t i = 0; i < NUM_LEDS; i++) {

        shift_x = beatsin8(3);                  // the x position of the noise field swings @ 17 bpm
        shift_y = millis() / 100;                // the y position becomes slowly incremented

        real_x = (_xhue[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        real_y = (_yhue[i] + shift_y) * scale;       // based on the precalculated positions

        _noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down
        
        _hue = _noise * 3;                        // map led color based on noise data
        _sat = 255;
        _val = _noise;

        leds[i] = CHSV( _hue, _sat, _val);
        
    }
}

// just moving along one axis = "lavalamp effect"
void noise2() {

    uint8_t _noise, _hue, _sat, _val;
    uint16_t shift_x, shift_y;
    uint32_t real_x, real_y;

    for (uint16_t i = 0; i < NUM_LEDS; i++) {

        shift_x = millis() / 47;                 // x as a function of time
        shift_y = 0;

        real_x = (_xhue[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        real_y = (_yhue[i] + shift_y) * scale;       // based on the precalculated positions

        _noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down
        
        _hue = _noise * 3;                        // map led color based on noise data
        _sat = 255;
        _val = _noise;

        leds[i] = CHSV( _hue, _sat, _val);
    }
}

// no x/y shifting but scrolling along z
void noise3() {

    uint8_t _noise, _hue, _sat, _val;
    uint16_t shift_x, shift_y;
    uint32_t real_x, real_y, real_z;

    for (uint16_t i = 0; i < NUM_LEDS; i++) {

        shift_x = 0;                             // no movement along x and y
        shift_y = 0;

        real_x = (_xhue[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        real_y = (_yhue[i] + shift_y) * scale;       // based on the precalculated positions

        real_z = millis() * 19;                  // increment z linear

        _noise = inoise16(real_x, real_y, real_z) >> 8;           // get the noise data and scale it down

        _hue = _noise * 3;                        // map led color based on noise data
        _sat = 255;
        _val = _noise;

        leds[i] = CHSV( _hue, _sat, _val);
    }
}

uint8_t fadeval = 235, frameRate = 45;
void fire(){ // my own simpler 'fire' code - randomly generate fire and move it up the strip while fading
    EVERY_N_MILLISECONDS(1000/frameRate){
        uint8_t _hue = 0, _sat = 255, _val = 0;
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        LEFT [NUM_LEDS/2-1] = CHSV( _hue, _sat, _val*_val/255);
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 50);
        _hue = _val/255.0 *_val/255.0 * 55;
        RIGHT[0] = CHSV( _hue, _sat, _val*_val/255);
    }
    for(int i = 0; i < NUM_LEDS/2-1; i++){
        LEFT [i] = LEFT [i+1].nscale8(fadeval); if(LEFT [i].g > 0) LEFT [i].g--;
        int j = NUM_LEDS/2-1-i;
        RIGHT[j] = RIGHT[j-1].nscale8(fadeval); if(RIGHT[j].g > 0) RIGHT[j].g--;

    }
}

void fireSparks(){ // randomly generate color and move it up the strip while fading, plus some yellow 'sparkles'
    fire();
    EVERY_N_MILLISECONDS(1000/10){
        CRGB spark = CRGB::Yellow;
        if( random8() < 80)
            leds[random8(NUM_LEDS-1)] = spark;
    }
}

uint8_t blurval = 100;
void ripple_blur(){ // randomly drop a light somewhere and blur it using blur1d
    EVERY_N_MILLISECONDS(1000/30){
        blur1d(leds, NUM_LEDS, blurval);
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
double width = 10; // set this for the "width" of the bell curve (how many LEDs to light)
double _smear = 4.0*sqrt(1.0/(width*width*width)); // this calculates the necessary coefficient for a width of w
void cylon(){
    nscale8( leds, NUM_LEDS, 20);
    int pos, val;
    pos = beatsin16(20, 0.9*width*temp, (NUM_LEDS-0.9*width)*temp);  // range of input. the 'w' prevents it from hitting the sides
    double a;
    double scaledpos = pos/(NUM_LEDS*temp) * NUM_LEDS; // range scaled down to working length
    for(int i = 0; i < NUM_LEDS; i++){
        a = i-scaledpos;       // 
        a = -_smear*a*a;        // generate bell curve with coefficient calculated above for chosen width
        val = 255.0*pow(2, a);  // 
        leds [i] |= CHSV(0, 255, val);
    }
}

void cylon1(){
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
