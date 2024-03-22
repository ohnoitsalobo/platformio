boolean debug = false; //true;              // set this variable to false on the final code to decrease the request time.

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
        V_prev[variableIndex] = V[variableIndex];
        float value = valueAsText.toFloat();        // convert the value to float. The valueAsText have to be numerical
        if (variableIndex<V_memory_count) V[variableIndex]=value;              // copy the received value to arduino V memory array
    
        int _1rev = STEPS_PER_REV * stepMultiply();
        if       (variableIndex == 0){   // run 1 rev forward
            if((int)value > 0) stepper1.move(_1rev);
            
            // _serial_.print("\n");
            // _serial_.print(STEPS_PER_REV);
            // _serial_.print("\t");
            // _serial_.print(stepMultiply());
            // _serial_.print("\t");
            // _serial_.print(RPMToSteps(1));
            // _serial_.print("\t");
            // _serial_.print(stepsToRPM(800));
            // _serial_.print("\t");
            // _serial_.print(turnsToSteps(1));
            // _serial_.print("\t");
            // _serial_.print(stepsToTurns(800));
            
        }else if (variableIndex == 1){   // run 1 rev backward
            if((int)value > 0) stepper1.move(-_1rev);
        }else if (variableIndex == 2){  // run forward
            if((int)value > 0) stepper1.setSpeed(6*STEPS_PER_REV);
            else               stepper1.setSpeed(0);
        }else if (variableIndex == 3){  // run backward
            if((int)value > 0) stepper1.setSpeed(-6*STEPS_PER_REV);
            else               stepper1.setSpeed(0);
        }else if (variableIndex == 4){  // total steps moved
            
        }else if (variableIndex == 5){  // set RPM
            desiredRPM = RPMToSteps((int)value);
        }else if (variableIndex == 6){  // set number of turns
            desiredTurns = turnsToSteps((int)value);
        }else if (variableIndex == 10){  // set number of turns
            if((int)value > 0){
                winder_state = WINDING;
                stepper1.setMaxSpeed(desiredRPM);
                if(stepper1.distanceToGo() == 0){
                    stepper1.move(desiredTurns);
                }
            }else{
                winder_state = PAUSE;
            }
        }
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
    String* response= virtuino.getResponse();    // get the text that has to be sent to Virtuino as reply. The library will check the inputBuffer and it will create the response text
    if (debug) _serial_.println("Response : "+*response);
    client.print(*response);
    client.flush();
    delay(10);
    client.stop(); 
    if (debug) _serial_.println("Disconnected");
}


 //============================================================== vDelay
  void vDelay(int delayInMillis){long unsigned t=millis()+delayInMillis;while (millis()<t) virtuinoRun();}
