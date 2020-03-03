#define FASTLED_ALLOW_INTERRUPTS 0
// #define FASTLED_INTERRUPT_RETRY_COUNT 0
#include <FastLED.h>

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
    #warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

// #define LED_TYPE    LPD8806
// #define LED_PINS    2, 3 // 13, 12  
// #define COLOR_ORDER BRG
// #define NUM_LEDS    48
#define LED_TYPE    WS2812    // LED strip type
#define COLOR_ORDER GRB       // order of color in data stream
#define LED_PINS    2         // data output pin
#define NUM_LEDS    60        // number of LEDs in strip
#define BRIGHTNESS  25       // overall brightness (0-255)
#define FRAMES_PER_SECOND  120
// CRGB leds[NUM_LEDS];          // array of LED color values
CRGBArray<NUM_LEDS> leds;
CRGBSet LEFT  (leds (0,            NUM_LEDS/2-1)   );  // < subset containing only left  LEDs
CRGBSet RIGHT (leds (NUM_LEDS/2,   NUM_LEDS)       );  // < subset containing only right LEDs
CRGB lastPressed;             // holder for last-detected key color
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, rainbow_LCM, confetti, sinelon, dot_beat, dot_beat1, juggle, bpm, blendwave, beatwave };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns
bool patterns = false;
CRGB manualColor = 0xFFFF88, endclr, midclr;
CHSV manualHSV (0, 255, 255), gradient1(0, 255, 255), gradient2(0, 255, 255);

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType    currentBlending;


// #include <MIDI.h>                // MIDI library
// MIDI_CREATE_DEFAULT_INSTANCE();  // 
#include <WiFiUdp.h>
#include <AppleMidi.h>
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h
bool isConnected = false;

void LEDsetup(){
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);
    
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

    for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = CHSV((i/double(NUM_LEDS))*255, 255, 50);
    }
    // fill_solid(leds, NUM_LEDS, CRGB::Black);
    // FastLED.show();        

    // MIDI.setHandleNoteOn(handleNoteOn);
    // MIDI.setHandleNoteOff(handleNoteOff);
    // MIDI.setHandlePitchBend(handlePitchBend);
    // MIDI.setHandleControlChange(handleControlChange);
    
    // MIDI.begin(MIDI_CHANNEL_OMNI);

    AppleMIDI.begin("test");

    AppleMIDI.OnConnected(OnAppleMidiConnected);
    AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

    AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
    // AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);
}

void LEDloop(){
    if(patterns){
        EVERY_N_MILLISECONDS( 1000 / FRAMES_PER_SECOND ){  // Call the current pattern function once, updating the 'leds' array
            gPatterns[gCurrentPatternNumber]();
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
        // MIDI.read();
        AppleMIDI.run();
        // EVERY_N_SECONDS(1){
            // byte note = 45;
            // byte velocity = 55;
            // byte channel = 1;

            // AppleMIDI.sendNoteOn(note, velocity, channel);
            // AppleMIDI.sendNoteOff(note, velocity, channel);
        // }
        EVERY_N_MILLISECONDS(20){ 
            // fill_rainbow( leds, NUM_LEDS, _hue, 7);
            // FastLED.show();
            // fadeToBlackBy( leds, NUM_LEDS, fadeval );
            fadeToBlackBy( leds, NUM_LEDS, ( sustain ? 3 : 10) );
        }
        FastLED.show();
    }
    EVERY_N_MILLISECONDS(50){ _hue++; gHue1++; gHue2--;}
    yield();
}

// -----------------------------------------------------------------------------

// do X when a key is pressed
void handleNoteOn(byte channel, byte pitch, byte velocity) {
    // MIDI note values 0 - 127 
    // 36-96 (for 61-key) mapped to LED 0-60
    int temp = map(pitch, 36, 96, 60, 0);
    
    if(temp < 0)
        temp = -temp;                   // if note goes above 60 or below 0
    else if(temp > 60)                  //      reverse it
        temp = 60 - (temp%60);
    
    uint8_t _pitch = map(temp, 0, 60, 0, 224); // map note to color 'hue'
    uint8_t _pos = map(temp, 0, 60, 0, NUM_LEDS-1);
    // assign color based on note position and intensity (velocity)
    leds[_pos] = CHSV(_pitch + _hue, 255 - velocity, velocity/127.0 * 255);
    lastPressed = leds[_pos]; // remember last-detected note color
    yield();
}

// do X when a key is released
void handleNoteOff(byte channel, byte pitch, byte velocity) {
    yield();
}

// do X when pitch bend is used
void handlePitchBend(byte channel, int bend) {
    // fill strip with solid color based on pitch bend amount
    fill_solid(leds, NUM_LEDS, CHSV(map(bend, -8192, 8192, 0, 224), 255, 125)); // 0  8192  16383
    // FastLED.show();
}

// do X when control channels are used
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
    // FastLED.show();
}


#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
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

    clr1 = blend(CHSV(beatsin8(3,0,255),255,255), CHSV(beatsin8(4,0,255),255,255), speed);
    clr2 = blend(CHSV(beatsin8(4,0,255),255,255), CHSV(beatsin8(3,0,255),255,255), speed);
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

// BLYNK_WRITE(V1)
// {
    // patterns = param.asInt();
    // if(patterns)
        // FastLED.setBrightness(25);
    // else
        // FastLED.setBrightness(255);
// }

// BLYNK_WRITE(V2)
// {
    // nextPattern();
// }

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, char* name) {
  isConnected  = true;
  Serial.print(F("Connected to session "));
  Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  isConnected  = false;
  Serial.println(F("Disconnected"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
  // Serial.print(F("Incoming NoteOn from channel:"));
  // Serial.print(channel);
  // Serial.print(F(" note:"));
  // Serial.print(note);
  // Serial.print(F(" velocity:"));
  // Serial.print(velocity);
  // Serial.println();
    int temp = map(note, 36, 96, 0, NUM_LEDS-1);
    
    if(temp < 0)
        temp = -temp;                   // if note goes above 60 or below 0
    else if(temp > NUM_LEDS-1)                  //      reverse it
        temp = NUM_LEDS-1 - (temp%NUM_LEDS-1);
    
    uint8_t _pitch = map(temp, 0, NUM_LEDS-1, 0, 224); // map note to color 'hue'
    uint8_t _pos = map(temp, 0, NUM_LEDS-1, 0, NUM_LEDS-1);
    // assign color based on note position and intensity (velocity)
    leds[_pos] = CHSV(_pitch + gHue1, 255 - velocity, velocity/127.0 * 255);
    // leds[_pos] = CHSV(_pitch + gHue1, 255 - velocity, (velocity*velocity)/(127.0*127.0) * 255);
    lastPressed = leds[_pos]; // remember last-detected note color
    yield();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOff from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
  yield();
}