#include <SPIFFS.h>
#include <FS.h>
File fsUploadFile;

#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
const char* ssid = "linksys1";
const char* password = "9182736450";
// const char* ssid = "Home";
// const char* password = "12345678";

// #include <TelnetStream.h>
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
APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();
bool MIDIconnected = false;
byte MIDIdata[] = {0, 0, 0};
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?
bool MidiEventReceived = false;
CRGB lastPressed;  // holder for last-detected key color

////////////////////////////  VIRTUINO FUNCTIONS  //////////////////////////////
#define _virtuino 1
#ifdef _virtuino
#include <VirtuinoCM.h>
VirtuinoCM virtuino;               
#define V_memory_count 32          // the size of V memory. You can change it to a number <=255)
float V[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.
float V_prev[V_memory_count];
#define debug 1
WiFiServer virtuinoServer(8000);                   // Default Virtuino Server port 

////////////////////////////  END VIRTUINO FUNCTIONS  //////////////////////////////
#endif

#define _data  27 // DS   - IC14
#define _clock 26 // SHCP - IC11
#define _latch 25 // STCP - IC12
#include <ShiftRegister74HC595.h>
ShiftRegister74HC595<1> sr(_data, _clock, _latch);
