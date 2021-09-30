/*
  ReadAnalogVoltage

  Reads an analog input on pin 0, converts it to voltage, and prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/ReadAnalogVoltage
*/

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}
int max = 0;
// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
    int sensorValue = analogRead(A0);
    float voltage = sensorValue * (5.0 / 1023.0);
  if (sensorValue>max){
      max = sensorValue;
    Serial.print(sensorValue);
    Serial.print("\t");
    Serial.print(voltage);
    Serial.println("\r\n");
  }
    if(voltage > 1)
        digitalWrite(LED_BUILTIN, HIGH);
    else
        digitalWrite(LED_BUILTIN, LOW);

  if(millis()%50 == 0)
      max--;
}
