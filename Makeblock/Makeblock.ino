class Motor{
  private:
    int p_pwm, p_d,vel;
    bool invertir = false;
  public:
    Motor(int pwm, int p1){
      p_pwm = pwm;
      p_d = p1;
      pinMode(p_d, OUTPUT);
      pinMode(p_pwm, OUTPUT);
    }
  void Avanzar(){
    if(invertir){
      digitalWrite(p_d, false);
      analogWrite(p_pwm, 255);  
    }
    else{
      digitalWrite(p_d, true);
      analogWrite(p_pwm, 255);
    }
  }
  void Invertir(){
    invertir = true;
  }
  
};
Motor m1 = Motor(5,4);
Motor m2 = Motor(6,7);
  
void setup() {
  m2.Invertir();
}

void loop() {
  m1.Avanzar();
  m2.Avanzar();
}
