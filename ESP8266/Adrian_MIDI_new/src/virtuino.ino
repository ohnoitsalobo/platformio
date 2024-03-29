//============================================================== onCommandReceived
//==============================================================
/* This function is called every time Virtuino app sends a request to server to change a Pin value
 * The 'variableType' can be a character like V, T, O  V=Virtual pin  T=Text Pin    O=PWM Pin 
 * The 'variableIndex' is the pin number index of Virtuino app
 * The 'valueAsText' is the value that has sent from the app   */
void onReceived(char variableType, uint8_t variableIndex, String valueAsText){     
    if (variableType=='V'){
        float value = valueAsText.toFloat();        // convert the value to float. The valueAsText have to be numerical
        if (variableIndex<V_memory_count) V[variableIndex]=value;              // copy the received value to arduino V memory array

        if (variableIndex < 3){
            _midi   = (variableIndex == 0) ? true : false;
            _auto   = (variableIndex == 1) ? true : false;
            _manual = (variableIndex == 2) ? true : false;
            if(_midi)
                _setBrightness = 255;
            else
                _setBrightness = 100*100/255;
        }
        else if(variableIndex == 3){ // brightness
            int x = (int)V[variableIndex];
            x = (x*x)/255.0;
            _setBrightness = x;
            V_prev[variableIndex] = V[variableIndex];
        }
        else if(variableIndex == 4){ // next pattern
            nextPattern();
            V[variableIndex] = 0;
        }
        else if(variableIndex == 5){ // previous pattern
            previousPattern();
            V[variableIndex] = 0;
        }
        else if(variableIndex > 5){
            int x = (int)V[variableIndex];
            x = (x*x)/255.0;
            if      (variableIndex == 6 ){ // red
                manualColor.r = x;
            }else if(variableIndex == 7 ){ // green
                manualColor.g = x;
            }else if(variableIndex == 8 ){ // blue
                manualColor.b = x;
            }else if(variableIndex == 9 ){ // hue
                manualHSV.h = x;
                manualColor = manualHSV;
            }else if(variableIndex == 10){ // saturation
                manualHSV.s = x;
                manualColor = manualHSV;
            }
            setManualcolor();
            V_prev[variableIndex] = V[variableIndex];
        }
        V[0] = _midi  ;
        V[1] = _auto  ;
        V[2] = _manual;
        
    }
}

void setManualcolor(){
    manualColor_L = manualColor;
    manualColor_R = manualColor;
}

//==============================================================
/* This function is called every time Virtuino app requests to read a pin value*/
String onRequested(char variableType, uint8_t variableIndex){     
    if (variableType=='V') {
        if (variableIndex<V_memory_count)
            return  String(V[variableIndex]);   // return the value of the arduino V memory array
    }
    return "";
}


 //==============================================================
void virtuinoRun(){
    WiFiClient client = virtuinoServer.available();
    if (!client)
        return;
#ifdef debug
    Serial.println("Connected");
#endif
    unsigned long timeout = millis() + 3000;
    while (!client.available() && millis() < timeout)
        delay(1);
    if (millis() > timeout) {
        Serial.println("timeout");
        client.flush();
        client.stop();
        return;
    }
    virtuino.readBuffer="";    // clear Virtuino input buffer. The inputBuffer stores the incoming characters
    while (client.available()>0) {        
        char c = client.read();         // read the incoming data
        virtuino.readBuffer+=c;         // add the incoming character to Virtuino input buffer
#ifdef debug
        Serial.write(c);
#endif
    }
    client.flush();
#ifdef debug
    Serial.println("\nReceived data: "+virtuino.readBuffer);
#endif
    String* response= virtuino.getResponse();    // get the text that has to be sent to Virtuino as reply. The library will check the inptuBuffer and it will create the response text
#ifdef debug
    Serial.println("Response : "+*response);
#endif
    client.print(*response);
    client.flush();
    delay(10);
    client.stop(); 
#ifdef debug
    Serial.println("Disconnected");
#endif
    yield();
}


 //============================================================== vDelay
void vDelay(int delayInMillis){
    unsigned long t=millis()+delayInMillis;
    while (millis()<t)
        virtuinoRun();
}

void setupVirtuino(){
    virtuino.begin(onReceived,onRequested,256);  // Start Virtuino. Set the buffer to 256. With this buffer Virtuino can control about 28 pins (1 command = 9bytes) The T(text) commands with 20 characters need 20+6 bytes
    virtuino.key="1234";                         // This is the Virtuino password. Only requests the start with this key are accepted from the library
    virtuinoServer.begin();
}

