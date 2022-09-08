
const static uint8_t RADIO_ID = 2;
const static uint8_t DESTINATION_RADIO_ID = 0;
const static uint8_t PIN_RADIO_CE  = 9;
const static uint8_t PIN_RADIO_CSN = 10;


#include <SPI.h>
#include <NRFLite.h>
#include <DHT.h>
#include <Metro.h>

//PaketTypen
const static uint8_t ptHumidAussen = 1;
const static uint8_t ptTempAussen = 2;

struct __attribute__((packed)) RadioPacket // Note the packed attribute.
{
  uint8_t FromRadioId;
  uint8_t PaketTyp;
  uint32_t Data;
} _MyRadioPacket;

Metro tmrRead(1*60000);
Metro tmrSendData(10000);

NRFLite _radio;
bool _RadioConnected;
DHT dht(8, DHT22);

float AktHumid = -250;
float AktTemp = -250;

void Blink(byte led, byte times){ // poor man's display
  for (byte i=0; i< times; i++){
    digitalWrite(led,HIGH);
    delay(500);
    digitalWrite(led,LOW);
    delay(500);
  }
}

void setup()
{   
  Serial.begin(115200);
  
  _RadioConnected = true;
  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS))
  {
    Serial.println(F("Cannot communicate with radio"));
    _RadioConnected = false;
  }

  //Init Paketdata
  _MyRadioPacket.FromRadioId = RADIO_ID;
  _MyRadioPacket.PaketTyp = 0;
  _MyRadioPacket.Data = 0;

  dht.begin();

  randomSeed(micros());

  ReadDHTSensor();
  
  Serial.println(F("Setup fertig"));  
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


void ReadDHTSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();  
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  AktHumid = h;
  AktTemp = t;
  
  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.println(" *C ");
}

Metro tmrVoltage(3*60000);

void loop()
{
  if (tmrRead.check())
    ReadDHTSensor();
  
  if (tmrSendData.check())
  {
    //Paket schnüren und senden
    _MyRadioPacket.PaketTyp = ptHumidAussen;
    _MyRadioPacket.Data = AktHumid*1000.0;
    SendData();

    _MyRadioPacket.PaketTyp = ptTempAussen;
		_MyRadioPacket.Data = abs(AktTemp) * 100.0;
		if (AktTemp < 0.000) {
			_MyRadioPacket.Data = _MyRadioPacket.Data + 10000;
		}
    SendData();
    
  }//tmrSendData

}//loop
