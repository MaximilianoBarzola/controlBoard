#include <Arduino.h>
#include "RB_QMIND_PLUS.h"
#include "RB_QMIND.h"
#define KP 7
#define KD 2000
#define KI 2
#define PERIODO 5
int valor = 0;
int valor_motor = 0;
RB_EncoderMotor  M1(1);
RB_EncoderMotor  M2(2);

int terminal(){
    uint8_t grados;
    static String dato;
    char caracter = NULL;
    if(Serial.available()){
        dato = "";
        while(Serial.available() > 0){
            caracter = (char)Serial.read();
            dato = dato + (String)caracter;
            if(caracter == "\n"){
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
    if(PID_total <= -500){
      PID_total = -500;
    }
    if(PID_total > 500){
      PID_total = 500;
    }
  }

  return map(PID_total, -500, 500, -100, 100);
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

void loop() {
  valor = terminal();
  valor_motor = PID(0 , valor);
  motor(valor_motor);
}
