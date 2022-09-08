#include <dummy.h>

#include <Event.h>
#include <Timer.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <MQTT.h>
#include <PubSubClient.h>

#define DEBUG 
//command power sensor
#define SW_SENSOR   4 
#define DATA_SENSOR 5

//for jumper configuration
#define HEATER_NUM0 12
#define HEATER_NUM1 13
#define HEATER_NUM2 14

#ifdef DEBUG
  #define REFRESH_PERIOD 5000 
  #define DEEP_SLEEP (25 * 1000000)
#else
  #define REFRESH_PERIOD 60*1000*5
  #define DEEP_SLEEP (60 * 5 * 1000000)
#endif
// DS18S20 Temperature chip i/o
#define DS18B20 0x28     // Adresse 1-Wire du DS18B20
OneWire ds(DATA_SENSOR);  // on pin 10
//24-05 :Timer mainTimer;

//global variables
float Temperature;
unsigned int TemperatureRAW;
unsigned int VBAT;

const char *pass =  "PassWord";   
char ssid[25];

 char *RoomID = "Maison";
 char *DeviceID = "Température";
 char *DeviceIDTemp = "Sonde  ";//6th caracters to add the number done by jumper on GPIO[12..14]
 
 char sensorNumber;




// Update these with values suitable for your network.
IPAddress server(192,168,1,13);
const char *MQTTClient = "ESP8266_sensor";

WiFiClient wclient;
PubSubClient client(wclient, server);

/*
 * SETUP
 */
void setup() {
  // Setup console
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();

  pinMode (SW_SENSOR, OUTPUT);
  pinMode (HEATER_NUM0, INPUT); 
  pinMode (HEATER_NUM1, INPUT); 
  pinMode (HEATER_NUM2, INPUT); 
  pinMode(A0, INPUT);
  delay (10);//to be sure that the input are stabilized
 
//add the Heater number to the name of device ID
  sensorNumber = 0x30 + digitalRead(HEATER_NUM0) + digitalRead(HEATER_NUM1) * 2 + digitalRead(HEATER_NUM2)*4;
  DeviceIDTemp[6] = sensorNumber;
  #ifdef DEBUG
    Serial.print("Sensor number ");
    Serial.println(DeviceIDTemp);
  #endif
}

/*
 * ReadVBAT
 * Read the VBAT power by reading the Vdiode @ 0.6V
 * return VBAT in mV
 */
void MeasureVBAT()
{
long temp;
temp = (long)analogRead(A0);
temp = temp * 1100;
temp = temp / 39;
temp = temp * 139;
temp = temp / 1024;
VBAT =  (int)temp;
}

 
/* Read the temperature sensor 
 * return float equal to the temperature in °C
 */
void ReadSensor_DS18B20()
{
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  char strTemp[6];
  
  Temperature = 0.0;
 //search the DS18B20 chip adresse
 ds.reset_search();
  if ( !ds.search(addr)) {
      Serial.print("No more address.\n");
      ds.reset_search();
      return;
  }
//display the serial chip ID on debug interface
/*  Serial.print("R=");
  for( int i = 0; i < 8; i++) {
    Serial.print(addr[i], HEX);
    Serial.print(" ");
  }*/

if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
      return;
  }
switch(addr[0])
{
  case 0x10 :
//      Serial.print("Device is a DS18S20 family device.\n");
  break;
  case 0x28:
//      Serial.print("Device is a DS18B20 family device.\n");
  break;
  default:
     Serial.print("Device family is not recognized: 0x");
     Serial.println(addr[0],HEX);
     return;
  break;
}

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  TemperatureRAW = ((data[1] << 8) + data[0] ) ;
  Temperature = TemperatureRAW *0.0625;
return ;
}


/*
 * PublishSenso : send on MQTT the sensor value
 */
void PublishSensor()
{
String MQTTPublishMsg;
char strTemp[6];
int entier = Temperature;
int decimal = Temperature*10-entier*10;

sprintf (strTemp,"%2d.%d°C",entier,decimal );
#ifdef DEBUG
  Serial.print(" Temp=");
  Serial.print(strTemp);
  Serial.print("\n\r");
#endif

if (client.connected()) {
  // update message to MQTT provider
  //Publish Temp in °C
  MQTTPublishMsg = "/home/"+ String(RoomID) +"/" + String(DeviceIDTemp) +"/Mesure";
  client.publish(MQTTPublishMsg,strTemp);

  //publish VBAT in mV
  MQTTPublishMsg = "/home/"+ String(RoomID) +"/" + String(DeviceIDTemp) +"/VBat";
  sprintf (strTemp,"%d",VBAT);
  #ifdef DEBUG
    Serial.print(" VBAT =");
    Serial.print(strTemp);
    Serial.print("\n\r");
  #endif
  client.publish(MQTTPublishMsg,strTemp);
  
  }
else
  {
    //nothing
  }
return;
}


/*
 * The main LOOP of programme
 * 
 */
void loop() {
 
  byte i;
  String bestSSID;
  long bestRSSI = -999;//very low RSSI
  unsigned int u16_oldTemperatureRAW;
  String MQTTsubscribmsg;
//read the sensor
  digitalWrite(SW_SENSOR,HIGH);  //switch ON the sensor
  delay(100); //wait to be sure the power is enought to start measurement
  ReadSensor_DS18B20();
  MeasureVBAT();
  digitalWrite(SW_SENSOR,LOW);  //switch ON the sensor
//read memory
ESP.rtcUserMemoryRead(0, &u16_oldTemperatureRAW, sizeof(u16_oldTemperatureRAW));
#ifdef DEBUG
  Serial.print(" TempRaw=");
  Serial.print(TemperatureRAW);
  Serial.print("\n\r");
  Serial.print(" Old TempRaw=");
  Serial.print(u16_oldTemperatureRAW);
  Serial.print("\n\r");

#endif

if( (TemperatureRAW >= (u16_oldTemperatureRAW + 4)) || (TemperatureRAW <= (u16_oldTemperatureRAW -4)))
  {
  WiFi.forceSleepWake();  
  WiFi.mode(WIFI_STA);
  ESP.rtcUserMemoryWrite(0, &TemperatureRAW, sizeof(TemperatureRAW));  
     //Wifi connection
    if (WiFi.status() != WL_CONNECTED) {
  
      int n = WiFi.scanNetworks();
    #ifdef DEBUG
      Serial.println("scan done ");
      Serial.print(n);
      Serial.println(" network found");
    #endif
      for (int ij = 0; ij < n; ++ij)
      {
        // USe the best SSID found for connexion
        if (WiFi.RSSI(ij) > bestRSSI)
          {
            bestSSID = WiFi.SSID(ij);
            bestRSSI = WiFi.RSSI(ij);
          }
        delay(10);
      }
    #ifdef DEBUG
      Serial.print("Trying connect to ");
    #endif
      bestSSID.toCharArray(ssid,bestSSID.length()+1);
  
    #ifdef DEBUG
      Serial.print(ssid);
      Serial.print(" - RSSI value = ");
      Serial.print(bestRSSI);
      Serial.println("...");
    #endif
      WiFi.begin(ssid, pass);
      i = 0;
      while(WiFi.status() != WL_CONNECTED){
        delay (1000);
        i++;
        if(i>20)//more thant 20sec that we aiting for connection on network
          {
          #ifdef DEBUG
            Serial.printf("Unable to connecte server, trying next wake-up");
          #endif
          WiFi.mode(WIFI_OFF);  
          ESP.deepSleep( DEEP_SLEEP, WAKE_RF_DEFAULT);
          }
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
   
      if (!client.connected()) {
        if (client.connect("ESP8266_Sensor")) {
            #ifdef DEBUG
              Serial.println("Connected to MQTT server");
            #endif
            PublishSensor();
            }//end of if (client.connect("ESP8266_Sensor"))
            else Serial.print("Could not connect to MQTT Server");
         }//end of if (!client.connected())
  /*25-05:     else{
          mainTimer.update();
          client.loop();
          } //end of if (client.connected())*/
    }//end if (WiFi.status() == WL_CONNECTED)
  WiFi.disconnect();  
  WiFi.mode(WIFI_OFF);  
  ESP.deepSleep( DEEP_SLEEP, WAKE_RF_DEFAULT);
  }//end of if( (TemperatureRAW > (u16_oldTemperatureRAW + 4)) || (TemperatureRAW < (u16_oldTemperatureRAW -4)))
 else // there no  difference between oldvalue en new value then go to sleep to save battery
   {
   WiFi.mode(WIFI_OFF);  
   ESP.deepSleep( DEEP_SLEEP, WAKE_RF_DEFAULT);
   }
}

