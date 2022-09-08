#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager


#define SLEEP_TIME_SECONDS 15      //900 for 15 minutes, but sometimes it depend on internal resonator


// Initialize the client library
WiFiClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();


  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  internet_available();

  ESP.deepSleep(SLEEP_TIME_SECONDS*60*1000, WAKE_RF_DEFAULT);
}

bool internet_available(){
  if (client.connect("google.de", 80)) {
    Serial.println("Internet here :-) ");
    return true;
  }
  else {
    Serial.println("no fucking Internet here");
    return false;
  }
}

void loop() {
  // put your main code here, to run repeatedly:


}
