#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
const char* ssid = "linksys1";
const char* password = "9182736450";

String hostname = "keyboard-MODX";

#include <MIDI.h>
// MIDI_CREATE_DEFAULT_INSTANCE();
MIDI_CREATE_INSTANCE(HardwareSerial, Serial,  MIDI);

#include <AppleMIDI.h>
// APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI_W, hostname.c_str(), DEFAULT_CONTROL_PORT);
unsigned long t0 = millis();
int8_t isConnected = 0;

uint16_t lastClock = 0, currentClock = 0, _BPM = 60;
byte ticks = 0;

#if __cplusplus > 199711L
#define register      // Deprecated in C++11.
#endif  // #if __cplusplus > 199711L

#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 1
// #define FASTLED_INTERNAL
#include <FastLED.h>

#define LED_TYPE    WS2812    // LED strip type
#define COLOR_ORDER GRB       // order of color in data stream
#define LED_PINS    2         // data output pin
#define NUM_LEDS    56        // number of LEDs in strip
#define _bright_    255        // number of LEDs in strip
#define BRIGHTNESS  _bright_*_bright_/255     // overall brightness (0-255)
CRGBArray<NUM_LEDS> leds;
CRGBSet LEFT  (leds (0,            NUM_LEDS/2-1)   );  // < subset containing only left  LEDs
CRGBSet RIGHT (leds (NUM_LEDS/2,   NUM_LEDS)       );  // < subset containing only right LEDs
CRGB lastPressed;             // holder for last-detected key color
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?
uint8_t gHue = 0, gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns

bool _midi   = true;
bool _auto   = false;
bool _manual = false;
CRGB manualColor = 0x000000, manualColor_L = 0x000000, manualColor_R = 0x000000;
CHSV manualHSV (0, 255, 255);
uint8_t currentBrightness = BRIGHTNESS, _setBrightness = BRIGHTNESS;


////////////////////////////  VIRTUINO FUNCTIONS  //////////////////////////////

#include <VirtuinoCM.h>
VirtuinoCM virtuino;               
#define V_memory_count 32          // the size of V memory. You can change it to a number <=255)
float V[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.
float V_prev[V_memory_count];
// #define debug 1
WiFiServer virtuinoServer(8000);                   // Default Virtuino Server port 


////////////////////////////  END VIRTUINO FUNCTIONS  //////////////////////////////

