/*
  SD card datalogger
 
 This example shows how to log data from three analog sensors 
 to an SD card using the SD library.
 	
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 
 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe
 
 This example code is in the public domain.
 	 
 */

#include <SD.h>

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 4;

//global Variable 
bool SDCardInitialized = false;

bool setupDatalogger()
{
  if (!SDCardInitialized) {
    // make sure that the default chip select pin is set to
    // output, even if you don't use it:
    pinMode(10, OUTPUT);
    
    // see if the card is present and can be SDCardInitialized:
    SDCardInitialized = SD.begin(chipSelect);
  };
  
  Serial.print(F("init SDcard:"));
  if (!SDCardInitialized) {
    Serial.println(F("FAILED")); }
  else
    Serial.println(F("SUCCESS"));
    
  return SDCardInitialized;
}

void LogData()
{
  //setupDatalogger(); //initialize SDCard
  SD.begin(chipSelect);
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print(Timestamp()+";"); 
    dataFile.print(getTemp(),1); dataFile.print(";"); 
    dataFile.print(analogRead(3));dataFile.print(";"); 
    dataFile.print(VCC(),2);dataFile.print(";");
    dataFile.print(freeRam());dataFile.print(";");
    dataFile.println();
    dataFile.close();
    // print to the serial port too:
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println(F("error opening datalog.txt"));
    SDCardInitialized = false;
  } 
  SD.end();
}








