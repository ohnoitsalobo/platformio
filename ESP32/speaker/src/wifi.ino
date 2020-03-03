const char* ssid = "linksys1";
const char* password = "9182736450";
const char* APssid = "speaker";
const char* APpassword = "9182736450";

void setupWiFi(){
    // btStop();
    Serial_1.println("\nStarting Wifi");
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
            Serial_1.println("\nStart updating " + type);
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        })
        .onEnd([]() {
            Serial_1.println("\nEnd");
            delay(10);
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            uint32_t temp = progress / (total / 100);
            digitalWrite(2, HIGH);
            // Serial_1.printf("Progress: %u%%\r", (progress / (total / 100)));
            Serial_1.printf("Progress: %u%%\r", temp);
            if(temp<99){
                leds[map(temp, 0, 99, 0, NUM_LEDS)] = 0x111111;
                FastLED.show();
            }else if(temp == 99){
                fill_solid (leds, NUM_LEDS, CRGB::Black);
                FastLED.show();
            }
            digitalWrite(2, LOW);
        })
        .onError([](ota_error_t error) {
            fill_solid (leds, NUM_LEDS, CRGB::Red);
            FastLED.show();
            Serial_1.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial_1.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial_1.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial_1.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial_1.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial_1.println("End Failed");
        });

    ArduinoOTA.begin();
}

void wifiLoop(){
    if(WiFi.status() == WL_CONNECTED){
        ArduinoOTA.handle();
        server.handleClient();
        webSocket.loop();
        // EVERY_N_SECONDS(5){
            // Serial_1.print("\nIP Address: ");
            // Serial_1.println(WiFi.localIP());
        // }
    }
    if(WiFi.status() != WL_CONNECTED){
        EVERY_N_SECONDS(30){
            // FastLED.setBrightness(music ? 40 : 15 );
            // wifiScan();
            // /* Serial_1.print("\nIP Address: "); */
            // /* Serial_1.println(WiFi.localIP()); */
        }
    }
}

void beginServer(){
    // server.on("/upload", HTTP_POST, [](){ server.send(200); }, handleFileUpload);
    server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
    server.begin();
    Serial_1.print("\r\nServer started\r\n");
    if (MDNS.begin(host)) {
        Serial_1.print("\r\nMDNS responder started\r\n");
    }
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
    // MDNS.addService("wss", "tcp", 82);
}

void handleNotFound(){ // if the requested file or page doesn't exist, return a 404 not found error
  if(!handleFileRead(server.uri())){          // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
    Serial_1.println("handleFileRead: " + path);
    if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
    String contentType = getContentType(path);             // Get the MIME type
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
            path += ".gz";                                         // Use the compressed verion
        File file = SPIFFS.open(path, "r");                    // Open the file
        size_t sent = server.streamFile(file, contentType);    // Send it to the client
        file.close();                                          // Close the file again
        Serial_1.println(String("\tSent file: ") + path);
        return true;
    }
    Serial_1.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
    return false;
}

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

////////////////////////////////////

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial_1.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial_1.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial_1.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial_1.print("  DIR : ");
            Serial_1.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial_1.print("  FILE: ");
            Serial_1.print(file.name());
            Serial_1.print("\tSIZE: ");
            Serial_1.println(file.size());
        }
        file = root.openNextFile();
    }
}

void startSPIFFS() { // Start the SPIFFS and list all contents
    if(!SPIFFS.begin()){
        Serial_1.println("SPIFFS Mount Failed");
        return;
    }
    
    listDir(SPIFFS, "/", 0);

    // SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
    // Serial_1.println("SPIFFS started. Contents:");
    // {
        // Dir dir = SPIFFS.openDir("/");
        // while (dir.next()) {                      // List the file system contents
            // String fileName = dir.fileName();
            // size_t fileSize = dir.fileSize();
            // Serial_1.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
        // }
        // Serial_1.printf("\n");
    // }
}

void startWebSocket() { // Start a WebSocket server
    webSocket.begin();                          // start the websocket server
    webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
    Serial_1.println("WebSocket server started.");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) { // When a WebSocket message is received
    switch(type) {
    case WStype_DISCONNECTED:
        Serial_1.printf("[%u] Disconnected!\n", num);
        connectedClient = 0;
        break;
    case WStype_CONNECTED:
        {
            IPAddress ip = webSocket.remoteIP(num);
            Serial_1.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket.sendTXT(num, "Connected");
            connectedClient = 1;
        }
        break;
    case WStype_TEXT:
        Serial_1.printf("[%u] get Text: %s\n", num, payload);
        WSdata = "";
        for(int i = 0; i < length; i++)
            WSdata += String(char(payload[i])); 
        // Serial_1.println(WSdata);
        handleESPval();
        break;
    case WStype_BIN:
        Serial_1.printf("[%u] get binary length: %u\n", num, length);

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
    }
}

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
    if      (bytes < 1024)                 return String(bytes) + "B";
    else if (bytes < (1024 * 1024))        return String(bytes / 1024.0) + "KB";
    else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + "MB";
}



