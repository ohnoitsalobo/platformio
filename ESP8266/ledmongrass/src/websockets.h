#define USE_SERIAL telnet
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\r\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				
				// send message to client
				webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\r\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[%u] get binary length: %u\r\n", num, length);
            hexdump(payload, length);

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

void setupWebSockets() {
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void webSocket_loop() {
    webSocket.loop();
}