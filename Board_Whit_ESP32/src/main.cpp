#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#define RXD2 2
#define TXD2 4
#define BIN1 25
#define BIN2 33
#define AIN1 26
#define AIN2 27
#define A2 36
#define A1 23
#define KP 4
#define KD 0
#define KI 0
#define PERIODO 5
#define VALOR_MAXIMO 800
class motor{
  private:
    int channel1, channel2;
    int vel;
    boolean invertir = true;

  public:
    motor(int canal1, int canal2, int p_int1, int p_int2){
      channel1 = canal1;
      channel2 = canal2;
      pinMode(p_int1, OUTPUT);
      pinMode(p_int2, OUTPUT);
      ledcSetup(channel1, 40000, 10);
      ledcSetup(channel2, 40000, 10);
      ledcAttachPin(p_int1, channel1);
      ledcAttachPin(p_int2, channel2);
    }
    int Avanzar(int set){
      if(set >= 1023){
        vel = 1023;
      }
      if(set <= 0){
        vel = 0;
      }
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
      if(set >= 1023){
        vel = 1023;
      }
      if(set <= 0){
        vel = 0;
      }
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
};
WiFiServer server(80);
motor m1 = motor(5, 6, AIN1, AIN2);
motor m2 = motor(3, 4, BIN1, BIN2);
const char* ssid     = "ESP32";
const char* password = "7790895000232";
String header;
String pagina = "<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8' />"
"<title>Servidor Web ESP32</title>"
"</head>"
"<body>"
"<center>"
"<h1>Servidor Web ESP32</h1>"
"<p><a href='/forward'><button style='height:200px;width:100px'>FORWARD</button></a></p>"
"<p><a href='/left'><button style='height:100px;width:200px'>LEFT</button></a>"
"<a href='/stop'><button style='height:100px;width:100px'>STOP</button></a>"
"<a href='/right'><button style='height:100px;width:200px'>RIGHT</button></a></p>"
"<p><a href='/reverse'><button style='height:200px;width:100px'>REVERSE</button></a></p>"
"</center>"
"</body>"
"</html>";
int16_t wifiControl(){
  static int16_t direction;
  int16_t pocicion[2]; 
  WiFiClient client = server.available();   // Escucha a los clientes entrantes

  if (client) {                             // Si se conecta un nuevo cliente
    Serial.println("New Client.");          // 
    String currentLine = "";                //
    String substring;
    while (client.connected()) {            // loop mientras el cliente está conectado
      if (client.available()) {             // si hay bytes para leer desde el cliente
        char c = client.read();             // lee un byte
        Serial.write(c);                    // imprime ese byte en el monitor serial
        header += c;
        if (c == '\n') {                    // si el byte es un caracter de salto de linea
          // si la nueva linea está en blanco significa que es el fin del 
          // HTTP request del cliente, entonces respondemos:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // establese la dirección del auto
            pocicion[0] = header.indexOf("GET /");
            pocicion[1] = header.indexOf("!");
            substring = header.substring(pocicion[0] + 5, (pocicion[0] + 5) - pocicion[1]);
            Serial.println(substring);
            direction = substring.toInt();  
            Serial.println(direction);  
            // Muestra la página web
            client.println(pagina);
            
            // la respuesta HTTP termina con una linea en blanco
            client.println();
            break;
          } else { // si tenemos una nueva linea limpiamos currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
    }
    // Limpiamos la variable header
    header = "";
    // Cerramos la conexión
    client.stop();
//    Serial.println("Client disconnected.");
//    Serial.println("");
  }
  return direction;
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
    if(PID_total <= -2000){
      PID_total = -2000;
    }
    if(PID_total > 2000){
      PID_total = 2000;
    }
  }

  return map(PID_total, -2000, 2000, -VALOR_MAXIMO, VALOR_MAXIMO);
}
void direccionMotor(int v_PID){
  if(v_PID == 0){
    m1.Avanzar(VALOR_MAXIMO);
    m2.Avanzar(VALOR_MAXIMO);
  }
  else if(v_PID > 0){
    m1.Avanzar(VALOR_MAXIMO);
    m2.Avanzar(VALOR_MAXIMO - v_PID);
  }
  else if(v_PID < 0){
    m1.Avanzar(VALOR_MAXIMO - v_PID * -1);
    m2.Avanzar(VALOR_MAXIMO);
  }
}
void setup() {
  m1.Invertir();
  Serial.begin(115200);
  WiFi.softAP(ssid, password);        //Start Acces point mode
  IPAddress ip = WiFi.softAPIP();
  
  Serial.print("Nombre de mi red esp32: ");
  Serial.println(ssid);
  Serial.print("La IP es: ");
  Serial.println(ip);
  server.begin();
  delay(100);

  //Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

}

void loop() {
  //Serial.println(wifiControl());
  wifiControl();
  //m1.Retroceder(1023);
  //m2.Avanzar(1023);
  //digitalWrite(AIN1, 0);
  //digitalWrite(AIN2, 0);
}