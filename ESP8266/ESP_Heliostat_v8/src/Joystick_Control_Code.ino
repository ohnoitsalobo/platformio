void checkJoystick(){
	if(joystickActivate == 1 && joystickModeOnOff == 0){
		joystickModeOnOff=1;
		justFinishedManualControl=1;
		MachineOn(machineNumber);
		Serial.println("Joystick Control Mode");
		//~ timer.deleteTimer(blynkTimer);
		//~ blynkTimer = timer.setInterval(20L, runBlynk);
	}
	if(joystickActivate == 0 && joystickModeOnOff == 1){
		joystickModeOnOff=0;
		//~ timer.deleteTimer(blynkTimer);
		//~ blynkTimer = timer.setInterval(100L, runBlynk);
	}
	if(joystickModeOnOff==1){
		machineNumber=manualMachineNumber;
		ManualControlThroughJoystick();
	}
}


void ManualControlThroughJoystick(){
	float LR = joyX;
	float UD = joyY;
	if (UD>UDCenter+30 || UD<UDCenter-30 || LR>LRCenter+30 || LR<LRCenter-30){

		altMove = ((UD - UDCenter)/UDCenter)*0.75*invertUD;
		azMove = ((LR - LRCenter)/LRCenter)*0.75*invertLR;

		altManualSpeed = abs(altManualSpeedSwap*altMove); 
		azManualSpeed = abs(azManualSpeedSwap*azMove);

		if(altManualSpeed<5){altManualSpeed=5;}
		if(azManualSpeed<5){azManualSpeed=5;}

		if (float(pgm_read_float(&MachineSettings[machineNumber][1]))==1){
			SunsAltitude=MachinesPrevAlt[machineNumber]+altMove;
			SunsAzimuth=MachinesPrevAz[machineNumber]+azMove;
		}

		if (float(pgm_read_float(&MachineSettings[machineNumber][1]))==2){
			MachineTargetAlt[machineNumber]=MachineTargetAlt[machineNumber]+altMove;
			MachineTargetAz[machineNumber]=MachineTargetAz[machineNumber]+azMove;     
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void joystickMoveMotors( long altsteps, int altitudeStepPin, int altitudeDirPin, float altManualSpeed, long azsteps, int azimuthStepPin, int azimuthDirPin, float azManualSpeed  ){
	float altMotorDelay, azMotorDelay, MotorDelay;
	long mostSteps;
	if (abs(altsteps)==altsteps){digitalWrite(altitudeDirPin, HIGH);}else{digitalWrite(altitudeDirPin, LOW);}  
	if (abs(azsteps)==azsteps){digitalWrite(azimuthDirPin, HIGH);}else{digitalWrite(azimuthDirPin, LOW);}  
	altMotorDelay = ( 1000000 * ( 60 / (steps * altManualSpeed) ) ) / 2;           
	azMotorDelay = ( 1000000 * ( 60 / (steps * azManualSpeed) ) ) / 2;           
	if (altMotorDelay<azMotorDelay){MotorDelay = altMotorDelay;}else{MotorDelay = azMotorDelay;}
	if(abs(altsteps)>abs(azsteps)){mostSteps=abs(altsteps);}else{mostSteps=abs(azsteps);}
	long x=0;
	while (x<mostSteps){
		if(x<abs(altsteps)){digitalWrite(altitudeStepPin, HIGH);}
		if(x<abs(azsteps)){digitalWrite(azimuthStepPin, HIGH);}
		delayInMicroseconds((int)MotorDelay);  
		if(x<abs(altsteps)){digitalWrite(altitudeStepPin, LOW);}
		if(x<abs(azsteps)){digitalWrite(azimuthStepPin, LOW);}
		delayInMicroseconds((int)MotorDelay);
		x+=1;
	}
}

