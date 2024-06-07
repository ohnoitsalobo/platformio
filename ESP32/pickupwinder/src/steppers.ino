// NEMA 17 motors -> 200 steps/rev
// Arduino 16MHz  -> max 4000 steps/sec -> 20 rps -> 1200 rpm
// ESP8266 160MHz -> 

/* MICROSTEPPING TABLE
MS1, MS2, MS3 - step size

0, 0, 0 - full step  | 200/rev
1, 0, 0 -  1/2 step  | 400/rev
0, 1, 0 -  1/4 step  | 800/rev
1, 1, 0 -  1/8 step  | 1600/rev
1, 1, 1 - 1/16 step  | 3200/rev
    
 */
#define STEPS_PER_REV -200.0

int maxSpeed1 = 3500, acc1 = 10000;
int maxSpeed2 = 3500, acc2 = 10000;
int desiredSpeed = 0, currentSpeed = 0;

void setupSteppers(){
    attachInterrupt(digitalPinToInterrupt(RX), rpmCounter, RISING);

    stepper1.setMaxSpeed(maxSpeed1);    stepper1.setAcceleration(acc1);
    stepper2.setMaxSpeed(maxSpeed2);    stepper2.setAcceleration(acc2);
    stepper1.setSpeed(desiredSpeed);
    stepper2.setSpeed(desiredSpeed);
}

void runSteppers(){
    switch(winder_state){
        case IDLE:
            if((int)V[2] || (int)V[3]){
                stepper1.runSpeed();
            }else if(stepper1.distanceToGo() != 0){
                stepper1.run();
            }
        break;
        case WINDING:
            if(stepper1.distanceToGo() != 0){
                stepper1.run();
            }else{
                winder_state = IDLE;
                stepper1.setMaxSpeed(maxSpeed1);
                currentTurns=0;
                V[10] = 0;
            }
        break;
        case PAUSE:
            
        break;
        default:
            
        break;
    }
    V[4] = stepsToTurns(stepper1.currentPosition());
    if(rpmCount > rpmCount_prev){
        _serial_.println(rpmCount);
        rpmCount_prev = rpmCount;
    }
}

void stopAll(){
    winder_state = IDLE;
    stepper1.setSpeed(0);
    stepper2.setSpeed(0);
}

float stepsToRPM(int x){
    return x*(60.0/(STEPS_PER_REV * stepMultiply()));
}
int RPMToSteps(float x){
    return x*(STEPS_PER_REV * stepMultiply())/60.0;
}

int turnsToSteps(float x){
    return x*(STEPS_PER_REV * stepMultiply());
}
float stepsToTurns(int x){
    return x/(STEPS_PER_REV * stepMultiply());
}

uint8_t stepMultiply(){
    if(microstepping == ms_4){
        return 16;
    }else if(microstepping == ms_3){
        return 8;
    }else if(microstepping == ms_2){
        return 4;
    }else if(microstepping == ms_1){
        return 2;
    }else
        return 1;
}

IRAM_ATTR void rpmCounter(){
    rpmInterrupt = millis();
    if (rpmInterrupt - rpmInterrupt_prev > 30){
        rpmCount++;
        rpmInterrupt_prev = rpmInterrupt;
    }
}

/*
Defining & Configuring Motors
AccelStepper mystepper(1, pinStep, pinDirection);
A stepper motor controlled by a dedicated driver board.

AccelStepper mystepper(2, pinA, pinB);
A bipolar stepper motor controlled by an H-Bridge circuit.

AccelStepper mystepper(4, pinA1, pinA2, pinB1, pinB2);
A unipolar stepper motor, controlled by 4 transistors.

mystepper.setMaxSpeed(stepsPerSecond);
Sets the maximum speed. The default is very slow, so this must be configured. When controlled by setting position, the stepper will accelerate to move at this maximum speed, and decelerate as it reaches the destination.

mystepper.setAcceleration(stepsPerSecondSquared);
Sets the acceleration to be used, in steps per second per second.

Position Based Control
mystepper.moveTo(targetPosition);
Move the motor to a new absolute position. This returns immediately. Actual movement is caused by the run() function.

mystepper.move(distance);
Move the motor (either positive or negative) relative to its current position. This returns immediately. Actual movement is caused by the run() function.

mystepper.currentPosition();
Read the motor's current absolute position.

mystepper.distanceToGo();
Read the distance the motor is from its destination position. This can be used to check if the motor has reached its final position.

mystepper.run();
Update the motor. This must be called repetitively to make the motor move.

mystepper.runToPosition();
Update the motor, and wait for it to reach its destination. This function does not return until the motor is stopped, so it is only useful if no other motors are moving.

Speed Based Control
mystepper.setSpeed(stepsPerSecond);
Set the speed, in steps per second. This function returns immediately. Actual motion is caused by called runSpeed().

mystepper.runSpeed();
Update the motor. This must be called repetitively to make the motor move.
*/