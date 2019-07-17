#include <Adafruit_NeoPixel.h>
#define LED_PIN 12
#define LED_COUNT 6
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);



//PIN assignment variables
const int NUM = 8;
const int BUTTONS[NUM] = {2, 3, 4, 5, 6, 7, 9, 10};
const int CHOICES = 6;

//State variable -- incs on button push
int states[NUM] = {0, 0, 0, 0, 0, 0, 0, 0};

//Variables for button debounce
unsigned long t = 250;
unsigned long now;
unsigned long delta;
const int threshold = 210;



void setup() {
  strip.begin();
  strip.show();
  Serial.begin(9600);
}

void loop() {
  //Calculate delta
  now = millis();
  delta = now - t;
  
  //Read button inputs and inc state variables
  for (byte i=0; i<NUM; i++) {
    if (digitalRead(BUTTONS[i]) == HIGH and delta > threshold) {
      if (states[i] >= 5) {states[i] = 0;}
      else {states[i]++;}
      Serial.print(states[i]);
      t = millis();
    }
  }

  for (int i=2; i<NUM; i++) { //iterate through states and set colors
    if (i<5) {setColor(states[i], i-2, states[0]);}
    else {setColor(states[i], i-2, states[1]);}
  }
  strip.show();
}






void setColor(int state, int led, int ch) {
  Serial.print(ch);
  if (ch % 2 == 0) {
    if (state % CHOICES == 0) {strip.setPixelColor(led, 0, 0, 64);}
    else if (state % CHOICES == 1) {strip.setPixelColor(led, 64, 0, 0);}
    else if (state % CHOICES == 2) {strip.setPixelColor(led, 0, 64, 0);}
    else if (state % CHOICES == 3) {strip.setPixelColor(led, 25, 64, 12);}
    else if (state % CHOICES == 4) {strip.setPixelColor(led, 48, 48, 48);}
    else if (state % CHOICES == 5) {strip.setPixelColor(led, 25, 50, 64);}
  }
  else {strip.setPixelColor(led, 0, 0, 0);} 
}



