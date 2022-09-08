#include <SPI.h>
#include <NRFLite.h>
#include <Metro.h>
#include <NewPing.h>

const static uint8_t RADIO_ID = 3;
const static uint8_t DESTINATION_RADIO_ID = 0;
const static uint8_t PIN_RADIO_CE  = 9;
const static uint8_t PIN_RADIO_CSN = 10;

const static uint8_t relaisPumpe = 2;
const static uint8_t relaisVentil = 3;
const static uint8_t SonarTrigger  = 6; // Arduino pin tied to trigger pin on the ultrasonic sensor.
const static uint8_t SonarEcho = 5; // Arduino pin tied to echo pin on the ultrasonic sensor.  

//PaketTypen
const static uint8_t ptHumidAussen = 1;
const static uint8_t ptTempAussen = 2;
const static uint8_t ptMoistGewaechshaus = 3;
const static uint8_t ptTempGewaechshaus = 4; 
const static uint8_t ptRelais = 5; 
const static uint8_t ptFuellStand = 6; 

struct __attribute__((packed)) RadioPacket // Note the packed attribute.
{
  uint8_t FromRadioId;
  uint8_t PaketTyp;
  uint32_t Data;
} _MyRadioPacket;

Metro tmrTonne(500);
Metro tmrSendData(5000);

NewPing sonar(SonarTrigger, SonarEcho, 180); // NewPing setup of pins and maximum distance.

NRFLite _radio;
bool _RadioConnected;

float aktFuellStand = 0.0;

void setup()
{
  initPins();
	
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
  
  if (!lox.begin(0x29)) {
    Serial.println(F("Failed to boot VL53L0X"));
  }
  
  
  aktFuellStand = (float)sonar.ping_cm();
  
  Serial.println(F("Setup fertig"));
}

void SendData()
{
  if (_radio.send(DESTINATION_RADIO_ID, &_MyRadioPacket, sizeof(_MyRadioPacket)))
  {
    Serial.println("success");
		//Blink(13,1);
  }
  else
  {
    Serial.println("failed");
		//Blink(13,3);
  }  
}

void loop()
{
  //Read Füllstand
  if (tmrTonne.check()) {
    aktFuellStand = (float)sonar.ping_cm();
	
	Serial.print(aktFuellStand);Serial.println(" cn");
	
	digitalWrite(relaisPumpe, !digitalRead(relaisPumpe));
	digitalWrite(relaisPumpe, !digitalRead(relaisVentil));
  }

  if (tmrSendData.check())
  {
    //Paket schnüren und senden
    _MyRadioPacket.PaketTyp = ptFuellStand;
    _MyRadioPacket.Data = aktFuellStand;
    SendData();
		
  }//tmrSendData

}//loop

void initPins(){
	for (int i=2; i<13;i++){
		pinMode(i, INPUT);
		digitalWrite(i, LOW);
	}
	pinMode(relaisPumpe, OUTPUT);
}

void Blink(byte led, byte times){ // poor man's display
  for (byte i=0; i< times; i++){
    digitalWrite(led,HIGH);
    delay (150);
    digitalWrite(led,LOW);
    delay (150);
  }
}
