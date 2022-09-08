//********************************
//   Pinbelegung
//********************************
const int voltageFlipPin1 = 5;
const int voltageFlipPin2 = 6;
const int sensorPin = 1;
const int relaisPin = 4;
const int EimerPin = 7;
const int serial2CarriotPowerPin = 10;

//********************************
//   Einstellungen 
//********************************
const unsigned long messTime = 300000;
const int flipTimer = 1000;
const unsigned long pumpTime = 60000;
int pumpThreshhold = 135;


//interne Variablen
long milMessen=0;
long milPump=0;

unsigned long lastTimeHeartBeat=0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Welcomme to m3rlin's Garduino!\n"));
  
  //Serial1.begin(115200);
  
  pinMode(serial2CarriotPowerPin, OUTPUT);
  digitalWrite(serial2CarriotPowerPin,HIGH);
  
  pinMode(voltageFlipPin1, OUTPUT);
  pinMode(voltageFlipPin2, OUTPUT);
  digitalWrite(voltageFlipPin1,LOW);
  digitalWrite(voltageFlipPin2,LOW);
  
  pinMode(sensorPin, INPUT);
  
  pinMode(relaisPin, OUTPUT);
  digitalWrite(relaisPin,LOW);
  
  SetupTempSens();
  
  //Serial.println(F("stored Sensor Values are:"));
  //printEEPROM();

  milMessen=millis();
  lastTimeHeartBeat=millis();
}

void loop()
{  
  commands();
  if(millis()-milPump>pumpTime){
    if(get(relaisPin)){
      pump(false);
      //min einen Messzyklus aussetzen
      milMessen=millis();
    }
  }
  
  if(millis()-milMessen>messTime) {  // Zeit fürs messen
    if (GetMoisture(true) < pumpThreshhold) {
      if(analogRead(EimerPin)<100) { //Eimer leer
        writeEEPROM(111);
        pump(false);
      } 
      else
      {
        pump(true);
        writeEEPROM(222);
        milPump=millis();  
      }
    }
    milMessen=millis();
  }
  
  if(analogRead(EimerPin)<100) { //Eimer leer
    pump(false);
  }
  
  delay(100);
}

void commands(){
  if (Serial.available() > 0){
    char inByte = Serial.read();

    if (inByte=='w'){
      wipeEEPROM();
    }
    else if (inByte=='p'){
      if(get(relaisPin)){
        pump(false);
        Serial.println("pump OFF");
        Serial.flush();
      }
      else{
        pump(true);
        milPump=millis(); //damit nicht gleich wieder abgeschaltet wird
        Serial.println("pump ON");
        Serial.flush();
      }
    }
    else if (inByte=='r'){
      GetMoisture(false);
      //GetTempVal();
      Serial.flush();
    }
    else if (inByte=='x'){
      //Serial1.println("t186m125;");
      Serial.flush();
    }    
    else {
      printEEPROM();
    }

    while(Serial.available()>0){
      inByte=Serial.read();
    }
  }  
}

void pump(boolean TurnOn){
  if (TurnOn){
    set(relaisPin,HIGH);
    set(13,HIGH);
  }
  else {
    set(relaisPin,LOW);
    set(13,LOW);    
  }
  
}

void set(int pin, boolean wert) {
  digitalWrite(pin, wert);
}
boolean get(int pin) {
  return digitalRead(pin);
}

void blink(){
  pinMode(13, OUTPUT);
  set(13, HIGH);
  delay(400);
  set(13, LOW);
  delay(400);
  set(13, HIGH);
  delay(400);
  set(13, LOW);
  delay(400);
  set(13, HIGH);
  delay(400);
  set(13, LOW);
}



