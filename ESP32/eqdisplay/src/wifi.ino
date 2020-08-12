// #include <EasyDDNS.h>

const char* ssid = "linksys1";
const char* password = "9182736450";

String WSdata = "";
File fsUploadFile;
WebServer server(80); const char* host = "eqdisplay";
WebSocketsServer webSocket(81);    // create a websocket server on port 81
bool connectedClient = 0;

void setupWiFi(){
    _serial_.println("\r\nStarting Wifi");

    WiFi.disconnect();
    // WiFi.mode(WIFI_AP_STA);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    // WiFi.softAP(host, password);


    setupOTA();
    
    startSPIFFS();
    
    beginServer();
    
    startWebSocket();
    
    // EasyDDNS.service("noip");
    // EasyDDNS.client("lobo-esp32.ddns.net", "cataclysm9.8@gmail.com", "cataclysm9.8");
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
            else // U_SPIFFS
            type = "filesystem";

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
            // _serial_.printf("Progress: %u%%\r", (progress / (total / 100)));
            _serial_.printf("Progress: %u%%\r", temp);
            if(temp<99){
                fill_solid (LEFT, map(temp, 0, 99, 0, NUM_LEDS/2), 0x020202);
                fill_solid (RIGHT, map(temp, 0, 99, 0, NUM_LEDS/2), 0x020202);
            }else if(temp == 99){
                fill_solid (leds, NUM_LEDS, 0x020202);
            }
            FastLED.show();
        })
        .onError([](ota_error_t error) {
            fill_solid (leds, NUM_LEDS, CRGB::Red);
            FastLED.show();
            _serial_.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) _serial_.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) _serial_.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) _serial_.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) _serial_.println("Receive Failed");
            else if (error == OTA_END_ERROR) _serial_.println("End Failed");
            fill_solid (leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        });

    ArduinoOTA.begin();
}

void wifiLoop(){
    if(WiFi.status() == WL_CONNECTED){
        ArduinoOTA.handle();
        server.handleClient();
        webSocket.loop();
        // EVERY_N_MILLISECONDS(20){
            // if(music && webSocketConn()){
                // eqBroadcast = "E";
                // for(uint8_t i = 0; i < NUM_LEDS/4; i++){
                    // eqBroadcast += ",";
                    // eqBroadcast += String(eq[0][i]);
                    // if(eq[0][i] != 0) eq[0][i] /= 5.0;
                // }
                // for(uint8_t i = 0; i < NUM_LEDS/4; i++){
                    // eqBroadcast += ",";
                    // eqBroadcast += String(eq[1][i]);
                    // if(eq[1][i] != 0) eq[1][i] /= 5.0;
                // }
                // webSocket.broadcastTXT(eqBroadcast);
                // eqBroadcast = "";
            // }
        EVERY_N_MILLISECONDS(500){
            if(music && webSocketConn()){
                // eqBroadcast = "";
                // for (uint16_t i = 2; i < samples/2; i++){
                    // eqBroadcast += i;
                    // eqBroadcast += "\t";
                    // eqBroadcast += ((i-1) * 1.0 * samplingFrequency) / samples;
                    // eqBroadcast += "  \t";
                    // eqBroadcast += (int)spectrum[0][i];
                    // eqBroadcast += "\r\n";
                // }
                // webSocket.broadcastTXT(eqBroadcast);
            }
        }
        if(!digitalRead(2))
            digitalWrite(2, HIGH);
            // EasyDDNS.update(30000);
    }
    
    if(WiFi.status() != WL_CONNECTED){
        EVERY_N_SECONDS(10){
            WiFi.begin(ssid, password);
        }
        if(digitalRead(2))
            digitalWrite(2, LOW);
    }
    yield();
}

void beginServer(){
    // server.on("/upload", HTTP_POST, [](){ server.send(200); }, handleFileUpload);
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
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
            path += ".gz";                                         // Use the compressed verion
        File file = SPIFFS.open(path, "r");                    // Open the file
        size_t sent = server.streamFile(file, contentType);    // Send it to the client
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
        handleSliders();
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
    String temp = WSdata.substring(1, WSdata.length()-1);
    if(WSdata.startsWith("M")){
        music = temp.endsWith("0") ? true : false;
        _auto = temp.endsWith("1") ? true : false;
        manual = temp.endsWith("2") ? true : false;
        gCurrentPatternNumber = 0;
        if(_auto)
            FastLED.setBrightness(30);
        else
            FastLED.setBrightness(255);
    }if(WSdata.startsWith("V")){
        int x = temp.toInt();
        x = (x*x)/255.0;
        FastLED.setBrightness(x);
    }
    // FastLED.show();
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
            fill_solid (LEFT, NUM_LEDS/2, manualColor);
        }else if(WSdata.endsWith("R")){
            fill_solid (RIGHT, NUM_LEDS/2, manualColor);
        }else if(WSdata.endsWith("B")){
            fill_solid (leds, NUM_LEDS, manualColor);
        }
    }
    WSdata = "";
}
