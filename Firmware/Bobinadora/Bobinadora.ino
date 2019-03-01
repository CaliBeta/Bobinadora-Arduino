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
#include "Bobinadora.h"
//----------------------------------------------------------

//Declaramos las variables globales necesarias
boolean inicio = false; //controla el inicio de los Timer
int contador = 0;
int contador2 = 0;
float diametro = 0.418; //diametro alambre calibre AWG26 recubrimiento doble
int longitud = 12;
int numEspiras = 0;   //numero de espiras que digita el usuario
int conEspiras = 104; //contador de espiras realizadas por la bobinadora
long numPulsos = 0;   //Pulsos que debe dar el carro para cambiar de direccion
float escala = 0;     //factor para la velocidad el motor2
float cTiempo = 0;    //numero de pulsos por vuelta del motor2 (carro)
//----------------------------------------------------------

void setup() {
  Serial.begin(115200);

  pinMode(STEP1, OUTPUT);
  pinMode(STEP2, OUTPUT);
  pinMode(DIR1, OUTPUT);
  pinMode(DIR2, OUTPUT);
  pinMode(EN1, OUTPUT);
  pinMode(EN2, OUTPUT);

  //Configuracion de los puertos C y D
  DDRC |= ((1 << DDC0) | (1 << 1)); //PC0 y PC1 como salidas digitales
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
  OCR2A = round((CLOCK / FREC/PRESCALER2)-1);
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
}
//----------------------------------------------------------

void loop() {
  //Calculamos los parametros para mover los motores acorde con los
  //datos proporcionados por el usuario
  numPulsos = (longitud * PULSOS_REV2) / AVANCE;
  cTiempo = (diametro / AVANCE) * PULSOS_REV2;
  escala = PULSOS_REV1 / cTiempo;
  
  //Configuramos el valor del contador para el Timer1
  float counTimer1 = round((CLOCK / FREC) * escala);
  OCR1A = counTimer1 - 1;

  inicio = true;
}
