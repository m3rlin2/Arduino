
#include <dummy.h>

// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

/*
#include <DHT.h>

#define DHTPIN 1     // what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE,30);
// NOTE: For working with a faster chip, like an Arduino Due or Teensy, you
// might need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// Example to initialize DHT sensor for Arduino Due:
//DHT dht(DHTPIN, DHTTYPE, 30);
*/

void setup() {
  Serial.begin(115200); 
  Serial.println("DHTxx test!");
  
  for(int i=1;i<15;i++){
    Serial.print("pin");Serial.println(i);
    pinMode(i, OUTPUT);
    digitalWrite(i,HIGH);
    delay(2000);
    digitalWrite(i,LOW);
  }
  
//  dht.begin();
}

void loop() {
   
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  /*float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();

  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else
  {
    Serial.print("Humidity: "); 
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(t);
    Serial.println(" C ");
  }
*/
  delay(2001);

}
