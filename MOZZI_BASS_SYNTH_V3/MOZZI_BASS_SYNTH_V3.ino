/*
 * BASS SYNTH:
 * Duophony bass synth with ADSR envelopes for each voice, waveform choices, and bitwise distortion
 * still needs:
 *          - frequency tables
 *          - frequency mapping
 *          - updateAudio stuff
 *          - waveform setting and waveform includes
 */


#include <MozziGuts.h>
#include <Oscil.h>
#include <AutoMap.h>
#include <ADSR.h>

#include <tables/sin2048_int8.h>
#include <tables/halfsin2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/saw2048_int8.h>



//PINS
const byte ANALOG_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
const byte TRIGGER_PINS[2] = {2, 3};
const byte MODE_PIN = 4;


//GLOBAL VARIABLES
bool triggers[2] = {0, 0};
byte waveform_choice = 0;
byte scale = 0;
Oscil <2048, AUDIO_RATE> oscils[2];
ADSR <AUDIO_RATE, AUDIO_RATE> envelopes[2];

float note_freq[41] = {20.60, 21.83, 23.12, 24.50, 25.96, 27.50, 29.14, 30.87,
                      32.70, 34.65, 36.71, 38.89, 41.20, 43.65, 46.25, 49.00,
                      51.91, 55.00, 58.27, 61.74, 69.30, 73.42, 77.78, 82.41,
                      87.31, 92.50, 98.00, 103.83, 110.00, 116.54, 123.47, 130.81,
                      138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00, 207.65, 220.00};

//FUNCTION DECLARATIONS
void read_trigger(byte n) {
  if (digitalRead(TRIGGER_PINS[n]) == HIGH and triggers[n] == LOW) {
    envelopes[n].noteOn();
    triggers[n] = HIGH;
  }
  else if (digitalRead(TRIGGER_PINS[n]) == LOW and triggers[n] == HIGH) {
    envelopes[n].noteOff();
    triggers[n] = LOW;
  }
}


void setup() {
  startMozzi();
  for (int i;i<8;i++) { pinMode(ANALOG_PINS[i], INPUT); }
  for (int i;i<2;i++) { pinMode(TRIGGER_PINS[i], INPUT); }
  pinMode(MODE_PIN, INPUT);
  envelopes[0].setLevels(255, 150, 150, 0);
  envelopes[1].setLevels(255, 150, 150, 0);
  envelopes[0].setTimes(20, 20, 4000, 20);
  envelopes[1].setTimes(20, 20, 4000, 20);
}


void updateControl() {
  read_trigger(0);
  read_trigger(1);
  
  int x;
  x = mozziAnalogRead(ANALOG_PINS[0]) >> 1;
  envelopes[0].setAttackTime(x + 20);
  envelopes[1].setAttackTime(x + 20);

  x = mozziAnalogRead(ANALOG_PINS[1]) >> 1;
  envelopes[0].setDecayTime(x + 20);
  envelopes[1].setDecayTime(x + 20);

  x = mozziAnalogRead(ANALOG_PINS[2]) >> 2;
  envelopes[0].setSustainLevel(x);
  envelopes[1].setSustainLevel(x);
  envelopes[0].setDecayLevel(x);
  envelopes[1].setDecayLevel(x);

  x = mozziAnalogRead(ANALOG_PINS[3]) >> 1;
  envelopes[0].setReleaseTime(x + 20);
  envelopes[1].setReleaseTime(x + 20);

  x = mozziAnalogRead(ANALOG_PINS[4]) >> 8;
  if (x==0) { 
    oscils[0].setTable(SIN2048_DATA); 
    oscils[1].setTable(SIN2048_DATA);
  }
  else if (x==1) { 
    oscils[0].setTable(SQUARE_NO_ALIAS_2048_DATA); 
    oscils[1].setTable(SQUARE_NO_ALIAS_2048_DATA); 
  }
  else if (x==2) { 
    oscils[0].setTable(HALFSIN2048_DATA); 
    oscils[1].setTable(HALFSIN2048_DATA);
  }
  else if (x==3) { 
    oscils[0].setTable(SAW2048_DATA);
    oscils[1].setTable(SAW2048_DATA);
  }

  x = mozziAnalogRead(ANALOG_PINS[6]) >> 5;
  oscils[0].setFreq(note_freq[x + 9] * 2);

  x = mozziAnalogRead(ANALOG_PINS[5]) >> 5;
  oscils[1].setFreq(note_freq[x + 9] * 2);
}


int updateAudio() {
  int voice1, voice2, audio1, audio2, combined;
  envelopes[0].update();
  envelopes[1].update();

  /*
  voice1 = oscils[0].next();
  audio1 = (voice1 * envelopes[0].next()) >> 2;

  voice2 = oscils[1].next();
  audio2 = (voice2 * envelopes[1].next()) >> 2;

  combined = (audio1 + audio2) >> 1;
  return combined;*/
  return (((oscils[0].next() * oscils[1].next()) >> 8) * envelopes[0].next()) >> 2;
}


void loop() {
  audioHook();
}
