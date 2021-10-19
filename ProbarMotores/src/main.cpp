#include <Arduino.h>
#define INTPUT1 7
#define INTPUT2 6
#define SWIHT 11
#define INTERUP 2
#define ENABLE 1
#define KP 3.0 //3.0
#define KI 0.10 //0.1
#define KD 600.0 //600.0
#define PERIODO 10
int setpoint = 485;
int pulso = 0;
int pulso_previous = 0;
int pid_previous = 0;
int before_pulse = 0;
unsigned long time1 = 0;
unsigned long time2 = 0;
boolean direccion = 1;
int pid;
int pulso_mayor = 0;
//u7300
boolean change = 0;

void encoder(){
  if(direccion == 0){
    pulso--;
  }
  else if (direccion == 1){
    pulso++;
  }
}//2.073.600 - 
int terminal(){
  String data;
  static int numero_pulse;
  if(Serial.available()){
    while(1){
      String comando;
      char caracter = Serial.read();
      comando = (String)caracter;
      data += comando;
      if(comando == "\n")
      break;
    }
    numero_pulse = 100;
  }
  return numero_pulse;
}
int PID(int setpoint, float kp, float ki, float kd){
  float PID_p;
  static float PID_i;
  float PID_d;
  static float PID_total;
  int error = setpoint - pulso;
  static int error_privious; 
  static unsigned long time;
  if(millis() > time + PERIODO){
    //Serial.println(error);
    PID_p = kp * error;
    PID_d = kd * ((error - error_privious)/PERIODO);
    if( 20 > error && -20 < error){
      PID_i += ki * error;
      //Serial.println(error);
      //Serial.println(PID_i);
    }
    else {
      PID_i = 0;
      //Serial.println(PID_i);
    }
    PID_total = PID_d + PID_i + PID_p;
    error_privious = error;
    if(PID_total < -1000){
      PID_total = -1000;
    }
    if(PID_total > 1000){
      PID_total = 1000;
    }
    time = millis();
  }
  return map(PID_total, -1000, 1000, -255, 255);
}
void motor(int pwm){
  if(pwm < 0 && pulso != setpoint){
    analogWrite(INTPUT1, pwm * -1);
    analogWrite(INTPUT2, 0);
    direccion = 0;
  }
  else if(pwm > 0 && pulso != setpoint){
    analogWrite(INTPUT2, pwm);
    analogWrite(INTPUT1, 0);
    direccion = 1;
  }
  else if(pulso == setpoint){
    analogWrite(INTPUT1, 0);
    analogWrite(INTPUT2, 0);
  }
}
void setup() {
  Serial.begin(9600);
  pinMode(INTPUT1, OUTPUT);
  pinMode(INTPUT2, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(INTERUP), encoder, RISING);
}

void loop() {
  pid = PID(setpoint , KP, KI, KD);
  motor(pid);
  if((change == 1) && (digitalRead(SWIHT) == 1)){
    if(time1 + millis()> 15){
      change = 0;
      pulso = 0;
      time1 = millis();
    }
 
  }
  else if (digitalRead(SWIHT) == 0){
    if(time1 + millis() > 15){
      change  = 1;
      time1 = millis();
    }
  }
  if (pulso != pulso_previous){
    Serial.println(pulso);
    pulso_previous = pulso;
    time2 = millis();
  }
  if (pulso > pulso_mayor){
    pulso_mayor = pulso;
  }
  if (millis() > time2 + 5000){
    Serial.println(pulso_mayor);
  }
  if (pid != pid_previous){
    Serial.print("PID ");
    Serial.println(pid);
    pid_previous = pid;
  }
}