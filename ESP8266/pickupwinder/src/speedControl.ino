uint16_t desiredPWM = 0, currentPWM = 0;
volatile uint16_t currentTurns = 0;
uint32_t currentMillis = 0, millisSinceLastInterrupt = 0;

uint16_t analogMin = 50, analogValue = 0;


void IRAM_ATTR interruptFalling() {
    if(millisSinceLastInterrupt - currentMillis > 15 && digitalRead(sensorPin) == LOW){
        currentTurns++;
    }
    millisSinceLastInterrupt = currentMillis;
}

void setupSpeedControl(){
    attachInterrupt(digitalPinToInterrupt(sensorPin), interruptFalling, FALLING);
}

void runSpeedControl(){
    currentMillis = millis();
    
    if((int)V[0] > desiredPWM) desiredPWM++;
    if((int)V[0] < desiredPWM) desiredPWM--;
    
    // _serial_.print (desiredPWM);
    // _serial_.print ("   \r");
    if(currentPWM != desiredPWM){
        analogWrite(motorA, desiredPWM);
        currentPWM = desiredPWM;
    }
    _serial_.print (currentTurns);
    _serial_.print ("\t");
    _serial_.print (currentMillis);
    _serial_.print ("\t");
    _serial_.print (analogRead(0));
    _serial_.print ("     \r");
    
    setRPM();
}

void getRPM(){
    // noInterrupts();
    
    // interrupts();
}

void setRPM(){
    analogValue = analogRead(0);
    if(analogValue < analogMin) analogValue = 0;
    analogWrite(motorA, analogValue);
}

