#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
String hostname = "smart-socket";

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
char blynkAuth[] = "JEJrKUpLU6cadxQdBGJ8E4oLTyxDSYsK";
IPAddress blynkServer(192,168,0,200);
int blynkPort = 8080;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    WiFiManager wifiManager;
    wifiManager.autoConnect(hostname.c_str());
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    // hostname = "smart-socket-" + String(ESP.getChipId()).c_str();
    hostname = hostname + "-" + String(ESP.getChipId());
    if (MDNS.begin(hostname.c_str())) {
        Serial.print("\r\nMDNS responder started, hostname: ");
        Serial.println(hostname);
    }
    MDNS.addService("http", "tcp", 80);

    setupOTA();

    Blynk.config(blynkAuth, blynkServer, blynkPort);
    
    //if you get here you have connected to the WiFi
    Serial.println("connected :)");
}

void loop() {
    // put your main code here, to run repeatedly:
    Blynk.run();
    ArduinoOTA.handle();
}

void setupOTA(){
    ArduinoOTA.setHostname(hostname.c_str());

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
        } else { // U_FS
        type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
}

