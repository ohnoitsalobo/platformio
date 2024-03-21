//---- Enter all the Tags here. You have to use the same Tags on Virtuino variables
const char* pin0_tag= "V0";         // tag for digital input
const char* pin1_tag= "V1";         // tag for digital input
const char* pin3_tag= "V3";         // tag for digital input

uint8_t pin0_lastValue=0;
uint8_t pin1_lastValue=0;
uint8_t pin3_lastValue=0;

void sendPinsStatus(){
    sendValue(pin0_tag, String(digitalRead(D0)));    // send the digital input D0 state
    sendValue(pin1_tag, String(digitalRead(D4)));    // send the digital input D1 state
    sendValue(pin3_tag, String(digitalRead(D3)));    // send the digital input D1 state

    // add more here like the next lines...
    // sendValue(pin6_tag, String(digitalRead(D6))); 
    // sendValue(pin8_tag, String(digitalRead(D8))); 
}

//===================================================== onValueReceived
// It is called every time a new value received
void onValueReceived(String tag, String value){
    Serial.println("Received: tag="+tag+ "  value="+value);
    if (tag== pin0_tag) {                    // write to digital pin D0
        int v=value.toInt();
        digitalWrite(D0, (v==1) ? HIGH : LOW);
    }
    if (tag== pin1_tag) {                    // write to digital pin D0
        int v=value.toInt();
        digitalWrite(D4, (v==1) ? HIGH : LOW);
    }

    // add more here like the next lines...
    /*
    if (tag==pin6_tag) {                    // write to digital Output D6
    int v=value.toInt();        //store the new PWM value to pin6_lastValue
    if (v==1) digitalWrite(D6,HIGH); else digitalWrite(6,LOW);
    }
    */

}

void setupVirtuino(){
    pinMode(D0,OUTPUT);                 // on this example the pin D5 is used as OUTPUT
    pinMode(D3,INPUT);
    pinMode(D4,OUTPUT);
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

}

void runVirtuino(){
    // webSocket.loop();
    
    
    int pin3 = digitalRead(D3);            // read the new state
    if (pin3_lastValue != pin3) {         // compare with the previous input state
        sendValue(pin3_tag, String(pin3));  // send the new state
        pin3_lastValue=pin3;                // copy the new state to variable pin0_lastValue 
    }
}

//===================================================== sendValue
// This function sends a value to a Tag. 
// The tag and the value are converted to a json message before sending 

bool sendValue(const char* tag, String value){
  String json = "{\"";
  json+=tag;
  json+="\":\"";
  json+=value;
  json+="\"}";
  Serial.println("Send: "+json);
  return webSocket.broadcastTXT(json);     // This function sends the message to all connected clients.
}

//===================================================== webSocketEvent
//This is the server handler. It receives all the messages

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    IPAddress _ip;
    switch(type) {
        case WStype_ERROR:
            
        break;
        case WStype_BIN:
            
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
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
        break;
        case WStype_CONNECTED: 
            _ip = webSocket.remoteIP(num);
            Serial.printf("[%u] New client connected - IP: %d.%d.%d.%d \n", num, _ip[0], _ip[1], _ip[2], _ip[3]);
            sendPinsStatus();         // send the initial pin and variable values to the connected clients
        break;
        case WStype_TEXT:  // a new message received 
            Serial.printf("[%u] Received: %s\n", num, payload);
            //-- The incoming payload is a json message. The following code extracts the value from json without extra library
            String str = (char*)payload;
            int p1 = str.indexOf("{\"");
            if (p1==0) {
                int p2 = str.indexOf("\":");
                if (p2>p1) {
                    String tag = str.substring(p1+2,p2);
                    p1 = str.indexOf(":\"",p2);
                    if (p1>0) {
                        p2 = str.lastIndexOf("\"}");  
                        if (p2>0){
                            String value = str.substring(p1+2,p2);
                            onValueReceived(tag,value);        
                        }
                    }
                }
            }
        break;
    }
}


//===================================================== vDelay
void vDelay(int delayInMillis){
    unsigned long t=millis()+delayInMillis;
    while (millis()<t) {
        // webSocket.loop();
    }
}
