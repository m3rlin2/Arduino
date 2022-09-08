/*********************************************************************
  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98
*********************************************************************/
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Metro.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3f for a 16 chars and 2 line display

float oldTemp, temp;

void setup()   {                
  Serial.begin(115200);
  Serial.println(F("Setup begin"));
  
  // initialize the lcd
  lcd.init();     
  lcd.backlight();
  
  //Update Temperature
  temp = getTemp(); 
  
  Serial.println(F("Setup end"));
}

Metro timerLogToSD = Metro(5*60*1000);
Metro timerUpdate = Metro(1000);
Metro timerSerialPrint = Metro(1000);

void loop() {
  
  if(timerUpdate.check()) {
    temp = getTemp(); //Update Temperature
    
    //Update Time
    lcd.setCursor(0,0);
    lcd.print(Timestamp());
  }
  
  if(temp!=oldTemp){
    lcd.setCursor(0,1);
    lcd.print(F("Temp: ")); lcd.print(temp,1);
    //lcd.print(F(" V:")); lcd.print(VCC(),2);
    oldTemp=temp;
  } 
  
  if(timerLogToSD.check())
    LogData();
    
  if(timerSerialPrint.check()) {  
    Serial.print(freeRam());
    Serial.print(VCC(),2);
    Serial.println();
  }
  
  delay(1);  
}

int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

float VCC(){
  return (5.0/1024.0) * (analogRead(2));
}



