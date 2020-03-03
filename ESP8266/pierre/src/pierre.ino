#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
// #include <ESP8266Ping.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
ESP8266WebServer server(80);

#define BLYNK_FANCY_LOGO_3D
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#include <BlynkSimpleEsp8266.h>

const char* ssid = "Orangenet";
const char* pass = "acbed13245";
char auth[] = "8d8acb5a76af4231b3acd5d06dde0207";

// const char* ssid = "linksys1";
// const char* pass = "9182736450";
// char auth[] = "ed39310c710746dab8c073206486525c"; // TEST

#define FASTLED_ALLOW_INTERRUPTS 0
// #define FASTLED_INTERRUPT_RETRY_COUNT 0
#include <FastLED.h>

#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define LED_PINS    2 // 2 (ESP01)  // 5 (ESP12)
#define SEGMENTS    4
#define LED_SEG     41

#define NUM_LEDS    4*LED_SEG
#define BRIGHTNESS  10
#define FPS         120
CRGBArray<NUM_LEDS> leds;
CRGBSet SEG1  (leds (0,         LED_SEG-1));
CRGBSet SEG2  (leds (LED_SEG,   2*LED_SEG-1));
CRGBSet SEG3  (leds (2*LED_SEG, 3*LED_SEG-1));
CRGBSet SEG4  (leds (3*LED_SEG, NUM_LEDS-1));
CRGB manualRGB = CRGB::White;
CRGB seg1 = manualRGB, seg2 = manualRGB, seg3 = manualRGB, seg4 = CRGB::White;
bool _seg1 = true, _seg2 = true, _seg3 = true, _seg4 = true;
bool _auto = false;

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType    currentBlending;

void setup() {
    OTAsetup();
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);

    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
 
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);

    beginServer();

    delay(500);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(500);

    Blynk.begin(auth, ssid, pass);
    // Blynk.config(auth);
    
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, dot_beat, /* juggle, */ bpm, blendwave, beatwave };
// String patterns[]  = { "rainbow", "rainbowWithGlitter", "confetti", "sinelon", "dot_beat",/*  "juggle", */ "bpm", "blendwave", "beatwave" };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {

    maintain_network();
    if( _auto ){
        EVERY_N_MILLISECONDS( 1000 / FPS ){
            FastLED_patterns();
        }
    }
    
    yield();
}

void maintain_network(){
    server.handleClient();
    ArduinoOTA.handle();
    Blynk.run();
}

void FastLED_patterns(){
    gPatterns[gCurrentPatternNumber]();         // Call the current pattern function once, updating the 'leds' array
    // FastLED.delay( 1000 / FPS );  // insert a delay to keep the framerate modest
    EVERY_N_SECONDS( 30 ) { nextPattern(); }   // change patterns periodically
    EVERY_N_MILLISECONDS(100) {
        uint8_t maxChanges = 24; 
        nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   // AWESOME palette blending capability.
    }
    EVERY_N_SECONDS(5) {           // Change the target palette to a random one every 5 seconds.
        targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
    }
    EVERY_N_MILLISECONDS( 20 ) { gHue++; }     // slowly cycle the "base color" through the rainbow
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
    // Serial.println(patterns[gCurrentPatternNumber]);
}

void rainbow() 
{
    // FastLED's built-in rainbow generator
    fill_rainbow( leds, NUM_LEDS, gHue, 7);
    FastLED.show();
}

void rainbowWithGlitter() 
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
    FastLED.show();
}

void addGlitter( fract8 chanceOfGlitter) 
{
    if( random8() < chanceOfGlitter) {
        leds[ random16(NUM_LEDS) ] += CRGB::White;
    }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
    fadeToBlackBy( leds, NUM_LEDS, 10 );
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV( random8(255), 255, 255 );
    FastLED.show();
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 30);
    int pos = beatsin16(13, 0, LED_SEG-1);
    for(int i = 0; i < 4; i++){
        leds[i*LED_SEG+pos] += CHSV( gHue, 255, 192);
        yield();
    }
    FastLED.show();
}

void dot_beat() {
    uint8_t fadeval = 190;                                        // Trail behind the LED's. Lower => faster fade.
    uint8_t BPM = 30;


    uint8_t inner = beatsin8(BPM, LED_SEG/4, LED_SEG/4*3);    // Move 1/4 to 3/4
    uint8_t outer = beatsin8(BPM, 0, LED_SEG-1);               // Move entire length
    uint8_t middle = beatsin8(BPM, LED_SEG/3, LED_SEG/3*2);   // Move 1/3 to 2/3

    for(int i = 0; i < 4; i++){
        leds[i*LED_SEG+outer]  = CHSV( gHue    , 200, 255);
        leds[i*LED_SEG+middle] = CHSV( gHue+96 , 200, 255);
        leds[i*LED_SEG+inner]  = CHSV( gHue+160, 200, 255);
        yield();
    }

    nscale8(leds, NUM_LEDS, fadeval);                             // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);
    FastLED.show();
} // dot_beat()

void juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 10);
    byte dothue = 0;
    for( int i = 0; i < 8; i++) {
        leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
        dothue += 32;
        yield();
    }
    FastLED.show();
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
        leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
        yield();
    }
    FastLED.show();
}

void blendwave() {
    CRGB clr1, clr2;
    uint8_t speed, loc1, loc2, ran1, ran2;

    speed = beatsin8(6,0,255);
    
    clr1 = blend(CHSV(beatsin8(3,0,255),255,255), CHSV(beatsin8(4,0,255),255,255), speed);
    clr2 = blend(CHSV(beatsin8(4,0,255),255,255), CHSV(beatsin8(3,0,255),255,255), speed);
    loc1 = beatsin8(10,0,NUM_LEDS-1);

    fill_gradient_RGB(leds, 0, clr2, loc1, clr1);
    fill_gradient_RGB(leds, loc1, clr2, NUM_LEDS-1, clr1);
    FastLED.show();
} // blendwave()


void beatwave() {
  
    uint8_t wave1 = beatsin8(9, 0, 255);                        // That's the same as beatsin8(9);
    uint8_t wave2 = beatsin8(8, 0, 255);
    uint8_t wave3 = beatsin8(7, 0, 255);
    uint8_t wave4 = beatsin8(6, 0, 255);

    for (int i=0; i<NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, i+wave1+wave2+wave3+wave4, 255, currentBlending); 
        yield();
    }
    FastLED.show();
} // beatwave()

///////////////////////////////////////
//              SERVER               //
///////////////////////////////////////

void OTAsetup(){
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname("aquarium");
    
    ArduinoOTA.onStart([]() {
        Serial.println("Start");
        fill_solid (leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
        fill_solid (leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        uint32_t temp = progress / (total / 100);
        Serial.printf("Progress: %u%%\r", temp);
        SEG1[map(temp, 0, 100, 0, LED_SEG)] = CRGB::White;
        SEG2[map(temp, 0, 100, 0, LED_SEG)] = CRGB::White;
        SEG3[map(temp, 0, 100, 0, LED_SEG)] = CRGB::White;
        SEG4[map(temp, 0, 100, 0, LED_SEG)] = CRGB::White;
        FastLED.show();
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
}

void handleUpload(){ long int temp = 0;
	if(server.uri() != "/update") return;
	HTTPUpload& upload = server.upload();
	Serial.setDebugOutput(true);
	if(upload.status == UPLOAD_FILE_START){
        fill_solid (leds, NUM_LEDS, CRGB::Black); FastLED.show();
		Serial.printf("\r\nReceiving update file: %s\r\n", upload.filename.c_str()); temp = millis();
		WiFiUDP::stopAll();
		uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
		if(!Update.begin(maxSketchSpace)){ //start with MAX[8] available size
			Update.printError(Serial);
		}
	} else if(upload.status == UPLOAD_FILE_WRITE){
		Serial.print(".");
        leds[random16(NUM_LEDS)] = CHSV( random8(255), 255-random8(75), random8(255)-random8(75)); FastLED.show();
		if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
			Update.printError(Serial);
		}
	} else if(upload.status == UPLOAD_FILE_END){
		if(Update.end(true)){ //true to set the size to the current progress
			temp = millis() - temp;
			Serial.printf("\r\nUpdate Success: %u bytes in %u ms (", upload.totalSize, temp);
			Serial.print((float)upload.totalSize/temp);
			Serial.printf(" kB/s)\r\nEffective program size: %u bytes\r\n", upload.totalSize - 202656);
			Serial.print("\r\nRebooting...\r\n");
            fill_solid (leds, NUM_LEDS, CRGB::Black); FastLED.show();
		} else {
			Update.printError(Serial);
		}
	}
	yield();
	Serial.setDebugOutput(false);
}

void beginServer(){
	server.onFileUpload(handleUpload);
	server.on("/update", HTTP_POST, [](){
		server.sendHeader("Connection", "close");
		server.sendHeader("Access-Control-Allow-Origin", "*");
		server.send(200, "text/plain", (Update.hasError())?"FAIL\n":"OK\n");
		ESP.restart();
	});
	server.begin();
	Serial.print("\r\nESP8266 server started\r\n");
}

///////////////////////////////////////
//              BLYNK                //
///////////////////////////////////////

BLYNK_WRITE(0){
    FastLED.setBrightness(param.asInt());
    FastLED.show();
}

BLYNK_WRITE(1){
    manualRGB = CRGB(param[0].asInt(), param[1].asInt(), param[2].asInt());
    if(!_auto){
        fill_segments();
        FastLED.show();
    }
}

BLYNK_WRITE(2){
    _seg1 = param.asInt();
}

BLYNK_WRITE(3){
    _seg2 = param.asInt();
}

BLYNK_WRITE(4){
    _seg3 = param.asInt();
}

BLYNK_WRITE(5){
    _seg4 = param.asInt();
}

BLYNK_WRITE(6){
    _auto = param.asInt();
    if(!_auto){
        // for(int i = 0*LED_SEG; i < 1*LED_SEG; i++){
            // leds[i] = seg1;
            // yield();
        // }
        // for(int i = 1*LED_SEG; i < 2*LED_SEG; i++){
            // leds[i] = seg2;
            // yield();
        // }
        // for(int i = 2*LED_SEG; i < 3*LED_SEG; i++){
            // leds[i] = seg3;
            // yield();
        // }
        // for(int i = 3*LED_SEG; i < 4*LED_SEG; i++){
            // leds[i] = seg4;
            // yield();
        // }
        fill_segments();
        FastLED.show();
    }
}

BLYNK_CONNECTED() {
    manualRGB = CRGB::White;
    fill_solid(leds, NUM_LEDS, manualRGB);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.show();
    int t = 10;
    Blynk.virtualWrite(0, BRIGHTNESS); delay(t);
    Blynk.virtualWrite(1, manualRGB.r, manualRGB.g, manualRGB.b); delay(t);
    Blynk.virtualWrite(2, seg1); delay(t);
    Blynk.virtualWrite(3, seg2); delay(t);
    Blynk.virtualWrite(4, seg3); delay(t);
    Blynk.virtualWrite(5, seg4); delay(t);
    Blynk.virtualWrite(6, _auto); delay(t);
    // Blynk.syncAll();
    // delay(10);
    // Blynk.syncVirtual(V0);
    // delay(10);
    // Blynk.syncVirtual(V1);
    // delay(10);
    // Blynk.syncVirtual(V2);
    // delay(10);
    // Blynk.syncVirtual(V3);
    // delay(10);
    // Blynk.syncVirtual(V4);
    // delay(10);
    // Blynk.syncVirtual(V5);
    // delay(10);
    // Blynk.syncVirtual(V6);
    // delay(10);
}

void fill_segments(){
    if(_seg1){
        fill_solid(SEG1, LED_SEG, manualRGB);
    }
    if(_seg2){
        fill_solid(SEG2, LED_SEG, manualRGB);
    }
    if(_seg3){
        fill_solid(SEG3, LED_SEG, manualRGB);
    }
    if(_seg4){
        fill_solid(SEG4, LED_SEG, manualRGB);
    }
}