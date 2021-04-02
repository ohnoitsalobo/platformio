void setupWiFi(){
    _serial_.println("\nStarting Wifi");

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    setupOTA();
    
    startSPIFFS();
    
    beginServer();
    
    startWebSocket();
    
    timeSetup();
    
#ifdef blynk_en
	blynkSetup();
#endif
}
    
void setupOTA(){
    ArduinoOTA.setPort(3232);

    ArduinoOTA.setHostname(host);
    
    ArduinoOTA
        .onStart([]() {
            digitalWrite(2, HIGH);
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else{ // U_SPIFFS
                type = "filesystem";
                SPIFFS.end();
            }

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            _serial_.println("\r\nStart updating " + type);
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        })
        .onEnd([]() {
            digitalWrite(2, LOW);
            _serial_.println("\r\nEnd");
            delay(10);
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            uint32_t temp = progress / (total / 100);
            digitalWrite(2, !digitalRead(2));
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
                 if (error == OTA_AUTH_ERROR)    { _serial_.println("Auth Failed");    }
            else if (error == OTA_BEGIN_ERROR)   { _serial_.println("Begin Failed");   }
            else if (error == OTA_CONNECT_ERROR) { _serial_.println("Connect Failed"); }
            else if (error == OTA_RECEIVE_ERROR) { _serial_.println("Receive Failed"); }
            else if (error == OTA_END_ERROR)     { _serial_.println("End Failed");     }
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        });
    ArduinoOTA.begin();
}

void wifiLoop(){
#ifdef debug
    _serial_.println("Starting wifiLoop");
#endif

    if(WiFi.status() == WL_CONNECTED){
        ArduinoOTA.handle();
        server.handleClient();
        webSocket.loop();
        // EVERY_N_SECONDS(1){
            // webSocket.cleanupClients();
        // }
#ifdef blynk_en
        blynkLoop();
#endif
        if(!digitalRead(2)){
            digitalWrite(2, HIGH);
            _serial_.println("Wifi connected!");
        }
    }
    
    if(WiFi.status() != WL_CONNECTED){
        EVERY_N_SECONDS(10){
            _serial_.println("Wifi disconnected");
            WiFi.begin(ssid, password);
        }
        if(digitalRead(2))
            digitalWrite(2, LOW);
    }
    yield();
#ifdef debug
    _serial_.println("Ending wifiLoop");
#endif
}

void beginServer(){
    // server.on("/upload", HTTP_POST, [](){ server.send(200); }, handleFileUpload);
    server.on("/reset", []() {
        server.send(200, "text/html", "Restart ESP32<br /><br /><a href=\"http:\\\\speaker.local\">Click to go to speaker LED control</a>");
        ESP.restart();
    });
    server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
    server.begin();
    _serial_.print("\r\nServer started\r\n");
    if (MDNS.begin(host)) {
        _serial_.print("\r\nMDNS responder started\r\n");
    }
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
}

void handleNotFound(){ // if the requested file or page doesn't exist, return a 404 not found error
    if(!handleFileRead(server.uri())){          // check if the file exists in the flash memory (SPIFFS), if so, send it
        server.send(404, "text/plain", "404: File Not Found");
    }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
    _serial_.println("handleFileRead: " + path);
    if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
    String contentType = getContentType(path);             // Get the MIME type
    String pathWithGz = path + ".gz";
    size_t sent;
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
            path += ".gz";                                         // Use the compressed verion
        File file = SPIFFS.open(path, "r");                    // Open the file
        sent = server.streamFile(file, contentType);    // Send it to the client
        file.close();                                          // Close the file again
        _serial_.println(String("\tSent file: ") + path);
        return true;
    }
    _serial_.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
    return false;
}

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

////////////////////////////////////

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    _serial_.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        _serial_.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        _serial_.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            _serial_.print("  DIR : ");
            _serial_.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            _serial_.print("  FILE: ");
            _serial_.print(file.name());
            _serial_.print("\tSIZE: ");
            _serial_.println(file.size());
        }
        file = root.openNextFile();
    }
}

void startSPIFFS() { // Start the SPIFFS and list all contents
    if(!SPIFFS.begin()){
        _serial_.println("SPIFFS Mount Failed");
        return;
    }
    
    listDir(SPIFFS, "/", 0);

}

void startWebSocket() { // Start a WebSocket server
    webSocket.begin();                          // start the websocket server
    webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
    _serial_.println("WebSocket server started.");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) { // When a WebSocket message is received
    switch(type) {
    case WStype_DISCONNECTED:
        _serial_.printf("[%u] Disconnected!\r\n", num);
        connectedClient = 0;
        break;
    case WStype_CONNECTED:
        {
            IPAddress ip = webSocket.remoteIP(num);
            _serial_.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket.sendTXT(num, "Connected");
            connectedClient = 1;
        }
        break;
    case WStype_TEXT:
        _serial_.printf("[%u] get Text: %s\r\n", num, payload);
        WSdata = "";
        for(int i = 0; i < length; i++)
            WSdata += String(char(payload[i])); 
        // _serial_.println(WSdata);
        if(WSdata.startsWith("!")){
            wifiText = "";
            String temp = WSdata.substring(18);
            if(temp.length() < 3)
                break;
            uint8_t r1 = strtoul(WSdata.substring( 3,  5).c_str(), NULL, 16);
            uint8_t g1 = strtoul(WSdata.substring( 5,  7).c_str(), NULL, 16);
            uint8_t b1 = strtoul(WSdata.substring( 7,  9).c_str(), NULL, 16);
            uint8_t r2 = strtoul(WSdata.substring(11, 13).c_str(), NULL, 16);
            uint8_t g2 = strtoul(WSdata.substring(13, 15).c_str(), NULL, 16);
            uint8_t b2 = strtoul(WSdata.substring(15, 17).c_str(), NULL, 16);
            if(r1 || g1 || b1 || r2 || g2 || b2)
                ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_GRAD_AH, r1, g1, b1, r2, g2, b2);
            else
                ScrollingMsg.SetTextColrOptions(COLR_HSV | COLR_GRAD_AH, 0, 224, 255, 224, 224, 255);
            // uint8_t h1 = strtoul(WSdata.substring( 3,  5).c_str(), NULL, 16);
            // uint8_t s1 = strtoul(WSdata.substring( 5,  7).c_str(), NULL, 16);
            // uint8_t v1 = strtoul(WSdata.substring( 7,  9).c_str(), NULL, 16);
            // uint8_t h2 = strtoul(WSdata.substring(11, 13).c_str(), NULL, 16);
            // uint8_t s2 = strtoul(WSdata.substring(13, 15).c_str(), NULL, 16);
            // uint8_t v2 = strtoul(WSdata.substring(15, 17).c_str(), NULL, 16);
            // ScrollingMsg.SetTextColrOptions(COLR_HSV | COLR_GRAD_CH, h1, s1, v1, h2, s2, v2);
            // temp.toUpperCase(); // needed for Robotron font
            wifiText = (char*)malloc(temp.length());
            strcpy(wifiText, temp.c_str());
            
            _serial_.print("Text: ");
            _serial_.print(wifiText);
            _serial_.print("\tSize: ");
            _serial_.print(strlen(wifiText));
            _serial_.print("\r\n");
        }else{
            handleSliders();
        }
        break;
    case WStype_BIN:
        _serial_.printf("[%u] get binary length: %u\r\n", num, length);

        // send message to client
        // webSocket.sendBIN(num, payload, length);
        break;
    case WStype_ERROR:
        break;
    case WStype_FRAGMENT_TEXT_START:
        break;
    case WStype_FRAGMENT_BIN_START:
        break;
    case WStype_FRAGMENT:
        break;
    case WStype_FRAGMENT_FIN:
        break;
    case WStype_PING:
        break;
    case WStype_PONG:
        break;
    }
}

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
    if      (bytes < 1024)                 return String(bytes) + "B";
    else if (bytes < (1024 * 1024))        return String(bytes / 1024.0) + "KB";
    else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + "MB";
    else return "";
}

bool webSocketConn(){
    return connectedClient;
}

void handleSliders(){
    if(WSdata.startsWith("reset")){
        WiFi.disconnect();
        digitalWrite(2, LOW);
        ESP.restart();
    }
    if(WSdata.startsWith("next")){
        nextPattern();
    }
    if(WSdata.startsWith("prev")){
        previousPattern();
    }
    String temp = WSdata.substring(1, WSdata.length()-1);
    if(WSdata.startsWith("M")){
        text   = temp.endsWith("0") ? true : false;
        _auto  = temp.endsWith("1") ? true : false;
        manual = temp.endsWith("2") ? true : false;
        gCurrentPatternNumber = 0;
        // if(_auto)
            // FastLED.setBrightness((125/255.0)*255.0);
        // else
            // _setBrightness = 150;
    }if(WSdata.startsWith("V")){
        int x = temp.toInt();
        x = (x*x)/255.0;
        _setBrightness = x;
    }
    if(manual){
         if(WSdata.startsWith("R")){
            int x = temp.toInt();
            x = (x*x)/255.0;
            manualColor.r = x;
        }else if(WSdata.startsWith("G")){
            int x = temp.toInt();
            x = (x*x)/255.0;
            manualColor.g = x;
        }else if(WSdata.startsWith("B")){
            int x = temp.toInt();
            x = (x*x)/255.0;
            manualColor.b = x;
        }else if(WSdata.startsWith("H")){
            gHue1 = temp.toInt();
            manualHSV.h = gHue1;
            manualColor = manualHSV;
        }else if(WSdata.startsWith("S")){
            manualHSV.s = temp.toInt();
            manualColor = manualHSV;
        }
        if(WSdata.endsWith("L")){
            manualColor_L = manualColor;
            // fill_solid (LEFT, NUM_LEDS/2, manualColor_L);
        }else if(WSdata.endsWith("R")){
            manualColor_R = manualColor;
            // fill_solid (RIGHT, NUM_LEDS/2, manualColor_R);
        }else if(WSdata.endsWith("B")){
            manualColor_L = manualColor;
            manualColor_R = manualColor;
            // fill_solid (LEFT , NUM_LEDS/2, manualColor_L);
            // fill_solid (RIGHT, NUM_LEDS/2, manualColor_R);
        }
    }
    // WSdata = "";
}

//////// TIME //////////

void timeSetup(){
    Udp.begin(localPort);
}

void timeLoop(){
	if(timeStatus() != timeSet){
        // EVERY_N_SECONDS(30){
            setSyncProvider(getNtpTime);
            setSyncInterval(5000);
        // }
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
