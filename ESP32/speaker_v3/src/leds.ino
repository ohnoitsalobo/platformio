#define TIMES_PER_SECOND(x) EVERY_N_MILLISECONDS(1000/x)
#define TIMES_PER_MINUTE(x) EVERY_N_SECONDS(60/x)
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#include <Gaussian.h>

FASTLED_USING_NAMESPACE

typedef void (*SimplePatternList[])();
SimplePatternList audioPatterns = { audioLight, audio_spectrum };
SimplePatternList gPatterns = { fire, tape_reel, fireworks, confetti, ripple_blur, cylon1, sinelon, juggle, bpm, rainbow, rainbowWithGlitter, rainbow_scaling, drawClock };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

void ledSetup(){
    FastLED.addLeds< LED_TYPE, LED_PINS, COLOR_ORDER >( leds, NUM_LEDS ).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(currentBrightness);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
    
    fill_solid (leds, NUM_LEDS, CRGB::Black);
    FastLED.show();

}

void ledLoop(){
    EVERY_N_SECONDS(1){
        checkMIDI();
    }
    EVERY_N_MILLISECONDS(50){ gHue++; gHue1++; gHue2--;}
    
    switch(_mode){
        case _audio:
            audioPatterns[gCurrentPatternNumber]();
        break;
        case _midi:
            MIDIloop();
            displayMIDI();
        break;
        case _auto:
            EVERY_N_MILLISECONDS( 1000 / 60 ){  // Call the current pattern function once, updating the 'leds' array
                gPatterns[gCurrentPatternNumber]();
            }
            if(auto_advance) {
                EVERY_N_SECONDS( 40 ) { 
                    nextPattern(); 
                }
            }   // change patterns periodically
        break;
        case _manual:
            adjustColors();
        break;
        default:
        break;
    }
    adjustBrightness();
    // R1[0] = CRGB::Blue;
    // R2[0] = CRGB::Yellow;
    // L1[0] = CRGB::Red;
    // L2[0] = CRGB::Green;
    
    FastLED.show();
    yield();
    
}

void adjustBrightness(){
         if(currentBrightness < _setBrightness) FastLED.setBrightness(++currentBrightness);
    else if(currentBrightness > _setBrightness) FastLED.setBrightness(--currentBrightness);
}
void adjustColors(){
    for(int i = 0; i < half_size; i++){
        for(int j = 0; j < 3; j++){
                 if(LEFT [i][j] < manualColor_L [j]) LEFT [i][j]++;
            else if(LEFT [i][j] > manualColor_L [j]) LEFT [i][j]--;
                 if(RIGHT[i][j] < manualColor_R [j]) RIGHT[i][j]++;
            else if(RIGHT[i][j] > manualColor_R [j]) RIGHT[i][j]--;
        }
    }
}

void nextPattern(){
    currentBrightness = 0;
    uint8_t temp = 0;
    if(_mode == _audio) temp = ARRAY_SIZE( audioPatterns );
    else                temp = ARRAY_SIZE( gPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % temp;
} // advance to the next pattern

void previousPattern(){
    currentBrightness = 0;
    uint8_t temp = 0;
    if(_mode == _audio) temp = ARRAY_SIZE( audioPatterns );
    else                temp = ARRAY_SIZE( gPatterns );
    gCurrentPatternNumber = (gCurrentPatternNumber + (temp-1)) % temp;
} // advance to the previous pattern

//////////////// Audio reactive patterns ////////////////

void audio_spectrum(){ // using arduinoFFT to calculate frequencies and mapping them to light spectrum
#ifdef _debug
    _serial_.println("Starting audio_spectrum");
#endif
    // EVERY_N_MILLISECONDS(100){ _serial_.println(_max); }
    uint8_t fadeval = 200;
    nscale8(leds, NUM_LEDS, fadeval); // smaller = faster fade
    CRGB tempRGB1, tempRGB2;
    uint8_t pos = 0, h = 0, s = 0, v = 0;
    double temp1 = 0, temp2 = 0;
    for(int i = 2; i < samples/2; i++){
        pos = spectrum[0][i];
        h = pos/(NUM_LEDS/4.0)*224;
        temp1 = spectrum[1][i]/MAX;
        s = 255 - (temp1*30.0);
        v = temp1*255.0;
        tempRGB1 = CHSV(h, s, v);
        uint8_t p = NUM_LEDS/4-pos;
        if(tempRGB1 > R1[pos]){
            R1[pos] = tempRGB1;
        }
        R2[p] = R1[pos];

        temp2 = spectrum[2][i]/MAX;
        s = 255 - (temp2*30.0);
        v = temp2*255.0;
        tempRGB2 = CHSV(h, s, v);
        if(tempRGB2 > L1[pos]){
            L1[pos] = tempRGB2;
        }
        L2[p] = L1[pos];
        yield();
    }
#ifdef _debug
    _serial_.println("Ending audio_spectrum");
#endif
}

void audioLight(){ // directly sampling ADC values mapped to brightness
#ifdef debug
    _serial_.println("Starting audioLight");
#endif
    EVERY_N_MILLISECONDS( 55 ) { gHue1++; }
    EVERY_N_MILLISECONDS( 57 ) { gHue2--; }
    EVERY_N_MILLISECONDS( 15 ) {
        uint8_t fadeval = 210;
        for(int i = 0; i < NUM_LEDS/4; i++){
            R1[NUM_LEDS/4-i] = R1[NUM_LEDS/4-1-i].nscale8(fadeval);
            R2[i] = R1[NUM_LEDS/4-i];
            L1[NUM_LEDS/4-i] = L1[NUM_LEDS/4-1-i].nscale8(fadeval);
            L2[i] = L1[NUM_LEDS/4-i];
        }
        uint16_t mid = 1800, _noise = 180;
        uint8_t _hue = 0, _sat = 255, _val = 0;
        int temp1 = abs(mid - analogRead( RightPin));
        if(temp1 > _noise){
            _val = (temp1-_noise)/float(mid) * 255;
            _hue = _val/255.0 * 65;
        }
        R1[0] = CHSV( _hue+gHue1, _sat, _val);
        R2[NUM_LEDS/4] = R1[0];
        
        _hue = 0; _val = 0;
        int temp2 = abs(mid - analogRead( LeftPin));
        if(temp2 > _noise){
            _val = (temp2-_noise)/float(mid) * 255;
            _hue = _val/255.0 * 65;
        }
        L2[NUM_LEDS/4] = CHSV( _hue+gHue2, _sat, _val);
        L1[0] = L2[NUM_LEDS/4];
    }
#ifdef debug
    _serial_.println("Ending audioLight");
#endif
}
/////////////////////////////////////////////////////////

//////////////// Normal patterns ////////////////

void rainbow() {
    // FastLED's built-in rainbow generator
    fill_rainbow( RIGHT, half_size, gHue1, 23);
    fill_rainbow( LEFT , half_size, gHue2, 23);
} // rainbow

void rainbowWithGlitter() {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter();
} // rainbow with glitter

void addGlitter() {
    EVERY_N_MILLISECONDS(1000/30){
        if( random8() < 80) {
            leds[ random16(NUM_LEDS) ] += CRGB::White;
        }
    }
}

void rainbow_scaling(){
    for(int i = 0; i <= quarter_size; i++){
        R1[i] = CHSV((millis()/77*i+1)%255 + gHue1, 255, 255);
        R2[quarter_size-i] = R1[i];
        L1[i] = CHSV((millis()/73*i+1)%255 - gHue2, 255, 255);
        L2[quarter_size-i] = L1[i];
    }
} // rainbow scaling


void cylon(){
    float width = 2.0f, res = 3.0f;
    static Gaussian _cylon(0, width*width);
    float pos = beatsin16(_BPM/2, width*res, (NUM_LEDS-1-width)*res);
    for (int i = 0; i < NUM_LEDS; i++){
        float _val;
        _cylon.mean = pos/res;
        _val = _cylon.plot(i)*(width/0.399);
        leds[i] = CHSV(0, 255, _val*255);
    }
    yield();
}


uint8_t fadeval = 200, frameRate = 60;
void fire(){ // my own 'fire' code - randomly generate color and move it up the strip while fading
    EVERY_N_MILLISECONDS(1000/frameRate){
        for(int i = 0; i < quarter_size; i++){
            R1[quarter_size-i] = R1[quarter_size-1-i].nscale8(fadeval); if(R1[quarter_size-i].g > 0) R1[quarter_size-i].g--;
            R2[i] = R1[quarter_size-i];
            L1[quarter_size-i] = L1[quarter_size-1-i].nscale8(fadeval); if(L1[quarter_size-i].g > 0) L1[quarter_size-i].g--;
            L2[i] = L1[quarter_size-i];
        }
        uint8_t _hue = 0, _sat = 255, _val = 0;
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 45);
        _hue = _val/255.0 *_val/255.0 * 55;
        R1[0] = CHSV( _hue, _sat, _val*_val/255);
        R2[quarter_size] = R1[0];
        
        _val = random(0, 255);
        _sat = 255 - (_val/255.0 * 45);
        _hue = _val/255.0 *_val/255.0 * 55;
        L2[quarter_size] = CHSV( _hue, _sat, _val*_val/255);
        L1[0] = L2[quarter_size];
    }
    yield();
}

const int sparks = 5;
FireWork _firework[2][sparks];
void fireworks(){
    fadeToBlackBy(leds, NUM_LEDS, 100);
    for (int i = 0; i < sparks; i++){
        _firework[0][i].draw(LEFT , half_size);
        _firework[1][i].draw(RIGHT, half_size);
    }
    EVERY_N_SECONDS(2){
        if(random8() < 150){
            for (int i = 0; i < sparks; i++){
                _firework[0][i].init(random16(NUM_LEDS/6, NUM_LEDS/3));
            }
        }
        if(random8() < 150){
            for (int i = 0; i < sparks; i++){
                _firework[1][i].init(random16(NUM_LEDS/6, NUM_LEDS/3));
            }
        }
    }
}

void firework_init(){
    for (int i = 0; i < sparks; i++){
        _firework[1][i].init(random16(NUM_LEDS/6, NUM_LEDS/3));
    }
}

void figure_8(){
    FastLED.clear();
    float b = beat16(20)/65535.0 * (half_size);
    DrawPixels(b, 2, CHSV(gHue, 255, 255), LEFT, half_size);
    float c = beat16(19)/65535.0 * (half_size);
    DrawPixels(c, 2, CHSV(gHue, 255, 255), RIGHT, half_size);
    FastLED.show();
}

void tape_reel(){
    //fadeToBlackBy( leds, NUM_LEDS, 200);
    FastLED.clear();
    float b;
    static float spots = 1.0; static float incr = 0.005;
    for(float i = 0; i < spots; i++){
        float offset = half_size/spots;

        float _fractL = beat16(23)/65535.0;
        b = _fractL * half_size + i*offset;
        DrawPixels(b, 1.5, CHSV(gHue1, 255, 255),  LEFT, half_size);

        float _fractR = beat16(22)/65535.0;
        b = (_fractR) * half_size + i*offset;
        DrawPixels(b, 1.5, CHSV(gHue2, 255, 255), RIGHT, half_size);
    }
    if(spots < 1){
        incr = 0.005;
    }
    if(spots > 9){
        incr = -0.005;
    }
    spots += incr;
}

void tape_reel3(){
    //fadeToBlackBy( leds, NUM_LEDS, 200);
    FastLED.clear();
    float b;
    b = (beat16(30)/65535.0) * half_size;
    DrawPixels(b, 1.5, CHSV(gHue, 255, 255),  LEFT, half_size);
    b+= NUM_LEDS/3;
    DrawPixels(b, 1.5, CHSV(gHue, 255, 255),  LEFT, half_size);
    b+= NUM_LEDS/3;
    DrawPixels(b, 1.5, CHSV(gHue, 255, 255),  LEFT, half_size);

    b = (beat16(29)/65535.0) * half_size;
    DrawPixels(b, 1.5, CHSV(gHue, 255, 255), RIGHT, half_size);
    b+= NUM_LEDS/3;
    DrawPixels(b, 1.5, CHSV(gHue, 255, 255), RIGHT, half_size);
    b+= NUM_LEDS/3;
    DrawPixels(b, 1.5, CHSV(gHue, 255, 255), RIGHT, half_size);
}

void confetti() 
{    // random colored speckles that blink in and fade smoothly
    EVERY_N_MILLISECONDS(1000/40){
        fadeToBlackBy( leds, NUM_LEDS, 20);
        if(random8() < 50){
            int pos = random16(NUM_LEDS);
            // leds[pos] += CHSV( random8(255), 255, 255);
            leds[pos] += CHSV( gHue + random8(64), 190+random8(65), 255);
        }
    }
}

uint8_t blurval = 30;
void ripple_blur(){ // randomly drop a light somewhere and blur it using blur1d
    EVERY_N_MILLISECONDS(1000/30){
        blur1d(leds, NUM_LEDS, blurval);
    }
    EVERY_N_MILLISECONDS(30){
        if( random8() < 15) {
            uint8_t pos = random(NUM_LEDS/2);
            LEFT [pos] = CHSV(random(0, 64)+gHue1, random(250, 255), 255);
        }
        if( random8() < 15) {
            uint8_t pos = random(NUM_LEDS/2);
            RIGHT [pos] = CHSV(random(0, 64)-gHue2, random(250, 255), 255);
        }
    }
}

double temp = 5.0; // set this for the "resolution" of the bell curve temp*NUM_LEDS
double width = 5; // set this for the "width" of the bell curve (how many LEDs to light)
double _smear = 4.0*sqrt(1.0/(width*width*width)); // this calculates the necessary coefficient for a width of w
void cylon1(){
    EVERY_N_MILLISECONDS( _BPM   ) { gHue1++; }
    EVERY_N_MILLISECONDS( _BPM-2 ) { gHue2--; }
    nscale8( leds, NUM_LEDS, 20);
    int posL, posR, val;
    posL = beatsin16(_BPM/2  , 0.9*width*temp, (NUM_LEDS-0.9*width)*temp);  // range of input. the 'w' prevents it from hitting the sides
    posR = beatsin16(_BPM/2+1, 0.9*width*temp, (NUM_LEDS-0.9*width)*temp);  // range of input. the 'w' prevents it from hitting the sides
    double a;
    double scaledposL = posL/(NUM_LEDS*temp) * NUM_LEDS/2; // range scaled down to working length
    double scaledposR = posR/(NUM_LEDS*temp) * NUM_LEDS/2; // range scaled down to working length
    for(int i = 0; i < NUM_LEDS/2; i++){
        a = i-scaledposL;       // 
        a = -_smear*a*a;        // generate bell curve with coefficient calculated above for chosen width
        val = 255.0*pow(2, a);  // 
        LEFT [i] |= CHSV(gHue2, 255, val);
        a = i-scaledposR;       // 
        a = -_smear*a*a;        // generate bell curve with coefficient calculated above for chosen width
        val = 255.0*pow(2, a);  // 
        RIGHT[i] |= CHSV(gHue1, 255, val);
    }
    yield();
}

void juggle() {
    // colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 50);
    byte dothue = 0;
    for( int i = 0; i < 4; i++) {
        float posL = beatsin16(i+6,0,half_size-2);
        float posR = beatsin16(i+7,0,half_size-2);
        // leds[beatsin16(i+7,0,NUM_LEDS-1)] |= CHSV(dothue, 200, 255);
        DrawPixels(posL, 1.5, CHSV(dothue+64, 200, 255), LEFT , half_size);
        DrawPixels(posR, 1.5, CHSV(dothue   , 200, 255), RIGHT, half_size);
        dothue += 32;
        yield();
    }
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    // uint8_t BeatsPerMinute = 62;
    uint8_t BeatsPerMinute = _BPM;
    // CRGBPalette16 palette = PartyColors_p;
    CRGBPalette16 palette = RainbowColors_p;
    uint8_t beatL = beatsin8( BeatsPerMinute-1, 64, 255);
    uint8_t beatR = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < half_size; i++) { //9948
        LEFT [i] = ColorFromPalette(palette, gHue+(i*2), beatL-gHue1+(i*10));
        RIGHT[half_size-i] = ColorFromPalette(palette, gHue+(i*2), beatR-gHue2+(i*10));
        yield();
    }
}

void blendwave() {
    CRGB clr1, clr2;
    uint8_t speed, loc1;

    speed = beatsin8(6,0,255);

    clr1 = blend(CHSV(beatsin8(3,0,255),255,255), CHSV(beatsin8(4,0,255),255,255), speed);
    clr2 = blend(CHSV(beatsin8(4,0,255),255,255), CHSV(beatsin8(3,0,255),255,255), speed);
    loc1 = beatsin8(13,0,NUM_LEDS-1);

    fill_gradient_RGB(leds, 0, clr2, loc1, clr1);
    fill_gradient_RGB(leds, loc1, clr2, NUM_LEDS-1, clr1);
    
    yield();
} // blendwave()

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 60);
    float posL = beatsin16(20, 0, (half_size-1.5)*5.0)/5.0;
    float posR = beatsin16(21, 0, (half_size-1.5)*5.0)/5.0;
    DrawPixels(posL, 1.5, ColorFromPalette(RainbowColors_p, posL, 255, LINEARBLEND), LEFT , half_size);
    DrawPixels(posR, 1.5, ColorFromPalette(RainbowColors_p, posR, 255, LINEARBLEND), RIGHT, half_size);
    // leds [pos] = ColorFromPalette(RainbowColors_p, pos, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.
    yield();
}

void drawClock(){
    if(timeStatus() == timeNotSet){
        timeLoop();
        if(WSdata.startsWith("prev"))
            previousPattern();
        else
            nextPattern();
    }else{
        fadeToBlackBy( leds, NUM_LEDS, 150);
        int   sec     = millis()%60000;
        float secPos  = fmod((1.0f - sec/60000.0) * half_size + quarter_size, half_sizef);
        int   min     = ::now()%(60*60);
        float minPos  = fmod((1.0f - min/(60.0*60.0)) * half_size + quarter_size, half_sizef);
        int   _hour   = ::now()%(60*60*12);
        float hourPos = fmod((1.0f - _hour/(60.0*60.0*12.0)) * half_size + quarter_size, half_sizef);
        int   p       = beatsin16(60, (NUM_LEDS*5)/3, (NUM_LEDS*5)/3*2);              // range of input
        float pPos    = p/(NUM_LEDS*5.0) * (NUM_LEDS/2-1); // range scaled down to working length
        
        DrawPixels(hourPos, 1.0, CRGB::Red  , RIGHT, half_size);
        DrawPixels(minPos , 1.0, CRGB::Green, RIGHT, half_size);
        DrawPixels(secPos , 1.0, CRGB::Blue , RIGHT, half_size);
        DrawPixels(pPos   , 1.0, CRGB::Brown, LEFT , half_size);
        /*  * /
        for(int i = 0; i < NUM_LEDS/2; i++){
            double a, b, c, y, w = 2;
            int bri = 255;
            a = i+secPos;              a = -w*a*a; // main pulse  << should be subtracted, but reverse direction.
            b = i+secPos-NUM_LEDS/2.0; b = -w*b*b; // prev pulse
            c = i+secPos+NUM_LEDS/2.0; c = -w*c*c; // next pulse
            y = pow(2, a)+pow(2, b)+pow(2, c);   // sum
            RIGHT[(i+NUM_LEDS/4)%(NUM_LEDS/2)] |= CHSV(160, 255, bri*y);
            a = i+minPos;              a = -w*a*a; // main pulse
            b = i+minPos-NUM_LEDS/2.0; b = -w*b*b; // prev pulse
            c = i+minPos+NUM_LEDS/2.0; c = -w*c*c; // next pulse
            y = pow(2, a)+pow(2, b)+pow(2, c);   // sum
            RIGHT[(i+NUM_LEDS/4)%(NUM_LEDS/2)] |= CHSV( 96, 255, bri*y);
            a = i+hourPos;              a = -w*a*a; // main pulse
            b = i+hourPos-NUM_LEDS/2.0; b = -w*b*b; // prev pulse
            c = i+hourPos+NUM_LEDS/2.0; c = -w*c*c; // next pulse
            y = pow(2, a)+pow(2, b)+pow(2, c);    // sum
            RIGHT[(i+NUM_LEDS/4)%(NUM_LEDS/2)] |= CHSV(  0, 255, bri*y);
            a = i-pPos; a = -2*a*a;
            y = pow(2, a);
            LEFT [i] |= CRGB(beatsin88(0.5*256, 10, 255)*y, beatsin88(0.7*256, 10, 255)*y, beatsin88(1.1*256, 10, 255)*y);
        }
        /*  */
        // int x = beatsin8(60, NUM_LEDS/2/3, NUM_LEDS/2/3*2);
        // LEFT[x] |= 0x444444;
    }
}


CRGB ColorFraction(CRGB colorIn, float fraction){
    fraction = min(1.0f, fraction);
    return CRGB(colorIn).fadeToBlackBy(255 * (1.0f - fraction));
}

void DrawPixels(float fPos, double count, CRGB color, CRGB *led_base, uint8_t num_leds){
    // Calculate how much the first pixel will hold
    float availFirstPixel = 1.0f - (fPos - (long)(fPos));
    float amtFirstPixel = min(fmod(availFirstPixel, num_leds), count);
    float remaining = count;
    int iPos = fPos;

    // Blend (add) in the color of the first partial pixel

    if (remaining > 0.0f){
        led_base[iPos++ % num_leds] += ColorFraction(color, amtFirstPixel);
        remaining -= amtFirstPixel;
    }

    // Now draw any full pixels in the middle

    while (remaining > 1.0f){
        led_base[iPos++ % num_leds] += color;
        remaining--;
    }

    // Draw tail pixel, up to a single full pixel

    if (remaining > 0.0f){
        led_base[iPos % num_leds] += ColorFraction(color, remaining);
    }
}
void DrawPixels1(float fPos, double count, CRGB color, CRGB *led_base, uint8_t num_leds){
    // Calculate how much the first pixel will hold
    float availFirstPixel = 1.0f - (fPos - (long)(fPos));
    float amtFirstPixel = min(fmod(availFirstPixel, num_leds), count);
    float remaining = count;
    int iPos = fPos;

    // Blend (add) in the color of the first partial pixel

    if (remaining > 0.0f){
        led_base[iPos++ % num_leds] |= ColorFraction(color, amtFirstPixel);
        remaining -= amtFirstPixel;
    }

    // Now draw any full pixels in the middle

    while (remaining > 1.0f){
        led_base[iPos++ % num_leds] |= color;
        remaining--;
    }

    // Draw tail pixel, up to a single full pixel

    if (remaining > 0.0f){
        led_base[iPos % num_leds] |= ColorFraction(color, remaining);
    }
}
