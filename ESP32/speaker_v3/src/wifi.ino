
void wifiSetup(){
#ifdef debug
    _serial_.println("\tStarting wifiSetup");
#endif
    WiFi.disconnect();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    setupOTA();

    MDNS.addService("http","tcp",80);

    SPIFFS.begin();

    // setupServer();
    
    timeSetup();
#ifdef debug
    _serial_.println("\tEnding wifiSetup");
#endif
}

void wifiLoop(){
    ArduinoOTA.handle();
    virtuinoRun();
    if(WiFi.status() == WL_CONNECTED){
        EVERY_N_MILLISECONDS(500){
            ws.cleanupClients();
        }
        EVERY_N_SECONDS(5){
            // _serial_.println(WiFi.localIP());
        }
        if(!digitalRead(2)){
            digitalWrite(2, HIGH);
            _serial_.println("Wifi connected!");
        }
    }
    
    if(WiFi.status() != WL_CONNECTED){
        EVERY_N_SECONDS(5){
            _serial_.println("Wifi disconnected");
            WiFi.begin(ssid, password);
        }
        if(digitalRead(2))
            digitalWrite(2, LOW);
    }
    yield();
}

void setupOTA(){
#ifdef debug
    _serial_.println("\t\tStarting setupOTA");
#endif
    ArduinoOTA.setPort(3232);

    ArduinoOTA.setHostname(hostname);
    
    ArduinoOTA
        .onStart([]() {
            digitalWrite(2, HIGH);
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "firmware";
            else{ // U_SPIFFS
                type = "filesystem";
                SPIFFS.end();
            }
            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            events.send("Update Start", "ota");
            _serial_.println("\r\nStart updating " + type);
            
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        })
        .onEnd([]() {
            digitalWrite(2, LOW);
            _serial_.println("\r\nEnd");
            events.send("Update End", "ota");
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
            delay(10);
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            digitalWrite(2, !digitalRead(2));
            char p[32];
            sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
            events.send(p, "ota");
            uint32_t temp = progress / (total / 100);
            _serial_.printf("Progress: %u%%\r", temp);
            if(temp<99){
                int t = map(temp, 0, 99, 0, NUM_LEDS/4);
                fill_solid( RIGHT(           0, NUM_LEDS/4), t, 0x020202);
                fill_solid( RIGHT(NUM_LEDS/2-t, NUM_LEDS/2), t, 0x020202);
                fill_solid( LEFT (           0, NUM_LEDS/4), t, 0x020202);
                fill_solid( LEFT (NUM_LEDS/2-t, NUM_LEDS/2), t, 0x020202);
            }else if(temp >= 99){
                fill_solid (leds, NUM_LEDS, 0x020202);
            }
            FastLED.show();
        })
        .onError([](ota_error_t error) {
            fill_solid (leds, NUM_LEDS, CRGB::Red);
            FastLED.show();
            _serial_.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)        { _serial_.println("Auth Failed");    events.send("Auth Failed", "ota");    }
            else if (error == OTA_BEGIN_ERROR)  { _serial_.println("Begin Failed");   events.send("Begin Failed", "ota");   }
            else if (error == OTA_CONNECT_ERROR){ _serial_.println("Connect Failed"); events.send("Connect Failed", "ota"); }
            else if (error == OTA_RECEIVE_ERROR){ _serial_.println("Receive Failed"); events.send("Recieve Failed", "ota"); }
            else if (error == OTA_END_ERROR)    { _serial_.println("End Failed");     events.send("End Failed", "ota");     }
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        });
    ArduinoOTA.begin();
#ifdef debug
    _serial_.println("\t\tEnding setupOTA");
#endif
}

//////// TIME //////////

void timeSetup(){
    Udp.begin(localPort);
}

void timeLoop(){
	if(timeStatus() != timeSet){
        setSyncProvider(getNtpTime);
        setSyncInterval(5000);
    }
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
    IPAddress ntpServerIP; // NTP server's ip address

    while (Udp.parsePacket() > 0) ; // discard any previously received packets
    _serial_.println("Transmit NTP Request");
    // get a random server from the pool
    WiFi.hostByName(ntpServerName, ntpServerIP);
    _serial_.print(ntpServerName);
    _serial_.print(": ");
    _serial_.println(ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
            _serial_.println("Receive NTP Response");
            Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
        }
    }
    _serial_.println("No NTP Response :-(");
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
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}