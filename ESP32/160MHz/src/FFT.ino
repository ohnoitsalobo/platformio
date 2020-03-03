#include <arduinoFFT.h>

#define SAMPLES 256                 // Must be a power of 2; if analyzing one channel, 1024 samples is fine
                                    // for stereo analysis, 1024 samples causes discernible lag so use 512
// #define SAMPLING_FREQUENCY 40000    // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define SAMPLING_FREQUENCY 40000    // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.

#define Left  true
#define Right false
#define LeftPin  36
#define RightPin 39

float MAX = 512;    // cap computed FFT values
int AMPLITUDE = 10; // scale computed FFT values
int noise = 1000;   // crude noise removal threshold

unsigned int sampling_period_us;
unsigned long microseconds;
// long int Lpeak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
// long int Rpeak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
long int Lpeak[BANDS];
long int Rpeak[BANDS];

double LvReal[SAMPLES];
double LvImag[SAMPLES];
double L_MajorPeak = 0;

double RvReal[SAMPLES];
double RvImag[SAMPLES];
double R_MajorPeak = 0;

unsigned long newTime, oldTime;

arduinoFFT LFFT = arduinoFFT(LvReal, LvImag, SAMPLES, SAMPLING_FREQUENCY); // setup FFT objects for left / right
arduinoFFT RFFT = arduinoFFT(RvReal, RvImag, SAMPLES, SAMPLING_FREQUENCY);

void FFTsetup(){
    sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
    // analogSetCycles(2);
    // analogSetSamples(1);
}

void FFTstuff(){

    for (int i = 0; i < SAMPLES; i++) {                              // read one channel followed by the other
        newTime = micros()-oldTime;                                  // and store in FFT object
        oldTime = newTime;                                           // 
        LvReal[i] = analogRead(LeftPin);                             // 
        LvImag[i] = 0;                                               // 
        RvReal[i] = analogRead(RightPin);                            // 
        RvImag[i] = 0;                                               // 
        while (micros() < (newTime + sampling_period_us)) {  }       // 
    }                                                                // 
    // for (int i = 0; i < SAMPLES; i++) {                              // 
        // newTime = micros()-oldTime;                                  // 
        // oldTime = newTime;                                           // 
        // /* // LvReal[i] = analogRead(LeftPin);                             //  */
        // /* // LvImag[i] = 0;                                               //  */
        // RvReal[i] = analogRead(RightPin);                            // 
        // RvImag[i] = 0;                                               // 
        // while (micros() < (newTime + sampling_period_us)) {  }       // 
    // }                                                                 
    
    LFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    // LFFT.Windowing(FFT_WIN_TYP_RECTANGLE, FFT_FORWARD);        // apply window function to FFT object
    LFFT.Compute(FFT_FORWARD);                            // compute FFT
    LFFT.ComplexToMagnitude();                            // convert to magnitudes
    // L_MajorPeak = LFFT.MajorPeak();
    
    RFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    // RFFT.Windowing(FFT_WIN_TYP_RECTANGLE, FFT_FORWARD);
    RFFT.Compute(FFT_FORWARD);
    RFFT.ComplexToMagnitude();
    // R_MajorPeak = RFFT.MajorPeak();
    
    processBands();

}

void processBands(){
    for (byte band = 0; band < BANDS; band++) {
        Lpeak[band] = 0;
        Rpeak[band] = 0;
    }
    for (int i = 2; i < (SAMPLES/2); i++){ // Don't use sample 0 and only the first SAMPLES/2 are usable.
        // Each array element represents a frequency and its value, is the amplitude. Note the frequencies are not discrete.
        if (LvReal[i] > noise || RvReal[i] > noise) { // Add a crude noise filter, 10 x amplitude or more
           // /*
            if (i<=2 )             { displayBand( 0,  0, (int)LvReal[i] ); displayBand( 1,  0, (int)RvReal[i] ); }
            if (i >2   && i<=4 )   { displayBand( 0,  1, (int)LvReal[i] ); displayBand( 1,  1, (int)RvReal[i] ); }
            if (i >4   && i<=7 )   { displayBand( 0,  2, (int)LvReal[i] ); displayBand( 1,  2, (int)RvReal[i] ); }
            if (i >7   && i<=11 )  { displayBand( 0,  3, (int)LvReal[i] ); displayBand( 1,  3, (int)RvReal[i] ); }
            if (i >11  && i<=15 )  { displayBand( 0,  4, (int)LvReal[i] ); displayBand( 1,  4, (int)RvReal[i] ); }
            if (i >15  && i<=21 )  { displayBand( 0,  5, (int)LvReal[i] ); displayBand( 1,  5, (int)RvReal[i] ); }
            if (i >21  && i<=27 )  { displayBand( 0,  6, (int)LvReal[i] ); displayBand( 1,  6, (int)RvReal[i] ); }
            if (i >27  && i<=33 )  { displayBand( 0,  7, (int)LvReal[i] ); displayBand( 1,  7, (int)RvReal[i] ); }
            if (i >33  && i<=40 )  { displayBand( 0,  8, (int)LvReal[i] ); displayBand( 1,  8, (int)RvReal[i] ); }
            if (i >40  && i<=47 )  { displayBand( 0,  9, (int)LvReal[i] ); displayBand( 1,  9, (int)RvReal[i] ); }
            if (i >47  && i<=55 )  { displayBand( 0, 10, (int)LvReal[i] ); displayBand( 1, 10, (int)RvReal[i] ); }
            if (i >55  && i<=63 )  { displayBand( 0, 11, (int)LvReal[i] ); displayBand( 1, 11, (int)RvReal[i] ); }
            if (i >63  && i<=70 )  { displayBand( 0, 12, (int)LvReal[i] ); displayBand( 1, 12, (int)RvReal[i] ); }
            if (i >70  && i<=97 )  { displayBand( 0, 13, (int)LvReal[i] ); displayBand( 1, 13, (int)RvReal[i] ); }
            if (i >97  && i<=124 ) { displayBand( 0, 14, (int)LvReal[i] ); displayBand( 1, 14, (int)RvReal[i] ); }
            if (i >124 && i<=146 ) { displayBand( 0, 15, (int)LvReal[i] ); displayBand( 1, 15, (int)RvReal[i] ); }
            if (i >146 && i<=179 ) { displayBand( 0, 16, (int)LvReal[i] ); displayBand( 1, 16, (int)RvReal[i] ); }
            if (i >179 && i<=206 ) { displayBand( 0, 17, (int)LvReal[i] ); displayBand( 1, 17, (int)RvReal[i] ); }
            if (i >206 && i<=233 ) { displayBand( 0, 18, (int)LvReal[i] ); displayBand( 1, 18, (int)RvReal[i] ); }
            if (i >233 && i<=255 ) { displayBand( 0, 19, (int)LvReal[i] ); displayBand( 1, 19, (int)RvReal[i] ); }
            if (i >255           ) { displayBand( 0, 20, (int)LvReal[i] ); displayBand( 1, 20, (int)RvReal[i] ); }
           // */
            // displayBand( Left, map(i, 2, SAMPLES/2, 0, BANDS), LvReal[i]);
            // displayBand(Right, map(i, 2, SAMPLES/2, 0, BANDS), RvReal[i]);
        }
    }
}
    
void displayBand(bool channel, int band, int dsize){
    dsize /= AMPLITUDE; // scale FFT values
    if (dsize > MAX) dsize = MAX; // cap FFT values
    // if (channel == Left ) { Lpeak[band] = (dsize > Lpeak[band]) ? dsize : Lpeak[band]; }
    // if (channel == Right) { Rpeak[band] = (dsize > Rpeak[band]) ? dsize : Rpeak[band]; }
    if (channel == Left ) { if (dsize > Lpeak[band]) {Lpeak[band] = dsize;} }
    if (channel == Right) { if (dsize > Rpeak[band]) {Rpeak[band] = dsize;} }
}
