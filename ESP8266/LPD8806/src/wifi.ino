#include <ESP8266WiFi.h>          // WiFi access
#include <WebSockets.h>
#include <ESP8266WebServer.h>     // web server setup
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Hash.h>

// #include <AppleMidi.h>
// APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h
bool isConnected = false;
CRGB lastPressed;

ESP8266WebServer server(80); const char* host = "ledstrip";
File fsUploadFile;

const char* ssid     = "linksys1";
const char* password = "9182736450";

IPAddress timeServer(148, 251, 69, 45); // in.pool.ntp.org
#define SECS_PER_HOUR 3600
#define timeZone 5.5 * SECS_PER_HOUR;     // Central European Time
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

void wifiSetup(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    beginServer();
    
    // Udp.begin(localPort);
    // setSyncProvider(getNtpTime);

    // AppleMIDI.begin("test");

    // AppleMIDI.OnConnected(OnAppleMidiConnected);
    // AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

    // AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
    // AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);
}

void wifiLoop(){
    // if(WiFi.status() == WL_CONNECTED){
        // EVERY_N_SECONDS(5){
            // String wifiInfo = "";
            // wifiInfo += "\r\nWiFi connected!\r\n";
            // wifiInfo += "\r\nNetwork SSID: " + WiFi.SSID();
            // wifiInfo += "\r\nIP address:" + (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1] + "." + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3] + "\r\n";
            // Serial.print(wifiInfo);
        // }
    // }
    server.handleClient();
    ArduinoOTA.handle();
    // EVERY_N_SECONDS(1){
        // if(timeStatus() == timeSet){
            // digitalClockDisplay();
            // Serial.println(elapsedSecsToday(now()));
        // }
    // }
    // AppleMIDI.run();
    // EVERY_N_SECONDS(1){
        // byte note = 45;
        // byte velocity = 55;
        // byte channel = 1;

        // AppleMIDI.sendNoteOn(note, velocity, channel);
        // AppleMIDI.sendNoteOff(note, velocity, channel);
    // }
}

void setupOTA(){
    // ArduinoOTA.setHostname("ledstrip");
    ArduinoOTA.setPort(8266);

    ArduinoOTA.onStart([]() {
         // Serial.setDebugOutput(true);
        String type;
       if (ArduinoOTA.getCommand() == U_FLASH){
            type = "sketch";
        } else {// U_SPIFFS
            type = "filesystem";
        }

        Serial.println("Start updating " + type);
        fill_solid (leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
        Serial.setDebugOutput(false);
        fill_solid (leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        uint32_t temp = progress / (total / 100);
        Serial.printf("Progress: %u%%\r", temp);
        // leds[map(temp, 0, 100, 0, NUM_LEDS)] = 0x111111;
        leds(0, map(temp, 0, 100, 0, NUM_LEDS)).fill_rainbow(temp/100.0*255);
        FastLED.show();

    });
    ArduinoOTA.onError([](ota_error_t error) {
        fill_solid (leds, NUM_LEDS, CRGB::Red);
        FastLED.show();
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
        Serial.setDebugOutput(false);
    });
    ArduinoOTA.begin();
}


//////////////////////////////////////////////
// WIFI
//////////////////////////////////////////////

void beginServer(){
    // server.on("/upload", HTTP_POST, [](){ server.send(200); }, handleFileUpload);
    server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
    server.begin();
    Serial.print("\r\nESP8266 server started\r\n");
    if (MDNS.begin(host)) {
        Serial.print("\r\nMDNS responder started\r\n");
    }
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
    // MDNS.addService("wss", "tcp", 82);
}

void handleNotFound(){ // if the requested file or page doesn't exist, return a 404 not found error
  if(!handleFileRead(server.uri())){          // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
    // unsigned long int getTime = getNtpTime();
    // if(getTime){
        // setTime(getTime);
        // millisSinceLastSync = millis()+(second()*1000);
    // }
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
    String contentType = getContentType(path);             // Get the MIME type
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {    // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
            path += ".gz";                                     // Use the compressed verion
        File file = SPIFFS.open(path, "r");                    // Open the file
        size_t sent = server.streamFile(file, contentType);    // Send it to the client
        file.close();                                          // Close the file again
        Serial.println(String("\tSent file: ") + path);
        return true;
    }
    Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
    return false;
}

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

/////////// NTP code ///////////

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
    while (Udp.parsePacket() > 0) ; // discard any previously received packets
    Serial.println("Transmit NTP Request");
    sendNTPpacket(timeServer);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
            Serial.println("Receive NTP Response");
            Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + timeZone;
        }
    }
    Serial.println("No NTP Response :-(");
    return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:                 
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}
/*  * /
void digitalClockDisplay(){
    // digital clock display of the time
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.print(" ");
    Serial.print(day());
    Serial.print(".");
    Serial.print(month());
    Serial.print(".");
    Serial.print(year()); 
    Serial.println(); 
}

void printDigits(int digits){
  // utility for digital clock display: prints preceding colon and leading 0
    Serial.print(":");
    if(digits < 10)
        Serial.print('0');
    Serial.print(digits);
}
/*  */
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
}