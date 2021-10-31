#include <Arduino.h>
#define RXD2 2
#define TXD2 4
String send[2] = {"",""};
boolean ready[2] = {0,0};

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

}

void loop() {
  if(Serial.available()){
    char x;
    ready[0] = 1;
    while(Serial.available() > 0){
      x = (char)Serial.read();
      send[0] += String(x);
    }
  }
  if(ready[0]){
    Serial2.println(String(send[0]));
    ready[0] = 0;
    send[0] = "";
  }
  if(Serial2.available()){
    char x;
    ready[1] = 1;
    while(Serial2.available() > 0){
      x = (char)Serial2.read();
      send[1] += String(x);
    }
  }
  if(ready[1]){
    Serial.println(String(send[1]));
    ready[1] = 0;
    send[1] = "";
  }
}