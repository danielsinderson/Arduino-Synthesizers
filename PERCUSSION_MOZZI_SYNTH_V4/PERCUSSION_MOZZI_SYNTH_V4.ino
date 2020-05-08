/*  
 *   VERSION 1: Uses the chum9 wavetable and passes it through a low pass filter with adjustable cutoff and resonance, also has an ADSR envelop with adjustable release
 *   VERSION 2: Want to add different modes: 
 *              1) first mode is identical to version 1 except with adjustable reverb: freq, cutoff, release, reverb
 *              2) second mode will be an improved and intentional version of the laserbeam distortion that was happening with the sawtooth wavetable and high cutoff frequency values: freq, attack, release, cutoff
 *              3) third mode will be an FM synth using the cosine wavetables: carrier freq, mod freq, mod depth, release
 *              4) fourth mode is white noise: attack, release, reverb delay, reverb feedback
*/

#include <MozziGuts.h>
#include <Oscil.h>
#include <AutoMap.h>
#include <ADSR.h>
#include <LowPassFilter.h>
#include <tables/saw2048_int8.h>
#include <tables/cos2048_int8.h>
#include <tables/whitenoise8192_int8.h>
#include <tables/pinknoise8192_int8.h>



//PINS
const byte KNOB_PINS[2][2] = {
  {A0, A1}, 
  {A2, A3}
};
const byte TRIGGER_PINS[2] = {2, 3};
const byte MODE_PINS[2] = {8, 7};



//GLOBAL VARIABLES
bool triggers[2] = {0, 0};

byte modes[2][3] = { // { mode, debounce value, counter value } 
  {0, 40, 0},
  {0, 40, 0}
};

byte pink_to_white_ratio = 0;
int counter = 0;
int print_stats = 100;

Oscil <2048, AUDIO_RATE> carriers[2];
Oscil <2048, AUDIO_RATE> modulators[2];
Oscil <2048, AUDIO_RATE> lasers[2];
Oscil <8192, AUDIO_RATE> whitenoise(WHITENOISE8192_DATA);
Oscil <8192, AUDIO_RATE> pinknoise(PINKNOISE8192_DATA);

ADSR <AUDIO_RATE, AUDIO_RATE> envelopes[2];
LowPassFilter lpfs[2];



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

void read_mode(byte n) {
  if (digitalRead(MODE_PINS[n]) == HIGH and modes[n][2] > modes[n][1]) {
    modes[n][0]++;
    modes[n][0] %= 3;
    modes[n][2] = 0;
  }
  else {
    modes[n][2]++;
  }
}

void update_voice_control(byte n, byte m) {
  int x;
  int y;
  x = mozziAnalogRead(KNOB_PINS[n][1]) >> 1;
  envelopes[n].setDecayTime(20 + x);
  x = mozziAnalogRead(KNOB_PINS[n][0]) >> 2;
  
  carriers[n].setFreq(x);
  y = (x%12) + 1;
  modulators[n].setFreq((y*x) >> 2);
  
  pink_to_white_ratio = x;

  lpfs[n].setCutoffFreq(x);
}

int update_voice_audio(byte n, byte m) {
  envelopes[n].update();
  int synth;
  if (m==0) {
    synth = (carriers[n].next() * modulators[n].next()) >> 8;
  }
  else if (m==1) {
    synth = (((whitenoise.next() * (255-pink_to_white_ratio)) >> 8) + ((pinknoise.next() * pink_to_white_ratio) >> 8)) >> 1;
  }
  else if (m==2){
    synth = lpfs[n].next(lasers[n].next());
  }
  return (synth * envelopes[n].next()) >> 2;
}


void setup(){
  //Serial.begin(115200);
  startMozzi();
  pinMode(KNOB_PINS[0][0], INPUT);
  pinMode(KNOB_PINS[0][1], INPUT);
  pinMode(KNOB_PINS[1][0], INPUT);
  pinMode(KNOB_PINS[1][1], INPUT);
  pinMode(TRIGGER_PINS[0], INPUT);
  pinMode(TRIGGER_PINS[1], INPUT);
  pinMode(MODE_PINS[0], INPUT);
  pinMode(MODE_PINS[1], INPUT);

  whitenoise.setFreq(55);
  pinknoise.setFreq(55);
  modulators[0].setTable(COS2048_DATA);
  modulators[1].setTable(COS2048_DATA);
  carriers[0].setTable(COS2048_DATA);
  carriers[1].setTable(COS2048_DATA);
  lasers[0].setTable(SAW2048_DATA);
  lasers[1].setTable(SAW2048_DATA);
  lasers[0].setFreq(100);
  lasers[1].setFreq(100);
  envelopes[0].setLevels(255, 0, 0, 0);
  envelopes[0].setTimes(20, 20, 20, 20);
  envelopes[1].setLevels(255, 0, 0, 0);
  envelopes[1].setTimes(20, 20, 20, 20);
  lpfs[0].setResonance(200);
  lpfs[1].setResonance(200);
}

void updateControl(){
  read_trigger(0);
  read_trigger(1);
  
  read_mode(0);
  read_mode(1);
  
  update_voice_control(0, modes[0][0]);
  update_voice_control(1, modes[1][0]);
  /*
  if (counter >= print_stats) {
    Serial.print("Mode 1:" );
    Serial.println(modes[0][0]);
    Serial.print("Mode 2:" );
    Serial.println(modes[1][0]);
    Serial.println();
    counter = 0;
  }
  counter++;
  */
}

int updateAudio(){
  int audio_1, audio_2;
  audio_1 = update_voice_audio(0, modes[0][0]);
  audio_2 = update_voice_audio(1, modes[1][0]);
  
  return (audio_1 + audio_2) >> 1;
}

void loop(){
  audioHook();
}
