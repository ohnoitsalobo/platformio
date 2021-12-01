///////////// WIFI
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
// const char* ssid = "linksys1";
// const char* password = "9182736450";
const char* ssid = "Home";
const char* password = "12345678";
const char* hostname = "speaker";

///////////// SERVER
#define CONFIG_ASYNC_TCP_RUNNING_CORE 0  
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
const char* http_username = "admin";
const char* http_password = "admin";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");
String WSdata = "";

///////////// TIME
#include <WiFiUdp.h>
#include <TimeLib.h>
static const char ntpServerName[] = "in.pool.ntp.org";
const double timeZone = 5.5; // IST

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);


///////////// FILESYSTEM
#include <SPIFFS.h>
#include <FS.h>
File fsUploadFile;

///////////// FastLED
// #define FASTLED_ALLOW_INTERRUPTS 0
// #define FASTLED_INTERRUPT_RETRY_COUNT 1
#define FASTLED_INTERNAL        // suppress #pragma messages
#include <Adafruit_I2CDevice.h>
#include <FastLED.h>

#define LED_TYPE     WS2812B    // LED strip type
#define COLOR_ORDER  GRB        // order of color in data stream
#define LED_PINS     13          // data output pin
#define NUM_LEDS     66         // number of LEDs in strip
#define half_size    NUM_LEDS/2
#define quarter_size NUM_LEDS/4
#define half_sizef    NUM_LEDS/2.0
#define quarter_sizef NUM_LEDS/4.0
#define _bright      150
#define BRIGHTNESS   _bright*_bright/255
// scale the brightness non-linearly which looks better because our eyes detect light intensity logarithmically
CRGBArray<NUM_LEDS> _leds_;
CRGBSet leds(_leds_, NUM_LEDS);
CRGBSet RIGHT(leds (0,            half_size-1) );  // < subset containing only right LEDs
CRGBSet LEFT (leds (half_size,   NUM_LEDS-1)   );  // < subset containing only left  LEDs

CRGBSet R1  (RIGHT (0,            quarter_size-1) );  // < subset 
CRGBSet R2  (RIGHT (quarter_size,   half_size-1) );  // < subset 
CRGBSet L1  (LEFT  (0,            quarter_size-1) );  // < subset 
CRGBSet L2  (LEFT  (quarter_size,   half_size-1) );  // < subset 
struct CRGB * led_array[] = { LEFT, RIGHT };
struct CRGB * led_array1[] = { L1, L2, R1, R2 };

uint8_t _hue = 0;             // modifier for color cycling
uint8_t gHue = 0, gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns
CRGB manualColor = 0x000000, manualColor_L = 0x000000, manualColor_R = 0x000000;
CHSV manualHSV (0, 255, 255);
uint8_t currentBrightness = BRIGHTNESS, _setBrightness = BRIGHTNESS;
uint8_t auto_advance = 0;
#include "firework.h"

enum mode_select {   // self-explanatory
    _audio,             // audio spectrum mode
    _auto,              // automatic patterns mode
    _manual,            // manual control mode
    _midi               // midi responsiveness mode
};
mode_select _mode = _audio; // initialize with audio spectrum

///////////// FFT
#include <arduinoFFT.h>
#include "FFT.h"
#include "dualCore.h"

///////////// MIDI
#include <AppleMIDI.h>                                                               
APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();
unsigned long t0 = millis();
int8_t isConnected = 0;  // counts how many wireless MIDI clients are connected
uint8_t midiNotes [127];
uint8_t _lastPressed;             // holder for last-detected key color
CRGB lastPressed;             // holder for last-detected key color
bool sustain = false;         // is sustain pedal on?

uint32_t lastClock = 0, currentClock = 0, _BPM = 60;  // counter to detect tempo
byte ticks = 0;                                       // 24 ticks per 1/4 note "crotchet"

uint32_t timeSinceLastMIDI = 0; // amount of time since the last MIDI input was detected
                                // used to switch to automatic patterns when no input


///////////// VIRTUINO
#include <VirtuinoCM.h>
VirtuinoCM virtuino;               
#define V_memory_count 32          // the size of V memory. You can change it to a number <=255)
int V[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.
int V_prev[V_memory_count];
// #define debug 1
WiFiServer virtuinoServer(8000);                   // Default Virtuino Server port 
