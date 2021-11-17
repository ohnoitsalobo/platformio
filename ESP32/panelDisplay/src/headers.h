#include <SPIFFS.h>
#include <FS.h>
File fsUploadFile;

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
const char* ssid = "linksys1";
const char* password = "9182736450";
// const char* ssid = "Home";
// const char* password = "12345678";

#include <WebServer.h>
#include <WebSocketsServer.h>
WebServer server(80); const char* host = "panelDisplay";
WebSocketsServer webSocket(81);    // create a websocket server on port 81
bool connectedClient = 0;
String WSdata = "";

#include <WiFiUdp.h>
#include <TimeLib.h>
static const char ntpServerName[] = "in.pool.ntp.org";
const double timeZone = 5.5; // IST

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

#include <arduinoFFT.h>
// #define FASTLED_ALLOW_INTERRUPTS 0
// #define INTERRUPT_THRESHOLD 1
// #define FASTLED_INTERRUPT_RETRY_COUNT 0
// #define FASTLED_ESP32_FLASH_LOCK 1
#define FASTLED_INTERNAL
#include <FastLED.h>
#include <Adafruit_I2CDevice.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>
#define MATRIX_WIDTH   8
#define MATRIX_HEIGHT  9
#define MATRIX_TYPE    HORIZONTAL_ZIGZAG_MATRIX
#define NUM_LEDS MATRIX_WIDTH * MATRIX_HEIGHT
bool _auto = 1;
bool manual = 0;
bool text = 0;
bool FFTenable = true;

