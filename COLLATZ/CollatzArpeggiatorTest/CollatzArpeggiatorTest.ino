//necessary Mozzi functions
#include <MozziGuts.h>
#include <Oscil.h>
#include <Smooth.h>
#include <AutoMap.h>




/*###########################
 * WAVETABLES AND MOZZI STUFF
############################ */
//wavetables (all same size for switching function)
#include <tables/sin2048_int8.h>

//Oscillator objects
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aCarrier_1(SIN2048_DATA);

//tone table
int tone_table[38] = {123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247,
                      262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
                      523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988, 1046};




/*####################################
 * PINS, INPUTS, AND CONTROL VARIABLES
 ####################################*/
//pushbuttons
const int numOfButtons = 8;
const int DIGITAL_PINS[numOfButtons] = {2, 3, 4, 5, 6, 7, 8, 11};

//tempo control potentiometer
const int TEMPO_PIN = 0;
int bpm, noteChange;
int counter = 0;

//note range control and mode control
const int RANGE_PIN = 1;
const int START_PIN = 2;
const int MODE_PIN = 12;
byte noteRange, noteStart;

//for Collatz Sequence
byte increase, toAdd;
int index = 0; 
long current_number = 1;
long new_number;





void setup() {
  Serial.begin(9600);
  startMozzi();
}


void updateControl(){
//#### READ INPUTS ####
  //PUSHBUTTONS
  toAdd = 0;
  for (int i=0; i<numOfButtons; i++) {
    increase = i + 2;
    if (digitalRead(DIGITAL_PINS[i]) == HIGH) {toAdd += increase*20;}
    else {toAdd += 0;}
  }
  //TEMPO
  bpm = mozziAnalogRead(TEMPO_PIN);
  noteChange = 4000 / (bpm+1);
  
  //RANGE AND START
  noteRange = mozziAnalogRead(RANGE_PIN) >> 5;
  noteStart = 2 + (12 * (mozziAnalogRead(START_PIN) >> 7));


//#### CHECK MODE PIN! DO STUFF! ####
  //COLLATZ!
  /*
   * If counter is greater than noteChange, it's time to move to the next note.
   * If current number is 1, then the end of the collatz sequence has been reached and it's time to get a new number
   * If the current number isn't 1, then find the next number in the collatz sequence.
   * Then use the current number to get the index for which tone to play
   */
  if (digitalRead(MODE_PIN) == LOW) {
    if (counter > noteChange) {
      if (current_number == 1) {current_number += toAdd;}
      else {
        if (current_number % 2 == 0) {
          int new_number = current_number / 2;
          current_number = new_number;
        }
        else {
          int new_number = current_number * 3;
          new_number++;
          current_number = new_number;
        }
      }
      index = current_number % 38;
      
      aCarrier_1.setFreq(tone_table[index]);
      Serial.print("Current Number: ");
      Serial.println(current_number);
      Serial.print("To Add: ");
      Serial.println(toAdd);
      Serial.print("Tone: ");
      Serial.println(tone_table[index]);
      
      counter = 0;
    }
  }

  //ARPEGGIATE!
  
  //increment counter and restart the loop
  counter++;
}
 


int updateAudio(){
  return aCarrier_1.next() << 4;
}


void loop() {
  audioHook();
}
