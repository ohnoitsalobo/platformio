/* ****************************************************************** *\
  Name    : Simple Arduino Sun Tracking / Heliostat Control Program
  Author  : Gabriel Miller
  Website : http://www.cerebralmeltdown.com
  Notes   : Code for controlling various types of solar machines
  
  * Modified by Anand Lobo for ESP8266 (ESP-12 module) *
\* ****************************************************************** */
/* ****************** PIN MAPPINGS ****************** *\
                      __ESP 12___
 3v3 -> 10k -> RESET |           | TX / GPIO1
                 ADC |           | RX / GPIO3
 3v3 -> 10k -> CH_PD |        lim| GPIO5
              GPIO16 |azS        | GPIO4
              GPIO14 |azD        | GPIO0 <- 10k <- 3v3
              GPIO12 |altS       | GPIO2 / TXD1
         (RX)/GPIO13 |altD     EN| GPIO15/(TX) <- 10k <- GND
                 VCC |3v3_____GND| GND

             SW1 -> RESET    SW2 -> GPIO0
\* ************************************************** */
#include <EEPROM.h>               // EEPROM storage
#include <ESP8266WiFi.h>          // WiFi access
#include <ESP8266WebServer.h>     // web server setup
#include <WiFiUdp.h>              // 
#include <Time.h>                 // timekeeping library
//~ #define BLYNK_PRINT Serial
//~ #include <BlynkSimpleEsp8266.h>   // Blynk wireless monitoring
#include <SimpleTimer.h>          // timer library
SimpleTimer timer; int printTimer, blynkTimer;

//~ extern "C" {
//~ #include <user_interface.h>
//~ }

// PUT YOUR LATITUDE, LONGITUDE, AND TIME ZONE HERE (NO Daylight Savings)
double latitude = 15.4989;
double longitude = 73.8278;
double timezone = 5.5;

// If you live in the northern hemisphere, put 0 here. If you live in the southern hemisphere put 1.
int useNorthAsZero = (latitude > 0) ? 0 : 1;

// INTERVAL BETWEEN MACHINE POSITION / TIME UPDATES
unsigned long updateEvery = 10;  // seconds between machine position updates
unsigned long updateClock = 60 * 60 * 24; // seconds between network time updates

// DEGREES TO MOVE OFF OF LIMIT SWITCHES
// Tells the program how many degrees to move off of limit switches after they have been triggered.
// Choose a positive number.
float moveAwayFromLimit = 15;

// MACHINE RESET TIME
// Put the HOUR you want your solar machines to reset on 
// their limit switches here. Remember that daylight saving time is not used, so the
// HOUR it resets won't necessarily match the clock in your home.
int hourReset = 21;

// Machine Wind Protection Park Angle
float machineAltParkAngle=90;
float machineAzParkAngle=0;

// STEPPER MOTOR SETUP 
// The number of steps required for your stepper to make one revolution. (Don't forget to take into 
// account the settings on your driver board. i.e. Microstepping, half stepping etc.)
float steps = 514;
// Set the travel speed for your stepper motors here (Roughly in Rev/Min)
float altSpeed=50;
float azSpeed=50; 
// Set the speed the motors travel when they reset here (Roughly in Rev/Min)
// Note: Max reset speed is limited since checking to see if the limit has been triggered slows things down. 
// The result of which means that the alt/az reset speeds may seem noticably slower than normal travel speed.
float altResetSpeed=70; 
float azResetSpeed=70;
// Speed of Stepper Motors when using manual control through joystick
float altManualSpeed=60; 
float azManualSpeed=60;

// Number of seconds (roughly) to accelerate to full speed.
float altAccel = 1;
float azAccel = 1;  
// the maximum number of steps the machine(s) will take when resetting
long altitudeMax = 170000;
long azimuthMax  = 170000;

// Stepper driver enable on HIGH or LOW
int enableHIGHorLOW = 0;
// Put a 0 here if your driver boards enable the stepper motors when the enable pin is 
// written LOW. 
// Put a 1 here if your driver boards enable the stepper motors when the enable pin is 
// written HIGH.


/////////////////////////////////////////////////////////// 
// PIN ASSIGNMENT and other user settings
///////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TWO WIRE STEP/DIR DRIVER BOARD Pin Assignment
int azimuthStepPin = 16;
int azimuthDirPin = 14;  

int altitudeStepPin = 12;
int altitudeDirPin = 13;

int EnablePin = 15;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Limit Switch Pin Assignment
const int altLimitPin = 5;
const int azLimitPin = 5; 

// Manual Control Pin Assignment
//~ const int manualModeOnOffPin = 0;
// Wind Protection Switch Pin Assignment
const int WindProtectionSwitch = 0;

int invertUD = 1;//Change value from 1 to -1 to invert up/down during joystick control
int invertLR = -1;//Change value from 1 to -1 to invert left/right during joystick control

//Put how many machines you want to control here
#define numberOfMachines 1

//Open the "ReadMe" file that was downloaded with this program to see what each of these settings do.
const float MachineSettings[][20] PROGMEM = {
//             LorG    SorH   Al_GRorTPU   Al_Mdir  Al_L.A  Al_L.B  Al_S.A.  Al_AcAb  Al_LS  Az_GRorTPU  Az_Mdir  Az_L.A  Az_L.B  Az_S.A.  Az_AcAb  Az_LS   MinAz  MinAlt  MaxAz  MaxAlt
//Setting #      0       1         2          3       4       5       6         7       8        9         10       11      12      13       14      15       16     17     18     19
             {   0  ,    2 ,      400 ,       1 ,     0  ,    0  ,    0  ,      0  ,    -10,     400 ,     -1 ,      0 ,     0 ,     0 ,      0,    -90,    -100,  -10,   160,   120 }
             //~ {   0  ,    2 ,      400 ,       0 ,     0  ,    0  ,    0  ,      0  ,    -10,     400 ,      0 ,      0 ,     0 ,     0 ,      0,    -90,    -100,  -10,   160,   120 }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//END OF USER SETTINGS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    



/////////////////////////////////////////////////////////// 
//MISC. VARIABLES USED THROUGHOUT THE PROGRAM
///////////////////////////////////////////////////////////  
float pi = 3.141592653589793, SunsAltitude, SunsAzimuth, h, delta;
int iterationsAfterReset = 0, preTargetsUsed, machineNumber, windToggleCount, midCycle = 0, FirstIterationAfterArduinoReset = 0, justFinishedManualControl;
int  targetsUsed = 1;
unsigned long updateTime = 0,  NOW = 0; 
float altManualSpeedSwap, azManualSpeedSwap, altMove, azMove, UDCenter = 128, LRCenter = 128;
int joystickModeOnOff = 0, joystickTriggerOnce, manualMachineNumber;

int joystickActivate = 0, joyX = 128, joyY = 128;  // keeps track of joystick mode
int windProtect = 0;
time_t lastSync;     // stores time of last successful sync
boolean serverOn = 0;  // placeholder

/////////////////////////////////////////////////////////// 
//END MISC. VARIABLES
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////  
//MISC. ARRAYS  
///////////////////////////////////////////////////////////  
float PrevAltTargets[numberOfMachines];
float PrevAzTargets[numberOfMachines];
float altLeftoverSteps[numberOfMachines];
float azLeftoverSteps[numberOfMachines];
float MachineTargetAlt[numberOfMachines];
float MachineTargetAz[numberOfMachines];
float MachinesPrevAlt[numberOfMachines];
float MachinesPrevAz[numberOfMachines];  

///////////////////////////////////////////////////////////  
//END MISC. ARRAYS  
///////////////////////////////////////////////////////////  


/////////////////////////////////////////////////////////// 
// VOID SETUP
/////////////////////////////////////////////////////////// 
void setup(){

	Serial.begin(115200);

	////////////////////////////////              
	//TWO WIRE STEP/DIR DRIVER BOARD CODE  
	pinMode(altitudeStepPin, OUTPUT);
	pinMode(altitudeDirPin, OUTPUT);
	pinMode(azimuthStepPin, OUTPUT);
	pinMode(azimuthDirPin, OUTPUT);
	pinMode(EnablePin, OUTPUT);  
	////////////////////////////////  

	pinMode(azLimitPin, INPUT_PULLUP);
	pinMode(altLimitPin, INPUT_PULLUP);
	pinMode(WindProtectionSwitch, INPUT_PULLUP);

	altManualSpeedSwap = altManualSpeed; 
	azManualSpeedSwap = azManualSpeed;

	// makes sure machine is off to begin with
	for (int i=0; i <= numberOfMachines - 1; i++){MachineOff(i);}
	
	printTimer = timer.setInterval(1000L, printStuff);   // print stuff to Serial every second
	timer.setInterval(100L, runServer);                  // keeps the server available, runs 10x per second 
	//~ blynkTimer = timer.setInterval(100L, runBlynk);      // perform Blynk magic, 10x per second 
}
/////////////////////////////////////////////////////////// 
//END VOID SETUP
/////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////// 
// VOID LOOP
/////////////////////////////////////////////////////////// 
void loop()
{
	// the machine will not track unless the clock is set
	if(timeStatus() == timeNotSet){
		if(WiFi.status() != WL_CONNECTED){ // if wifi not connected
			connectToWifi();               //   connect to wifi
		}
		if(WiFi.status() == WL_CONNECTED){    // if wifi connected,
			setSyncProvider(getNtpTime);      //   set time using getNtpTime function
			setSyncInterval(updateClock);     //   update time at the given interval
			if(serverOn == 0) beginServer();  //   start the server
		}
		if(iterationsAfterReset == 0){
			moveMachine(PrevAltTargets[machineNumber], PrevAzTargets[machineNumber], MachineTargetAlt[machineNumber], MachineTargetAz[machineNumber], float(pgm_read_float(&MachineSettings[machineNumber][1])), float(pgm_read_float(&MachineSettings[machineNumber][2])), float(pgm_read_float(&MachineSettings[machineNumber][3])), float(pgm_read_float(&MachineSettings[machineNumber][4])), float(pgm_read_float(&MachineSettings[machineNumber][5])), float(pgm_read_float(&MachineSettings[machineNumber][6])), float(pgm_read_float(&MachineSettings[machineNumber][7])), float(pgm_read_float(&MachineSettings[machineNumber][8])), float(pgm_read_float(&MachineSettings[machineNumber][9])), float(pgm_read_float(&MachineSettings[machineNumber][10])), float(pgm_read_float(&MachineSettings[machineNumber][11])), float(pgm_read_float(&MachineSettings[machineNumber][12])), float(pgm_read_float(&MachineSettings[machineNumber][13])), float(pgm_read_float(&MachineSettings[machineNumber][14])), float(pgm_read_float(&MachineSettings[machineNumber][15])), float(pgm_read_float(&MachineSettings[machineNumber][16])), float(pgm_read_float(&MachineSettings[machineNumber][17])), float(pgm_read_float(&MachineSettings[machineNumber][18])), float(pgm_read_float(&MachineSettings[machineNumber][19])));
			iterationsAfterReset = 1; FirstIterationAfterArduinoReset=1;
		}
	}
	//~ receiveUDP();
	// if the time has been set, start tracking
	if(timeStatus() == timeSet || timeStatus() == timeNeedsSync){
		// this code runs if not in manual / joystick mode.
		if ((joystickModeOnOff!=1)){
			if ((hour() == hourReset)&&(minute() == 0)&&(second() < 10)){
				iterationsAfterReset = 0;
				// If the variable "iterationsAfterReset" is set to 0, the machines reset themselves on the limit switches
			} 
			NOW = millis();
			if ( (NOW + updateEvery*1000) < updateTime){updateTime=NOW+updateEvery;}
		}


		// this updates the position of the sun, which in turn tells the machine(s) that it is time to move.
		if ((NOW>=updateTime)||(iterationsAfterReset == 0)){
			updateTime=updateEvery*1000 + millis();

			findSunsAltAndAzOne(year(), month(), day(), timezone, hour(), minute(), second(), latitude, longitude);
			SunsAltitude = SunsAltitude + (1.02/tan((SunsAltitude + 10.3/(SunsAltitude + 5.11)) * pi/180.0))/60.0;
			// Sun Atmospheric Refraction Compensation: Meeus Pg. 105

			if(useNorthAsZero==1){ // adjust solar azimuth for Southern hemisphere
				if (SunsAzimuth<0){
					SunsAzimuth=(SunsAzimuth+180)*-1;
				}
				if (SunsAzimuth>0){
					SunsAzimuth=(SunsAzimuth-180)*-1;
				}
			}
		}

		TargetControl(); // Checks to see if the targets have been changed  
		checkJoystick(); // check if joystick mode is active

		for (int i=0; i <= numberOfMachines - 1; i++){

			moveMachine(PrevAltTargets[machineNumber], PrevAzTargets[machineNumber],
			MachineTargetAlt[machineNumber], MachineTargetAz[machineNumber],
			float(pgm_read_float(&MachineSettings[machineNumber][1])),     
			float(pgm_read_float(&MachineSettings[machineNumber][2])), 
			float(pgm_read_float(&MachineSettings[machineNumber][3])), 
			float(pgm_read_float(&MachineSettings[machineNumber][4])), 
			float(pgm_read_float(&MachineSettings[machineNumber][5])), 
			float(pgm_read_float(&MachineSettings[machineNumber][6])), 
			float(pgm_read_float(&MachineSettings[machineNumber][7])), 
			float(pgm_read_float(&MachineSettings[machineNumber][8])),
			float(pgm_read_float(&MachineSettings[machineNumber][9])),  
			float(pgm_read_float(&MachineSettings[machineNumber][10])),  
			float(pgm_read_float(&MachineSettings[machineNumber][11])), 
			float(pgm_read_float(&MachineSettings[machineNumber][12])),  
			float(pgm_read_float(&MachineSettings[machineNumber][13])),  
			float(pgm_read_float(&MachineSettings[machineNumber][14])),
			float(pgm_read_float(&MachineSettings[machineNumber][15])),
			float(pgm_read_float(&MachineSettings[machineNumber][16])),
			float(pgm_read_float(&MachineSettings[machineNumber][17])),
			float(pgm_read_float(&MachineSettings[machineNumber][18])),
			float(pgm_read_float(&MachineSettings[machineNumber][19]))         
			);

			PrevAltTargets[machineNumber] = MachineTargetAlt[machineNumber];
			PrevAzTargets[machineNumber] = MachineTargetAz[machineNumber];
			preTargetsUsed = targetsUsed;  

			if(joystickModeOnOff==1){break;}
		}//END for (float i=1; i <= numberOfMachines; i++)

		if (iterationsAfterReset < 3){iterationsAfterReset+=1;}
		
		if (((digitalRead(WindProtectionSwitch)==LOW) || windProtect) && (midCycle==1)){
			int iloop=0;
			while (iloop<2){
				delay(1000);
				Serial.print("\r\nWind Protection Mode Enabled\r\n");
				iterationsAfterReset=0; midCycle=0;
				if ((digitalRead(WindProtectionSwitch)==HIGH) || !windProtect){iloop=5;}
			}
		}
		if (digitalRead(WindProtectionSwitch)==LOW){midCycle=midCycle+1;}   
		FirstIterationAfterArduinoReset=1;

	}
	timer.run();
}//End Void Loop
/////////////////////////////////////////////////////////// 
// END VOID LOOP
/////////////////////////////////////////////////////////// 

