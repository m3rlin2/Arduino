#include <OneWire.h>
#include <DallasTemperature.h>
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 7
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature tempSens(&oneWire);

//Soil moisture
byte GetMoisture(boolean aWriteToEEPROM){
  //
  setSensorPolarity(true);
  delay(flipTimer);
  int val1 = analogRead(sensorPin);
  setSensorPolarity(false);
  delay(flipTimer);
  // invert the reading
  int val2 = 1023 - analogRead(sensorPin);
  //  
  //aus schalten
  set(voltageFlipPin1,LOW);
  set(voltageFlipPin2,LOW);
  
  int avg = (val1 + val2) / 2;
  //int->byte
  byte byteval = byte(map(avg, 0, 1023, 0, 255));
  
  Serial.print(val1);
  Serial.print("\t");
  Serial.print(val2);
  Serial.print(" : ");
  Serial.print(avg);
  Serial.print(" = ");
  Serial.println(byteval);
  Serial.flush();
  
  if (aWriteToEEPROM){
    writeEEPROM(byteval);  
  }
  
  //float temp = GetTempVal();
  
  return byteval;
}

void setSensorPolarity(boolean flip){
  if(flip){
    set(voltageFlipPin1, HIGH);
    set(voltageFlipPin2, LOW);
  }
  else{
    set(voltageFlipPin1, LOW);
    set(voltageFlipPin2, HIGH);
  }
}

void SetupTempSens()
{
  Serial.print(F("Setup TempSensor..."));
  tempSens.begin();
  Serial.println(F("done \n"));
  Serial.flush();
}

float GetTempVal()
{
  Serial.print(F("Read TempSensor..."));
  tempSens.requestTemperatures(); // Send the command to get temperatures
  float temp = tempSens.getTempCByIndex(0);
  Serial.print(F("done : "));Serial.println(temp);
  return temp;
}
