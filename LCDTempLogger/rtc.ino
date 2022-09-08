/*
 * TimeRTC.pde
 * example code illustrating Time library with Real Time Clock.
 * 
 */

#include <Time.h>  
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

//global Variable 
bool RTCInitialized = false; 

bool setupRTC()  {
  if(!RTCInitialized){
    //setTime(19,7,00,18,12,2014);
    //RTC.set(now());
    
    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    setSyncInterval(60);
    
    RTCInitialized = (timeStatus()==timeSet);
  }  
  
  return RTCInitialized;  
}


String Timestamp(){
  setupRTC();
  
  // digital clock display of the time
  String tmp = String(hour())+":"+Print2Digit(minute())+":"+Print2Digit(second());
  tmp += " "+String(day())+"."+String(month())+"."+String(year()); 

  return tmp;
}


String Print2Digit(byte Val)
{
  String dataString = "";
  if (Val < 10)
  {
    dataString = "0";
  }  
  dataString += String(Val, DEC);
  return dataString;
}


