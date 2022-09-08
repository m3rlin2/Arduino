#include "DHT.h"

//DHT11 Signal Pin
#define DHT11PIN 9
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHT11PIN, DHTTYPE);

float getTemp(void){
  float tempDHT = -255;
// Read temperature as Celsius
  tempDHT = dht.readTemperature();
  
  return tempDHT; 
}

void printTemp(void)
{   
  Serial.print("Temperature is: ");
  Serial.println(getTemp());  
}

