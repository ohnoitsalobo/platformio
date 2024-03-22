/* Virtuino Iot example: ESP8266 advance example
 * Supported boards: all ESP8266 boards
 * 
 * This example contains the following features
 * 1. How to read enable an Arduino led from Virtuino dashboard
 * 2. How to send the state of an arduino pin to Virtuino dashboard
 * 3. How to send sensor values to Virtuino app periodically
 * 4. How to send analog value to Virtuino app periodically
 * 5. How to control a PWM pin from Virtuino app
 * 6. How send a value from Virtuino dashboard. How to store it on arduino code
 
 * Created by Ilias Lamprou
 * Updated Mar 10 2022
 */
 
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h> // Download the library WebSockets by Markus Sattler from arduino library manager (ver 2.3.5)

//--- SETTINGS ----------------------------------------
  const char* ssid = "WiFi Name";                      // WIFI network SSID
  const char* password = "WiFi Password";              // WIFI network PASSWORD
  
  WebSocketsServer webSocket = WebSocketsServer(8000); // Default Virtuino Server port = 8000 
  //IPAddress ip(192, 168, 1, 150);                    // where 150 is the desired IP Address. The first three numbers must be the same as the router IP
  //IPAddress gateway(192, 168, 1, 1);                 // set gateway to match your network. Replace with your router IP

//---- Enter all the Tags here. You have to use the same Tags on Virtuino variables
  const char* pin0_tag= "V0";         // tag for digital input
  const char* pin1_tag= "V1";         // tag for digital input
  const char* pin5_tag= "V5";         // tag for digital output
  const char* pin6_tag= "V6";         // tag for PWM output
  const char* pinA0_tag= "V9";        // tag for analog input
  const char* sensor1_tag= "V10";     // tag for sensor1 value
  const char* sensor2_tag= "V11";     // tag for sensor2 value
  const char* myVariable_tag= "V12";  // tag for a variable


//---- Enter some variables to hold the last state of the inputs.
  int pin0_lastValue=0;
  uint8_t pin1_lastValue=0;
  uint8_t pin5_lastValue=0;
  uint8_t pin6_lastValue=0;

  int myVariable= 0;                 // This variable stores a value that is sent from Virtuino app 
  unsigned long lastSensorRedingTime=0; 


//===================================================== sendPinStatus
// It is called every time a new client is connected.
// The void informs Virtuino app about the current pin states and variable values.

void sendPinsStatus(){
  sendValue(pin0_tag, String(digitalRead(D0)));    // send the digital input D0 state
  sendValue(pin1_tag, String(digitalRead(D1)));    // send the digital input D1 state
  sendValue(pin5_tag, String(digitalRead(D5)));    // send the digital ouput D5 state
  sendValue(pin6_tag, String(pin6_lastValue));     // send the last PWM value of D6
  sendValue(pinA0_tag, String(analogRead(A0)));    // send the analog value of A0 
  sendValue(myVariable_tag, String(myVariable));   // send the value of myVariable
  // add more here...
  }

//===================================================== onValueReceived
// It is called every time a new value received
  void onValueReceived(String tag, String value){
    Serial.println("Received: tag="+tag+ "  value="+value);
    if (tag== pin5_tag) {                    // write to digital pin D5
      int v=value.toInt();
      if (v==1) digitalWrite(D5,HIGH); else digitalWrite(D5,LOW);
    }
    if (tag==pin6_tag) {                    // write to PWM pin D6
      pin6_lastValue= value.toInt();        //store the new PWM value to pin6_lastValue
      analogWrite(D6,pin6_lastValue);
    }
    if (tag==myVariable_tag) {              // store the incoming value to the variable: myVariable  
      int v=value.toInt();                  // Convert the incoming text value to int or float or...
      myVariable = v;
    }

    // add more here...
    
    }

//===================================================== setup
//=====================================================
  void setup() {
      Serial.begin(115200);
      while (!Serial) delay(1);
      pinMode(D5,OUTPUT);                 // on this example the pin D5 is used as OUTPUT
      pinMode(D6,OUTPUT);                 // on this example the pin D6 is used as PWM OUTPUT
      connectToWiFiNetwork();
      webSocket.begin();
      webSocket.onEvent(webSocketEvent);
  }

//===================================================== loop
//=====================================================
  void loop() {
      webSocket.loop();     
  
      //---Example 1: How to send sensor values to Virtuino every 5 seconds.
      if (millis()-lastSensorRedingTime > 5000) {
        int sensor1 = random(50);                     // replace with your sensor 1 value
        float sensor2 = 5.67;                         // replace with your sensor 2 value
        sendValue(sensor1_tag, String(sensor1));      // send sensor 1 value
        sendValue(sensor2_tag, String(sensor2,2));    // send sensor 2 value
        sendValue(pinA0_tag, String(analogRead(A0))); // send the analog input value of A0
        lastSensorRedingTime=millis();                // store the last sending time
      }
      
  
      //---Example 2: How to send a Digital Input state to Virtuino app every time it changes
      int pin0= digitalRead(D0);            // read the new state
      if (pin0_lastValue != pin0) {         // compare with the previous input state
        sendValue(pin0_tag, String(pin0));  // send the new state
        pin0_lastValue=pin0;                // copy the new state to variable pin0_lastValue 
      }
  
      //--- Be careful: Don't add a line like the next. It will send values to Virtuino app on each loop circle.
      // sendValue(pinD0_tag, digitalRead(D0));


      
      // avoid to use the void Delay() in your code. This causes communication delays
      // vDelay(1000);   // Use the vDelay() instead of Delay() or better none of them. Prefer the technique of the example 1 in this loop
  }


/*=====================================================
  ======================  UTILS =======================
  =====================================================
  You don't need to make changes to the code below
*/
//===================================================== connectToWiFiNetwork
void connectToWiFiNetwork(){
  Serial.println("Connecting to "+String(ssid));
  // If you don't want to config IP manually disable the next two lines and the disables IPAddress variables  at the top of the code
  //IPAddress subnet(255, 255, 255, 0);        // set subnet mask to match your network
  //WiFi.config(ip, gateway, subnet);          // If you don't want to config IP manually disable this line
  WiFi.mode(WIFI_STA);                         // Configure the module as station only.
  WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
    }
   Serial.println("\nWiFi connected");
   Serial.println(WiFi.localIP());              // Insert this IP on Virtuino IoT server settings
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
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:{ 
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] New client connected - IP: %d.%d.%d.%d \n", num, ip[0], ip[1], ip[2], ip[3]);
            sendPinsStatus();         // send the initial pin and variable values to the connected clients
            break;
        }
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

 //============================================================== vDelay
  void vDelay(int delayInMillis){long t=millis()+delayInMillis;while (millis()<t) webSocket.loop();}
  
