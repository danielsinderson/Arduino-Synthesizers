/*
Brief Sketch for an 12-channel Relative Tempo LFO
*/

const int pin = 0; 
byte output, output2;
int period, period2, pin_value;
long t_zero, t_one, delta_t, t_off_zero, t_off_one;


void setup() {
  for (int i=0;i<13;i++) {
    pinMode(i, OUTPUT);
  }
  t_zero = millis();
  t_off_zero = millis();
  output = 0;
  output2 = 0;
}

void loop() {
    pin_value = analogRead(pin);
    period = pin_value >> 1;
    period2 = (3 * period) / 2;  //2 * ((3 * period) / 4)), times 3/4 to map to off beat and times 8 to slow down since PORTB is only 5 bits instead of 8
    t_one = millis();
    delta_t = t_one - t_zero;
    if (delta_t >= period) {
      if (output == 255) {output = 0;}
      else {output++;}
      PORTD = output;
      t_zero = t_one;
    }
    t_off_one = millis();
    delta_t = t_off_one - t_off_zero;
    if (delta_t >= period2) {
      if (output2 == 64) {output2 = 0;}
      else {output2++;}
      PORTB = output2;
      t_off_zero = t_off_one;
    }
}
