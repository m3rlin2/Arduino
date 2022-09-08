#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <NRFLite.h>
#include <Metro.h>


const static uint8_t RADIO_ID = 1;
const static uint8_t DESTINATION_RADIO_ID = 0;
const static uint8_t PIN_RADIO_CE  = 9;
const static uint8_t PIN_RADIO_CSN = 10;

const static uint8_t moisturePin1 = 5;
const static uint8_t moisturePin2 = 4;
const static uint8_t moistureReadPin = A0;
const static uint16_t flipTimer      = 200;

const static uint8_t ONE_WIRE_BUS = 2;

//PaketTypen
const static uint8_t ptMoistGewaechshaus = 3;
const static uint8_t ptTempGewaechshaus = 4; 

struct __attribute__((packed)) RadioPacket // Note the packed attribute.
{
  uint8_t FromRadioId;
  uint8_t PaketTyp;
  uint32_t Data;
} _MyRadioPacket;

Metro tmrMoisture(5*60000);
Metro tmrTemp(5*60000);
Metro tmrSendData(10000);

NRFLite _radio;
bool _RadioConnected;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature TempSensor(&oneWire);

int aktMoisture = -1;
float aktTemp = 0.0;

void setup()
{
  Serial.begin(115200);

  _RadioConnected = true;
  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS))
  {
    Serial.println(F("Cannot communicate with radio"));
    _RadioConnected = false;
  }

  _MyRadioPacket.FromRadioId = RADIO_ID;
  _MyRadioPacket.PaketTyp = 0;
  _MyRadioPacket.Data = 0;

  TempSensor.begin();

  aktTemp = ReadTemp();
  aktMoisture = GetMoisture();
  
	Serial.println(F("Setup fertig"));
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
  tempC = constrain(tempC, -20.0, 65.0);
  return tempC;
}

void SendData()
{
  if (_radio.send(DESTINATION_RADIO_ID, &_MyRadioPacket, sizeof(_MyRadioPacket)))
  {
    Serial.println("success");
  }
  else
  {
    Serial.println("failed");
  }  
}

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
    //Paket schnüren und senden
    _MyRadioPacket.PaketTyp = ptMoistGewaechshaus;
    _MyRadioPacket.Data = aktMoisture;
    SendData();

    _MyRadioPacket.PaketTyp = ptTempGewaechshaus;
		_MyRadioPacket.Data = abs(aktTemp) * 100.0;
		if (aktTemp < 0.000) {
			_MyRadioPacket.Data = _MyRadioPacket.Data + 10000;
		}
    SendData();
		
  }//tmrSendData

}//loop
