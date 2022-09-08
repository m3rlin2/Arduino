#include <ESP8266WiFi.h>       //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <WiFiClientSecure.h>

/////////  Config   ///////////////////
const char* ts_api_key = "6S3SX1K0OV13XNHI";  // here you write your thinspeak API KEY (write API KEY)
#define SLEEP_TIME_SECONDS 20      //900 for 15 minutes, but sometimes it depend on internal resonator


/*////////  mapping   ///////////////// 
#define PIN_WIRE_SDA (4)
#define PIN_WIRE_SCL (5)

static const uint8_t LED_BUILTIN = 16;
const uint8_t BUILTIN_LED = 16;

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10 = 1;  */

void setup() {
  //blinkLED(3);
      digitalWrite(BUILTIN_LED, HIGH);
    delay(300);
    digitalWrite(BUILTIN_LED, LOW);
    delay(300);
  Serial.begin(115200);
  Serial.println("Booting");

  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  //wifiManager.setTimeout(30);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("ConfigESP", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
 
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  /*Serial.println("MyGetTimey");
  MyGetTime();*/
  Serial.println("ThingSpeakSend");
  ThingSpeakSend();
  Serial.println(' ');

  //ESP.deepSleep(SLEEP_TIME_SECONDS*1000);
}

void ThingSpeakSend() {
  const char* host = "api.thingspeak.com";
  WiFiClientSecure client;
  
  if (client.connect(host, 443))
  {
    if (client.verify("78 60 18 44 81 35 BF DF 77 84 D4 0A 22 0D 9B 4E 6C DC 57 2C", host)) {
      Serial.println("certificate matches");
    } else {
      Serial.println("certificate doesn't match");
    }

    String url = "/update?api_key=";
    url += ts_api_key;
    url += "&field1=";
    url += 1;
    url += "&field2=";
    url += 2;
    url += "&field3=";
    url += 3;
    url += "&field4=";
    url += 4; //(float)analogRead(A0);
    Serial.print("requesting URL: ");
    Serial.println(url);
  
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: ESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    /*String dataToThingSpeak = "";
    dataToThingSpeak += "GET /update?api_key=";
    dataToThingSpeak += ts_api_key;

    dataToThingSpeak += "&field1=";
    dataToThingSpeak += 0;         // writing to field 1 temperature from first sensor

    dataToThingSpeak += "&field2=";
    dataToThingSpeak += 1;        // writing to field 2 temperature from second sensor

    dataToThingSpeak += "&field3=";
    dataToThingSpeak += 2;       // write to field 3 difference of both sensors

    dataToThingSpeak += "&field4=";
    dataToThingSpeak += 3;        // writing to field 4 temperature from second sensor

    dataToThingSpeak += " HTTP/1.1\r\nHost: a.c.d\r\nConnection: close\r\n\r\n";
    dataToThingSpeak += "";*/

    delay(500);
    while (client.available()) {
      Serial.write((char)client.read());
    }
  }
  client.stop();
}

void MyGetTime() {
  String TimeDate;
  byte hours, minutes, seconds;

  WiFiClient client;
  while (!!!client.connect("google.com", 80)) {
    Serial.println("connection failed, retrying...");
  }

  client.print(String("HTTP/1.1\r\n") +
               "Host: google.com\r\n" + 
               "Connection: close\r\n\r\n");

  delay(500); //wait a few milisseconds for incoming message

  //if there is an incoming message
  if (client.available()) {
    while (client.available()) {
      Serial.write((char)client.peek());
      if (client.read() == '\n') {
        if (client.read() == 'D') {
          if (client.read() == 'a') {
            if (client.read() == 't') {
              if (client.read() == 'e') {
                if (client.read() == ':') {
                  client.read();
                  String theDate = client.readStringUntil('\r');
                  client.stop();

                  TimeDate = theDate.substring(7);
                  Serial.println(TimeDate);
                  // time starts at pos 14
                  String strCurrentHour = theDate.substring(17, 19);
                  String strCurrentMinute = theDate.substring(20, 22);
                  String strCurrentSecond = theDate.substring(23, 25);
                  Serial.println(strCurrentHour);
                  Serial.println(strCurrentMinute);
                  Serial.println(strCurrentSecond);
                  hours = strCurrentHour.toInt();
                  minutes = strCurrentMinute.toInt();
                  seconds = strCurrentSecond.toInt();
                }
              }
            }
          }
        }
      }
    }
  }
  //if no message was received (an issue with the Wi-Fi connection, for instance)
  else {
    seconds = 0;
    minutes += 1;
    if (minutes > 59) {
      minutes = 0;
      hours += 1;
      if (hours > 11) {
        hours = 0;
      }
    }
  }
  client.stop();
}

void blinkLED(int acount) {
  pinMode(BUILTIN_LED, OUTPUT);
  for (int i=0;i++;i<acount){
    digitalWrite(BUILTIN_LED, HIGH);
    delay(300);
    digitalWrite(BUILTIN_LED, LOW);
    delay(300);
  }
}

