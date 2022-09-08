
#include <Metro.h> 
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <Button.h>

const byte ingasgriff = 7; //Analog
const byte inBatterie = 0; //Analog
const byte inAutoCruseButton = 7; //digital
const bool cplott = false;

const float faktorVinVBat = 9.55;  //Volt an Analog zu VoltBatterie
const int gasThreshold = 175; //erst ab diesem Wert wird auf Gas reagiert
float minVolt = 34.0;
byte autoCruseOut = 0;

//Timer
Metro tmr_UpdateOutput(110); //150
Metro tmr_ReadSensorsValues(10); //10
//DAC
Adafruit_MCP4725 dac;
//Button
Button BAutoCruse(inAutoCruseButton);

float aktIn = 0.0;
float aktOut = 0.0;
float aktBatRead = 0.0;
float aktVolt = 0.0;
float moderator = 1.0;

void setup()  ///// Setup //////////////////////////////////////
{
  Serial.begin(9600);

  initPins();

  BAutoCruse.begin();

  aktIn = analogRead(ingasgriff);

  aktBatRead = analogRead(inBatterie);
  aktVolt = aktBatRead * 0.00488 * faktorVinVBat;

  dac.begin(0x60);

} /////////////////////////////////////////////////////////////

char incoming;
String inBuffer;
byte SollOut = 0;

void loop()
{
  //Read and buffer Serial kommunikation
  if (Serial.available() > 0) {
    incoming = Serial.read();
    if (incoming == ';') {
      handle_command();
      inBuffer = "";
    } else {
      inBuffer += incoming;
    }
  }

  if (tmr_ReadSensorsValues.check()) //Timer
  {
    ReadSensorValues();
  }


  if (tmr_UpdateOutput.check())
  {
    //BatterieSpannung berechnen
    aktVolt = aktBatRead * 0.00488 * faktorVinVBat;
    PrintVal("aktVolt", aktVolt);

    //OutValue anhand GasGriff oder AutoCruse
    SollOut = 0;
    if (autoCruseOut > 0)
    {
      SollOut = autoCruseOut;
      moderator = 0.02;
      PrintVal("AutoCruse", 1);
    }
    else
    {
      SollOut = calcOutputVal( (int)aktIn );
      PrintVal("AutoCruse", 0);
    }
    PrintVal("SollOut", SollOut);

    float SollDiff = (float)SollOut - aktOut;
    PrintVal("SollDiff", SollDiff);

    float tmpdiff = SollDiff;
    float minVoltDiff = aktVolt - minVolt;

    int debugInfo = 0;
    const float modVal = 0.0045; //Änderungsrate begrenzen

    //Diff verringern wegen undervoltage
    if (minVoltDiff < 0.00) {

      float _faktor = constrain(minVoltDiff, -1.50, -0.005);
      _faktor = abs(_faktor);
      _faktor /= 1.50;
      _faktor = -0.20 * _faktor;
      tmpdiff = aktOut * _faktor;

      //Undervolatge hat zugeschlagen, also moderator setzen
      moderator = min(modVal, moderator);

      if (tmpdiff < SollDiff) {
        SollDiff = tmpdiff;
        debugInfo = _faktor * 100.0;
      }
    }
    else // minVoltDiff > 0.0
    {
      if (SollDiff > 0.0) {
        //Diff verringern wegen Nähe zu minVolt
        float tmpFaktor = constrain(minVoltDiff, 0.005, 3.0);
        tmpFaktor /= 3.0;
        tmpdiff = SollDiff * (0.4 * tmpFaktor);

        if (tmpdiff < SollDiff) {
          SollDiff = tmpdiff;
          debugInfo = 100 + tmpFaktor * 10;
        }

        //Maximale Differenz
        float maxDiff = 50 * tmpFaktor + 1;
        if (maxDiff < SollDiff) {
          SollDiff = maxDiff;
          debugInfo = 200 + tmpFaktor * 10;
        }

        //moderator benutzen...
        SollDiff *= moderator;
        //und ausklingen lassen
        if ( moderator < 0.95 ) //modVal ist gesetzt
          moderator += ( (1.0 - modVal) / 80.0); //nach x ticks ist der Moderator wieder normal
        else
          moderator = 1.0;
      }
    }

    PrintVal("moderator", moderator);

    PrintVal("debugInfo", debugInfo);

    PrintVal("SollDiff2", SollDiff);

    //neuen Output schreiben
    if ( (aktIn < gasThreshold) && (autoCruseOut == 0) )
    {
      aktOut = 0.0;
      moderator = 1.0;
    }
    else
    {
      aktOut += SollDiff;
    }

    //aktOut an Endstufe schreiben
    int dacOut = map(aktOut, 0, 254, 0, 4095);
    dac.setVoltage(dacOut, false);

    PrintVal("aktOut", aktOut);

    Serial.println(" ");
  } //Timer tmr_UpdateOutput

  //AutoCruse Button
  if (BAutoCruse.toggled()) {
    if (BAutoCruse.read() == Button::PRESSED)
      autoCruseOut = (byte)SollOut; //(byte)aktOut;
    else
      autoCruseOut = 0;
  }
} //loop


void handle_command() {
  //Command bsp. "minVolt 33.5;"
  String command = inBuffer.substring(0, inBuffer.indexOf(' '));
  String parameters = inBuffer.substring(inBuffer.indexOf(' ') + 1);

  if (command.equalsIgnoreCase("minVolt")) {
    // parse the rest of the information
    float _volt = parameters.substring(0, parameters.indexOf(' ')).toFloat();

    if (inRange(_volt, 30.0, 40.0)) {
      minVolt = _volt;
      Serial.print("minVolt set to ");
      Serial.println(minVolt, 2);
    }
    else
      Serial.println("command minVolt error");
  }
  else if (command.equalsIgnoreCase("status")) {
    Serial.print("aktIn "); Serial.println(aktIn, 1);
    Serial.print("aktOut "); Serial.println(aktOut, 1);
    Serial.print("minVolt "); Serial.println(minVolt, 1);
    Serial.print("AktVolt "); Serial.println(aktVolt, 1);
    Serial.print("Moderator "); Serial.println(moderator, 2);
    Serial.print("autoCruse ");
    if (autoCruseOut > 1.0) {
      Serial.println("1");
    }
    else
      Serial.println("0");

  }
  else {
    Serial.print(command);
    Serial.println("command unknown");
  }
} //handle_command


void ReadSensorValues()
{
  int tmpRead = 0;
  
  //readGasGriff
  for(int i=0;  i<10 ; i++) 
  {
    tmpRead += analogRead(ingasgriff); 
  }
  tmpRead = ((float)tmpRead) / 10.0;
  
  if (tmpRead < gasThreshold)
  {
    aktIn = 0;
  }
  else
  {
    aktIn = tmpRead;
  }
  
  //readBatterie  
  tmpRead = 0;
  for(int i=0;  i<10 ; i++) 
  {
    tmpRead += analogRead(inBatterie); 
  }
  aktBatRead = ((float)tmpRead) / 10.0;

}

void initPins() {
  for (int i = 2; i < 13; i++) {
    pinMode(i, INPUT);
    digitalWrite(i, LOW);
  }
}

void PrintVal(const char *Name, const float value) {
  if (!cplott) {
    Serial.print(Name);
    Serial.print(" ");
  }
  Serial.print(value, 2);
  Serial.print("  ");
}
void PrintVal(const char *Name, const int value) {
  if (!cplott) {
    Serial.print(Name);
    Serial.print(" ");
  }
  Serial.print(value);
  Serial.print("  ");
}

void smooth(float &aktValue, const float newVal, const float eta) {
  //Lowpass filter : average = ETA * (newvalue) + (1-ETA) * average;
  aktValue = eta * newVal + (1 - eta) * aktValue;
}

const int ingasgriffval[15] =  {176, 180, 188, 225, 266, 323, 395, 457, 522, 610, 673, 720, 725, 730, 736};
const byte outgasgriffval[15] = { 13, 27, 40, 54, 68, 81, 95, 108, 122, 136, 149, 163, 176, 190, 230};

byte calcOutputVal(const int aInval)
{
  //index suchen
  byte Idx = 0;
  int x1, x2, y1, y2 = 0;
  if (aInval > ingasgriffval[Idx]) {
    for (Idx = 0; Idx < 15; Idx++) {
      x1 = ingasgriffval[Idx];
      if (Idx < 14) {
        x2 = ingasgriffval[Idx + 1];
        y2 = outgasgriffval[Idx + 1];
      }
      else {
        x2 = 800;
        y2 = 250;
      }

      if (inRange(aInval, x1, x2)) {
        y1 = outgasgriffval[Idx];
        break;
      }
    }
  }


  //map(value, fromLow, fromHigh, toLow, toHigh)
  if (inRange(aInval, 176, 2000))  //unter 175 gibts interferrenzen
  {
    int out = map(aInval, x1, x2, y1, y2);
    return out;
  }
  else
    return 0;
}

bool inRange(const int aVal, const int aMin, const int aMax) {
  return ((aVal >= aMin) && (aVal < aMax));
}
