#define LED_TYPE    WS2812    // LED strip type
#define COLOR_ORDER GRB       // order of color in data stream
#define LED_PINS    13         // data output pin
#define NUM_LEDS    72        // number of LEDs in strip
#define BRIGHTNESS  255       // overall brightness (0-255)
#define FRAMES_PER_SECOND  120

byte MIDIdata[] = {0, 0, 0};
CRGBArray<NUM_LEDS> leds;
CRGBSet LEFT  (leds (0,            NUM_LEDS/2-1)   );  // < subset containing only left  LEDs
CRGBSet RIGHT (leds (NUM_LEDS/2,   NUM_LEDS)       );  // < subset containing only right LEDs
CRGB lastPressed;             // holder for last-detected key color
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?

uint8_t gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns

void setupLED(){
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);
    
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

void runLED(){
    EVERY_N_MILLISECONDS(50){ _hue++; gHue1++; gHue2--;}
    EVERY_N_MILLISECONDS(20){ 
        fadeToBlackBy( leds, NUM_LEDS, 10); // ( sustain ? 3 : 10) );
    }
    if(MidiEventReceived)
        MIDI2LED();
    FastLED.show();
    yield();
}

// do X when a key is pressed
void handleNoteOn(byte channel, byte pitch, byte velocity) {
    MIDIdata[0] = channel;
    MIDIdata[1] = pitch;
    MIDIdata[2] = velocity;
    MidiEventReceived = true;
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
    uint8_t _pos = MIDIdata[1]/127.0 * (NUM_LEDS-1); // map note to position
    uint8_t _col = MIDIdata[1]/127.0 * 224; // map note to position
    
    // uint8_t _pos = map(temp, 0, NUM_LEDS, 0, NUM_LEDS-1);
    // assign color based on note position and intensity (velocity)
    leds[_pos] = CHSV(_col + _hue, 255 - (MIDIdata[2]/2.0), MIDIdata[2]/127.0 * 255);
    lastPressed = leds[_pos]; // remember last-detected note color
    MidiEventReceived = false;
}

// do X when a key is released
void handleNoteOff(byte channel, byte pitch, byte velocity) {
    MIDIdata[0] = channel;
    MIDIdata[1] = pitch;
    MIDIdata[2] = velocity;
    MidiEventReceived = true;
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
