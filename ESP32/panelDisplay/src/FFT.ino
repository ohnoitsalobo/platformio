#include <arduinoFFT.h>

#define audio_in 36
#define samples 256 // 512
#define samplingFrequency 10000 // 25000

#define _noise 1500
#define MAX 35000

unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[samples];
double vImag[samples];
double spectrum[2][samples/2];

arduinoFFT FFT = arduinoFFT();

void fftSetup(){
    sampling_period_us = round(1000000*(1.0/samplingFrequency));
    for (uint16_t i = 2; i < samples/2; i++){
        spectrum[0][i] = pow((i-2)/(samples/2-2.0), 0.5) * NUMBER_OF_LEDS/2;
        spectrum[1][i] = 0;
        Serial.print((int)spectrum[0][i]);
        Serial.print("\t");
    }
}

void fftLoop(){
    microseconds = micros();
    for(int i=0; i<samples; i++){
        vReal[i] = analogRead(audio_in);
        vImag[i] = 0;
        while(micros() - microseconds < sampling_period_us){  }
        microseconds += sampling_period_us;
    }

    FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, samples, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, samples);

    PrintVector(vReal, (samples >> 1));

    // delay(1000);
}

void PrintVector(double *vData, uint16_t bufferSize)
{
    for (uint16_t i = 2; i < bufferSize; i++){
        if(vData[i] > _noise){
            // Serial.println(vData[i], 4);
            spectrum[1][i] = vData[i]-_noise;
            if(spectrum[1][i] > MAX)
                spectrum[1][i] = MAX;
        }else{
            spectrum[1][i] = 0;
        }
    }
}
