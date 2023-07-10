#ifndef STASSID
#define STASSID "Home"
#define STAPSK  "12345678"
#endif
#define hostname "pickupwinder"

const char* ssid = STASSID;
const char* password = STAPSK;

void setupWiFi(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        _serial_.println("Connection Failed! Rebooting...");
        delay(1000);
        ESP.restart();
    }
}

void runWiFi(){
    
}

void setupOTA(){
    ArduinoOTA.setHostname(hostname);

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        _serial_.println("Start updating " + type);
    });
    
    ArduinoOTA.onEnd([]() {
        _serial_.println("\nEnd");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        _serial_.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        _serial_.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
        _serial_.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
        _serial_.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
        _serial_.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
        _serial_.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
        _serial_.println("End Failed");
        }
    });
    
    ArduinoOTA.begin();
    _serial_.println("Ready");
    _serial_.print("IP address: ");
    _serial_.println(WiFi.localIP());
}

void runOTA(){
    ArduinoOTA.handle();
}