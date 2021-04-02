#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define LED_TYPE    LPD8806
#define LED_PINS    13, 12
#define COLOR_ORDER BRG
#define NUM_LEDS     48
// #define LED_TYPE    WS2812
// #define COLOR_ORDER GRB
// #define LED_PINS    13
// #define NUM_LEDS    144

CRGBArray<NUM_LEDS> leds;
CRGBSet LEFT  (leds (0,            NUM_LEDS/2-1)   );  // < subset containing only left  LEDs
CRGBSet RIGHT (leds (NUM_LEDS/2,   NUM_LEDS)       );  // < subset containing only right LEDs

bool manual = false, _cycle = false, soundresponsive = false;
CRGB manualColor = 0xFFFF88, endclr, midclr;
CHSV manualHSV (0, 255, 255), gradient1(0, 255, 255), gradient2(0, 255, 255);

#define BRIGHTNESS         150*150/255.0
// #define BRIGHTNESS         255
#define FRAMES_PER_SECOND  120

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType    currentBlending;

#define BANDS 7
int strobe = 5, res = 4, values[BANDS+1], values1[BANDS+1], _max = 0, _min = 255;
int total = 0;
uint8_t matrix[NUM_LEDS][BANDS+1];

void ledSetup(){
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );

    FastLED.setBrightness(BRIGHTNESS);

    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

    if(manual){
        fill_solid (leds, NUM_LEDS, manualColor);
        FastLED.show();
    }
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {  audio_rainbow , simplePattern , audio_eq };
// SimplePatternList gPatterns = {  MIDI };
SimplePatternList gPatterns1 = { rainbow, rainbowWithGlitter, rainbow_LCM, confetti, sinelon, dot_beat, dot_beat1, juggle, bpm, blendwave, beatwave };
// SimplePatternList gPatterns1 = { clockLight };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns

void ledLoop(){
    if(!manual){
        EVERY_N_MILLISECONDS( 1000 / FRAMES_PER_SECOND ){  // Call the current pattern function once, updating the 'leds' array
            if(soundresponsive)
                gPatterns[gCurrentPatternNumber]();
            else
                gPatterns1[gCurrentPatternNumber]();
        }
        // FastLED.delay( 1000 / FRAMES_PER_SECOND );  // insert a delay to keep the framerate modest
        // EVERY_N_SECONDS( 80 ) { nextPattern(); }   // change patterns periodically
        EVERY_N_MILLISECONDS(100) {
            uint8_t maxChanges = 24;
            nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   // AWESOME palette blending capability.
        }
        EVERY_N_SECONDS(5) {           // Change the target palette to a random one every 5 seconds.
            targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
        }
    } else {
        if( _cycle ){
            blendwave();
        }
    }
    EVERY_N_MILLISECONDS( 20 ) { gHue1++; gHue2--; }     // slowly cycle the "base color" through the rainbow
    yield();
}

void MIDI(){
    EVERY_N_MILLISECONDS(20){ 
        // fill_rainbow( leds, NUM_LEDS, _hue, 7);
        // FastLED.show();
        // fadeToBlackBy( leds, NUM_LEDS, fadeval );
        fadeToBlackBy( leds, NUM_LEDS, 10 );
    }
    FastLED.show();
    yield();
}

// /*
// 63, 160, 400, 1000, 2500, 6250, 16000
// R    O    Y    G     Aq     B     Pu    Pi
// 0    32   64   96    128    160   192   224
void simplePattern(){ // frequencies from beginning to end of strip
    for(int i = 0; i < NUM_LEDS/2; i++){
        int pos = i*2/7.0, col = (224*i)/(NUM_LEDS/2); 
        int temp = values1[pos]; // *values1[pos]/255;
        LEFT [NUM_LEDS/2-1-i] = CHSV( col, map(values1[pos], 0, 255, 255, 200), temp);
        RIGHT[i]              = CHSV( col, map(values1[pos], 0, 255, 255, 200), temp);
        yield();
    }
    FastLED.show();
}

void audio_rainbow(){ // interpolated rainbow
    for(int i = 0; i < NUM_LEDS/2; i++){
        matrix[i][BANDS] = 0;                      // set sum of all bands = 0
        for(int j = 0; j < BANDS; j++){
            int dist = 4;
            if (NUM_LEDS > 48) dist = 10;
            int temp = values1[j]; // *values1[j]/255;
            int offset = abs (j * dist - i+2);  // offset the centers of the frequency bands
            if ( offset > dist )                  // if outside bounds
                matrix[i][j] = CRGB::Black;        // black for that frequency
            else                                   // color intensity interpolates with distance from frequency center
                matrix[i][j] = (1 - ((float)offset/(float)dist)) * temp;

            matrix[i][BANDS] += matrix[i][j];      // calculate sum
            yield();
        }
        LEFT[NUM_LEDS/2-1-i]  = CHSV((224*i)/(NUM_LEDS/2.0), map(matrix[i][BANDS], 0, 255, 255, 200), matrix[i][BANDS]); // set hue and brightness
        RIGHT[i] = LEFT[NUM_LEDS/2-1-i]; // set hue and brightness
    }
    FastLED.show();
}

void audio_eq(){ // segmented equalizer
    for(int i = 0; i < NUM_LEDS/2; i++){
        int pos = i*2/7.0, col = (224*i)/(NUM_LEDS/2);
        LEFT [NUM_LEDS/2-1-i] = CHSV( col, (values[pos] > 255) ? 210 : 255 , (values[pos] > 255) ? 255 : values[pos] );
        RIGHT[i]              = CHSV( col, (values[pos] > 255) ? 210 : 255 , (values[pos] > 255) ? 255 : values[pos] );
        values[pos] -= (values[pos] > 255) ? 255 : values[pos];
        yield();
    }
    FastLED.show();
}
/*  * /
#define div 1000
#define width 1

void clockLight(){
    int temp = ((millis() ) % div) * (NUM_LEDS/2+width*2);  // map to no. of LEDs without scaling so all calculation done as ints, preserve accuracy
    int val[NUM_LEDS/2+(width*2)];                                        // LED brightness to be calculated 
    for(int i = 1; i <= NUM_LEDS/2+(width*2); i++){
        if((abs(temp - i*div)) > (width*div))       // if LED is more than [width] away from center of light
            val[i-1] = 0;                                // zero brightness
        else{
            int t = width*div - abs(temp - i*div);  // if LED is within [width] from center of light
            val[i-1] = ((double)t / (width*div))*255.0;  // scale brightness to center
            val[i-1] *= val[i-1]/255.0;                       // square for good measure (less 'smear', more precise peak brightness)
        }
    }
    for(int i = 1; i <= NUM_LEDS/2; i++){
        leds[(i-1)] = CHSV(gHue1, 100, val[i+width-1]);        // set color / brightness
    }

    temp = (second()) * (NUM_LEDS/2);  // map to no. of LEDs without scaling so all calculation done as ints, preserve accuracy
    for(int i = 1; i <= NUM_LEDS/2; i++){
        if((abs(temp - i*60)) > (width*60))       // if LED is more than [width] away from center of light
            val[i-1] = 0;                                // zero brightness
        else{
            int t = width*60 - abs(temp - i*60);  // if LED is within [width] from center of light
            val[i-1] = ((double)t / (width*60))*255.0;  // scale brightness to center
            // val[i-1] *= val[i-1]/255.0;                       // square for good measure (less 'smear', more precise peak brightness)
        }
    }
    for(int i = 1; i <= NUM_LEDS/2; i++){
        leds[(i-1)+NUM_LEDS/2] = CHSV(gHue1, 255, val[i-1]);        // set color / brightness
    }
    FastLED.show();                                 // show on LED strip
    yield();
}
/*  */
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    if(soundresponsive)
        gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
    else
        gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns1);
}

void rainbow()
{
    // FastLED's built-in rainbow generator
    // fill_rainbow( leds, NUM_LEDS, gHue1, 7);
    leds(0, NUM_LEDS/2-1).fill_rainbow(gHue1);
    leds(NUM_LEDS/2, NUM_LEDS-1) = leds(NUM_LEDS/2-1 , 0);
    FastLED.show();
}

void rainbowWithGlitter()
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    if( random8() < 80) {
        leds[ random16(NUM_LEDS) ] += CRGB::White;
    }
    FastLED.show();
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
    fadeToBlackBy( leds, NUM_LEDS, 30);
    int pos = beatsin16(13,0,NUM_LEDS-1);
    leds[pos] += CHSV( gHue1, 255, 192);
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
    FastLED.show();
} // dot_beat()

void dot_beat1() {
    uint8_t fadeval = 190;                                        // Trail behind the LED's. Lower => faster fade.
    uint8_t BPM = 30;


    uint8_t inner = beatsin8(BPM-1, NUM_LEDS/4, NUM_LEDS/4*3);    // Move 1/4 to 3/4
    uint8_t outer = beatsin8(BPM+1, 0, NUM_LEDS-1);               // Move entire length
    uint8_t middle = beatsin8(BPM, NUM_LEDS/3, NUM_LEDS/3*2);   // Move 1/3 to 2/3

    leds[outer]  = CHSV( gHue1    , 200, 255); // CRGB::Aqua;
    leds[middle] = CHSV( gHue1+96 , 200, 255); // CRGB::Purple;
    leds[inner]  = CHSV( gHue1+160, 200, 255); // CRGB::Blue;

    nscale8(leds,NUM_LEDS,fadeval);                             // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);
    FastLED.show();
} // dot_beat()

void juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 30);
    byte dothue = 0;
    for( int i = 0; i < 8; i++) {
        leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
        dothue += 32;
        yield();
    }
    FastLED.show();
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS/2; i++) { //9948
        RIGHT[i]              = ColorFromPalette(palette, gHue1+(i*2), beat-gHue1+(i*10));
        LEFT [NUM_LEDS/2-1-i] = ColorFromPalette(palette, gHue2+(i*2), beat-gHue2+(i*10));
        yield();
    }
    FastLED.show();
}

void blendwave() {
    CRGB clr1, clr2;
    // uint8_t speed, loc1, loc2, ran1, ran2;
    uint8_t speed, loc1;

    speed = beatsin8(6,0,255);

    if(manual && _cycle){
        clr1 = blend(gradient1, gradient2, speed);
        clr2 = blend(gradient2, gradient1, speed);
    }else{
        clr1 = blend(CHSV(beatsin8(3,0,255),255,255), CHSV(beatsin8(4,0,255),255,255), speed);
        clr2 = blend(CHSV(beatsin8(4,0,255),255,255), CHSV(beatsin8(3,0,255),255,255), speed);
    }
    loc1 = beatsin8(10,0,NUM_LEDS-1);

    fill_gradient_RGB(leds, 0, clr2, loc1, clr1);
    fill_gradient_RGB(leds, loc1, clr2, NUM_LEDS-1, clr1);
    FastLED.show();
} // blendwave()


void beatwave() {

    uint8_t wave1 = beatsin8(9, 0, 255);                        // That's the same as beatsin8(9);
    uint8_t wave2 = beatsin8(8, 0, 255);
    uint8_t wave3 = beatsin8(7, 0, 255);
    uint8_t wave4 = beatsin8(6, 0, 255);

    for (int i=0; i<NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, i+wave1+wave2+wave3+wave4, 255, currentBlending);
        yield();
    }
    FastLED.show();
} // beatwave()
// */

void rainbow_LCM(){
    for(int i = 0; i < NUM_LEDS/2; i++){
        // leds[i]=CHSV((millis()/50*i+1) % (255) + gHue1, 255, 255);
        RIGHT[i]              = CHSV((millis()/71*i+1) % (255) + gHue1, 255, 255);
        LEFT [NUM_LEDS/2-1-i] = CHSV((millis()/73*i+1) % (255) + gHue1, 255, 255);
        // LEFT [i] = RIGHT[NUM_LEDS/2-1-i];
    }
    FastLED.show();
}

////////// MSGEQ7 code //////////
void setupMSGEQ7(){
    pinMode(res, OUTPUT); digitalWrite(res, LOW);
    pinMode(strobe, OUTPUT); digitalWrite(strobe, HIGH);

    for(int i = 0; i <= BANDS; i++){
        values[i] = 0;
    }
}

void readMSGEQ7(){
    if(soundresponsive){
        digitalWrite(res, HIGH);
        digitalWrite(res, LOW);
        values[7] = 0;
        total=0;
        String eq = "E";
        for(int i = 0; i < BANDS; i++){
            delayMicroseconds(100);      // allow output to settle
            digitalWrite( strobe, LOW ); // pulse strobe to read next band
            delayMicroseconds(30);       // allow output to settle
            delay(3);
            values[i] = analogRead(0);   // read band
            // values[i] = values[i] < 150 ? 0 : values[i];
            eq += ",";
            eq += String(values[i]);
            values[i] = (values[i]*values[i])/1024.0;
            total+=values[i];
            yield();
            values1[i] = values[i]/1024.0 * 255;
            values1[7] += values[i] / 7;
            digitalWrite( strobe, HIGH );
            yield();
        }
        // yield();
        EVERY_N_MILLISECONDS( 20 ) {
            webSocket.broadcastTXT(eq);
        }
    }
}
////////// end MSGEQ7 code //////////

void handleESPval(){
    String temp = WSdata.substring(1);
    if(WSdata.startsWith("next")){
        nextPattern();
        // autoadv = false;
        return;
    }
    if(WSdata.startsWith("M")){
        manual = WSdata.endsWith("2") ? true : false;
        soundresponsive = WSdata.endsWith("0") ? true : false;
        gCurrentPatternNumber = 0;
        if(!soundresponsive)
            FastLED.setBrightness(50);
        else
            FastLED.setBrightness(255);
    }if(WSdata.startsWith("V")){
        int x = temp.toInt();
        x = (x*x)/255.0;
        FastLED.setBrightness(x);
        gradient1.v = x;
        gradient2.v = x;
        manualHSV.v = x;
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
            gHue1 = gHue1/255.0*224;
            manualHSV.h = gHue1;
            manualColor = manualHSV;
        }else if(WSdata.startsWith("S")){
            manualHSV.s = temp.toInt();
            manualColor = manualHSV;
        }else if(WSdata.startsWith("Y")){
            gradient1.h = temp.toInt();
        }else if(WSdata.startsWith("Z")){
            gradient2.h = temp.toInt();
        }
        
        if (WSdata.startsWith("Y") || WSdata.startsWith("Z")){
            _cycle = true;
        }else if (!WSdata.startsWith("V") && !WSdata.startsWith("M")){
            _cycle = false;
        }
        if(!_cycle){
            fill_solid (leds, NUM_LEDS, manualColor);
            FastLED.show();
        }
    }
    WSdata = "";
}

