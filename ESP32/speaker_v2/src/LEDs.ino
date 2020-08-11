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

typedef void (*SimplePatternList[])();
// SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, dot_beat, juggle, bpm, inoise8_mover};
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, dot_beat, bpm };
SimplePatternList gPatterns1 = { audio_spectrum, audioLight };
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
    _serial_.println("Starting ledLoop");
#endif
    if(MIDIconnected()){
        runLED();
    }else{
        if(music){
            gPatterns1[gCurrentPatternNumber]();
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
            gPatterns[gCurrentPatternNumber]();
        }
        else if(manual){
            
        }
    }
    FastLED.show();
#ifdef debug
    _serial_.println("Ending ledLoop");
#endif
}

void audio_spectrum(){
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
    fadeToBlackBy( leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS/2);
    // leds[pos] += CHSV( random8(255), 255, 255);
    RIGHT[pos] += CHSV( gHue1 + random8(64), 190+random8(65), 255);
    LEFT [pos] += CHSV( gHue2 + random8(64), 190+random8(65), 255);
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
    fadeToBlackBy(leds, NUM_LEDS, 10);     
    uint8_t locn = 0, pixlen = 0;
    
    locn = inoise8(xscale, dist1+yscale) % 255;          // Get a new pixel location from moving noise.
    pixlen = map(locn,0,255,0,NUM_LEDS/2);                // Map that to the length of the strand.
    LEFT[pixlen] = ColorFromPalette(currentPalette, pixlen, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.

    locn = inoise8(xscale, dist2+yscale) % 255;          // Get a new pixel location from moving noise.
    pixlen = map(locn,0,255,0,NUM_LEDS/2);                // Map that to the length of the strand.
    RIGHT[pixlen] = ColorFromPalette(randomPalette2, pixlen, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.

    dist1 += beatsin8(10,1,4);                                                // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.                                             
    dist2 += beatsin8(13,1,4);                                                // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.                                             
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
