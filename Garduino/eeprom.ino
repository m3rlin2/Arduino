#include <EEPROM.h>

int getAktAddr(){
  return EEPROM.read(0)+EEPROM.read(1)+EEPROM.read(2);  
}

byte EEPROM_[100]={0};

void incAktAddr(){
  
  int aktAddr = getAktAddr();
  aktAddr+=1;
  if(aktAddr>765){aktAddr=3;};
  byte b0=constrain(aktAddr, 3, 255);
  byte b1=constrain(aktAddr-b0, 0, 255);
  byte b2=constrain(aktAddr-b0-b1, 0, 255);
  EEPROM.write(0,b0);
  EEPROM.write(1,b1);
  EEPROM.write(2,b2);
}

void writeEEPROM(byte aValue){
  int aktAddr=getAktAddr();
  byte aktVal = EEPROM.read(aktAddr);
  //Nur schreiben, wenn sich der Wert wirklich ändert
  if (aktVal!=aValue){
    EEPROM.write(aktAddr, aValue);
  };
  incAktAddr();  
}

void printEEPROM(){
  for (int i=0; i <= 765; i++){
    int val = EEPROM.read(i);
    if (val > 0){
      Serial.print(i);
      Serial.print("\t");
      Serial.print(val);
      Serial.println();   
      Serial.flush();
    }
  } 
}

void wipeEEPROM(){
  //Speicherzellen zurücksetzen
  for (int i=1; i < 766; i++){
    EEPROM.write(i,0);
  }  
  EEPROM.write(0,3);
  
  Serial.println("EEPROM wiped");
}
