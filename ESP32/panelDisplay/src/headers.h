#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>
#include <BlynkSimpleEsp32.h>

#define FASTLED_INTERNAL
#include <FastLED.h>
#define kMatrixWidth    8
#define kMatrixHeight   9
#define NUMBER_OF_LEDS kMatrixWidth*kMatrixHeight
