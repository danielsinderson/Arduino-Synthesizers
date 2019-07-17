//necessary Mozzi functions
#include <MozziGuts.h>
#include <Oscil.h>
#include <Smooth.h>
#include <AutoMap.h>
//wavetables (all same size for switching function)
#include <tables/sin512_int8.h>
#include <tables/saw512_int8.h>
#include <tables/square_no_alias512_int8.h>
#include <tables/triangle512_int8.h>
#include <tables/handmade_1_512.h>
#include <tables/handmade_2_512.h>
#include <tables/handmade_4_512.h>
#include <tables/handmade_5_512.h>


// desired carrier frequency max and min, for AutoMap
const int MIN_CARRIER_FREQ = 22;
const int MAX_CARRIER_FREQ = 262;

// desired intensity max and min, for AutoMap, note they're inverted for reverse dynamics
const int MIN_INTENSITY = 700;
const int MAX_INTENSITY = 10;

// desired mod speed max and min, for AutoMap, note they're inverted for reverse dynamics
const int MIN_MOD_SPEED = 10000;
const int MAX_MOD_SPEED = 1;

//automap functions to more quickly map ADC inputs to desired values
AutoMap kMapCarrierFreq(0,1023,MIN_CARRIER_FREQ,MAX_CARRIER_FREQ);
AutoMap kMapIntensity(0,1023,MIN_INTENSITY,MAX_INTENSITY);
AutoMap kMapModSpeed(0,1023,MIN_MOD_SPEED,MAX_MOD_SPEED);

//PINS!
const int CARRIER_PIN_1 = 0;
const int MODULATOR_PIN_1 = 1;
const int INTENSITY_PIN_1 = 2;
const int CARRIER_PIN_2 = 3;
const int MODULATOR_PIN_2 = 4;
const int INTENSITY_PIN_2 = 5;
const int WF_PIN_1A = 8; //aCarrier_1
const int WF_PIN_1B = 7; //aCarrier_1
const int WF_PIN_2A = 6; //aCarrier_2
const int WF_PIN_2B = 5; //aCarrier_2
const int WF_PIN_3A = 4; //aMod_1
const int WF_PIN_4A = 3; //aMod_2

//Oscillator objects
Oscil<512, AUDIO_RATE> aCarrier_1;
Oscil<512, AUDIO_RATE> aModulator_1;
Oscil<512, CONTROL_RATE> kIntensityMod_1;
Oscil<512, AUDIO_RATE> aCarrier_2;
Oscil<512, AUDIO_RATE> aModulator_2;
Oscil<512, CONTROL_RATE> kIntensityMod_2;

// for controlling table swaps
boolean D_1A; 
boolean D_1B; 
boolean D_2A; 
boolean D_2B;
boolean D_3A; 
boolean D_4A;


int mod_ratio = 5; // brightness (harmonics)
long fm_intensity_1; // carries control info from updateControl to updateAudio
long fm_intensity_2;


float smoothness = 0.95f;
Smooth <long> aSmoothIntensity(smoothness);


void setup() {
  
  startMozzi();
}


void updateControl(){
  //read digital inputs to set wavetables
  D_1A = digitalRead(WF_PIN_1A); //aCarrier_1
  D_1B = digitalRead(WF_PIN_1B); //aCarrier_1
  D_2A = digitalRead(WF_PIN_2A); //aCarrier_2
  D_2B = digitalRead(WF_PIN_2B); //aCarrier_2
  D_3A = digitalRead(WF_PIN_3A); //aMod_1
  D_4A = digitalRead(WF_PIN_4A); //aMod_2
  //aCarrier_1
  if (D_1A == HIGH and D_1B == HIGH) {aCarrier_1.setTable(SIN512_DATA);}
  else if (D_1A == HIGH and D_1B == LOW) {aCarrier_1.setTable(SAW512_DATA);}
  else if (D_1A == LOW and D_1B == HIGH) {aCarrier_1.setTable(SQUARE_NO_ALIAS512_DATA);}
  else if (D_1A == LOW and D_1B == LOW) {aCarrier_1.setTable(TRIANGLE512_DATA);}
  //aCarrier_2
  if (D_2A == HIGH and D_2B == HIGH) {aCarrier_2.setTable(SIN512_DATA);}
  else if (D_2A == HIGH and D_2B == LOW) {aCarrier_2.setTable(SAW512_DATA);}
  else if (D_2A == LOW and D_2B == HIGH) {aCarrier_2.setTable(SQUARE_NO_ALIAS512_DATA);}
  else if (D_2A == LOW and D_2B == LOW) {aCarrier_2.setTable(TRIANGLE512_DATA);}
  //aModulator_1 and kIntensityMod_1
  if (D_3A == HIGH) {
    aModulator_1.setTable(HANDMADE_1_512_DATA);
    kIntensityMod_1.setTable(HANDMADE_1_512_DATA);
  }
  else if (D_3A == LOW) {
    aModulator_1.setTable(HANDMADE_2_512_DATA);
    kIntensityMod_1.setTable(HANDMADE_2_512_DATA);
  }
  //aModulator_2 and kIntensityMod_2
  if (D_4A == HIGH) {
    aModulator_2.setTable(HANDMADE_4_512_DATA);
    kIntensityMod_2.setTable(HANDMADE_4_512_DATA);
  }
  else if (D_4A == LOW) {
    aModulator_2.setTable(HANDMADE_5_512_DATA);
    kIntensityMod_2.setTable(HANDMADE_5_512_DATA);
  }
  

  
  // read the knob
  int carrier_value_1 = mozziAnalogRead(CARRIER_PIN_1); // value is 0-1023
  // map the knob to carrier frequency
  int carrier_freq_1 = kMapCarrierFreq(carrier_value_1);
  //calculate the modulation frequency to stay in ratio
  int mod_freq_1 = carrier_freq_1 * mod_ratio;
  // set the FM oscillator frequencies
  aCarrier_1.setFreq(carrier_freq_1); 
  aModulator_1.setFreq(mod_freq_1);
  // read the light dependent resistor on the width Analog input pin
  int intensity_value_1= mozziAnalogRead(INTENSITY_PIN_1); // value is 0-1024
  int intensity_calibrated_1 = kMapIntensity(intensity_value_1);
 // calculate the fm_intensity
  fm_intensity_1 = ((long)intensity_calibrated_1 * (kIntensityMod_1.next()+128))>>8; // shift back to range after 8 bit multiply
  // read the light dependent resistor on the speed Analog input pin
  int modulator_value_1 = mozziAnalogRead(MODULATOR_PIN_1); // value is 0-1023
  // use a float here for low frequencies
  float mod_speed_1 = (float)kMapModSpeed(modulator_value_1)/1000;
  kIntensityMod_1.setFreq(mod_speed_1);


  int carrier_value_2 = mozziAnalogRead(CARRIER_PIN_2);
  int carrier_freq_2 = kMapCarrierFreq(carrier_value_2);
  int mod_freq_2 = carrier_freq_2 * mod_ratio;
  aCarrier_2.setFreq(carrier_freq_2); 
  aModulator_2.setFreq(mod_freq_2);
  int intensity_value_2= mozziAnalogRead(INTENSITY_PIN_2);
  int intensity_calibrated_2 = kMapIntensity(intensity_value_2);
  fm_intensity_2 = ((long)intensity_calibrated_2 * (kIntensityMod_2.next()+128))>>8;
  int modulator_value_2 = mozziAnalogRead(MODULATOR_PIN_2);
  float mod_speed_2 = (float)kMapModSpeed(modulator_value_2)/1000;
  kIntensityMod_2.setFreq(mod_speed_2);
}


int updateAudio(){
  long modulation_1 = aSmoothIntensity.next(fm_intensity_1) * aModulator_1.next();
  long modulation_2 = aSmoothIntensity.next(fm_intensity_2) * aModulator_2.next();
  return (aCarrier_1.phMod(modulation_1) + aCarrier_2.phMod(modulation_2)) >> 1;
}


void loop() {
  audioHook();
}
