const char* ssid = "linksys1";
const char* password = "9182736450";

void setupWiFi(){
    Serial.println("\nStarting Wifi");

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    setupOTA();
}
    
void setupOTA(){
    ArduinoOTA.setPort(3232);

    ArduinoOTA.setHostname("ESP32");

    ArduinoOTA
        .onStart([]() {
            pinMode(2, OUTPUT); digitalWrite(2, HIGH);
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
            else // U_SPIFFS
            type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("\nStart updating " + type);
            // fill_solid (leds, NUM_LEDS, CRGB::Black);
            // FastLED.show();
        })
        .onEnd([]() {
            digitalWrite(2, LOW);
            Serial.println("\nEnd");
            delay(10);
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            uint32_t temp = progress / (total / 100);
            digitalWrite(2, !digitalRead(2));
            // Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
            Serial.printf("Progress: %u%%\r", temp);
            // if(temp<99){
                // fill_solid (leds, map(temp, 0, 99, 0, NUM_LEDS), 0x111111);
                // FastLED.show();
            // }else if(temp == 99){
                // fill_solid (leds, NUM_LEDS, CRGB::Black);
                // FastLED.show();
            // }
        })
        .onError([](ota_error_t error) {
            // fill_solid (leds, NUM_LEDS, CRGB::Red);
            // FastLED.show();
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

    ArduinoOTA.begin();
}

void wifiLoop(){
    if(WiFi.status() == WL_CONNECTED){
        ArduinoOTA.handle();
        // server.handleClient();
        // webSocket.loop();
    }
    if(WiFi.status() != WL_CONNECTED){
        
    }
    yield();
}

