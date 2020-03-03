#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "linksys1";
const char* password = "9182736450";

const char* host = "ESP32_160";

void setupWiFi(){
    // btStop();
    Serial.println("\nStarting Wifi");
    WiFi.disconnect();
    // WiFi.mode(WIFI_AP_STA);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    // WiFi.softAP(APssid, APpassword);
}
    
void setupOTA(){
    // Port defaults to 3232
    ArduinoOTA.setPort(3232);

    // Hostname defaults to esp3232-[MAC]
    ArduinoOTA.setHostname(host);

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA
        .onStart([]() {
            pinMode(2, OUTPUT);
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
            else // U_SPIFFS
            type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("\nStart updating " + type);
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        })
        .onEnd([]() {
            Serial.println("\nEnd");
            delay(10);
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            uint32_t temp = progress / (total / 100);
            digitalWrite(2, HIGH);
            // Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
            Serial.printf("Progress: %u%%\r", temp);
            if(temp<99){
                fill_solid(leds, map(temp, 0, 99, 0, NUM_LEDS), 0x111111);
                FastLED.show();
            }else if(temp >= 99){
                fill_solid (leds, NUM_LEDS, CRGB::Black);
                FastLED.show();
            }
            digitalWrite(2, LOW);
        })
        .onError([](ota_error_t error) {
            fill_solid (leds, NUM_LEDS, CRGB::Red);
            FastLED.show();
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
        // EVERY_N_SECONDS(5){
            // Serial_1.print("\nIP Address: ");
            // Serial_1.println(WiFi.localIP());
        // }
    }
    if(WiFi.status() != WL_CONNECTED){
        // EVERY_N_SECONDS(30){
            // FastLED.setBrightness(music ? 40 : 15 );
            // wifiScan();
            // /* Serial_1.print("\nIP Address: "); */
            // /* Serial_1.println(WiFi.localIP()); */
        // }
    }
}
