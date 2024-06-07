#define _serial_ Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#ifndef STASSID
#define STASSID "Home"
#define STAPSK  "12345678"
#endif
// #define hostname "pickupwinder"

// #include <TelnetStream.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
WebServer server(80); const char* host = "pickupwinder";
WebSocketsServer webSocket(81);    // create a websocket server on port 81
bool connectedClient = 0;
String WSdata = "";

////////////////////////////  VIRTUINO FUNCTIONS  //////////////////////////////
#define _virtuino 1
#ifdef _virtuino
#include <VirtuinoCM.h>
VirtuinoCM virtuino;               
#define V_memory_count 32          // the size of V memory. You can change it to a number <=255)
float V[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.
float V_prev[V_memory_count];
// #define debug 1
WiFiServer virtuinoServer(8000);                   // Default Virtuino Server port 

////////////////////////////  END VIRTUINO FUNCTIONS  //////////////////////////////
#endif

enum  { IDLE, WINDING, PAUSE } winder_state = IDLE;
enum  { ms_0, ms_1, ms_2, ms_3, ms_4} microstepping = ms_2;

#include <FastLED.h>

#include <AccelStepper.h>
#define stepPin1  23
#define  dirPin1  22
#define stepPin2  17
#define  dirPin2  16

#define MS1 21
#define MS2 19
#define MS3 18

AccelStepper stepper1(AccelStepper::DRIVER, stepPin1, dirPin1);
AccelStepper stepper2(AccelStepper::DRIVER, stepPin2, dirPin2);

void setupPins(){
    pinMode( stepPin1, OUTPUT ); digitalWrite( stepPin1, LOW );
    pinMode(  dirPin1, OUTPUT ); //digitalWrite(  dirPin1, LOW );
    pinMode( stepPin2, OUTPUT ); digitalWrite( stepPin2, LOW );
    pinMode(  dirPin2, OUTPUT ); //digitalWrite(  dirPin2, LOW );
    // pinMode(      MS1, OUTPUT ); digitalWrite(      MS1, LOW );
    // pinMode(      MS2, OUTPUT ); digitalWrite(      MS2, LOW );
    // pinMode(      MS3, OUTPUT ); digitalWrite(      MS3, LOW );
    pinMode(      RX, INPUT );
}

uint32_t rpmInterrupt = 0, rpmInterrupt_prev = 0;
uint32_t rpmCount = 0, rpmCount_prev = 0;
long int currentSteps = 0;
long int desiredSteps = 0;
int currentRPM = 0;
int desiredRPM = 0;
int currentTurns = 0;

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

        EN          23  x
only in 36          22  x
only in 39          1   tx0
only in 34          3   rx0
only in 35          21  x
    x   32          19  x
    x   33          18  x
    x   25          5
    x   26          17  x
    x   27          16  x
        14          4
        12          2
    x   13          15
        GND         GND
        Vcc         3v3

*/
/*
GPIO       Input      Output    Notes
GPIO0      Pulled up  OK        Outputs PWM signal at boot, must be LOW to enter flashing mode
GPIO1      TX pin     OK        debug output at boot
GPIO2      OK         OK        connected to on-board LED, must be left floating or LOW to enter flashing mode
GPIO3      OK         RX        pin HIGH at boot
GPIO4      OK         OK        
GPIO5      OK         OK        outputs PWM signal at boot, strapping pin
GPIO6      x          x         connected to the integrated SPI flash
GPIO7      x          x         connected to the integrated SPI flash
GPIO8      x          x         connected to the integrated SPI flash
GPIO9      x          x         connected to the integrated SPI flash
GPIO10     x          x         connected to the integrated SPI flash
GPIO11     x          x         connected to the integrated SPI flash
GPIO12     OK         OK        boot fails if pulled high, strapping pin
GPIO13     OK         OK        
GPIO14     OK         OK        outputs PWM signal at boot
GPIO15     OK         OK        outputs PWM signal at boot, strapping pin
GPIO16     OK         OK 
GPIO17     OK         OK 
GPIO18     OK         OK 
GPIO19     OK         OK 
GPIO20     OK         OK 
GPIO21     OK         OK 
GPIO22     OK         OK 
GPIO23     OK         OK 
GPIO25     OK         OK 
GPIO26     OK         x 
GPIO27     OK         OK 
GPIO32     OK         OK 
GPIO33     OK         OK 
GPIO34     OK         input only
GPIO35     OK         input only
GPIO36     OK         input only
GPIO39     OK         input only
*/