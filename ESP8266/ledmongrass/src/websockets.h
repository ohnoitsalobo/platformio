#define USE_SERIAL telnet

void handleWSdata(){
   // WSdata.toCharArray(WSdata_temp, 1024);
   
    for(int i = 0; i < _index; i++){
        WSdata_int[i] = WSdata_temp[i].toInt();
        WSdata_temp[i] = "";
        // telnet.print(WSdata_int[i]);
        // telnet.print(".");
    }
    // telnet.print(_index);
    // telnet.print("\r\n");
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            _mode = _auto;
            USE_SERIAL.printf("[%u] Disconnected!\r\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				_mode = _midi;
				// send message to client
				webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            // USE_SERIAL.printf("[%u] get Text: %s\r\n", num, payload);
            if(num > 0)
                break;
            WSdata = "";
            _index = 0;
            for(int i = 0; i < length; i++){
                // WSdata += String(char(payload[i]));
                char comma = char(payload[i]);
                if(comma != ',')
                    WSdata_temp[_index].concat(comma);
                else
                    _index++;
                
            }
            // if(WSdata != "")
                handleWSdata();
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

