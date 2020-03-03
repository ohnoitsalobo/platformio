#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
// #include <WebSockets.h>

void setupOTA(){
    ArduinoOTA.setPort(8266);
    
    ArduinoOTA.onStart([]() {
        Serial.println("\nReceiving network update, start ... ");
        fill_solid (leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nUpdate successful!");
        fill_solid (leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        uint32_t temp = progress / (total / 100);
        Serial.printf("Progress: %u%%\r", temp);
        leds[map(temp, 0, 100, 0, NUM_LEDS)] = 0x050505;
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

void setupWifi(){
#ifdef DEBUGGER
    Serial.print("\nStarting WiFi ... ");   
#endif
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    // WiFi.softAP("keyboard", "password");
    // WiFi.begin("linksys1", "9182736450");
    WiFi.begin("HaikuJAM GF", "jeetkumar");
#ifdef DEBUGGER
    Serial.print("setting up mDNS ...");
#endif
    if (MDNS.begin(sessionID)) {
        Serial.print("\r\nMDNS responder started, hostname: ");
        Serial.println(sessionID);
    }
    MDNS.addService("http", "tcp", 80);
#ifdef DEBUGGER
    Serial.print("Setting up OTA ... ");
#endif
    setupOTA();
#ifdef DEBUGGER
    Serial.print("done!\n");
#endif
}

void runWifi(){
#ifdef DEBUGGER
    Serial.print("WiFi loop ... ");
    // if(WiFi.status() == WL_CONNECTED){
        // EVERY_N_SECONDS(5){
            // String wifiInfo = "";
            // wifiInfo += "\r\nNetwork SSID: " + WiFi.SSID();
            // wifiInfo += "\r\nIP address:" + (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1] + "." + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3] + "\r\n";
            // Serial.print(wifiInfo);
        // }
    // }
#endif
    ArduinoOTA.handle();
    yield();
#ifdef DEBUGGER
    Serial.print("done.\n");
#endif
}

