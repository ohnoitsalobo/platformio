///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code moves the stepper motors
void moveToPosition(int AccelYesOrNo, long altsteps, long azsteps){
	if (joystickModeOnOff==1 && iterationsAfterReset>0){//This code runs when manually controlling the machines through joystick
		joystickMoveMotors( altsteps, altitudeStepPin, altitudeDirPin, altManualSpeed, azsteps, azimuthStepPin, azimuthDirPin, azManualSpeed  );
		AccelYesOrNo = 3;
	}
	if (AccelYesOrNo==1){//This code runs during normal operation
		moveMotorWithAccel(azsteps, azimuthStepPin, azimuthDirPin, azSpeed, azAccel);
		moveMotorWithAccel(altsteps, altitudeStepPin, altitudeDirPin, altSpeed, altAccel);
	}
}  


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code moves the stepper motors with acceleration
void moveMotorWithAccel(long numOfSteps,int stepPin, int DirPin, float maxspeed,float Accel){
	if (abs(numOfSteps)==numOfSteps){digitalWrite(DirPin, HIGH);}else{digitalWrite(DirPin, LOW);}  
	float minMotorDelay  = (1000000/((maxspeed*steps)/60))/2;
	float halfWay = abs(long(numOfSteps/2));
	float maxMotorDelay=((1000000/((maxspeed*steps)/60))*100)/2;
	float motorDelay=maxMotorDelay;  

	float motorDelayAdjust=((maxMotorDelay-minMotorDelay)*(maxMotorDelay-minMotorDelay))/(Accel*1000000);               
	float numberOfStepsToReachMinFromMax=long((maxMotorDelay-minMotorDelay)/motorDelayAdjust);    
	if(-0.00001<Accel&&Accel<0.00001){motorDelay=minMotorDelay;}

	if (abs(numOfSteps)>numberOfStepsToReachMinFromMax*2){//This code runs if the accleration slope "flat lines"
		for (long doSteps=1; doSteps <= abs(numOfSteps); doSteps++){
			delayInMicroseconds(motorDelay);
			digitalWrite(stepPin, HIGH);
			delayInMicroseconds(motorDelay);
			digitalWrite(stepPin, LOW);
			if ((motorDelay>minMotorDelay)&&(doSteps<halfWay)){motorDelay=motorDelay-motorDelayAdjust;}//Accelerates the motor
			if ((motorDelay<minMotorDelay)){motorDelay=minMotorDelay;}//Holds the motor at its max speed
			if ((doSteps>(abs(numOfSteps)-numberOfStepsToReachMinFromMax))){motorDelay=motorDelay+motorDelayAdjust;}//Deccelerates the motor after it gets close to the end of its move
		}
	}else{//This code runs if the acceleration slope is an upside down V.
		for (long doSteps=1; doSteps <= abs(numOfSteps); doSteps++){
			delayInMicroseconds(motorDelay);
			digitalWrite(stepPin, HIGH);     
			delayInMicroseconds(motorDelay);
			digitalWrite(stepPin, LOW);
			if ((motorDelay>minMotorDelay)&&(doSteps<halfWay)){motorDelay=motorDelay-motorDelayAdjust;}//Accelerates the motor
			if ((motorDelay<minMotorDelay)){motorDelay=minMotorDelay;}//Holds the motor at its max speed
			if (doSteps>halfWay){motorDelay=motorDelay+motorDelayAdjust;}//Decelerates the motor after it gets close to the end of its move
			if (motorDelay>maxMotorDelay){motorDelay=maxMotorDelay;}
		}
	} 
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code is used to delay between stepper motor steps
void delayInMicroseconds(long delayInMicrosec){
	long t1,t2;
	t1=micros(); 
	t2=micros();
	while ((t1+delayInMicrosec)>t2){
		t2=micros();
		if (t2<t1){
			t2=t1+delayInMicrosec+1;
		}//Check if micros() rolled over
	}
	yield();
	timer.run();
}
void delayInMilliseconds(long delayInMillisec){
	long t1,t2;
	t1=millis(); 
	t2=millis();
	while ((t1+delayInMillisec)>t2){
		t2=millis();
		if (t2<t1){
			t2=t1+delayInMillisec+1;
		}//Check if millis() rolled over
	}
	yield();
	timer.run();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code resets the machine on the limit switches
void findLimits(int altOrAz, int motorDirection, float limitAngle){
	if (altOrAz==1){ 
		searchForLimit(limitAngle, altitudeDirPin, altitudeStepPin, altResetSpeed, altLimitPin, altitudeMax, motorDirection );
	}
	if (altOrAz==2){
		searchForLimit(limitAngle, azimuthDirPin, azimuthStepPin, azResetSpeed, azLimitPin, azimuthMax, motorDirection );
	}
}

void searchForLimit(float limitAngle, int DirPin, int stepPin, int ResetSpeed, int LimitPin, long maxResetSteps, int motorDirection){
	long x;
	int whichDir;
	if (motorDirection!=0){//A motorDirection of 0 will skip the reset  
		if (abs(limitAngle) != limitAngle){whichDir = 1;}else{whichDir = -1;}
		if (motorDirection*whichDir != 1){digitalWrite(DirPin, LOW);}else{digitalWrite(DirPin, HIGH);}
		float MotorDelay = ( 1000000 * ( 60 / (steps * ResetSpeed) ) ) / 2; 
		while(x<maxResetSteps){
			digitalWrite(stepPin, HIGH);
			delayInMicroseconds(MotorDelay);          
			digitalWrite(stepPin, LOW); 
			delayInMicroseconds(MotorDelay);
			if (digitalRead(LimitPin)==LOW){
				x=maxResetSteps;
				Serial.print("limit detected\r\n");
			}
			x+=1;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code calculates the angle to move the machine to after the limit switch has been triggered.
float positionAfterReset(float limitAngle){
	float endAltAndAz;
	if (abs(limitAngle) == limitAngle){endAltAndAz = limitAngle - moveAwayFromLimit;}else{endAltAndAz = limitAngle + moveAwayFromLimit;} 
	return endAltAndAz; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code calculates the angles for the heliostat (returnaltaz = 1 will return alt, 2 returns az)
void FindHeliostatAltAndAz(float SunsAltitude, float SunsAzimuth, float targetalt, float targetaz, float &machinealt, float &machineaz){
	float x, x1, x2, \
	      y, y1, y2, \
	      z, z1, z2, \
	      hyp, dist;

	z1 = sin(to_rad(SunsAltitude));
	hyp = cos(to_rad(SunsAltitude));
	x1 = hyp*cos(to_rad(SunsAzimuth*-1));
	y1 = hyp*sin(to_rad(SunsAzimuth*-1));

	z2 = sin(to_rad(targetalt));
	hyp = cos(to_rad(targetalt));
	x2 = hyp*cos(to_rad(targetaz*-1));
	y2 = hyp*sin(to_rad(targetaz*-1));  

	x=(x1-x2)/2+x2;
	y=(y1-y2)/2+y2;
	z=(z1-z2)/2+z2;

	dist=sqrt(x*x+y*y+z*z);
	if ((dist>-0.0001) && (dist <0.0001)){
		dist=0.0001;
	}

	machinealt=to_deg(asin(z/dist));
	machineaz=to_deg(atan2(y*-1,x));

}

float to_rad(float angle){
	return angle*(pi/180);
}
float to_deg(float angle){
	return angle*(180/pi);
}

void MachineOn(int number){
	if (enableHIGHorLOW==0){
		digitalWrite(EnablePin, LOW);
	}else{
		digitalWrite(EnablePin, HIGH);
	}

}

void MachineOff(int number){
	//~ if (digitalRead(manualModeOnOffPin)!=LOW){
		if (enableHIGHorLOW==0){
			digitalWrite(EnablePin, HIGH);
		}else{
			digitalWrite(EnablePin, LOW);
		}
	//~ } 
}
//~ Regular print function called by timer
void printStuff(){
	if(timeStatus() == timeNotSet){ // print nothing if time is not set
		Serial.print("\r\nClock not yet synchronized. Time / Date unknown.\r\n");
		receiveUDP();
	}else{
		if(joystickModeOnOff != 1){
			if(timeStatus() == timeSet || timeStatus() == timeNeedsSync){
				if(timeStatus() == timeNeedsSync){
					Serial.print("The last successful clock sync was at: ");
					printTimeDate(lastSync);
				}
				Serial.print("\r\nCurrent time: ");
				printTimeDate(now());
			}
			Serial.print("\tSun\tTarget\tMachine\r\n");
			Serial.print("Alt:\t");
			Serial.print(SunsAltitude, 3);        Serial.print("\t");
			Serial.print(MachineTargetAlt[0], (abs(MachineTargetAlt[0]) >= 100) ? 2 : 3); Serial.print("\t");
			Serial.print(MachinesPrevAlt[0], 3);  Serial.print("\r\n");
			Serial.print("Az:\t");
			Serial.print(SunsAzimuth, 3);         Serial.print("\t");
			Serial.print(MachineTargetAz[0], (abs(MachineTargetAz[0]) >= 100) ? 2 : 3);  Serial.print("\t");
			Serial.print(MachinesPrevAz[0], 3);   Serial.print("\r\n");
		}
	}
}
///////// Print date and time 
void printTimeDate(time_t t){
	Serial.setDebugOutput(true);
	Serial.printf("%02d:%02d:%02d on %02d %s %d\r\n", \
				hour(t), minute(t), second(t), \
				day(t), monthStr(month(t)), year(t));
	Serial.setDebugOutput(false);
	sendUDP(now());
}
/*
////////// BLYNK STUFF /////////////
//// Reset machine via blynk app
BLYNK_WRITE(0) {
	if(param.asInt() == 0){
		iterationsAfterReset = 0;
	}
}
//// Enable Joystick control via blynk app
BLYNK_WRITE(1) { // Joystick Toggle
	joystickActivate = param.asInt();
}
//// Control via Blynk Joystick
BLYNK_WRITE(2) { // Joystick
	int x = param[0].asInt();
	int y = param[1].asInt();
	joyX = x;
	joyY = y;
	//~ Serial.print(x);
	//~ Serial.print("\t");
	//~ Serial.println(y);
}
BLYNK_WRITE(3) {
	windProtect = param.asInt();
}
//// Blynk Reset manually programmed target to 0
BLYNK_WRITE(31) { // Target Reset
	if(param.asInt() == 1){
		union u_tag{ byte b[4]; float f; } u;
		
		EEPROM.begin(8);
		
		u.f = 0.0;
		EEPROM.write(0, u.b[0]);
		EEPROM.write(1, u.b[1]);
		EEPROM.write(2, u.b[2]);
		EEPROM.write(3, u.b[3]);
		
		u.f = 0.0;
		EEPROM.write(4, u.b[0]);
		EEPROM.write(5, u.b[1]);
		EEPROM.write(6, u.b[2]);
		EEPROM.write(7, u.b[3]);
		
		EEPROM.end();
	}
}
*/
