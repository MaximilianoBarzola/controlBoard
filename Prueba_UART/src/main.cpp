#include <Arduino.h>
String send = "";
boolean ready = 0;
void setup() {
  Serial.begin(9600);
}

void loop() {
  if(Serial.available()){
    char x;
    ready = 1;
    while(Serial.available() > 0){
      x = (char)Serial.read();
      send += String(x);
    }
  }
  if(ready){
    Serial.println(String(send));
    ready = 0;
    send = "";
  }
}