#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>

#define BANDS 21

String WSdata = "";
File fsUploadFile;
WebServer server(80); const char* host = "ESP32_160";
WebSocketsServer webSocket(81);    // create a websocket server on port 81
bool connectedClient = 0;

bool music = true;          // use to change light patterns if music is playing or not
HardwareSerial Serial_1(2);

void setup(){
    
    Serial.begin(115200);
    Serial_1.begin(115200);
    
    setupWiFi();

    setupOTA();
    
    beginServer();

    startSPIFFS();               // Start the SPIFFS and list all contents
	
    startWebSocket();            // Start a WebSocket server

    LEDsetup();
    
    FFTsetup();
}

void loop(){
    wifiLoop();

    if (music)
        FFTstuff();

    LEDloop();
}

