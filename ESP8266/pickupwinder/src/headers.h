        //    STATE at boot | boot FAILS if
#define D0 16  // 1 | -
#define D1 5
#define D2 4
#define D3 0   // - | 0
#define D4 2   // 1 | 0
#define D5 14
#define D6 12
#define D7 13
#define D8 15  // - | 1
#define RX 3   // 1 | -
#define TX 1   // 1 | 0

#define _serial_ Serial

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <VirtuinoCM.h>
#define virtuinoServerPort 8000
#define V_memory_count 50          // the size of V memory. You can change it to a number <=255)
VirtuinoCM virtuino;               
WiFiServer virtuinoServer(virtuinoServerPort);  // Default Virtuino Server port 
float V[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.
float V_prev[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.

enum  { IDLE, WINDING, PAUSE } winder_state = IDLE;
enum  { ms_0, ms_1, ms_2, ms_3, ms_4} microstepping = ms_1;

#include <AccelStepper.h>
#define STEPS_PER_REV 200
#define stepPin1  D1
#define  dirPin1  D2
#define stepPin2  D5
#define  dirPin2  D6

#define MS1 D0
#define MS2 D7
#define MS3 RX

AccelStepper stepper1(AccelStepper::DRIVER, stepPin1, dirPin1);
AccelStepper stepper2(AccelStepper::DRIVER, stepPin2, dirPin2);

void setupPins(){
    pinMode( stepPin1, OUTPUT ); digitalWrite( stepPin1, LOW );
    pinMode(  dirPin1, OUTPUT ); //digitalWrite(  dirPin1, LOW );
    pinMode( stepPin2, OUTPUT ); digitalWrite( stepPin2, LOW );
    pinMode(  dirPin2, OUTPUT ); //digitalWrite(  dirPin2, LOW );
    pinMode(      MS1, OUTPUT ); digitalWrite(      MS1, LOW );
    pinMode(      MS2, OUTPUT ); digitalWrite(      MS2, LOW );
    pinMode(      MS3, OUTPUT ); digitalWrite(      MS3, LOW );
}


long int currentSteps = 0;
long int desiredSteps = 0;
int currentRPM = 0;
int desiredRPM = 0;
float currentTurns = 0;

int desiredTurns = 0;
int coilHeight = 0;
int wireDiameter = 0;

void resetValues(){
    desiredSteps = 0;
    desiredRPM = 0;

    desiredTurns = 0;
    coilHeight = 0;
    wireDiameter = 0;

}
/*


*/

/*
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
/ *  */