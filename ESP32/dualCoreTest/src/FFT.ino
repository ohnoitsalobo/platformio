#include <arduinoFFT.h>

#define LeftPin  36
#define RightPin 39
#define samples  256 // must ALWAYS be a power of 2
#define samplingFrequency 25000 // 25000

#define noise 1500
#define MAX 30000

unsigned int sampling_period_us;
unsigned long microseconds;

double RvReal[samples];
double RvImag[samples];
double LvReal[samples];
double LvImag[samples];
double Lspectrum[2][samples/2];
double Rspectrum[2][samples/2];

arduinoFFT LFFT = arduinoFFT(LvReal, LvImag, samples, samplingFrequency);
arduinoFFT RFFT = arduinoFFT(RvReal, RvImag, samples, samplingFrequency);

void fftSetup(){
    sampling_period_us = round(1000000*(1.0/samplingFrequency));
    for (uint16_t i = 2; i < samples/2; i++){
        Lspectrum[0][i] = pow((i-2)/(samples/2.0-2), 0.5) * NUMBER_OF_LEDS/2;
        Lspectrum[1][i] = 0;
        Rspectrum[0][i] = Lspectrum[0][i];
        Rspectrum[1][i] = 0;

        // Serial.print(i);
        // Serial.print("\t");
        // Serial.print(((i-1) * 1.0 * samplingFrequency) / samples);
        // Serial.print("  \t");
        // Serial.print((int)spectrum[0][i]);
        // Serial.print("\r\n");
    }
}

void fftLoop(){
    microseconds = micros();
    for(int i=0; i<samples; i++){
        LvReal[i] = analogRead(LeftPin);
        RvReal[i] = analogRead(RightPin);
        LvImag[i] = 0;
        RvImag[i] = 0;
        while(micros() - microseconds < sampling_period_us){  }
        microseconds += sampling_period_us;
    }

    LFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    LFFT.Compute(FFT_FORWARD);
    LFFT.ComplexToMagnitude();
    
    RFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    RFFT.Compute(FFT_FORWARD);
    RFFT.ComplexToMagnitude();

    PrintVector(LvReal, (samples >> 1), 1);
    PrintVector(RvReal, (samples >> 1), 2);

    // delay(1000);
}

void PrintVector(double *vData, uint16_t bufferSize, int leftRight) {
    for (uint16_t i = 2; i < bufferSize; i++){
        if(vData[i] > noise){
            if     (leftRight == 1) { Lspectrum[1][i] = vData[i]-noise; if(Lspectrum[1][i] > MAX) Lspectrum[1][i] = MAX; }
            else if(leftRight == 2) { Rspectrum[1][i] = vData[i]-noise; if(Rspectrum[1][i] > MAX) Rspectrum[1][i] = MAX; }
        }else{
            if     (leftRight == 1) { Lspectrum[1][i] = 0; }
            else if(leftRight == 2) { Rspectrum[1][i] = 0; }
        }
        yield();
    }
}
