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
    Serial.println( az );

    //map(value, fromLow, fromHigh, toLow, toHigh)
    colorWipe(strip.Color(
        map( abs(ax), 0, 125, 0, 254 ),
        map( abs(ay), 0, 125, 0, 254 ),
        map( abs(az), 0, 125, 0, 254 )
    ));
    
    delay(20);            
}

void smooth(float &aktValue, const float newVal, const float eta) {
  //Lowpass filter : average = ETA * (newvalue) + (1-ETA) * average;
  aktValue = eta * newVal + (1 - eta) * aktValue;
}

