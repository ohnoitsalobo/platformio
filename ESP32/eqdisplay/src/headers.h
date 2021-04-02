#include <SPIFFS.h>
#include <FS.h>
File fsUploadFile;

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
const char* ssid = "linksys1";
const char* password = "9182736450";
// const char* ssid = "Home";
// const char* password = "12345678";

#include <TelnetStream.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
WebServer server(80); const char* host = "eqdisplay";
WebSocketsServer webSocket(81);    // create a websocket server on port 81
bool connectedClient = 0;
String WSdata = "";

#include <WiFiUdp.h>
#include <TimeLib.h>
static const char ntpServerName[] = "in.pool.ntp.org";
const double timeZone = 5.5; // IST

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

// #define blynk_en 1
#ifdef blynk_en
#include <BlynkSimpleEsp32.h>
#endif

#include <arduinoFFT.h>
// #define FASTLED_ALLOW_INTERRUPTS 0
// #define INTERRUPT_THRESHOLD 1
// #define FASTLED_INTERRUPT_RETRY_COUNT 0
// #define FASTLED_ESP32_FLASH_LOCK 1
#define FASTLED_INTERNAL
#include <FastLED.h>
#define NUM_LEDS 142
bool _auto = 0;
bool manual = 0;
bool music = 1;
bool FFTenable = true;

#define APPLEMIDI_INITIATOR
#include <AppleMIDI.h>
bool MIDIconnected = false;
byte MIDIdata[] = {0, 0, 0};
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?
bool MidiEventReceived = false;
CRGB lastPressed;  // holder for last-detected key color


// APPLEMIDI_CREATE_DEFAULTSESSION_ESP32_INSTANCE();
// #define APPLEMIDI_CREATE_DEFAULTSESSION_ESP32_INSTANCE() \
// APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "ESP32", DEFAULT_CONTROL_PORT);
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, host, DEFAULT_CONTROL_PORT);
