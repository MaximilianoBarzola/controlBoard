
///CONTROL DE TEMPERATURA PARA ULTRAFREEZER///
///ALARMAS POR ALTA Y POR BAJA TEMPERATURA///
///FALTA DE RED ELECTRICA Y TIEMPO DE PUERTA ABIERTA///


///***LIBRERIAS***///

#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_MAX31865.h>

///***DEFINICIONES Y VARIABLES GLOBALES***///

#define RREF      430
#define RNOMINAL  100
#define SSRA 6  //salida para el ssr primera etapa de -20
#define SSRB 7  //salida para el ssr segunda etapa de -80
#define SSRC 8  //salida para forzadores
#define SSRD 9  //salida auxiliar
#define Digital 4 //pin para sensores digitales 
#define Puerta 5  //Entrada esto de la puerta abierta o cerrada
//#define PIN_A0 14 //le asigno a la entrada A1 el pin digital 14
#define Alimentacion 14 //pin para detectar si hay presencia de red electrica
//#define PIN_A1 15 //le asigno a la entrada A1 el pin digital 15
#define LedAlta 15  //pin para el led de alta temperatura
//#define PIN_A2 16 //le asigno a la entrada A2 el pin digital 16
#define LedBaja 16  //pin para el led de baja temperatura
//#define PIN_A3 17 // le asigno a la entrada A3 el pin digital 17
#define Buzzer 17 //pin parra el buzzer de alarmas
#define BTN_MENU  0
#define BTN_EXIT  3
#define BTN_UP    1
#define BTN_DOWN  2
#define S_INICIO  0
#define S_INICIO2 1
#define S_TEMPSET 2
#define S_ALARMAALTA 3
#define S_ALARMABAJA 4
int TempSet; //variable para almacenar el valor para escribir en pantalla
int TempAct; //variable para almacenar el valor de la temperatura
int TempAmb; //variable para almacenar el valor de la temperatura ambiente
int TempCond1;  //variable para almacenar el valor de la temperatura del condensador de la primera etapa
int TempCond2;  //variable para almacenar el valor de la temperatura del condensador de la segunda etapa
float Bateria = A6;  //variable para almacenar el valor de bateria
int EstadoCargaMap; // almacena EstadoCarga pero ya mapeado
float EstadoCarga;  //almacena la lectura de la bateria
String Situacion;  //determino que la variable estado es una cadena de 7 caracteres
int AlarmaAlta; //variable que guarda el valor de la alarma por alta
int AlarmaBaja; //variable que guarda el valor de la alarma por baja
uint8_t button[4] = {0, 1, 2, 3}; // Este arreglo contiene los pines utilizados para los botones
uint8_t button_estate[4]; // Este arreglo contiene el último estado conocido de cada línea
uint8_t estado = S_INICIO;  // Estado inicial de la maquina de estados
static long TiempoBateria = 0;  //variable para el inicio del cronometro
static long TiempoSensor = 0;  //variable para el inicio del cronometro
static long TiempoSalidas = 0;  //variable para el inicio del cronometro
static long TiempoPuerta = 0;
static long TiempoBuzzer = 0;
static long TiempoAlimentacion = 0;
long Hora;
bool Pip = false;
bool Light = false;
bool valor = false;
bool Red = false;
///***CREACION DE OBJETOS***///

Adafruit_MAX31865 thermo = Adafruit_MAX31865(10);
LiquidCrystal_I2C lcd(0x3F, 20, 4); //crea el objeto lcd y determina la direccion del I2C las columnas y filas
byte customChar[] = {B11100, B10100, B11100, B00000, B00000, B00000, B00000, B00000}; //Crea el caracter especial del simbolo grado "°"
OneWire ourWire(Digital); //Se establece el pin 4  como bus OneWire
DallasTemperature sensors(&ourWire); //Se declara una variable u objeto para nuestro sensor
DeviceAddress address1 = {0x28, 0x89, 0xD7, 0x94, 0x06, 0x00, 0x00, 0x53};//dirección del sensor 1
DeviceAddress address2 = {0x28, 0x18, 0xC0, 0x94, 0x06, 0x00, 0x00, 0xF8};//dirección del sensor 2
DeviceAddress address3 = {0x28, 0x6B, 0x37, 0xB6, 0x06, 0x00, 0x00, 0xB9};//dirección del sensor 3



///***FUNCION DE DETECCION DEL BOTON***///

uint8_t flancoBajada(int btn) {
  uint8_t valor_nuevo = digitalRead(button[btn]);
  uint8_t result = button_estate[btn] != valor_nuevo && valor_nuevo == 0;
  button_estate[btn] = valor_nuevo;
  return result;
}

///***FUNCION DE PANTALLA PRINCIPAL***///

void printINICIO() {
  static long TiempoLCD = 0;
  long Hora = millis();
  uint16_t rtd = thermo.readRTD();  //lee el valor digital de la termocupla
  float ratio = rtd;  //establece que ratio es flotante
  ratio /= 32768; //divide RTD por 32768 y lo nombra ratio
  lcd.setCursor(0, 0); // posiciona el cursor en columna 0 y fila 0
  lcd.print("Temp Actual:"); lcd.print(TempAct); lcd.write(0); lcd.print("C");//escribe la temperatura
  lcd.setCursor(0, 1); //posiciona el cursor en columna 0 y fila 1
  lcd.print("Temp Seteo:"); lcd.print(EEPROM.get(0, TempSet)); lcd.write(0); lcd.print("C");//escribe el valor de seteo
  lcd.setCursor(0, 2); //posiciona el cursor en columna 0 y fila 2
  lcd.print("Bateria:"); lcd.print(EstadoCargaMap); lcd.print("%"); //escribe el valor de carga de la bateria
  lcd.setCursor(0, 3); //posiciona el cursor en columna 0 y fila 3
  lcd.print("Puerta:"); lcd.print(Situacion);  // escribe el estado de la puerta
  EEPROM.get(2, AlarmaAlta);
  EEPROM.get(4, AlarmaBaja);
  if (Hora - TiempoLCD > 5000) {
    lcd.clear();
    TiempoLCD = Hora;
  }
}

///***FUNCION DE PANTALLA SECUNDARIA***///

void printINICIO2() {
  static long TiempoLCD = 0;
  long HoraLCD = millis();
  lcd.setCursor(0, 0); // posiciona el cursor en columna 0 y fila 0
  lcd.print("Alarma Alta:"); lcd.print(EEPROM.get(2, AlarmaAlta)); lcd.write(0); lcd.print("C");//
  lcd.setCursor(0, 1); //posiciona el cursor en columna 0 y fila 1
  lcd.print("Alarma Baja:"); lcd.print(EEPROM.get(4, AlarmaBaja)); lcd.write(0); lcd.print("C");//
  lcd.setCursor(0, 2); //posiciona el cursor en columna 0 y fila 2
  lcd.print("Temp. Ambiente:"); lcd.print(TempAmb); lcd.write(0); lcd.print("C");//
  lcd.setCursor(0, 3);
  lcd.print(TempCond1); lcd.print(TempCond2);
  if (HoraLCD - TiempoLCD > 5000) {
    lcd.clear();
    TiempoLCD = HoraLCD;
  }
}

///***FUNCION DE PANTALLA DE SETEO DE TEMPERATURA***///

void printTEMPSET() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Selecccione la");
  lcd.setCursor(0, 1);
  lcd.print("temperatura de Seteo");
  lcd.setCursor(8, 3);
  lcd.print(TempSet); lcd.write(0); lcd.print("C");
  EEPROM.put(0, TempSet);
}

///***FUNCION DE PANTALLA DE ALARMA POR ALTA TEMPERATURA***///

void printALARMAALTA() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Ajuste Alarma alta");
  lcd.setCursor(4, 1);
  lcd.print("temperatura");
  lcd.setCursor(8, 3);
  lcd.print(AlarmaAlta); lcd.write(0); lcd.print("C");
  EEPROM.put(2, AlarmaAlta);
}

///***FUNCION DE PANTALLA DE ALARMA POR BAJA TEMPERATURA***///

void printALARMABAJA() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Ajuste Alarma baja");
  lcd.setCursor(4, 1);
  lcd.print("temperatura");
  lcd.setCursor(8, 3);
  lcd.print(AlarmaBaja); lcd.write(0); lcd.print("C");
  EEPROM.put(4, AlarmaBaja);
}

///***FUNCION INTERMITENCIA DEL BUZZER***///

void Pitido() {
  Pip = !Pip;
  if (Pip) {
    digitalWrite (Buzzer, HIGH);
  }
  if (!Pip) {
    digitalWrite (Buzzer, LOW);
  }
}

///***FUNCION INTERMITENCIA DEL BACKLIGHT***///

void Backlight() {
  Light = !Light;
  if (Light) {
    lcd.backlight();
  }
  if (!Light) {
    lcd.noBacklight();
  }
}


///***SETUP***///

void setup() {
  thermo.begin(MAX31865_4WIRE); //inicializa el MAX31865
  lcd.init(); //inicializa el lcd
  lcd.backlight();  //enciende el backline
  pinMode(SSRA, OUTPUT);  //define al pin SSRA como salida para la primera etapa
  pinMode(SSRB, OUTPUT);  //define al pin SSRB como salida para la segunda etapa
  pinMode(SSRC, OUTPUT);  //define al pin SSRC como salida para los forzadores
  pinMode(SSRD, OUTPUT);  //define al pin SSRD como salida auxiliar
  pinMode(Puerta, INPUT_PULLUP); //define al pin como entrada del estado de la puerta
  pinMode(Bateria, INPUT); //mido la bateria
  pinMode(LedAlta, OUTPUT); //Salida led por alta temperatura
  pinMode(LedBaja, OUTPUT); //Salida led por baja temperatura
  pinMode(Buzzer, OUTPUT);  //Salida Buzzer de alarmas
  pinMode(button[BTN_MENU], INPUT); //define al boton menu como entrada
  pinMode(button[BTN_EXIT], INPUT); //define al boton exit como entrada
  pinMode(button[BTN_UP], INPUT); //define al boton up como entrada
  pinMode(button[BTN_DOWN], INPUT); //define al boton down como entrada
  pinMode(Alimentacion, INPUT_PULLUP);  //define al pin como entrada
  button_estate[0] = HIGH;  //define el estado por defecto como alto del boton menu
  button_estate[1] = HIGH;  //define el estado por defecto como alto del boton exit
  button_estate[2] = HIGH;  //define el estado por defecto como alto del boton up
  button_estate[3] = HIGH;  //define el estado por defecto como alto del boton down
  lcd.createChar(0, customChar);  //inicializa el caracter personalizado
}


///***INICIO DEL LOOP***///

void loop() {

  ///***ALARMAS POR ALTA Y POR BAJA TEMPERATURA RESPECTO DEL VALOR LA TEMPERATURA ACTUAL***///

  Hora = millis();

  if (TempAct > AlarmaAlta) { //si la temperatura actual es mayor que la alarma de alta
    digitalWrite(LedAlta, HIGH);  //enciende el led
    if (Hora - TiempoBuzzer > 100) {  //cadencia del pitido por alta temperatura
      Pitido(); //ejecuta la funcion
      TiempoBuzzer = Hora;  //reinicia el cronometro
    }
  }
  if (TempAct < AlarmaBaja) { //si la temperatura actual es menor que la alarma de baja
    digitalWrite(LedBaja, HIGH); //enciende el led
    if (Hora - TiempoBuzzer > 1000) { //cadencia del pitido por baja temperatura
      Pitido(); //ejecuta la funcion
      TiempoBuzzer = Hora;  //reinicia el cronometro
    }
  }
  if (TempAct >= AlarmaBaja && TempAct <= AlarmaAlta) { //si la temperatura actual es mayor que la alarma de baja Y es menor que la alarma de alta
    digitalWrite(LedAlta, LOW); //apaga el led de alta
    digitalWrite(LedBaja, LOW); //apaga el led de baja
  }
  if (TempAct >= AlarmaBaja && TempAct <= AlarmaAlta && valor == LOW) { //si la temperatura actual es mayor que la alarma de baja Y es menor que la alarma de alta
    digitalWrite(Buzzer, LOW);  //apaga el buzzer
  }

  ///***DETECCION DEL ESTADO DE LA PUERTA***///

  valor = digitalRead(Puerta);  // lee el estado del sensor de puerta
  if (valor == HIGH) {  // si esta alto
    Situacion = "ABIERTA";  //graba en el string la palabra ABIERTA
    if (Hora - TiempoPuerta > 20000) {  //pasado este tiempo que la puerta esta abierta
      digitalWrite(Buzzer, HIGH); //activa el buzzer
    }
  }
  else {  
    Situacion = "CERRADA";  //graba en el string la palabra CERRADA
    TiempoPuerta = Hora;  //reinicia el cronometro
  }

  ///***ALARMA POR FALTA DE RED ELECTRICA***///

  Red = digitalRead(Alimentacion);  //lee la pata donde esta el optoacoplador
  if (Red == HIGH && Hora - TiempoAlimentacion > 1000) {  //si esta alto y pasa 1 segundo
    Backlight();  //ejecuta la funcion Backlight
    TiempoAlimentacion = Hora;  //reinicia el cronometro
  }
  else {
    lcd.backlight();  //si no, deja prendido el backlight
  }
  ///***LECTURA DE SENSORES SECUNDARIOS***///

  if (Hora - TiempoSensor > 10000) {  //retraso en la lectura del estado de los sensores
    sensors.requestTemperatures();   //envía el comando para obtener las temperaturas
    TempAmb = sensors.getTempC(address1); //Se obtiene la temperatura en °C del sensor 1
    TempCond1 = sensors.getTempC(address2); //Se obtiene la temperatura en °C del sensor 2
    TempCond2 = sensors.getTempC(address3); //Se obtiene la temperatura en °C del sensor 3
    TiempoSensor = Hora;  //pongo el delta del cronometro en 0
  }

  ///***CONTROL DE TEMPERATURA PRINCIPAL***///

  TempAct = (thermo.temperature(RNOMINAL, RREF)); //lecrura de temperatura desde el conversor
  if (TempAct > TempSet) { //si la temperatura es mayor
    digitalWrite(SSRB, HIGH); //enciende los forzadores
    if (Hora - TiempoSalidas > 20000) { //pone un retraso en el encendido a la primera etapa expresado en ms
      digitalWrite(SSRA, HIGH); //enciende la etapa de alta
    }
  }
  if (Hora - TiempoSalidas > 60000) { //pone un retraso en el encendido a la segunda etapa expresado en ms
    digitalWrite(SSRC, HIGH); //enciende la etapa de baja
  }
  if (TempAct <= TempSet) { //si la temperatura es menor
    TiempoSalidas = Hora;  //reinicia el cronometro
    digitalWrite(SSRA, LOW); //apaga la etapa de alta
    digitalWrite(SSRB, LOW); //apaga la etapa de baja
    digitalWrite(SSRC, LOW); //apaga los forzadores
  }


  ///***ESTADO DE CARGA DE LA BATERIA***///

  if (Hora - TiempoBateria > 15000) {  //retraso en la lectura del estado de la bateria
    EstadoCarga = analogRead(Bateria); //tomo el valor de la bateria
    EstadoCargaMap = map(EstadoCarga, 672, 1023, 0, 100); // mapea el valor minimo y maximo entre 0% y 100%
    EstadoCargaMap = constrain(EstadoCargaMap, 0, 100); //restrinjo los valores maximo y minimo en la variable
    TiempoBateria = Hora;  //pongo el delta del cronometro en 0
  }

  ///***MAQUINA DE ESTADOS***///

  switch (estado) {
    case S_INICIO:
      if (flancoBajada(BTN_MENU)) {
        estado = S_TEMPSET;
        printTEMPSET();
        break;
      }
      if (flancoBajada(BTN_DOWN)) {
        estado = S_INICIO2;
        printINICIO2();
        break;
      }
      printINICIO();
      break;

    ///***FIN ESTADO INICIO***///

    case S_INICIO2:
      printINICIO2();
      if (flancoBajada(BTN_UP)) {
        estado = S_INICIO;
        printINICIO();
        break;
      }
      if (flancoBajada(BTN_MENU)) {
        estado = S_TEMPSET;
        printTEMPSET();
        break;
      }
      break;

    ///***FIN ESTADO INICIO2***///

    case S_TEMPSET:
      if (flancoBajada(BTN_UP)) {
        TempSet++;
        printTEMPSET();
      }
      if (flancoBajada(BTN_DOWN)) {
        TempSet --;
        printTEMPSET();
        break;
      }
      if (flancoBajada(BTN_EXIT)) {
        estado = S_INICIO;
        printINICIO();
        break;
      }
      if (flancoBajada(BTN_MENU)) {
        estado = S_ALARMAALTA;
        printALARMAALTA();
      }
      break;

    ///***FIN ESTADO TEMPSET***//

    case S_ALARMAALTA:
      if (flancoBajada(BTN_UP)) {
        AlarmaAlta++;
        printALARMAALTA();
      }
      if (flancoBajada(BTN_DOWN)) {
        AlarmaAlta--;
        printALARMAALTA();
        break;
      }
      if (flancoBajada(BTN_EXIT)) {
        estado = S_INICIO;
        printINICIO();
        break;
      }
      if (flancoBajada(BTN_MENU)) {
        estado = S_ALARMABAJA;
        printALARMABAJA();
      }
      break;

    ///***FIN ESTADO ALARMA ALTA***///

    case S_ALARMABAJA:
      if (flancoBajada(BTN_UP)) {
        AlarmaBaja++;
        printALARMABAJA();
      }
      if (flancoBajada(BTN_DOWN)) {
        AlarmaBaja--;
        printALARMABAJA();
        break;
      }
      if (flancoBajada(BTN_EXIT)) {
        estado = S_INICIO;
        printINICIO();
        break;
      }
      if (flancoBajada(BTN_MENU)) {
        estado = S_TEMPSET;
        printTEMPSET();
      }
      break;
  }
}

    ///***FIN ESTADO TEMPSET***/
