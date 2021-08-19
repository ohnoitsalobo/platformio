  
String hostname = "ESP8266";

void setupWifi(){
    WiFiManager wifiManager;
    wifiManager.setSTAStaticIPConfig(ip, gateway, subnet);
    wifiManager.autoConnect(hostname.c_str());

    hostname = hostname + "-" + String(ESP.getChipId());
    if (MDNS.begin(hostname.c_str())) {
        Serial.print("\r\nMDNS responder started, hostname: ");
        Serial.println(hostname);
    }
    MDNS.addService("http", "tcp", 80);
    
    setupOTA();
    
    virtuino.begin(onReceived,onRequested,256);  // Start Virtuino. Set the buffer to 256. With this buffer Virtuino can control about 28 pins (1 command = 9bytes) The T(text) commands with 20 characters need 20+6 bytes
    virtuino.key="1234";                         // This is the Virtuino password. Only requests the start with this key are accepted from the library
    virtuinoServer.begin();

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