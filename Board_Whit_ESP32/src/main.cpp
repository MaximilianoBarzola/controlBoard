#include <Arduino.h>
#define RXD2 2
#define TXD2 4
class motor{
  private:
    int channel1, channel2;
    int vel;
    int kp, ki, kd;
    int periodo = 1;
    int current_val;
    boolean invertir = true;

  public:
    motor(int canal1, int canal2, int p_int1, int p_int2){
      channel1 = canal1;
      channel2 = canal2;
      ledcSetup(channel1, 40000, 10);
      ledcSetup(channel2, 40000, 10);
      ledcAttachPin(p_int1, channel1);
      ledcAttachPin(p_int2, channel2);
    }
    int PID(int setpoint, int current_value){
      float error = setpoint - current_value;
      static float error_previous, PID_i;
      float PID_p, PID_d, PID_total;
      static unsigned long time;
      if(millis() > time + periodo){
        PID_p = kp * error;
        PID_d = kd * ((error - error_previous)/periodo);
        if(error < 20 && error > -20){
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
    int Value(int valor){
      current_val = valor;
    }
    int Periodo(int per){
      periodo = per;
    }
    int avanzar(int set){
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
    int retroceder(int set){
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
    void invertir(){
      invertir = false;
    }
};


void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

}

void loop() {

}