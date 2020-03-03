
void TargetControl(){
	if (FirstIterationAfterArduinoReset!=0 && joystickModeOnOff!=1){

		/////////////////////////////////////////////////////////////////////////////
		//PROGRAM THE TARGETS HERE
		//////////////////////////////////////////////////////////////////////////////

		// SAVES TARGET SETTINGS WHEN MANUAL CONTROL THROUGH EITHER THE SERIAL MONITOR OR THE JOYSTICK IS TURNED OFF
		if (justFinishedManualControl==1){
			Serial.println("Writing to EEPROM");
			for (int i=0; i <= numberOfMachines - 1; i++){
				
				union u_tag{ byte b[4]; float f; } u;
				
				EEPROM.begin(8);
				
				u.f = MachineTargetAlt[i];
				EEPROM.write(0, u.b[0]);
				EEPROM.write(1, u.b[1]);
				EEPROM.write(2, u.b[2]);
				EEPROM.write(3, u.b[3]);
				
				u.f = MachineTargetAz[i];
				EEPROM.write(4, u.b[0]);
				EEPROM.write(5, u.b[1]);
				EEPROM.write(6, u.b[2]);
				EEPROM.write(7, u.b[3]);
				
				EEPROM.end();
			}
			justFinishedManualControl=0;
		}

		// READS TARGET SETTINGS
		for (int i=0; i <= numberOfMachines - 1; i++){

			//~ MachineTargetAlt[i] = 0.0;
			//~ MachineTargetAz[i]  = -70.0;

			union u_tag{ byte b[4]; float f; } u;
			
			EEPROM.begin(8);
			
			u.b[0] = EEPROM.read(0);
			u.b[1] = EEPROM.read(1);
			u.b[2] = EEPROM.read(2);
			u.b[3] = EEPROM.read(3);
			MachineTargetAlt[i] = u.f;
			
			u.b[0] = EEPROM.read(4);
			u.b[1] = EEPROM.read(5);
			u.b[2] = EEPROM.read(6);
			u.b[3] = EEPROM.read(7);
			MachineTargetAz[i] = u.f;
			
			EEPROM.end();
		}
		/////////////////////////////////////////////////////////////////////////////
		//END PROGRAM THE TARGETS
		//////////////////////////////////////////////////////////////////////////////
	}
}

