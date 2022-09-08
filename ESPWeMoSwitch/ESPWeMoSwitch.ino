#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include "WemoSwitch.h"
#include "WemoManager.h"
#include "CallbackFunction.h"

// prototypes
boolean connectWifi();

//on/off callbacks
void lightOn();
void lightOff();

//for LED status
#include <Ticker.h>
Ticker ticker;

WemoManager wemoManager;
WemoSwitch *light = NULL;

void tick()
{
  //toggle state
  int state = digitalRead(13);  // get the current state of GPIO1 pin
  digitalWrite(13, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  //set led pin as output
  pinMode(13, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();
  //keep LED on
  digitalWrite(13, LOW);
  wemoManager.begin();
  // Format: Alexa invocation name, local port no, on callback, off callback
  light = new WemoSwitch("leuchte", 91, lightOn, lightOff);
  wemoManager.addDevice(*light);
}

void loop() {
  wemoManager.serverLoop();
}

void lightOn() {
    Serial.print("Switch 1 turn on ...");
    digitalWrite(12, LOW);
}

void lightOff() {
    Serial.print("Switch 1 turn off ...");
    digitalWrite(12, HIGH);
}
