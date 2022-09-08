
//Zugangsdaten
const char* ssid = "Matrix";
const char* password = "ursacheundwirkung123";
const char* mqtt_server = "nas";
const char* mqtt_user = "admin";
const char* mqtt_pass = "dasistmaleinpasswort";

//MQTT Topics
const char* mqtt_topic_gewaechshaus_moisture = "garduino/sensor1/moisture";
const char* mqtt_topic_gewaechshaus_temperature = "garduino/sensor1/temperature";
const char* mqtt_topic_aussen_humid = "garduino/sensor2/humid";
const char* mqtt_topic_aussen_temp = "garduino/sensor2/temperature";
const char* mqtt_topic_volts = "garduino/gateway/voltage";
const char* mqtt_topic_wasser_relais = "garduino/wasser/relais";
const char* mqtt_topic_wasser_fuellstand = "garduino/wasser/fuellstand";

//Radio (NRF241l) Konfiguration
const static uint8_t RADIO_ID = 0;       // Our radio's id.  The transmitter will send to this id.
const static uint8_t PIN_RADIO_CE = 4;
const static uint8_t PIN_RADIO_CSN = 15;

//PaketTypen
const static uint8_t ptHumidAussen = 1;
const static uint8_t ptTempAussen = 2;
const static uint8_t ptMoistGewaechshaus = 3;
const static uint8_t ptTempGewaechshaus = 4; 
const static uint8_t ptRelais = 5; 
const static uint8_t ptFuellStand = 6; 

#include <SPI.h>
#include <NRFLite.h>
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
#include <Metro.h>

struct __attribute__((packed)) RadioPacket // Note the packed attribute.
{
  uint8_t FromRadioId;
  uint8_t PaketTyp;
  uint32_t Data;
} _MyRadioPacket;

Metro tmrVoltage(3*60000);
Metro tmrSendData(10000);

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
long lastMsg = 0;
int value = 0;

NRFLite _radio;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //mqtt_client.publish("outTopic", "hello world");
      // ... and resubscribe
      mqtt_client.subscribe(mqtt_topic_wasser_relais);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool inRange(float val, float minimum, float maximum)
{
  return ((minimum <= val) && (val <= maximum));
}

//Prüft ob 
bool isDiff(float aLast, float aNew)
{
	return (aLast != aNew);
}

void setup()
{
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
    
  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS))
  {
    Serial.println("Cannot communicate with radio");
  }

  //Init Paketdata
  _MyRadioPacket.FromRadioId = RADIO_ID;
  _MyRadioPacket.PaketTyp = 0;
  _MyRadioPacket.Data = 0;

  WiFi.mode(WIFI_STA);
  setup_wifi();
  
  randomSeed(micros());
  
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
}

uint16_t aktMoistureGewaechshaus = 0, lastMoistureGewaechshaus = 0;
uint16_t aktFuellstandTonne = 0, lastFuellstandTonne = 0;
float aktTempGewaechshaus = -250.0, lastTempGewaechshaus = -250.0;
float aktVolt = -250.0, lastVolt = -250.0;
float aktTempAussen = -250.0, lastTempAussen = -250.0;
float aktHumidAussen = -250.0, lastHumidAussen = -250.0;

void loop()
{
	//WLAN Verbinden
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  
	//MQTT verbinden und bei Dateneingang senden
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  mqtt_client.loop();
	
  //Funksensor empfangen
  byte paketsize = _radio.hasData();
	int tmpint;
	float tmpflt;
	float faktor = 1.0;
  while (paketsize)
  {
    if (paketsize == sizeof(_MyRadioPacket))
    {
			_radio.readData(&_MyRadioPacket); 

			String msg = "Radio ";
			msg += _MyRadioPacket.FromRadioId;
			msg += ", PaketTyp: ";
			msg += _MyRadioPacket.PaketTyp;
			msg += ", Data: ";
			msg += _MyRadioPacket.Data;
			Serial.println(msg);
			
			//Daten anhand des PaketTyps auswerten
			switch (_MyRadioPacket.PaketTyp) 
			{
				case ptHumidAussen:
					tmpflt = (float)_MyRadioPacket.Data / 1000.0;
					if (inRange(tmpflt, 1.0, 100.0)) {
						aktHumidAussen = tmpflt;
					}
					break;
					
				case ptTempAussen:
					faktor = 1.0;
					if (_MyRadioPacket.Data > 10000) {
						_MyRadioPacket.Data = _MyRadioPacket.Data - 10000;
						faktor = -1.0;
					}
					tmpflt = (float)_MyRadioPacket.Data / 100.0 * faktor;
					if (inRange(tmpflt, -20.0, 50.0)) {
						aktTempAussen = tmpflt;
					}
					break;

				case ptMoistGewaechshaus:
					tmpint = _MyRadioPacket.Data;
					if (inRange(tmpint, 1, 1023)) {
						aktMoistureGewaechshaus = tmpint;
					}
					break;
					
				case ptTempGewaechshaus:
					faktor = 1.0;
					if (_MyRadioPacket.Data > 10000) {
						_MyRadioPacket.Data = _MyRadioPacket.Data - 10000;
						faktor = -1.0;
					}
					tmpflt = (float)_MyRadioPacket.Data / 100.0 * faktor;
					if (inRange(tmpflt, -20.0, 50.0)) {
						aktTempGewaechshaus = tmpflt;
					}
					break;
				
				case ptTempGewaechshaus:	
					aktFuellstandTonne = _MyRadioPacket.Data;
					break;
				
				default:
					Serial.print(_MyRadioPacket.PaketTyp);
					Serial.println(F(" ungültiger PaketTyp"));
					break;
			}
    }
    else
    {
      byte trash;
      _radio.readData(&trash);
      Serial.println("Müll empfangen");
    }

    paketsize = _radio.hasData();
  }

  if (tmrSendData.check())
  {
    if (isDiff(aktTempGewaechshaus, lastTempGewaechshaus) && inRange(aktTempGewaechshaus,-10.0,50.0))
		{
      mqtt_client.publish(mqtt_topic_gewaechshaus_temperature,String(aktTempGewaechshaus).c_str());
			lastTempGewaechshaus = aktTempGewaechshaus;
		}
    if (isDiff(aktMoistureGewaechshaus, lastMoistureGewaechshaus) && inRange(aktMoistureGewaechshaus,1.0,1023.0))
		{
      mqtt_client.publish(mqtt_topic_gewaechshaus_moisture,String(aktMoistureGewaechshaus).c_str());
			lastMoistureGewaechshaus = aktMoistureGewaechshaus;
		}
    if (isDiff(aktTempAussen,lastTempAussen) && inRange(aktTempAussen,-20.0,50.0))
		{
      mqtt_client.publish(mqtt_topic_aussen_temp,String(aktTempAussen).c_str());
			lastTempAussen = aktTempAussen;
		}
    if (isDiff(aktHumidAussen, lastHumidAussen) && inRange(aktHumidAussen,1.0,1023.0))
		{
      mqtt_client.publish(mqtt_topic_aussen_humid,String(aktHumidAussen).c_str());
			lastHumidAussen = aktHumidAussen;
		}
		if (isDiff(aktFuellstandTonne, lastFuellstandTonne) && inRange(aktFuellstandTonne,0.0,500.0))
		{
      mqtt_client.publish(mqtt_topic_wasser_fuellstand,String(aktFuellstandTonne).c_str());
			lastFuellstandTonne = aktFuellstandTonne;
		}
  }

  if (tmrVoltage.check())
  {
    aktVolt = ReadVolt();
    mqtt_client.publish(mqtt_topic_volts,String(aktVolt).c_str()); 
  }
  
} //loop 

float ReadVolt() {
    uint16_t aVal = analogRead(A0);
    float volts = 3.05/1024.0*(float)aVal;
    Serial.print(volts,3);Serial.print("V an A0  :=  ");
    volts = volts * (26.45); 
    Serial.print(volts,2);Serial.println('V');
    return volts;
}
