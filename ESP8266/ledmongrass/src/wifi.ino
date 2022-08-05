#include <LittleFS.h>
FS* fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

#include"websockets.h"

void setupWifi(){
    
    WiFi.mode(WIFI_STA);
    WiFi.config(_ip, _gateway, _netmask);
    WiFi.begin(ssid, password);
    
    if (MDNS.begin(hostname.c_str())) {
        Serial.print("\r\nMDNS responder started, hostname: ");
        Serial.println(hostname);
    }
    // MDNS.addService("http", "tcp", 80);
    
    setupOTA();
    
    setupVirtuino();
    
    setupWebSockets();
    
    setupTelnet();
    
    setupServer();
}

void runWifi(){
    if(WiFi.status() == WL_CONNECTED){
        ArduinoOTA.handle();
        virtuinoRun();        // Necessary function to communicate with Virtuino. Client handler
        webSocket_loop();
        handleTelnet();
        server.handleClient();

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

void setupTelnet(){
    TelnetServer.begin();
    TelnetServer.setNoDelay(true);
}

void handleTelnet(){
    if(TelnetServer.hasClient()){
        if(!telnet || !telnet.connected())
            if(telnet)
                telnet.stop();
        telnet = TelnetServer.available();
    } else {
        TelnetServer.available().stop();
    }
}

void replyOK() {
  server.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg) {
  server.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg) {
  server.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg) {
  telnet.println(msg);
  server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg) {
  telnet.println(msg);
  server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

bool handleFileRead(String path) {
  telnet.println(String("handleFileRead: ") + path);

  if (path.endsWith("/")) {
    path += "index.htm";
  }

  String contentType = mime::getContentType(path);

  if (!fileSystem->exists(path)) {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (fileSystem->exists(path)) {
    File file = fileSystem->open(path, "r");
    if (server.streamFile(file, contentType) != file.size()) {
      telnet.println("Sent less data than expected!");
    }
    file.close();
    return true;
  }

  return false;
}

void handleNotFound() {
  String uri = ESP8266WebServer::urlDecode(server.uri()); // required to read paths with blanks

  if (handleFileRead(uri)) {
    return;
  }

  // Dump debug data
  String message;
  message.reserve(100);
  message = F("Error: File not found\n\nURI: ");
  message += uri;
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += '\n';
  for (uint8_t i = 0; i < server.args(); i++) {
    message += F(" NAME:");
    message += server.argName(i);
    message += F("\n VALUE:");
    message += server.arg(i);
    message += '\n';
  }
  message += "path=";
  message += server.arg("path");
  message += '\n';
  telnet.print(message);

  return replyNotFound(message);
}

void setupServer(){
    fileSystemConfig.setAutoFormat(false);
    fileSystem->setConfig(fileSystemConfig);
    bool fsOK = fileSystem->begin();
    
    server.onNotFound(handleNotFound);
    
    server.begin();

}