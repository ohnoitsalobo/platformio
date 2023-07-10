uint32_t msSinceLastRPS = 0, timerms = 0, deltams = 0, currentms = 0;
uint32_t currentTurns = 0, desiredTurns = 0;
volatile uint32_t RPS = 0;
float currentRPM = 0, desiredRPM = 0;
uint16_t currentPWM = 0, desiredPWM = 0;
int8_t currentAccel = 0, desiredAccel = 0;
int8_t jerk = 0, direction = 0;

void IRAM_ATTR speedSense() {
    RPS++;
    msSinceLastRPS = currentms;
}

float RPMtoPWM(float desiredrpm){
    return desiredrpm/3000.0 * 100.0;
}

void setupSpeedControl(){
    attachInterrupt(digitalPinToInterrupt(sensorPin), speedSense, RISING);
}

void runSpeedControl(){
    currentms = millis();
    getRPM();
    setRPM();
    // uint16_t temp = V[v]*V[v]/float(analogRange);
    // _serial_.println(temp);
    // analogWrite(LED_BUILTIN, analogRange - temp);
}

void getRPM(){
    noInterrupts();
    if(millis() - timerms > 100){
        deltams = millis() - msSinceLastRPS;
        currentRPM = 1000.0/deltams;
        // V[1] = currentRPM;
        _serial_.print(deltams);
        _serial_.print("   ");
        _serial_.print(RPS*60);
        _serial_.print("   ");
        _serial_.print(60000.0/deltams);
        _serial_.print("   ");
        _serial_.print(currentPWM);
        _serial_.print("   \r");
        timerms = millis();
        if (RPS > 0) RPS/=2;
    }
    interrupts();
}

void setRPM(){
    if(desiredPWM > currentPWM){
        if(currentPWM < analogRange)
            currentPWM++;
    }
    else if(desiredPWM < currentPWM){
        if(currentPWM > 0)
            currentPWM--;
    }
    uint16_t temp = currentPWM*currentPWM/float(analogRange);
    analogWrite(LED_BUILTIN, analogRange - temp);
}

/*
V0 = desired RPM
V1 = current RPM
V2 = desired turns
V3 = current turns
V4 = forward
V5 = reverse
V6 = tension stop
V7 = emergency stop
*/
/*
D0    GPIO16       GPIO0   D3
D1    GPIO5        GPIO1   TX
D2    GPIO4        GPIO2   D4/led
D3    GPIO0        GPIO3   RX
D4    GPIO2/led    GPIO4   D2
D5    GPIO14       GPIO5   D1
D6    GPIO12       GPIO9   S2
D7    GPIO13       GPIO10  S3
D8    GPIO15       GPIO12  D6
RX    GPIO3        GPIO13  D7
TX    GPIO1        GPIO14  D5
S3    GPIO10       GPIO15  D8
S2    GPIO9        GPIO16  D0
*/