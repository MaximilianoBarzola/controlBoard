#include <Arduino.h>
#include <Wire.h>
#define KP 7
#define KD 2000
#define KI 2
#define PERIODO 5


int terminal(){
    uint8_t grados;
    static String dato;
    char caracter = NULL;
    if(Serial.available()){
        dato = "";
        while(Serial.available() > 0){
            caracter = (char)Serial.read();
            dato =+ caracter;
            if(dato.length() == 2){
              break;
            }
        }
    }
    grados = dato.toInt();
    return grados; 
}

int PID(int setpoint, int current_value){
  float error = setpoint - current_value;
  static float error_previous, PID_i;
  float PID_p, PID_d, PID_total;
  static unsigned long time;
  if(millis() > time + PERIODO){
    PID_p = KP * error;
    PID_d = KD * ((error - error_previous)/PERIODO);
    if(error < 20 && error > -20){
      PID_i += KI * error;
    }
    else{
      PID_i = 0;
    }
    PID_total = PID_p + PID_d + PID_i;
    error_previous = error;
    if(PID_total < 0){
      PID_total = 0;
    }
    if(PID_total > 180){
      PID_total = 180;
    }
  }

  return map(PID_total, 0, 180, -100, 100);
}
void motor(int speet){
    if(speet < 0){
        M1.SetMotorPwm(speet * -1);
        M2.SetMotorPwm(0);
    }
    else{
        M1.SetMotorPwm(0);
        M2.SetMotorPwm(speet);
    }
    
}
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("\nI2C Scanner");

}

void loop() {
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(5000);     
}