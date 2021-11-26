bool FFTenable = true;

#define LeftPin  36
#define RightPin 39
#define samples  256 // must ALWAYS be a power of 2
#define samplingFrequency 25600 // 25000

#define noise 1500
#define MAX 50000

unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[2][samples];
double vImag[2][samples];
double spectrum[3][samples/2];

arduinoFFT LFFT = arduinoFFT(vReal[0], vImag[0], samples, samplingFrequency);
arduinoFFT RFFT = arduinoFFT(vReal[1], vImag[1], samples, samplingFrequency);

void fftSetup(){
    sampling_period_us = round(1000000*(1.0/samplingFrequency));
    
    for (uint16_t i = 2; i < samples/2; i++){
        spectrum[0][i] = pow((i-2)/(samples/2.0-2), 0.5) * NUM_LEDS/4; //0.32
        spectrum[1][i] = 0;
        spectrum[2][i] = 0;

        // _serial_.print(i);
        // _serial_.print(",");
        // _serial_.print(((i-1) * 1.0 * samplingFrequency) / samples);
        // _serial_.print(",");
        // _serial_.print((int)spectrum[0][i]);
        // _serial_.print("\r\n");
        /*  * /
        for(uint8_t x = 0; x < 40; x++){
            _serial_.print((int)(pow((i-2)/(samples/2.0-2), (0.4+x/100.0)) * NUMBER_OF_LEDS/2));
            _serial_.print(",");
        }
        /*  */
    }
}

const uint16_t max_max = MAX;
const uint16_t min_max = MAX/10;
const uint16_t max_min = noise;
const uint16_t min_min = noise/5;
uint16_t _min = max_min, _max = min_max;
void PrintVector(double *vData, uint16_t bufferSize, int leftRight) {
    for (uint16_t i = 2; i < bufferSize; i++){
        if(vData[i] > noise){
            // _serial_.println(vData[i], 4);
            spectrum[leftRight][i] = vData[i]-noise;
            if(spectrum[leftRight][i] > _max) { _max =  spectrum[leftRight][i]; }
        }else{
            spectrum[leftRight][i] = 0;
        }
        yield();
    }
}

void fftLoop(){
#ifdef debug
    _serial_.println("Starting fftLoop");
#endif

    microseconds = micros();
    for(int i=0; i<samples; i++){
        vReal[0][i] = analogRead(LeftPin);
        vReal[1][i] = analogRead(RightPin);
        vImag[0][i] = 0;
        vImag[1][i] = 0;
        while(micros() - microseconds < sampling_period_us){  }
        microseconds += sampling_period_us;
    }

    LFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    LFFT.Compute(FFT_FORWARD);
    LFFT.ComplexToMagnitude();
    
    RFFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    RFFT.Compute(FFT_FORWARD);
    RFFT.ComplexToMagnitude();

    PrintVector(vReal[0], (samples >> 1), 1);
    PrintVector(vReal[1], (samples >> 1), 2);
    if(_max > min_max) _max -= 100;

#ifdef debug
    _serial_.println("Ending fftLoop");
#endif
}
