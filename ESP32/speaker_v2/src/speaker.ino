#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <TelnetStream.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#define _serial_ Serial_1

#define NUMBER_OF_LEDS 66

// #define debug 1

HardwareSerial Serial_1(2);

bool music = 1;
bool FFTenable = true;
String WSdata = "";

#define APPLEMIDI_INITIATOR
#include <AppleMIDI.h>
USING_NAMESPACE_APPLEMIDI
// unsigned long t0 = millis();
bool isConnected = false;
byte MIDIdata[] = {0, 0, 0};
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?
bool MidiEventReceived = false;

// APPLEMIDI_CREATE_DEFAULTSESSION_ESP32_INSTANCE();
// #define APPLEMIDI_CREATE_DEFAULTSESSION_ESP32_INSTANCE() \
// APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "ESP32", DEFAULT_CONTROL_PORT);
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "speaker", DEFAULT_CONTROL_PORT);

/////////// DUAL CORE ///////////

TaskHandle_t Task0;

void core0_Task0( void * parameter )
{
  for (;;) {
    long now = micros();
    
    if(FFTenable){
        fftLoop();
        delay(1);
    }else{
        delay(100);
    }
    
if(millis()%500 > 450){
    long t = micros() - now;
    _serial_.print("Loop time : ");
    _serial_.print(t);
    _serial_.print(" us (");
    _serial_.print(1000000.0/t);
    _serial_.print("Hz)\t\r");
}
  }
}

/////////// DUAL CORE ///////////

void setup(){

    pinMode(2, OUTPUT);
    
    // Serial.begin(115200);
    Serial_1.begin(115200);

    setupWiFi();
    
    fftSetup();
    
    ledSetup();
    
    MIDIsetup();

    TelnetStream.begin();
    
    // _serial_.print("\nCPU is running at ");
    // _serial_.print(getCpuFrequencyMhz());
    // _serial_.print(" MHz\n\n");
    
    xTaskCreatePinnedToCore(
        core0_Task0,    /* Task function. */
        "core0Task0",   /* name of task. */
        1000,           /* Stack size of task */
        NULL,           /* parameter of the task */
        10,              /* priority of the task */
        &Task0,         /* Task handle to keep track of created task */
        0               /* pin task to core 0 */
    );
    delay(500);  // needed to start-up task1
}

void loop(){
#ifdef debug
    _serial_.println("Starting loop");
#endif
    
    wifiLoop();
    
    MIDIloop();

    ledLoop();
    
#ifdef debug
    _serial_.println("Ending loop");
#endif
}


void MIDIsetup(){
    MIDI.begin(1); // listen on channel 1
    AppleMIDI.setHandleConnected(OnAppleMidiConnected);
    AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);
    AppleMIDI.setHandleError(OnAppleMidiError);

    MIDI.setHandleNoteOn(handleNoteOn);
    // MIDI.setHandleNoteOff(handleNoteOff);

    // MDNS.begin(AppleMIDI.getName());
    MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());
    MDNS.addService("http", "tcp", 80);
    IPAddress remote(192, 168, 1, 4);
    AppleMIDI.sendInvite(remote); // port is 5004 by default
}

void MIDIloop(){
    MIDI.read();
}

bool MIDIconnected(){
    return isConnected;
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(const ssrc_t & ssrc, const char* name) {
    isConnected = true;
    Serial.print(F("Connected to session "));
    Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(const ssrc_t & ssrc) {
    isConnected = false;
    Serial.println(F("Disconnected"));
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiError(const ssrc_t& ssrc, int32_t err) {
    Serial.print  (F("Exception "));
    Serial.print  (err);
    Serial.print  (F(" from ssrc 0x"));
    Serial.println(ssrc, HEX);

    switch (err){
        case Exception::NoResponseFromConnectionRequestException:
            Serial.println(F("xxx:yyy did't respond to the connection request. Check the address and port, and any firewall or router settings. (time)"));
        break;
        }
    }

// -----------------------------------------------------------------------------
// 
// -----------------------------------------------------------------------------
// static void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
    // Serial.print(F("Incoming NoteOn  from channel: "));
    // Serial.print(channel);
    // Serial.print(F(", note: "));
    // Serial.print(note);
    // Serial.print(F(", velocity: "));
    // Serial.println(velocity);
// }

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// static void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
    // Serial.print(F("Incoming NoteOff from channel: "));
    // Serial.print(channel);
    // Serial.print(F(", note: "));
    // Serial.print(note);
    // Serial.print(F(", velocity: "));
    // Serial.println(velocity);
// }

static const char ntpServerName[] = "us.pool.ntp.org";
const double timeZone = 5.5; // IST

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
