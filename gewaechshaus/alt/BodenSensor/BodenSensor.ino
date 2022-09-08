#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <NRFLite.h>
#include <Metro.h>


const static uint8_t RADIO_ID = 2;
const static uint8_t DESTINATION_RADIO_ID = 0;
const static uint8_t PIN_RADIO_CE  = 9;
const static uint8_t PIN_RADIO_CSN = 10;

const static uint8_t moisturePin1 = 5;
const static uint8_t moisturePin2 = 4;
const static uint8_t moistureReadPin = A0;
const static uint16_t flipTimer      = 200;

const static uint8_t ONE_WIRE_BUS = 2;

struct RadioPacket
{
  uint16_t Moisture;
  int16_t Temperature;
  uint16_t FromRadioId;
};

Metro tmrMoisture = Metro(5*60000-10);
Metro tmrTemp = Metro(5*60000-10);
Metro tmrSendData = Metro(10000);
Metro tmrMustSendData = Metro(5*60000);

NRFLite _radio;
RadioPacket _radioData;
bool _RadioConnected;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature TempSensor(&oneWire);

void setup()
{
  Serial.begin(115200);

  _RadioConnected = true;
  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS))
  {
    Serial.println(F("Cannot communicate with radio"));
    _RadioConnected = false;
  }

  _radioData.FromRadioId = RADIO_ID;
  _radioData.Moisture = -1;
  _radioData.Temperature = -1;

  TempSensor.begin();
}

void setSensorPolarity(boolean flip) {
  if (flip) {
    digitalWrite(moisturePin1, HIGH);
    digitalWrite(moisturePin2, LOW);
  }
  else {
    digitalWrite(moisturePin1, LOW);
    digitalWrite(moisturePin2, HIGH);
  }
}

int GetMoisture() {
  pinMode(moisturePin1, OUTPUT);
  pinMode(moisturePin2, OUTPUT);
  setSensorPolarity(true);
  delay(flipTimer);
  int val1 = analogRead(moistureReadPin);
  setSensorPolarity(false);
  delay(flipTimer);
  int val2 = 1023 - analogRead(moistureReadPin);  // invert the reading

  //ausschalten
  digitalWrite(moisturePin1, LOW);
  digitalWrite(moisturePin2, LOW);

  int avg = (val1 + val2) / 2;

  Serial.print("Read1: "); Serial.print(val1); Serial.print("   ");
  Serial.print("Read2: "); Serial.print(val2); Serial.print("   = ");
  Serial.print(avg); Serial.println("");
  avg = constrain(avg,0,1024);
  return avg;
}

float ReadTemp() {
  TempSensor.requestTemperatures(); // Send the command to get temperatures
  float tempC = TempSensor.getTempCByIndex(0);
  Serial.print("Temperature: ");Serial.println(tempC);  
  tempC = constrain(tempC, -10.0, 65.0);
  return tempC;
}

int aktMoisture = -1;
float aktTemp = 0.0;

void loop()
{
  //Read Moisture
  if (tmrMoisture.check())
    aktMoisture = GetMoisture();

  //Read Temperature
  if (tmrTemp.check())
    aktTemp = ReadTemp();

  if (tmrSendData.check())
  {
      _radioData.Temperature = aktTemp*100;
      _radioData.Moisture = aktMoisture;
      if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData)))
      {
        Serial.println("success");
      }
      else
      {
        Serial.println("failed");
      }     
    /* //Try reconnect
    if (!_RadioConnected) {
      _RadioConnected = true;
      if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS))
      {
        Serial.println(F("Cannot communicate with radio"));
        _RadioConnected = false;
      }
    }

    //überprüfen ob sich Werte geändert haben
    bool dataChanged = tmrMustSendData.check();
    
    if ( tmrMustSendData.check() || 
         (((float)abs(aktMoisture - _radioData.Moisture) / (float)aktMoisture) > 0.05) 
       )//filter variatoins of 5%
    {
      dataChanged = true;
      _radioData.Moisture = aktMoisture;
    }
    if ( tmrMustSendData.check() || 
         (((float)abs(aktTemp - (float)_radioData.Temperature) / (float)aktTemp) > 0.01))
    {
      dataChanged = true;
      _radioData.Temperature = aktTemp;
    }

    //send Data
    if (_RadioConnected && dataChanged)
    {
      _radioData.Temperature = 999;//aktTemp;Serial.println(_radioData.Temperature);
      _radioData.Moisture = 550;//aktMoisture;
      if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData)))
      {
        Serial.println("success");
      }
      else
      {
        Serial.println("failed");
      } 
    } */
  }//tmrSendData

}//loop
