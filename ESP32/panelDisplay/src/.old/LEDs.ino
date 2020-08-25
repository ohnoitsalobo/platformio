#include <FastLED.h>
FASTLED_USING_NAMESPACE

#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define LED_PINS    13
#define NUM_LEDS    NUMBER_OF_LEDS
#define BRIGHTNESS  50 //110*110/255
#define FRAMES_PER_SECOND 60

CRGBArray<NUM_LEDS> leds;                              // LED array containing all LEDs
CRGBSet LEFT  (leds (0,          NUM_LEDS/2-1 ) );  // < subset containing only left  LEDs
CRGBSet RIGHT (leds (NUM_LEDS/2, NUM_LEDS-1)  );  // < subset containing only right LEDs

CRGBPalette16 currentPalette, randomPalette1;
CRGBPalette16 targetPalette, randomPalette2;
TBlendType    currentBlending;
uint8_t maxChanges = 24;        // Value for blending between palettes.

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { confetti, fillNoise, noise_noise2, Fire2018, matrixTest };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

bool manual = 0, _auto = 1;
CRGB manualColor = 0x000000;
CHSV manualHSV (0, 255, 255);
uint8_t gHue = 0, gHue1 = 0, gHue2 = 0; // rotating "base color" used by many of the patterns

String eqBroadcast = "";

static uint16_t x, y, z;
uint32_t scale_x, scale_y;
uint8_t CentreX =  (kMatrixWidth / 2) - 1;
uint8_t CentreY = (kMatrixHeight / 2) - 1;

void ledSetup(){
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5,1500);

    randomSeed(analogRead(5));
    x = random16();
    y = random16();
    z = random16();
    
    setup_tables();
}

void ledLoop(){
    if(music){
        audio_spectrum();
    }
    else if(_auto){
        EVERY_N_SECONDS(20){
            targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
            randomPalette1 = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
            randomPalette2 = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
        }
        EVERY_N_MILLISECONDS( 29 ) {
            gHue1++;
        }
        EVERY_N_MILLISECONDS( 23 ) {
            gHue2--;
        }
        gPatterns[gCurrentPatternNumber]();
    }
    else if(manual){
        
    }
    FastLED.show();
}

void audio_spectrum(){
    eqBroadcast = "";
    uint8_t pos = 0;
    for(int i = 2; i < samples/2; i++){
        uint8_t pos = spectrum[0][i];
        uint8_t h = pos/(NUM_LEDS/2.0)*224;
        double temp = spectrum[1][i]/MAX;
        uint8_t s = 255 - (temp*30.0);
        uint8_t v = temp*255.0;
        // if(i < 12){
            // int temp = v*12/i;
            // v = temp > 255 ? 255 : temp;
        // }
        RIGHT[pos] = CHSV(h, s, v);
        // RIGHT(1,2) = RIGHT[0];
        // RIGHT[3] = RIGHT[4];
        // RIGHT[5] = RIGHT[6];
        // RIGHT[8] = RIGHT[7];
        // RIGHT(1, 5) = RIGHT[0];
        // RIGHT(7, 8) = RIGHT[6];
        // RIGHT[10]   = RIGHT[9];
        // RIGHT[13]   = RIGHT[12];
        // RIGHT[17]   = RIGHT[16];
        LEFT = -RIGHT;

        if(music && webSocketConn()){
            eqBroadcast += ",";
            eqBroadcast += String(v);
        }
    }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern(){
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns );
}

void rainbow() {
    // FastLED's built-in rainbow generator
    fill_rainbow( LEFT , NUM_LEDS/2, gHue2);
    fill_rainbow( RIGHT, NUM_LEDS/2, gHue1);
}

void rainbowWithGlitter() {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter();
}

void addGlitter() {
    if( random8() < 80) {
        leds[ random16(NUM_LEDS) ] += CRGB::White;
    }
}

void confetti() 
{
    EVERY_N_MILLISECONDS(1000 / 50){
        fadeToBlackBy( leds, NUM_LEDS, 10);
        int pos = random16(NUM_LEDS);
        leds[pos] += CHSV( gHue1 + random8(64), 190+random8(65), 255);
    }
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 10);
    int pos1 = beatsin16(11, 0, NUM_LEDS/2-1);
    int pos2 = beatsin16(13, 0, NUM_LEDS/2-1);
    int pos3 = beatsin16( 9, 0, NUM_LEDS/2-1);
    int pos4 = beatsin16(15, 0, NUM_LEDS/2-1);
    LEFT [pos1] = ColorFromPalette(randomPalette1, pos1, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.
    RIGHT[pos2] = ColorFromPalette(randomPalette2, pos2, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.
    LEFT [pos3] += CHSV( gHue2, 255, 255);
    RIGHT[pos4] += CHSV( gHue1, 255, 255);
}

void dot_beat() {
    uint8_t fadeval = 190;                                        // Trail behind the LED's. Lower => faster fade.
    uint8_t BPM = 30;


    uint8_t inner = beatsin8(BPM, NUM_LEDS/4, NUM_LEDS/4*3);    // Move 1/4 to 3/4
    uint8_t outer = beatsin8(BPM, 0, NUM_LEDS-1);               // Move entire length
    uint8_t middle = beatsin8(BPM, NUM_LEDS/3, NUM_LEDS/3*2);   // Move 1/3 to 2/3

    leds[outer]  = CHSV( gHue1    , 200, 255); // CRGB::Aqua;
    leds[middle] = CHSV( gHue1+96 , 200, 255); // CRGB::Purple;
    leds[inner]  = CHSV( gHue1+160, 200, 255); // CRGB::Blue;

    nscale8(leds,NUM_LEDS,fadeval);                             // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);
} // dot_beat()

void juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 15);
    byte dothue1 = 0, dothue2 = 0;
    for( int i = 0; i < 8; i++) {
        RIGHT[beatsin16(i+7,0,NUM_LEDS/2)] |= CHSV(dothue1, 200, 255);
        LEFT [beatsin16(i+5,0,NUM_LEDS/2)] |= CHSV(dothue2, 200, 255);
        dothue1 += 32;
        dothue2 -= 32;
        yield();
    }
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    // CRGBPalette16 palette = PartyColors_p;
    CRGBPalette16 palette = RainbowColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS/2; i++) { //9948
        RIGHT[i]              = ColorFromPalette(palette, gHue1+(i*2), beat-gHue1+(i*10));
        LEFT [NUM_LEDS/2-1-i] = ColorFromPalette(palette, gHue2+(i*2), beat-gHue2+(i*10));
        yield();
    }
}


uint16_t XY( uint8_t x, uint8_t y){
    uint16_t i;
    x %= kMatrixWidth;
    y %= kMatrixHeight;
    if( y & 0x01) {
        // Odd rows run backwards
        uint8_t reverseX = (kMatrixWidth - 1) - x;
        i = (y * kMatrixWidth) + reverseX;
    } else {
        // Even rows run forwards
        i = (y * kMatrixWidth) + x;
    }
    
    return NUM_LEDS-1-i;
}

void matrixTest()
{
    EVERY_N_MILLISECONDS(1000 / 60){
        uint32_t ms = millis();
        int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / kMatrixWidth));
        int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / kMatrixHeight));
        DrawOneFrame( ms / 65536, yHueDelta32 / 32768, xHueDelta32 / 32768);
    }
}

void DrawOneFrame( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8)
{
  byte lineStartHue = startHue8;
  for( byte y = 0; y < kMatrixHeight; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;      
    for( byte x = 0; x < kMatrixWidth; x++) {
      pixelHue += xHueDelta8;
      leds[ XY(x, y)]  = CHSV( pixelHue+gHue1, 255, 175);
    }
  }
}

void fillNoise(){
    // fill_2dnoise16 (CRGB *leds, int width, int height, bool serpentine, uint8_t octaves, uint32_t x, int xscale, uint32_t y, int yscale, uint32_t time, uint8_t hue_octaves, uint16_t hue_x, int hue_xscale, uint16_t hue_y, uint16_t hue_yscale, uint16_t hue_time, bool blend, uint16_t hue_shift=0)


    uint16_t _speed = 1;  // 1 = like painting; 100 = really fast
    uint16_t _scale = 100; // zoom in/out
    uint8_t noise_[MAX_DIMENSION][MAX_DIMENSION];

    for(int i = 0; i < MAX_DIMENSION; i++) {
        int ioffset = _scale * i;
        for(int j = 0; j < MAX_DIMENSION; j++) {
            int joffset = _scale * j;
            noise_[i][j] = inoise8(x + ioffset,y + joffset,z);
        }
    }
    z += _speed;
    for(int i = 0; i < kMatrixWidth; i++) {
        for(int j = 0; j < kMatrixHeight; j++) {
            // We use the value at the (i,j) coordinate in the noise
            // array for our brightness, and the flipped value from (j,i)
            // for our pixel's hue.
            leds[XY(i,j)] = CHSV(gHue1 + noise_[j][i], 255, noise_[i][j]);

            // You can also explore other ways to constrain the hue used, like below
            // leds[XY(i,j)] = CHSV(gHue1 + (noise_[j][i]>>2), 255, noise_[i][j]);
        }
    }
}

// storage for the noise data
// adjust the size to suit your setup
uint8_t noise_[16][16];

// heatmap data with the size matrix width * height
uint8_t heat[kMatrixHeight*kMatrixWidth];

// the color palette
CRGBPalette16 Pal = HeatColors_p;
void Fire2018() {

  // get one noise value out of a moving noise space
  uint16_t ctrl1 = inoise16(11 * millis(), 0, 0);
  // get another one
  uint16_t ctrl2 = inoise16(13 * millis(), 100000, 100000);
  // average of both to get a more unpredictable curve
  uint16_t  ctrl = ((ctrl1 + ctrl2) / 2);

  // this factor defines the general speed of the heatmap movement
  // high value = high speed
  uint8_t speed = 25;

  // here we define the impact of the wind
  // high factor = a lot of movement to the sides
  x = 3 * ctrl * speed;

  // this is the speed of the upstream itself
  // high factor = fast movement
  y = 13 * millis() * speed;

  // just for ever changing patterns we move through z as well
  z = 3 * millis() * speed ;

  // ...and dynamically scale the complete heatmap for some changes in the
  // size of the heatspots.
  // The speed of change is influenced by the factors in the calculation of ctrl1 & 2 above.
  // The divisor sets the impact of the size-scaling.
  scale_x = ctrl1 / 2;
  scale_y = ctrl2 / 2;

  // Calculate the noise array based on the control parameters.
  uint8_t layer = 0;
  for (uint8_t i = 0; i < kMatrixWidth; i++) {
    uint32_t ioffset = scale_x * (i - CentreX);
    for (uint8_t j = 0; j < kMatrixHeight; j++) {
      uint32_t joffset = scale_y * (j - CentreY);
      uint16_t data = ((inoise16(x + ioffset, y + joffset, z)) + 1);
      noise_[i][j] = data >> 8;
    }
  }


  // Draw the first (lowest) line - seed the fire.
  // It could be random pixels or anything else as well.
  for (uint8_t x = 0; x < kMatrixWidth; x++) {
    // draw
    leds[XY(x, kMatrixHeight-1)] = ColorFromPalette( Pal, noise_[x][0]);
    // and fill the lowest line of the heatmap, too
    heat[XY(x, kMatrixHeight-1)] = noise_[x][0];
  }

  // Copy the heatmap one line up for the scrolling.
  for (uint8_t y = 0; y < kMatrixHeight - 1; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {
      heat[XY(x, y)] = heat[XY(x, y + 1)];
    }
  }

  // Scale the heatmap values down based on the independent scrolling noise array.
  for (uint8_t y = 0; y < kMatrixHeight - 1; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {

      // get data from the calculated noise field
      uint8_t dim = noise_[x][y];

      // This number is critical
      // If it´s to low (like 1.1) the fire dosn´t go up far enough.
      // If it´s to high (like 3) the fire goes up too high.
      // It depends on the framerate which number is best.
      // If the number is not right you loose the uplifting fire clouds
      // which seperate themself while rising up.
      dim = dim / 1.2;

      dim = 255 - dim;

      // here happens the scaling of the heatmap
      heat[XY(x, y)] = scale8(heat[XY(x, y)] , dim);
    }
  }

  // Now just map the colors based on the heatmap.
  for (uint8_t y = 0; y < kMatrixHeight - 1; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {
      leds[XY(x, y)] = ColorFromPalette( Pal, heat[XY(x, y)]);
    }
  }

  // Done. Bring it on!

  // I hate this delay but with 8 bit scaling there is no way arround.
  // If the framerate gets too high the frame by frame scaling doesn´s work anymore.
  // Basically it does but it´s impossible to see then...

  // If you change the framerate here you need to adjust the
  // y speed and the dim divisor, too.
  delay(10);

}

//noise_smooth params//
CRGBArray<NUM_LEDS> buffer1, buffer2;
// parameters and buffer for the noise array
#define NUM_LAYERS 2
uint32_t X[NUM_LAYERS];
uint32_t Y[NUM_LAYERS];
uint32_t Z[NUM_LAYERS];
uint32_t scale_X[NUM_LAYERS];
uint32_t scale_Y[NUM_LAYERS];
uint16_t Noise[NUM_LAYERS][kMatrixWidth][kMatrixHeight];
// colortables
uint8_t a[1024];
uint8_t b[1024];
uint8_t c[1024];
//control parameters
uint8_t ctrl[6];
///////////////////

void setup_tables() {
    for (uint16_t i = 0; i < 1024; i++) {
      a[i] = sin8(i/4) ;
      b[i] = 0;
      c[i] = cubicwave8( i/2) ;
    }
  // for (uint16_t i = 256; i < 768; i++) {
    // a[i] = triwave8(127 + (i / 2)) ;
    //b[i] = 0;
    // c[i] = triwave8(127 + (i / 2)) ;
  // }
  
    X[0] = random(60000);
    Y[0] = random(60000);
    Z[0] = random(60000);
    X[1] = random(60000);
    Y[1] = random(60000);
    Z[1] = random(60000);
}

void adjust_gamma()
{
  // minimal brightness you want to allow
  // make sure to have the global brightnes on maximum and run no other color correction
  // a minimum of min = 1 might work fine for you and allow more contrast
  uint8_t min = 3;
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i].r = dim8_video(leds[i].r);
    leds[i].g = dim8_video(leds[i].g);
    leds[i].b = dim8_video(leds[i].b);

    if (leds[i].r < min) leds[i].r = min;
    if (leds[i].g < min) leds[i].g = min;
    if (leds[i].b < min) leds[i].b = min;
  }
}

void noise_noise2() {

  // LAYER ONE
  //top left
  ctrl[0] = (ctrl[0] + Noise[0][0][0] + Noise[0][1][0] + Noise [0][0][1] + Noise[0][1][1]) / 20;
  //top right
  ctrl[1] = (ctrl[1] + Noise[0][kMatrixWidth - 1][0] + Noise[0][kMatrixWidth - 2][0] + Noise [0][kMatrixWidth - 1][1] + Noise[0][kMatrixWidth - 2][1]) / 20;
  //down left
  ctrl[2] = (ctrl[2] + Noise[0][0][kMatrixHeight - 1] + Noise[0][0][kMatrixHeight - 2] + Noise [0][1][kMatrixHeight - 1] + Noise[0][1][kMatrixHeight - 2]) / 20;
  //middle left
  ctrl[3] = (ctrl[3] + Noise[0][0][CentreY] + Noise[0][1][CentreY] + Noise [0][0][CentreY + 1] + Noise[0][1][CentreY + 1]) / 20;
  //center
  ctrl[4] = (ctrl[4] + Noise[0][kMatrixWidth - 1][CentreY] + Noise[0][kMatrixWidth - 2][CentreY] + Noise [0][kMatrixWidth - 1][CentreY + 1] + Noise[0][kMatrixWidth - 2][CentreY + 1]) / 20;
  ctrl[5] = (ctrl[5] + ctrl[0] + ctrl[1]) / 12;

  X[0] = X[0] + (ctrl[0]) - 127;
  Y[0] = Y[0] + (ctrl[1]) - 127;
  Z[0] += 1 + (ctrl[2] / 4);
  scale_X[0] = 8000 + (ctrl[3] * 16);
  scale_Y[0] = 8000 + (ctrl[4] * 16);

  //calculate the Noise data
  uint8_t layer = 0;
  for (uint8_t i = 0; i < kMatrixWidth; i++) {
    uint32_t ioffset = scale_X[layer] * (i - CentreX);
    for (uint8_t j = 0; j < kMatrixHeight; j++) {
      uint32_t joffset = scale_Y[layer] * (j - CentreY);
      uint16_t data = inoise16(X[layer] + ioffset, Y[layer] + joffset, Z[layer]);
      if (data < 11000) data = 11000;
      if (data > 51000) data = 51000;
      data = data - 11000;
      data = data / 41;
      Noise[layer][i][j] = (Noise[layer][i][j] + data) / 2;
    }
  }

  //map the colors
  //here the red layer
  for (uint8_t Y = 0; Y < kMatrixHeight; Y++) {
    for (uint8_t X = 0; X < kMatrixWidth; X++) {
      uint16_t i = Noise[0][X][Y];
      buffer1[XY(X, Y)] = CRGB(a[i], c[i], 0);
    }
  }

  // LAYER TWO
  //top left
  ctrl[0] = (ctrl[0] + Noise[1][0][0] + Noise[1][1][0] + Noise [1][0][1] + Noise[1][1][1]) / 20;
  //top right
  ctrl[1] = (ctrl[1] + Noise[1][kMatrixWidth - 1][0] + Noise[1][kMatrixWidth - 2][0] + Noise [1][kMatrixWidth - 1][1] + Noise[1][kMatrixWidth - 2][1]) / 20;
  //down left
  ctrl[2] = (ctrl[2] + Noise[1][0][kMatrixHeight - 1] + Noise[1][0][kMatrixHeight - 2] + Noise [1][1][kMatrixHeight - 1] + Noise[1][1][kMatrixHeight - 2]) / 20;
  //middle left
  ctrl[3] = (ctrl[3] + Noise[1][0][CentreY] + Noise[1][1][CentreY] + Noise [1][0][CentreY + 1] + Noise[1][1][CentreY + 1]) / 20;
  //center
  ctrl[4] = (ctrl[4] + Noise[1][kMatrixWidth - 1][CentreY] + Noise[1][kMatrixWidth - 2][CentreY] + Noise [1][kMatrixWidth - 1][CentreY + 1] + Noise[1][kMatrixWidth - 2][CentreY + 1]) / 20;
  ctrl[5] = (ctrl[5] + ctrl[0] + ctrl[1]) / 12;

  X[1] = X[1] + (ctrl[0]) - 127;
  Y[1] = Y[1] + (ctrl[1]) - 127;
  Z[1] += 1 + (ctrl[2] / 4);
  scale_X[1] = 8000 + (ctrl[3] * 16);
  scale_Y[1] = 8000 + (ctrl[4] * 16);

  //calculate the Noise data
  layer = 1;
  for (uint8_t i = 0; i < kMatrixWidth; i++) {
    uint32_t ioffset = scale_X[layer] * (i - CentreX);
    for (uint8_t j = 0; j < kMatrixHeight; j++) {
      uint32_t joffset = scale_Y[layer] * (j - CentreY);
      uint16_t data = inoise16(X[layer] + ioffset, Y[layer] + joffset, Z[layer]);
      if (data < 11000) data = 11000;
      if (data > 51000) data = 51000;
      data = data - 11000;
      data = data / 41;
      Noise[layer][i][j] = (Noise[layer][i][j] + data) / 2;
    }
  }

  //map the colors
  //here the blue layer
  for (uint8_t Y = 0; Y < kMatrixHeight; Y++) {
    for (uint8_t X = 0; X < kMatrixWidth; X++) {
      uint16_t i = Noise[1][X][Y];
      buffer2[XY(X, Y)] = CRGB(0, a[i], c[i]);
    }
  }

  // blend 
  //for (uint16_t i = 0; i < NUM_LEDS; i++) {leds[i] = buffer1[i] + buffer2[i];}
  for (uint8_t Y = 0; Y < kMatrixHeight; Y++) {
    for (uint8_t X = 0; X < kMatrixWidth; X++) {
      leds[XY(X, Y)] = blend(buffer1[XY(X, Y)], buffer2[XY(X, Y)], Noise[1][Y][X] / 4);
      // you could also just add them:
      // leds[XY(X, Y)] = buffer1[XY(X, Y)] + buffer2[XY(X, Y)];
    }
  }

  //make it looking nice
  adjust_gamma();

}
