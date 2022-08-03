void setupWifi(){
    
    WiFi.mode(WIFI_STA);
    WiFi.config(_ip, _gateway, _netmask);
    WiFi.begin(ssid, password);
    
    if (MDNS.begin(hostname.c_str())) {
        Serial.print("\r\nMDNS responder started, hostname: ");
        Serial.println(hostname);
    }
    MDNS.addService("http", "tcp", 80);
    
    setupOTA();
    
    setupVirtuino();
    
}

void runWifi(){
    if(WiFi.status() == WL_CONNECTED){
        ArduinoOTA.handle();
        virtuinoRun();        // Necessary function to communicate with Virtuino. Client handler
        handleE131();
    }
    if(WiFi.status() != WL_CONNECTED){
        EVERY_N_SECONDS(10){
            WiFi.config(_ip, _gateway, _netmask);
            WiFi.begin(ssid, password);
        }
    }
    yield();
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
        FastLED.setBrightness(100);
        FastLED.clear();
        FastLED.show();
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
        FastLED.clear();
        FastLED.show();
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        int temp = progress / (total / 100);
        Serial.printf("Progress: %u%%\r", temp);
        leds[map(temp, 0, 100, 0, NUM_LEDS)] = 0x111111;
        FastLED.show();
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

void setupE131(){
    // Choose one to begin listening for E1.31 data
    if (e131.begin(E131_UNICAST))                               // Listen via Unicast
    // if (e131.begin(E131_MULTICAST, UNIVERSE, UNIVERSE_COUNT))   // Listen via Multicast
        Serial.println(F("Listening for data..."));
    else 
        Serial.println(F("*** e131.begin failed ***"));
}

void handleE131(){
    if (!e131.isEmpty()) {
        e131_packet_t packet;
        e131.pull(&packet);     // Pull packet from ring buffer
        
        // telnet.print("Universe %u / %u Channels | Packet#: %u / Errors: %u / CH1: %u\n",
                // htons(packet.universe),                 // The Universe for this packet
                // htons(packet.property_value_count) - 1, // Start code is ignored, we're interested in dimmer data
                // e131.stats.num_packets,                 // Packet counter
                // e131.stats.packet_errors,               // Packet error counter
                // packet.property_values[1]);             // Dimmer data for Channel 1
    }
}