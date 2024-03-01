#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define RX 3
#define TX 1
#define S3 10
#define S2 9
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
float V_prev[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.

#define sensorPin D2         // rpm sensor

#define  dir_A D3
#define  motorA D1

void setupPins(){
    pinMode(LED_BUILTIN, OUTPUT);               digitalWrite(LED_BUILTIN, LOW);
    pinMode(sensorPin, INPUT_PULLUP);
    
    pinMode( dir_A, OUTPUT);       digitalWrite( dir_A, LOW);
    pinMode( motorA, OUTPUT);       digitalWrite( motorA, LOW);
    
    analogWriteRange(1024);
    analogWriteFreq(100);
}

/*
V0 = desired RPM
V1 = current RPM
V2 = desired turns
V3 = current turns
V4 = forward
V5 = reverse
V6 = tension stop
V7 = emergency stop
*/
/*
D0    GPIO16       GPIO0   D3
D1    GPIO5        GPIO1   TX
D2    GPIO4        GPIO2   D4/led
D3    GPIO0        GPIO3   RX
D4    GPIO2/led    GPIO4   D2
D5    GPIO14       GPIO5   D1
D6    GPIO12       GPIO9   S2
D7    GPIO13       GPIO10  S3
D8    GPIO15       GPIO12  D6
RX    GPIO3        GPIO13  D7
TX    GPIO1        GPIO14  D5
S3    GPIO10       GPIO15  D8
S2    GPIO9        GPIO16  D0
*/
/*  * /
#if __cplusplus > 199711L 
    #define register
#endif
#include <Adafruit_I2CDevice.h>
#define FASTLED_INTERNAL
#include <FastLED.h>
/*  */