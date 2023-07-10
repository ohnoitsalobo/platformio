boolean debug = false; //true;              // set this variable to false on the finale code to decrease the request time.

void setupVirtuino(){
    virtuino.begin(onReceived, onRequested, 256);   //Start Virtuino. Set the buffer to 256. With this buffer Virtuino can control about 28 pins (1 command = 9bytes) The T(text) commands with 20 characters need 20+6 bytes
    virtuinoServer.begin();
    for(int i = 0; i < V_memory_count; i++) V[i] = 0;
}

void runVirtuino(){
    virtuinoRun();        // Necessary function to communicate with Virtuino. Client handler
    // vDelay(1000);     // This is an example of the recommended delay function. Remove this if you don't need
}

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
    }
    uint8_t v = variableIndex;
    if(v == 0){
        float temp = V[v];
        if (temp < 0){
            desiredPWM = -temp;
            direction = -1;
        } else if (temp > 0){
            desiredPWM = temp;
            direction = 1;
        }
    }
    if(v == 1){
        
    }
    if(v == 2){
        
    }
    if(v == 3){
        
    }
    if(v == 4){
        
    }
    if(v == 5){
        
    }
    if(v == 6){
        
    }
    if(v == 7){
        
    }
    if(v == 8){
        RPS++;
        msSinceLastRPS = currentms;
    }
}

//==============================================================
/* This function is called every time Virtuino app requests to read a pin value*/
String onRequested(char variableType, uint8_t variableIndex){     
    if (variableType=='V') {
        if (variableIndex<V_memory_count)
            return String(V[variableIndex]);   // return the value of the arduino V memory array
    }
    return "";
}


 //==============================================================
void virtuinoRun(){
    WiFiClient client = virtuinoServer.accept();
    if (!client) return;
    if (debug) _serial_.println("Connected");
    unsigned long timeout = millis() + 3000;
    while (!client.available() && millis() < timeout) delay(1);
    if (millis() > timeout) {
        _serial_.println("timeout");
        client.flush();
        client.stop();
        return;
    }
    virtuino.readBuffer="";    // clear Virtuino input buffer. The inputBuffer stores the incoming characters
    while (client.available()>0) {        
        char c = client.read();         // read the incoming data
        virtuino.readBuffer+=c;         // add the incoming character to Virtuino input buffer
        if (debug) _serial_.write(c);
    }
    client.flush();
    if (debug) _serial_.println("\nReceived data: "+virtuino.readBuffer);
    String* response= virtuino.getResponse();    // get the text that has to be sent to Virtuino as reply. The library will check the inptuBuffer and it will create the response text
    if (debug) _serial_.println("Response : "+*response);
    client.print(*response);
    client.flush();
    delay(10);
    client.stop(); 
    if (debug) _serial_.println("Disconnected");
}


 //============================================================== vDelay
  void vDelay(int delayInMillis){long unsigned t=millis()+delayInMillis;while (millis()<t) virtuinoRun();}