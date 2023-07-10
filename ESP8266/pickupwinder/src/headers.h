#define _serial_ Serial

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <VirtuinoCM.h>
#define virtuinoServerPort 8000
#define V_memory_count 32          // the size of V memory. You can change it to a number <=255)
VirtuinoCM virtuino;               
WiFiServer virtuinoServer(virtuinoServerPort);  // Default Virtuino Server port 
float V[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.

#define sensorPin 0         // rpm sensor
#define tension_stop 12     // tension stop switch
#define emergency_stop 13   // e-stop switch
#define analogRange 512    // analogWrite range

#define motorEN 5    // D1
#define  motorA 15   // D8
#define  motorB 13   // D7

void setupPins(){
    pinMode(LED_BUILTIN, OUTPUT);               digitalWrite(LED_BUILTIN, LOW);
    pinMode(sensorPin, INPUT_PULLUP);
    pinMode(tension_stop, INPUT_PULLUP);
    pinMode(emergency_stop, INPUT_PULLUP);
    
    pinMode(motorEN, OUTPUT);       digitalWrite(motorEN, LOW);
    pinMode( motorA, OUTPUT);       digitalWrite( motorA, LOW);
    pinMode( motorB, OUTPUT);       digitalWrite( motorB, LOW);
    
    analogWriteRange(analogRange);
}