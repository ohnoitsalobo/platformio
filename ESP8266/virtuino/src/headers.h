#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager

IPAddress ip(192, 168, 0, 210);            // desired IP Address. The first three numbers must be the same as the router IP
IPAddress gateway(192, 168, 0, 1);         // set gateway to match your network. Replace with your router IP
IPAddress subnet(255, 255, 255, 0);        // set subnet mask to match your network

////////////////////////////  VIRTUINO FUNCTIONS  //////////////////////////////

#include <VirtuinoCM.h>
VirtuinoCM virtuino;               
#define V_memory_count 32          // the size of V memory. You can change it to a number <=255)
float V[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.
// float V_prev[V_memory_count];
// #define debug 1
WiFiServer virtuinoServer(8000);                   // Default Virtuino Server port 

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
}


 //============================================================== vDelay
void vDelay(int delayInMillis){
    unsigned long t=millis()+delayInMillis;
    while (millis()<t)
        virtuinoRun();
}

////////////////////////////  END VIRTUINO FUNCTIONS  //////////////////////////////
