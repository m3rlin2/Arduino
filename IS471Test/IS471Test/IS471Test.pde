int i = 0;
int m1_a = 2;

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
    
    pinMode(8, INPUT);
    pinMode(3, INPUT);
    blink();
}

void loop() {
  set(9, get(8));
  set(4, get(3));

}

// void forw(){
//  set(m1_a, HIGH);
//  set(m1_b, LOW);
//  set(m2_a, HIGH);
//  set(m2_b, LOW);
//}
//
//void bacw(){
//  set(m1_a, LOW);
//  set(m1_b, HIGH);
//  set(m2_a, LOW);
//  set(m2_b, HIGH);  
//}
//
//void left()
//{
//  set(m1_a, LOW);
//  set(m1_b, HIGH);  
//  set(m2_a, HIGH);
//  set(m2_b, LOW);
//}
//
//void right()
//{
//  set(m2_a, LOW);
//  set(m2_b, HIGH);
//  set(m1_a, HIGH);
//  set(m1_b, LOW);
//}
  
/*void m1(boolean richtung, byte s)
{
 //STOP
  set(m1_a, LOW);
  set(m1_b, LOW);
  delay(250);
  
  //Forwärts
  if(richtung)
  {
    set(m1_a, LOW);
    set(m1_b, HIGH);
    }
    else{
      //Rückwärts
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
  
  //Forwärts
  if(richtung)
  {
      set(m2_a, LOW);
      set(m2_b, HIGH);
    }
    else{
      //Rückwärts
      set(m2_a, HIGH);;
      set(m2_b, LOW);
     }
    
   //Speed
   analogWrite(m1_e,s); 
 }*/
 
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
 

