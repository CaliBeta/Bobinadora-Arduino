//Funcion de interupcion del Timer1 por comparacion
ISR(TIMER1_COMPA_vect) {
  if (inicio == true) {
    contador2++;
    PORTC ^= (1 << STEP2); //Cambia el estado del pin PD4 a su valor negado
    if (contador2 == numPulsos * 1.91) {
      PORTC ^= (1 << DIR2);
      contador2 = 0;
    }
  }
}
//-----------------------------------------------------------------------

//Funcion de interupcion del Timer2 por comparacion
ISR(TIMER2_COMPA_vect) {
  if (inicio == true) {
    contador++;
    PORTD ^= (1 << STEP1); //Cambia el estado del pin PD4 a su valor negado
    if (contador == 800) {
      conEspiras++;
      contador = 0;
    }
  }
}
//-----------------------------------------------------------------------
