#ifndef __Bobinadora_H__
#define __Bobinadora_H__

//Pines para controlar el Hardware
//motores PAP NEMA 17
#define STEP1         PD4
#define STEP2         PC1
#define DIR1          5
#define DIR2          PC0
#define EN1           3
#define EN2           A2

//Teclado 4x4
#define BUFFER        5   //buffer para la conversion char to int
#define F1            13
#define F2            12
#define F3            11
#define F4            10
#define C1            9
#define C2            8
#define C3            7
#define C4            6

//Parametros constantes
//Display LCD 1602
#define DIRECCION     0x27
#define COLUMNA       16
#define FILA          2

//Motor NEMA17
#define TIPO_CONTROL  1     //tipo de control (1= control driver 2 pines)
#define PULSOS_REV1   400   //Numero de pulsos por revolucion1 (200*resol_microstep)
#define PULSOS_REV2   1600  //Numero de pulsos por revolucion2 (200*resol_microstep)
#define AVANCE        8.0   //avance husillo en mm/revolucion
#define FREC          3200  //Frecuencia en Hz para los pulsos del motor 1(max 3620)

//Timers
#define PRESCALER1    1     //Prescaler del Timer1
#define PRESCALER2    64    //Prescaler del Timer2
#define CLOCK         16000000//Frecuencia del oscilador del microcontrolador

#endif
