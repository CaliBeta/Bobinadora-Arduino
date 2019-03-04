/*
  Project : Control motor PAP
  Version : 0.9.1beta
  Date    : 28/02/2019
  Author  : Carlos Andres Betancourt
  Company : Habitarte SAS
  Comments:

  Chip type               : ATmega328P-AU
  Program type            : Application
  AVR Core Clock frequency: 16.000000 MHz
  Memory model            : Small
  External RAM size       : 0
  Data Stack size         : 512
  commit: se puede controlar 2 motores PAP bipolares
  ------------------------------------------------------------
  Conexiones electricas
  Driver A4988 motor carro desplazador
  VMOT pin to +12V
  VDD pin to +5V
  GND pin to GND
  RST pin to SLEEP pin
  STEP pin to STEP1 pin
  DIR pin to DIR1 pin
  EN pin no EN1 pin

  Driver A4988 motor bobinador
  VMOT pin to +12V
  VDD pin to +5V
  GND pin to GND
  RST pin to SLEEP pin
  STEP pin to STEP2 pin
  DIR pin to DIR2 pin
  EN pin no EN2 pin
*/
//----------------------------------------------------------

//incluimos las librerias necesarias
#include <LiquidCrystal_I2C.h>  //Control LCD por I2C
#include "Bobinadora.h"
#include <Keypad.h>   //Manejo teclados tipo matriz
#include <Wire.h> //Protocolo de comunicacion I2C 
//----------------------------------------------------------

//Declaramos las variables globales necesarias
boolean inicio = false; //controla el inicio de los Timer
int contador = 0;
int contador2 = 0;
float diametro = 0.438; //diametro alambre calibre AWG26 recubrimiento doble
int longitud = 12;    //Longitus de la bobina en mm
int numEspiras = 0;   //numero de espiras que digita el usuario
int conEspiras = 0;   //contador de espiras realizadas por la bobinadora
long numPulsos = 0;   //Pulsos que debe dar el carro para cambiar de direccion
float escala = 0;     //factor para la velocidad el motor2
float cTiempo = 0;    //numero de pulsos por vuelta del motor2 (carro)

//Variables para el teclado 4x4
const byte ROWS = 4; //Cuatro filas
const byte COLS = 4; //Cuatro columnas
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {F1, F2, F3, F4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {C1, C2, C3, C4}; //connect to the column pinouts of the keypad
int numteclas = 0;
char data[BUFFER];
int estado = 0;
//----------------------------------------------------------

//Declaramos los obejtos de las librerias
Keypad teclado = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
LiquidCrystal_I2C lcd(DIRECCION, COLUMNA, FILA);
//----------------------------------------------------------

void setup() {
  Serial.begin(115200);
  lcd.init();   //Inicializamos el LCD
  lcd.backlight();  //Activamos la luz de fondo

  pinMode(STEP1, OUTPUT);
  pinMode(STEP2, OUTPUT);
  pinMode(DIR1, OUTPUT);
  pinMode(DIR2, OUTPUT);
  pinMode(EN1, OUTPUT);
  pinMode(EN2, OUTPUT);

  //Configuracion de los puertos C y D
  DDRC = ((1 << DDC0) | (1 << 1)); //PC0 y PC1 como salidas digitales
  //pines 2 a 7 como salidas, 0(RX) y 1(TX) como entradas
  DDRD = DDRD | B11111100;

  SREG = (SREG & 0b01111111); //desabilitamos las interrupciones
  //cli();  //realiza la misma funcion que el comando de arriba

  //Configuramos las interrupciones por Timer1
  TCCR1A = 0; //Configura el registro TCCR1A a 0
  TCCR1B = 0; //Lo mismo para el registro TCCR1B
  TCNT1  = 0; //inicializa el valor del contador a 0
  //Configuramos el registro de comparacion con el valor calculado para 1KHz
  //OCR1A = round((CLOCK/FREC)-1);// = [16*10^6) / (frec*prescaler] - 1 (menor a <65536)
  OCR1A = 65000;  //Debemos cargarle un valor inicial para que el LCD funcione
  //Encendemos el modo CTC para las comparaciones
  TCCR1B |= (1 << WGM12);
  //Configuramos el pin CS10 en 1 para un prescaler de 1
  TCCR1B |= (1 << CS10);
  //Habilitamos las interrupciones por comparacion
  TIMSK1 |= (1 << OCIE1A);

  //configuramos el Timer2
  TCCR2A = 0; //Configura el registro TCCR2A a 0
  TCCR2B = 0; //Lo mismo para el registro TCCR2B
  TCNT2  = 0; //inicializa el valor del contador a 0
  //Configuramos el valor del registro de comparacion para interupciones a la
  //frecuencia deseada, =[clock Hz)/(frec*prescaler]-1 -->>(menor a 256)
  OCR2A = round((CLOCK / FREC / PRESCALER2) - 1);
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  //Configuramos el pin CS22 en 1 para un prescaler de 64
  TCCR2B |= (1 << CS22);
  //Habilitamos las interrupciones por comparacion
  TIMSK2 |= (1 << OCIE2A);

  //ponemos todos los pines de los driver en BAJO
  digitalWrite(STEP1, LOW);
  digitalWrite(STEP2, LOW);
  digitalWrite(DIR1, LOW);
  digitalWrite(DIR2, LOW);
  digitalWrite(EN1, LOW);
  digitalWrite(EN2, LOW);

  SREG = (SREG & 0b01111111) | 0b10000000; //Habilita las interrupciones
  //sei();  //realiza la misma funcion que el comando de arriba

  //mensaje de inicio
  lcd.setCursor(3, 0); //Ubicamos el cursor en la columna 1, fila 0
  lcd.print("BOBINADORA");  //Escribimos el mensaje
  lcd.setCursor(1, 1); //Ubicamos el cursor en la columna 5, fila 1
  lcd.print("SEMIAUTOMATICA");
  delay(2000);  //Retardo de tiempo de 2000ms ( 2 segundos)
  lcd.clear();
}
//----------------------------------------------------------

void loop() {
  if (estado == 0) {
    lcd.setCursor(0, 0);
    lcd.print("INGRESE #ESPIRAS");
    lcd.cursor();
  }

  char key = teclado.getKey();

  if (key && estado == 0) {
    if (key != 'D') {
      lcd.setCursor(numteclas + 6, 1);
      lcd.print(key);

      data[numteclas] = key;
      Serial.println(data[numteclas]);
      numteclas++;

    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("#ESPIRAS=");
      for (int i = 0; i < numteclas; i++) {
        lcd.print(data[i]);
      }

      lcd.setCursor(0, 1);
      lcd.print("Continuar...");
      estado = 1;
      numteclas = 0;
      numEspiras = atoi(data); //convierte el buffer de datos a un numero entero
      //Serial.println(numEspiras, DEC); //imprime el valor en decimal

    }
  }
  else if (key && estado == 1) {
    if (key == 'D') {
      inicio = true;
      TCNT1  = 0; //inicializa el valor del contador a 0
      TCNT2  = 0; //inicializa el valor del contador a 0
      lcd.noCursor();
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("INICIO BOBINAR");
      lcd.setCursor(2, 1);
      lcd.print("ESPIRAS:");
    }
  }
  //Calculamos los parametros para mover los motores acorde con los
  //datos proporcionados por el usuario
  numPulsos = round((longitud * PULSOS_REV2) / AVANCE);
  cTiempo = (diametro / AVANCE) * PULSOS_REV2;
  escala = PULSOS_REV1 / cTiempo;

  //Configuramos el valor del contador para el Timer1
  float counTimer1 = round((CLOCK / FREC) * escala);
  OCR1A = counTimer1 - 1;

  while (inicio == true) {
    lcd.setCursor(2 + 8, 1);
    lcd.print(conEspiras);

    if (conEspiras == numEspiras) {
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("TERMINADO!");
      lcd.setCursor(2, 1);
      lcd.print("ESPIRAS:");
      lcd.print(conEspiras);
      conEspiras = 0;
      contador2 = 0;
      contador = 0;
      inicio = false;
    }
    break;
  }
}
