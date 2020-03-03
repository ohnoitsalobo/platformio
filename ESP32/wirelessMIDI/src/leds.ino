#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define LED_TYPE    WS2812    // LED strip type
#define COLOR_ORDER GRB       // order of color in data stream
#define LED_PINS    13         // data output pin
#define NUM_LEDS    144        // number of LEDs in strip
#define BRIGHTNESS  255       // overall brightness (0-255)
#define FRAMES_PER_SECOND  120

CRGBArray<NUM_LEDS> leds;
CRGBSet LEFT  (leds (0,            NUM_LEDS/2-1)   );  // < subset containing only left  LEDs
CRGBSet RIGHT (leds (NUM_LEDS/2,   NUM_LEDS)       );  // < subset containing only right LEDs
CRGB lastPressed;             // holder for last-detected key color
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?

uint8_t gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns

void setupLED(){
#ifdef DEBUGGER
    Serial.println("Setting up LEDs ... ");
#endif
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);
    
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
#ifdef DEBUGGER
    Serial.println("Success!");
#endif
}

void runLED(){
#ifdef DEBUGGER
    Serial.print("LED loop ... ");
#endif
    EVERY_N_MILLISECONDS(20){ 
        fadeToBlackBy( leds, NUM_LEDS, 10); // ( sustain ? 3 : 10) );
    }
    FastLED.show();
    EVERY_N_MILLISECONDS(50){ _hue++; gHue1++; gHue2--;}
#ifdef DEBUGGER
    Serial.println("done.");
#endif
    yield();
}

// void runLED(){ 
    // EVERY_N_MILLISECONDS(20){ _hue++; gHue1++; gHue2--;}
    // int div = 1000, width = 10;
    // int temp = ((millis() / 10) % div) * NUM_LEDS;
    // for(int i = 1; i <= NUM_LEDS + 2*width; i++){
        // int pos = (NUM_LEDS - width + i-1) % NUM_LEDS;
        // if((abs(temp - i*div)) > (width*div))
            // leds[pos] = CRGB::Black;
        // else{
            // int t = width*div - abs(temp - i*div);
            // int val = ((double)t / (width*div))*255.0;
            // leds[pos] = CHSV(_hue, 255, val);
            // /* leds[i-1] = CHSV(_hue, 255, (val*val*val)/(255.0*255.0)); */
        // }
    // }
   // FastLED.show();
   // yield();
// }

// do X when a key is pressed
void handleNoteOn(byte channel, byte pitch, byte velocity) {
    // MIDI note values 0 - 127 
    // 36-96 (for 61-key) mapped to LED 0-60
    // Serial.println(pitch);
    // int temp = map(pitch, 36, 96, 0, NUM_LEDS-1);
    int temp = pitch;
    
    if(temp < 0)
        temp = -temp;                   // if note goes above 60 or below 0
    else if(temp > NUM_LEDS)                  //      reverse it
        temp = NUM_LEDS - (temp%NUM_LEDS);
    
    uint8_t _pitch = map(temp, 0, NUM_LEDS, 0, 224); // map note to color 'hue'
    uint8_t _pos = map(temp, 0, NUM_LEDS, 0, NUM_LEDS-1);
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
    yield();
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
    yield();
}
