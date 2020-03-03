/* ****************** PIN MAPPINGS ****************** *\
                      __ESP 12___
 3v3 -> 10k -> RESET |           | TX / GPIO1
                 ADC |           | RX / GPIO3
 3v3 -> 10k -> CH_PD |           | GPIO5
              GPIO16 |           | GPIO4
              GPIO14 |           | GPIO0 <- 10k <- 3v3
              GPIO12 |           | GPIO2 / TXD1
         (RX)/GPIO13 |           | GPIO15/(TX) <- 10k <- GND
                 VCC |___________| GND

             SW1 -> RESET    SW2 -> GPIO0
\* ************************************************** */

///////////////////
///////////////////

#include <FS.h>
#include <WebSocketsServer.h>
#include <WebSocketsClient.h>

WebSocketsServer webSocket(81);    // create a websocket server on port 81
String WSdata = "";
unsigned long int millisSinceLastSync = 0;

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    
    wifiSetup();
    
    setupOTA();
    
    startSPIFFS();               // Start the SPIFFS and list all contents
    
    startWebSocket();            // Start a WebSocket server

    setupMSGEQ7();
    
    ledSetup();
    
}

void loop(){
    wifiLoop();
    webSocket.loop();                           // constantly check for websocket events
    
    readMSGEQ7();
    
    ledLoop();
    
    yield();
}

///////////////////
///////////////////

void startSPIFFS() { // Start the SPIFFS and list all contents
    SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
    Serial.println("SPIFFS started. Contents:");
    {
        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {                      // List the file system contents
            String fileName = dir.fileName();
            size_t fileSize = dir.fileSize();
            Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
        }
        Serial.printf("\n");
    }
}

void startWebSocket() { // Start a WebSocket server
    webSocket.begin();                          // start the websocket server
    webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
    Serial.println("WebSocket server started.");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) { // When a WebSocket message is received
    switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
    break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    }
    break;
    case WStype_TEXT:                     // if new text data is received
        // Serial.printf("[%u] get Text: %s\n", num, payload);
        WSdata = "";
        for(int i = 0; i < length; i++)
            WSdata += String(char(payload[i])); 
        // Serial.println(WSdata);
        handleESPval();
    }
}

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
    if      (bytes < 1024)                 return String(bytes) + "B";
    else if (bytes < (1024 * 1024))        return String(bytes / 1024.0) + "KB";
    else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + "MB";
}

