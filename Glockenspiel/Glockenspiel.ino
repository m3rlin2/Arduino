/*
  Name:        Glockenspiel.ino
  Created:    26.11.2018 15:37:08
  Author:    michael kotzolt

  ursprung: Stefan Thesen 08/2015 - free for anyone
  TTP 1.1 Webserver als AccessPoint for ESP8266

  Does HTTP 1.1 with defined connection closing.
  Handles empty requests in a defined manner.
  Handle requests for non - exisiting pages correctly.

  This demo allows to switch two functions :
  Function 1 creates serial output and toggels GPIO12
  Function 2 just creates serial output.

  Serial output can e.g.be used to steer an attached
  Arduino, Raspberry etc.
  --------------------------------------------------* /
*/

#include <ESP8266WiFi.h>

const char* ssid = "Glockenspiel";
const char* password = "Glockenspiel";  // set to "" for open access point w/o passwortd
int count;
unsigned long ulReqcount;


// Create an instance of the server on Port 80
WiFiServer server(80);


void setup()
{
  // setup globals
  ulReqcount = 0;
  count = 0;

  // prepare GPIO12
  pinMode(12, OUTPUT);
  digitalWrite(12, 0);

  // start serial
  Serial.begin(9600);
  delay(1);


  Serial.print("Setting soft-AP ... ");
  boolean result = WiFi.softAP("Glockenspiel", "Glockenspiel");
  if (result == true)
  {
    Serial.println("Ready");
  }
  else
  {
    Serial.println("Failed!");
  }
  //server.on("/", handleRoot);      //Which routine to handle at root location

  server.begin();                  //Start server
  Serial.println("HTTP server started");
}


void loop()
{
  /*Serial.print("Stations connected = ");
    Serial.print(WiFi.softAPgetStationNum());
    Serial.println();
  */
  //Timer zum beenden des MP3 Players
  if (count == 1) {
    digitalWrite(12, 0);
  }
  else if (count > 0) {
    count = (count - 1);
    delay(1000);
    Serial.print("Counter: ");Serial.println(count);
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (client)
  {
    // digitalWrite(LEDpin, HIGH);
    // Wait until the client sends some data
    Serial.println("new client");
    unsigned long ultimeout = millis() + 250;
    while (!client.available() && (millis() < ultimeout))
    {
      delay(1);
    }
    if (millis() > ultimeout)
    {
      Serial.println("client connection time-out!");
      return;
    }

    // Read the first line of the request
    String sRequest = client.readStringUntil('\r');
    Serial.println(sRequest);
    client.flush();

    // stop client, if request is empty
    if (sRequest == "")
    {
      Serial.println("empty request! - stopping client");
      client.stop();
      return;
    }

    // get path; end of path is either space or ?
    // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
    String sPath = "", sParam = "", sCmd = "";
    String sGetstart = "GET ";
    int iStart, iEndSpace, iEndQuest;
    iStart = sRequest.indexOf(sGetstart);
    if (iStart >= 0)
    {
      iStart += +sGetstart.length();
      iEndSpace = sRequest.indexOf(" ", iStart);
      iEndQuest = sRequest.indexOf("?", iStart);

      // are there parameters?
      if (iEndSpace > 0)
      {
        if (iEndQuest > 0)
        {
          // there are parameters
          sPath = sRequest.substring(iStart, iEndQuest);
          sParam = sRequest.substring(iEndQuest, iEndSpace);
        }
        else
        {
          // NO parameters
          sPath = sRequest.substring(iStart, iEndSpace);
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////////
    // output parameters to serial, you may connect e.g. an Arduino and react on it
    ///////////////////////////////////////////////////////////////////////////////
    if (sParam.length() > 0)
    {
      int iEqu = sParam.indexOf("=");
      if (iEqu >= 0)
      {
        sCmd = sParam.substring(iEqu + 1, sParam.length());
        Serial.println(sCmd);
      }
    }


    ///////////////////////////
    // format the html response
    ///////////////////////////
    String sResponse, sHeader;

    ////////////////////////////
    // 404 for non-matching path
    ////////////////////////////
    if (sPath != "/")
    {
      Serial.println("Fehlerhafte Anfrage!?");
      sResponse = "<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";

      sHeader = "HTTP/1.1 404 Not found\r\n";
      sHeader += "Content-Length: ";
      sHeader += sResponse.length();
      sHeader += "\r\n";
      sHeader += "Content-Type: text/html\r\n";
      sHeader += "Connection: close\r\n";
      sHeader += "\r\n";
    }
    ///////////////////////
    // format the html page
    ///////////////////////
    else
    {
      Serial.println("hier bin ich");
      ulReqcount++;
      sResponse = "<html><head><title>Glockenspiel</title></head><body>";
      sResponse += "<font color=\"#000000\"><body bgcolor=\"#d0d0f0\">";
      sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
      sResponse += "<h1>Glockenspiel starten per WEB</h1>";
      sResponse += "Button 1 schaltet das Glockenspiel ein.<BR>";
      sResponse += "Es schaltet sich selbst nach 115 Sekunden aus.<BR>";
      sResponse += "<FONT SIZE=+1>";
      sResponse += "<p>Glockenspiel <a href=\"?pin=Start\"><button>starten</button></a>&nbsp;<a href=\"?pin=Stop\"><button>anhalten</button></a></p>";
      //sResponse += "<p>Funktion 2 <a href=\"?pin=FUNCTION2ON\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=FUNCTION2OFF\"><button>ausschalten</button></a></p>";

      //////////////////////
      // react on parameters
      //////////////////////
      if (sCmd.length() > 0)
      {
        // write received command to html page
        sResponse += "Kommando:" + sCmd + "<BR>";

        // switch GPIO
        if (sCmd.indexOf("Start") >= 0)
        {
          digitalWrite(12, 1);
          //Timer 115 Sekunden zum beenden des MP3 Players setzen
          count = 115;
          Serial.print("Counter: ");Serial.println(count);
        }
        else if (sCmd.indexOf("Stop") >= 0)
        {
          digitalWrite(12, 0);
        }
      }
      Serial.println(sCmd);
      sResponse += "<FONT SIZE=-2>";
      sResponse += "<BR>Glochenspiel Starts=";
      sResponse += ulReqcount;
      sResponse += "<BR>";
      sResponse += "</body></html>";

      sHeader = "HTTP/1.1 200 OK\r\n";
      sHeader += "Content-Length: ";
      sHeader += sResponse.length();
      sHeader += "\r\n";
      sHeader += "Content-Type: text/html\r\n";
      sHeader += "Connection: close\r\n";
      sHeader += "\r\n";
    }

    // Send the response to the client
    client.print(sHeader);
    client.print(sResponse);

    delay(500);
    // und den client beenden
    client.stop();
    Serial.println("Client disonnected");
  }
  
}
