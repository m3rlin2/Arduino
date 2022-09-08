#include <OneWire.h>
#include <Wire.h> 
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Metro.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 6
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
#define TEMPERATURE_PRECISION 12

LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x3f for a 16 chars and 2 line display

Metro tmr1 = Metro(1000);
Metro tmr2 = Metro(16000);

//Custom Chars
uint8_t qa[8] = {  0x0,0x0,0x0,0x0,0x0,0x0,0xff};
uint8_t qs[8] = {  0x0,0x0,0x0,0x0,0x0,0xff,0xff};
uint8_t qd[8] = {  0x0,0x0,0x0,0x0,0xff,0xff,0xff};
uint8_t qf[8] = {  0x0,0x0,0x0,0xff,0xff,0xff,0xff};
uint8_t qg[8] = {  0x0,0x0,0xff,0xff,0xff,0xff,0xff};
uint8_t qh[8] = {  0x0,0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t qj[8] = {  0xff,0xff,0xff,0xff,0xff,0xff,0xff};

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  
  lcd.init();    // initialize the lcd 
  lcd.backlight();
  //Create Custom Chars
  lcd.createChar(0, qa);
  lcd.createChar(1, qs);
  lcd.createChar(2, qd);
  lcd.createChar(3, qf);
  lcd.createChar(4, qg);
  lcd.createChar(5, qh);
  lcd.createChar(6, qj);
  
  lcd.home();
  lcd.print("Temperatur: ");
  lcd.setCursor(0,1);
  
  //Test
  // lcd.printByte(0);
  // lcd.printByte(1);
  // lcd.printByte(2);
  // lcd.printByte(3);
  // lcd.printByte(4);
  // lcd.printByte(5);
  // lcd.printByte(6);
  // lcd.printByte(0);
  // lcd.printByte(1);
  // lcd.printByte(2);
  // lcd.printByte(3);
  
  // Start up the TempSensor
  sensors.begin();
  sensors.getDeviceCount();
  sensors.isParasitePowerMode();
  while (!sensors.getAddress(insideThermometer, 0)) {
    Serial.println(F("Unable to find address for Device 0"));
	lcd.clear();
	lcd.home();
	lcd.print("Thermometer nicht gefunden !!!");
	lcd.autoscroll();
	delay(3000);
  }
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  
  //Init TempArray
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(insideThermometer);
  InitTemp(tempC);
  calcGraph();
}

float oldTemp, oldTempDHT;

const int _maxIdx = 144;
float Temps[_maxIdx] = {0.0};
int index = 0;

const byte _bars = 11;
const int blocksize = (int)(_maxIdx / _bars);
byte graph[_bars+1];

float tempC;

void loop(void)
{ 
  serialEvent(); //Answer Commands
  
  //Every 1000ms
  if (tmr1.check()) { 
    //read Sensor
    sensors.requestTemperatures();
    tempC = sensors.getTempC(insideThermometer);
    
    //Print to LCD
    if(tempC!=oldTemp){
      lcd.setCursor(0,1);
      lcd.print(tempC,1);
      oldTemp=tempC;
    } 
	
	LogStatus(); //Log to Serial
  }
  
  if (tmr2.check()) { 
	addTemp(tempC); //AddTemp to Array
	
	//When a Block is full, calc Graph
	if (index>0 && index%blocksize==0) {
	  calcGraph(); 
	}
  }
}

void LogStatus(){
  Serial.println(' ');
  Serial.print(F("Temp: "));Serial.print(getLastTemp()); Serial.print(';'); 
  Serial.print(F("Index: "));Serial.print(index); Serial.print(';'); 
  //Serial.print(F("FreeRam: "));Serial.print(freeRam()); Serial.print(';');
  Serial.println(' ');
}

float getLastTemp(){
  return Temps[index];
}
void addTemp(float aTemp){
  index++;
  if (index >= _maxIdx){ index=0;};
  Temps[index] = aTemp;
}
float getTemp(int aidx){
  if (aidx > 0 && aidx<_maxIdx)
	return Temps[aidx];
}
float maxTemp(){
  float max = -255.0;
  
  for (int i =0; i<_maxIdx; i++)
    if (Temps[i] > max) max=Temps[i];
  
  return max;
}
float minTemp(){
  float min = 255.0;
  
  for (int i =0; i<_maxIdx; i++)
    if (Temps[i] < min) min=Temps[i];
  
  return min;
}
void InitTemp(float aTemp){
  for (int i=0; i<_maxIdx; i++) Temps[i] = aTemp;
}
void PrintTemps(){ 
  Serial.print("Temps: ");
  for (int i =0; i<_maxIdx; i++){
    Serial.print("[");Serial.print(i);Serial.print("]");Serial.print(Temps[i]);
  }
  Serial.println(' ');
  Serial.flush();
}

void calcGraph(){
  //Init Array
  for (int i=0; i<_bars+1; i++) graph[i] = 0;
  
  Serial.println(' ');
  Serial.println("CalcGraph ");
  // PrintTemps();
  
  int idx = 0;
  float _minT = minTemp();
  float _maxT = maxTemp();
  float diff = (_maxT - _minT);
  if(diff<0) diff=diff*-1.0;
  float korrektur=((1.0-diff)/2.0);
  if (abs(diff)<1) {_minT = _minT -korrektur; _maxT = _maxT +korrektur;} 
  
  // Serial.print(" / "); 
  Serial.print("Blocksize: ");Serial.print(_maxIdx);Serial.print(" / ");Serial.print(_bars);Serial.print(" = ");Serial.println(blocksize); 
  Serial.print("MinTemp: ");Serial.print(_minT);Serial.print(" (");Serial.print(minTemp());Serial.print(")");
  Serial.print(" MaxTemp: ");Serial.print(_maxT);Serial.print(" (");Serial.print(maxTemp());Serial.println(")");
  Serial.print("Diff: ");Serial.print(diff);Serial.print(" Korrektur: ");Serial.println(korrektur);
  
  //Temps sortieren
  float TempSort[_maxIdx];
  int start = index +1;
  if (start >= _maxIdx) start=0;
  int counter = 0;
  for (int i=start;i<_maxIdx;i++){
	TempSort[counter]=Temps[i];
	counter++;
  }
  if (counter<_maxIdx){
    for (int i=0;i<start;i++){
	  TempSort[counter]=Temps[i];
	  counter++;
    }
  }
  
  float blockval = 0.0;
  for (int i=0; i<_maxIdx; i++){
	if (i>0 && i%blocksize==0){
	  blockval = blockval/blocksize; //durchschnitt
	  //map(value, fromLow, fromHigh, toLow, toHigh)
	  graph[idx] = map((long)(blockval*100.0), (long)(_minT*100.0), (long)(_maxT*100.0), 0, 6);
	  Serial.print("(i");Serial.print(i);Serial.print(")");Serial.print("durchschnittTemp Block ");Serial.print(idx);Serial.print(": ");Serial.print(blockval);
	  Serial.print(" : ");Serial.println(graph[idx]);
      idx++;
	  blockval = 0.0;
	}
    blockval = blockval + TempSort[i]; 
  }
  
  lcd.setCursor(5,1);
  for (int i=0; i<_bars; i++) lcd.printByte(graph[i]); 
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 

    if (inChar == '?') {
      Serial.println("Jepp!");
    } 
    
  }
}

