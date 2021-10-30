#include <Arduino.h>
#define INTERRUP_PIN 2
#define INPUT3 7
#define INPUT4 6
#define KP 7
#define KD 2000
#define KI 2
#define PERIODO 5
int period;
char direccion;
int speet = 500;
int pid;
int period_previous = 0;
boolean direccion_motor = 1;
unsigned long time = 0;
char terminal(){
  String data;
  static int velocidad;
  char caracter = NULL;
  if(Serial.available()){
    while(Serial.available() > 0){
      caracter = (char)Serial.read();
    }
  }
    return caracter; 
}
  
void encoder(){
  unsigned long time = millis();
  static unsigned long time_previous;
  period = time - time_previous;
  time_previous = time;
}
int setpoint(char direccion, int setpoin_actual){
  switch (direccion){
  case 'w':
  if(setpoin_actual < 1000){
    setpoin_actual = setpoin_actual + 20;
  }
  break;
  case 's':
  if(setpoin_actual > 0){
    setpoin_actual = setpoin_actual - 20;
  }
  break;
  
  default:
    break;
  }
  return setpoin_actual;
}
int PID(int setpoint, int current_value){
  float error = setpoint - 1000 / current_value;
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
    if(PID_total > 1000){
      PID_total = 1000;
    }
  }

  return map(PID_total, 0, 1000, 0, 255);
}
void motor(int pwm, boolean direccion){
  if(direccion){
    analogWrite(INPUT3, pwm);
    analogWrite(INPUT4, 0);
  }
  else{
    analogWrite(INPUT4, pwm);
    analogWrite(INPUT3, 0);
  }
}
void setup() {
  Serial.begin(9600);
  pinMode(INPUT3, OUTPUT);
  pinMode(INPUT4, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(INTERRUP_PIN), encoder, RISING);
}

void loop() {
  direccion = terminal();
  speet = setpoint(direccion, speet);
  pid = PID(speet, period);
  motor(pid, direccion_motor);
  
  /*analogWrite(INPUT3, 220);
  analogWrite(INPUT4, 0);*/
  if(period != period_previous){
    Serial.println(1000/period);
    period_previous = period;
  }
  /*speet = setpoint(terminal(), speet);
  analogWrite(INPUT3, map(speet, 0, 1000, 0, 255));
  digitalWrite(INPUT4, 0);*/
  if( 1000 < millis() - time){
    time = millis();
    Serial.print("speet");
    Serial.println(speet);
  }

}