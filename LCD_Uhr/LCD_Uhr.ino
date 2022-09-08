template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; } 

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DS1307new.h>


LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x3f for a 16 chars and 2 line display

void setup()
{
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.print("Henne's LCD Uhr");
  lcd.setCursor(0,1);
  for(int i=0; i<16; i++){
    lcd.print("=");
    delay(70);
  }
}
char wd[7][3] = { "So", "Mo", "Tu", "We", "Th", "Fr", "Sa" };

void loop()
{
  lcd.clear();
  RTC.getTime();
  lcd << wd[RTC.dow] << " " << RTC.day << "." << RTC.month << "." << RTC.year;
  lcd.setCursor(0,1);
  lcd << RTC.hour << ":" << RTC.minute << ":" << RTC.second;
  //print_date(RTC.year, RTC.month, RTC.day);
 
  delay(1000);
  
}
