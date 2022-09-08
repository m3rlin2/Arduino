#include <SPI.h>
#include <NRFLite.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Metro.h>

const char* ssid = "Matrix";
const char* password = "ursacheundwirkung123";
const char* mqtt_server = "nas";
const char* mqtt_user = "admin";
const char* mqtt_pass = "dasistmaleinpasswort";

const char* mqtt_topic_moisture = "garduino/sensor1/moisture";
const char* mqtt_topic_temperature = "garduino/sensor1/temperature";
const char* mqtt_topic_volts = "garduino/gateway/voltage";
const char* mqtt_topic_pump = "garduino/gateway/pump";


const static uint8_t RADIO_ID = 0;       // Our radio's id.  The transmitter will send to this id.
const static uint8_t PIN_RADIO_CE = 4;
const static uint8_t PIN_RADIO_CSN = 15;

struct RadioPacket
{
  uint16_t Moisture;
  int16_t Temperature;
  uint16_t FromRadioId;
};

Metro tmrVoltage = Metro(3*60000);

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
long lastMsg = 0;
int value = 0;

NRFLite _radio;
RadioPacket _radioData;

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
      mqtt_client.subscribe(mqtt_topic_pump);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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

  _radioData.FromRadioId = RADIO_ID;
  _radioData.Moisture = -1;

  setup_wifi();
  
  randomSeed(micros());
  
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
}

uint16_t aktMoisture = 0;
float aktTemp = 0.0;
float aktVolt = 0.0;

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  
  ///////////////////////////////
  //   Funksensor empfangen
  //////////////////////////////
  boolean _DataIncome = false;
  byte paketsize = _radio.hasData();
  while (paketsize)
  {
    if (paketsize == sizeof(_radioData))
    {
        _radio.readData(&_radioData); 
    
        String msg = "Radio ";
        msg += _radioData.FromRadioId;
        msg += ", Moisture: ";
        msg += _radioData.Moisture;
        msg += ", Temperatur: ";
        msg += _radioData.Temperature;
        Serial.println(msg);
    
        aktTemp = constrain((float)_radioData.Temperature/100.0, -10.0, 65.0);
        aktMoisture = constrain(_radioData.Moisture, 0,1024);
        
        _DataIncome = true;
    }
    else
    {
      byte trash;
      _radio.readData(&trash);
      Serial.println("Müll empfangen");
    }

    paketsize = _radio.hasData();
  }

  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  mqtt_client.loop();

  if (_DataIncome)
  {
    mqtt_client.publish(mqtt_topic_temperature,String(aktTemp).c_str());
    mqtt_client.publish(mqtt_topic_moisture,String(aktMoisture).c_str());
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






