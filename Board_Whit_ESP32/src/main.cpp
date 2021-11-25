#include <Arduino.h>
#define RXD2 2
#define TXD2 4
#define BIN1 25
#define BIN2 33
#define AIN1 26
#define AIN2 27
#define A2 36
#define A1 23
class motor{
  private:
    int channel1, channel2;
    int vel;
    int kp, ki, kd;
    int periodo = 1;
    float current_val;
    boolean invertir = true;
    int PID(int setpoint, float current_value){
      float error = setpoint - current_value;
      static float error_previous, PID_i;
      float PID_p, PID_d, PID_total;
      static unsigned long time;
      if(millis() > time + periodo){
        PID_p = kp * error;
        PID_d = kd * ((error - error_previous)/periodo);
        if(error < 100 && error > -100){
          PID_i += ki * error;
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
        time = millis();
      }

      return map(PID_total, 0, 1000, 0, 1023);
    }
  public:
    motor(int canal1, int canal2, int p_int1, int p_int2){
      channel1 = canal1;
      channel2 = canal2;
      ledcSetup(channel1, 40000, 10);
      ledcSetup(channel2, 40000, 10);
      ledcAttachPin(p_int1, channel1);
      ledcAttachPin(p_int2, channel2);
    }
    void SetValue(int valor){
      current_val = valor;
    }
    void SetPeriodo(int per){
      periodo = per;
    }
    int Avanzar(int set){
      vel = PID(set, current_val);
      if(invertir){
        ledcWrite(channel1, vel);
        ledcWrite(channel2, 0);
      }
      else{
        ledcWrite(channel1, 0);
        ledcWrite(channel2, vel);
      }
    }
    int Retroceder(int set){
      vel = PID(set, current_val);
      if(invertir){
        ledcWrite(channel1, 0);
        ledcWrite(channel2, vel);
      }
      else{
        ledcWrite(channel1, vel);
        ledcWrite(channel2, 0);
      }
    }
    void Invertir(){
      invertir != invertir;
    }
    void SetPID(int set_kp, int set_kd, int set_ki){
      kp = set_kp;
      kd = set_kd;
      ki = set_ki;
    }
};
motor m1 = motor(1, 2, AIN1, AIN2);
motor m2 = motor(3, 4, BIN1, BIN2);
void IRAM_ATTR M1() {
  static unsigned long time;
  m1.SetValue(1000/(millis() - time));
  time = millis();
}
void IRAM_ATTR M2() {
  static unsigned long time;
  m2.SetValue(1000/(millis() - time));
  time = millis();
}

void setup() {
  Serial.begin(115200);
  //Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  attachInterrupt(A2, M1, FALLING);
  attachInterrupt(A1, M2, FALLING);
  m1.SetPID(20, 0, 0);
  m2.SetPID(20, 0, 0);
}

void loop() {
  m1.Retroceder(600);
  m2.Retroceder(600);

}