/*
Brief Sketch for an 8-channel Relative Tempo LFO
*/

const int pin = 0; 
byte output;
int freq, period, pin_value;
long t_zero, t_one, delta_t;


void setup() {
  for (int i=0;i<9;i++) {
    pinMode(i, OUTPUT);
  }
    t_zero = millis();
    output = 0;
}

void loop() {
    pin_value = analogRead(pin);
    freq = pin_value / 2;
    period = 4000 / freq;
    t_one = millis();
    delta_t = t_one - t_zero;
    if (delta_t >= period) {
      if (output == 255) {output = 0;}
      else {output++;}
      PORTD = output;
      t_zero = t_one;
    }
}
