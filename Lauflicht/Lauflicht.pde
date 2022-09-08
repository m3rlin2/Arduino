/*
 * AnalogInput
 * by DojoDave <http://www.0j0.org>
 *
 * Turns on and off a light emitting diode(LED) connected to digital  
 * pin 13. The amount of time the LED will be on and off depends on
 * the value obtained by analogRead(). In the easiest case we connect
 * a potentiometer to analog pin 2.
 *
 * http://www.arduino.cc/en/Tutorial/AnalogInput
 */


void setup() {
  pinMode(13, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(12, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(11, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(10, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(9, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(8, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(7, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(6, OUTPUT);  // declare the ledPin as an OUTPUT
  Serial.begin(9600);
}

void loop() {
  for (int i=13; i > 6; i--)
  {
    digitalWrite(i,HIGH);
    delay(analogRead(2));
    }
     for (int i=13; i > 6; i--)
  {
    digitalWrite(i,LOW);
    delay(analogRead(2));
    }
}
