#include "WProgram.h"
void setup();
void loop();
void forw();
void bacw();
void set(int pin, boolean wert);
void stop();
void blink();
int i = 0;
int m1_a = 2;
int m1_b = 3;
int m1_e =  10;
int m2_e = 9;
int m2_a =  8;
int m2_b =  7;

int timebuff = 0;
int x=0;
int y=0;

void setup() {
  for(i=2;i<13;i++)
  {
    pinMode(i,OUTPUT);
    digitalWrite(i,LOW);
    }
    Serial.begin(9600);
    
    pinMode(4, INPUT);
    pinMode(5, INPUT);
}

void loop() {
  
  analogWrite(m1_e,120);
  analogWrite(m2_e,120); 
  forw();
  delay(3000);
  stop();
  bacw();
  delay(3000);
  stop();
  
  //blink();
  //while(1){}

}

 void forw(){
  set(m1_a, LOW);
  set(m1_b, HIGH);
  set(m2_a, LOW);
  set(m2_b, HIGH);
}

void bacw(){
  
  set(m1_a, HIGH);
  set(m1_b, LOW);
  set(m2_a, HIGH);
  set(m2_b, LOW);
}

/*void m1(boolean richtung, byte s)
{
 //STOP
  set(m1_a, LOW);
  set(m1_b, LOW);
  delay(250);
  
  //Forw\u00e4rts
  if(richtung)
  {
    set(m1_a, LOW);
    set(m1_b, HIGH);
    }
    else{
      //R\u00fcckw\u00e4rts
     set(m1_a, HIGH);
     set(m1_b, LOW);
     }
    
   //Speed
   analogWrite(m1_e,s); 
 }*/

/*void m2(boolean richtung, byte s)
{
 //STOP
  set(m2_a, LOW);
  set(m2_b, LOW);
  delay(250);
  
  //Forw\u00e4rts
  if(richtung)
  {
      set(m2_a, LOW);
      set(m2_b, HIGH);
    }
    else{
      //R\u00fcckw\u00e4rts
      set(m2_a, HIGH);;
      set(m2_b, LOW);
     }
    
   //Speed
   analogWrite(m1_e,s); 
 }*/
 
 void set(int pin, boolean wert) {
  digitalWrite(pin, wert);
  }
  
  void stop(){
  set(m1_a, LOW);
  set(m1_b, LOW);
  set(m2_a, LOW);
  set(m2_b, LOW);
    
  delay(300);
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
 

int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

