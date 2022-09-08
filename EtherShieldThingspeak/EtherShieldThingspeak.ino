
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
byte myIP[] = { 192, 168, 11, 201 };

IPAddress thingSpeakAddress(52, 200, 157, 52);
String writeAPIKey = "FDT9A06N166H484R";
const int updateThingSpeakInterval = 16 * 1000;      // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

EthernetClient client;

// Variable Setup
long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  randomSeed(analogRead(0));

  Ethernet.begin(mac, myIP);
  delay(1000); //let Ethernet Initialize
}


unsigned long lastSend = 0;

void loop() {

  while(client.available())
  {
    char c = client.read();
    Serial.print(c);
  }

  if ((millis() - lastSend) > updateThingSpeakInterval) {
    sendData();
    lastSend = millis();
  }

}

void sendData() {
  client.stop();
  //read dht temp+hum and convert to String
  float t = random(200) * 0.51; //dht.readTemperature();

  char t_c[6];
  dtostrf(t, 1, 2, t_c);
  String t_s = String(t_c);

  Serial.println("connecting...");

  String tsData = "field1=" + t_s;
  if (client.connect(thingSpeakAddress, 80))
  {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");

    client.print(tsData);
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

