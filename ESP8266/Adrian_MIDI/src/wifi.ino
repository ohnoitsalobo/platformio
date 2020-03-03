#include <ESP8266WiFi.h>   // 
#include <ESP8266mDNS.h>   // ESP8266 stuff
#include <ArduinoOTA.h>

void OTAsetup(){
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname("keyboard");
    
    ArduinoOTA.onStart([]() {
        Serial.println("Start");
        fill_solid (leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
        fill_solid (leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        uint32_t temp = progress / (total / 100);
        Serial.printf("Progress: %u%%\r", temp);
        leds[map(temp, 0, 100, 0, NUM_LEDS)] = 0x111111;
        FastLED.show();
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
}

//////////////////////////////////////////////
// WIFI
//////////////////////////////////////////////

void wifiStuff() {
    Serial.print("\r\nStarting WiFi ... ");
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    // WiFi.softAP("keyboard", "password");
    WiFi.begin("linksys1", "9182736450");
}

void wifiLoop(){
    ArduinoOTA.handle();
    yield();
}