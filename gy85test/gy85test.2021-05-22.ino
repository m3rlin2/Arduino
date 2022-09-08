#include "GY_85.h"
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define npPIN 6 
// How many NeoPixels are attached to the Arduino?
#define npNUMPIXELS 6 
Adafruit_NeoPixel strip(npNUMPIXELS, npPIN, NEO_GRB + NEO_KHZ800);

GY_85 GY85;    //Gyro

void setup()
{
    randomSeed(analogRead(2));
    
    //Setup NeoPixels
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    strip.setBrightness(60); // Set BRIGHTNESS to about 1/5 (max = 255)
    delay(10); 
    
    Serial.begin(9600);
    delay(10); 
    
    //Setup Gyro
    Wire.begin();
    delay(10);
    GY85.init();
    delay(10);                     
}

const float _eta = 0.2;
float ax = 0.0;
float ay = 0.0;
float az = 0.0;   

void loop()
{
    
    float _ax = GY85.accelerometer_x( GY85.readFromAccelerometer() );
    float _ay = GY85.accelerometer_y( GY85.readFromAccelerometer() );
    float _az = GY85.accelerometer_z( GY85.readFromAccelerometer() );   
    
    smooth(ax, _ax, _eta);
    smooth(ay, _ay, _eta);
    smooth(az, _az, _eta);

    Serial.print  ( ax );
    Serial.print  ( " " );
    Serial.print  ( ay );
    Serial.print  ( " " );
    Serial.println  ( az );

    //map(value, fromLow, fromHigh, toLow, toHigh)
    colorWipe(strip.Color(
        map(abs(ax),0,125,0,255),
        map(abs(ay),0,125,0,255),
        map(abs(az),0,125,0,255)
    ));
    
    delay(20);             // only read every 0,5 seconds, 10ms for 100Hz, 20ms for 50Hz
}

void smooth(float &aktValue, const float newVal, const float eta) {
  //Lowpass filter : average = ETA * (newvalue) + (1-ETA) * average;
  aktValue = eta * newVal + (1 - eta) * aktValue;
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue)
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, strip.gamma32(color));    //  Set pixel's color (in RAM)
  }
  strip.show();                          //  Update strip to match
  delay(10);                           //  Pause for a moment
}

void pulseWhite(uint8_t wait) {
  for(int j=0; j<256; j++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    strip.show();
    delay(wait);
  }

  for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    strip.show();
    delay(wait);
  }
}
