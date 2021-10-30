#include <Arduino.h>
#include "RB_QMIND_PLUS.h"
#include "RB_QMIND.h"

#define CARACTER_COMUNICACION '!'

RB_EncoderMotor  M1(1);
RB_EncoderMotor  M2(2);
RB_RGBLed RGBLED(2,2);
String x = "";
String y = "";
int m1=0;
int m2=0;


void setup() {
  Serial.begin(115200);


  M1.SetMotionMode(0);
  M2.SetMotionMode(0);
  pinMode(20,OUTPUT);
  digitalWrite(20,HIGH);
  delay(500);

  delay(int(1 * 1000));
  M1.SetMotionMode(0);
  M2.SetMotionMode(0);
  pinMode(20,OUTPUT);
  digitalWrite(20,HIGH);


}

 void Mover(int m1=0, int m2=0){
   M1.SetMotorPwm(m1);
   M2.SetMotorPwm(m2);
   }
  

void loop() {
  if(Serial.available() > 0){
  x = Serial.readStringUntil(CARACTER_COMUNICACION);
  y = Serial.readStringUntil(CARACTER_COMUNICACION);
  }
  m1 = x.toInt();
  m2 = y.toInt();
  
  Mover(m1, m2);
}
