#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#define NUMBER_OF_LEDS 144

uint32_t loopCount0 = 0, loopCount1 = 0, loopCount2 = 0;
uint32_t total = 0;
bool wifiEnable = 0;

TaskHandle_t Task1, Task2;
SemaphoreHandle_t baton;

void setup(){

    Serial.begin(115200);
    pinMode(2, OUTPUT);

    setupWiFi();
    
    fftSetup();
    
    baton = xSemaphoreCreateMutex();

    // A viewer suggested to use :     &codeForTask1, because his ESP crashed

    xTaskCreatePinnedToCore(
        &codeForTask1,         // Task function
        "core1Task",           // name of task
        1000,                  // Stack size of task
        NULL,                  // parameter of the task
        1,                     // priority of the task
        &Task1,                // Task handle to keep track of created task
        0                      // pin task to core 0
    );
    delay(500);  // needed to start-up task1

    xTaskCreatePinnedToCore(
        &codeForTask2,         // Task function
        "core2Task",           // name of task
        1000,                  // Stack size of task
        NULL,                  // parameter of the task
        1,                     // priority of the task
        &Task2,                // Task handle to keep track of created task
        1                      // pin task to core 1
    );

}

void loop(){
    wifiLoop();
    delay(100);
}

void codeForTask1( void * parameter ){
 for (;;) {
// xSemaphoreTake( baton, portMAX_DELAY );
    long now = micros();

    uint16_t speed = micros() - now;
    loopCount1 = (loopCount1+1)%10000;
    if(loopCount1%4005 == 0){
        Serial.print("Loop 1 speed: ");
        Serial.print(1000.0/speed);
        Serial.print("\tkHz (");
        Serial.print(speed);
        Serial.print(" us) on Core ");
        Serial.print(xPortGetCoreID());
        Serial.print("\t\r");
    }
    delay(50);
// xSemaphoreGive( baton );
 }
}

void codeForTask2( void * parameter ){
 for (;;) {
// xSemaphoreTake( baton, portMAX_DELAY );
    long now = micros();
    
    // fftLoop();
    
    uint16_t speed = micros() - now;
    loopCount2 = (loopCount2+1)%10000;
    if(loopCount2%4010 == 0){
        Serial.print("Loop 2 speed:");
        Serial.print(1000.0/speed);
        Serial.print("\tkHz (");
        Serial.print(speed);
        Serial.print(" us) on Core ");
        Serial.print(xPortGetCoreID());
        Serial.print("\t\r");
    }
    delay(50);
    // feedTheDog();
// xSemaphoreGive( baton );
 }
}

void feedTheDog(){
    // feed dog 0
    TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable
    TIMERG0.wdt_feed=1;                       // feed dog
    TIMERG0.wdt_wprotect=0;                   // write protect
    // feed dog 1
    TIMERG1.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable
    TIMERG1.wdt_feed=1;                       // feed dog
    TIMERG1.wdt_wprotect=0;                   // write protect
}