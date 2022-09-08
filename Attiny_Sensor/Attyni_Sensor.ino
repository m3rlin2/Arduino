/* ATtiny85 as an I2C Slave            BroHogan                           1/12/11
 * Example of ATtiny I2C slave receiving and sending data to an Arduino master.
 * Gets data from master, adds 10 to it and sends it back.
 * SETUP:
 * ATtiny Pin 1 = (RESET) N/U                      ATtiny Pin 2 = (D3) N/U
 * ATtiny Pin 3 = (D4) to LED1                     ATtiny Pin 4 = GND
 * ATtiny Pin 5 = I2C SDA on DS1621  & GPIO        ATtiny Pin 6 = (D1) to LED2
 * ATtiny Pin 7 = I2C SCK on DS1621  & GPIO        ATtiny Pin 8 = VCC (2.7-5.5V)
 * NOTE! - It's very important to use pullups on the SDA & SCL lines!
 * Current Rx & Tx buffers set at 32 bytes - see usiTwiSlave.h
 * Credit and thanks to Don Blake for his usiTwiSlave code. 
 * More on TinyWireS usage - see TinyWireS.h
 */

#include "TinyWireS.h"                  // wrapper class for I2C slave routines
#include <Metro.h>

#define I2C_SLAVE_ADDRESS  0x26            // i2c slave address (38)
#define LEDPIN         1              // ATtiny Pin 6
#define MessPIN1       3              // ATtiny Pin 6
#define MessPIN2       2              // ATtiny Pin 6

byte aktMesswert = 0;
Metro messen = Metro(1000);

void setup(){
  pinMode(LEDPIN,OUTPUT);             // for general DEBUG use
  TinyWireS.begin(I2C_SLAVE_ADDRESS);
    
  Blink(LEDPIN,3);                    // show it's alive
}

void loop(){  
 
  if (TinyWireS.available()){           // got I2C input!
    //byteRcvd = TinyWireS.receive();     // get the byte from master
    //byteRcvd += 10;                     // add 10 to what's received
    while(TinyWireS.available())
    {
        TinyWireS.receive();
    }
    TinyWireS.send(aktMesswert);           // send it back to master

    Blink(LEDPIN,2);                  // show we transmitted
  }
  

  if (messen.check() == 1) {
    aktMesswert = readSensor(MessPIN1);
  }
}

byte readSensor(byte pin){
  pinMode(pin, OUTPUT);  
  digitalWrite(pin,HIGH);
  delay(500);
  pinMode(pin, INPUT);
  unsigned long ms = millis();
  digitalWrite(pin,LOW);
  while((digitalRead(pin)==HIGH) && ((millis()-ms)<100))
  {
     delay(1);  
  }
  return map(millis()-ms, 1, 100, 0, 254);
}

void Blink(byte led, byte times){ // poor man's display
  for (byte i=0; i< times; i++){
    digitalWrite(led,HIGH);
    delay (150);
    digitalWrite(led,LOW);
    delay (150);
  }
}



