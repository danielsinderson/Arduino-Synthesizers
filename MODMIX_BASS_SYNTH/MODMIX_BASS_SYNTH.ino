/*
 * ModMix Bass Synth
 */


/*PINS!*/
const byte FREQ_PIN = 14;
const byte AMP_PIN = 15;
const byte WF_PIN = 16;
const byte CHANNEL_PIN = 17;
const byte TOGGLE_PIN = 18;
const byte BITCRUSH_TOGGLE_PIN = 19;
const byte BITCRUSH_VALUE_PIN = 20;
const byte REVERB_TOGGLE_PIN = 21;
const byte REVERB_VALUE_PIN = 22;
const byte OUTPUT_FREQ_PIN = 23;


const byte MUX_SELECT_1 = 2;
const byte MUX_SELECT_2 = 3;
const byte MUX_SELECT_3 = 4;

const byte OUTPUT_PIN = 5;


/*GLOBAL VARIABLES!*/
#define PI 3.14159

bool update_settings_flag = 0;


bool operator_toggles[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint32_t operator_waveform[8];
uint32_t operator_frequency[8];
uint32_t operator_amplitude[8];
uint32_t operator_channel[8];

#define wavetable_size 256
uint8_t wavetable_size_log = 8;
uint16_t wavetable[wavetable_size];

bool bitcrush_toggle = 0;
uint32_t bitcrush_value = 0;

bool reverb_toggle = 0;
uint32_t reverb_value = 0;

uint32_t output_frequency;
uint32_t playback_counter = 0;
uint32_t playback_rate = 20000;
uint32_t playback_index = 0;

uint32_t mod_mix_value = 0;
uint32_t output_value = 0;

float frequency_table[64] = {23.12, 24.50, 25.96, 27.50, 29.14, 30.87,
               32.70, 34.65, 36.71, 38.89, 41.20, 43.65, 46.25, 49.00, 51.91, 55.00, 58.27, 61.74,
               65.41, 69.30, 73.42, 77.78, 82.41, 87.31, 92.50, 98.00, 103.8, 110.0, 116.5, 123.5,
               130.8, 138.6, 146.8, 155.6, 164.8, 174.6, 185.0, 196.0, 207.7, 220.0, 233.1, 246.9,
               261.6, 277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0, 466.2, 493.9,
               523.3, 554.4, 587.3, 622.3, 659.3, 698.5, 740.0, 784.0, 830.6, 880.0};
uint32_t playback_rates[128];



/*TIMERS!*/
IntervalTimer settingsUpdate;
IntervalTimer audioUpdate;

void set_update_flag() {
  update_settings_flag = 1;
}

void inc_playback_index() {
  playback_counter++;
}


/*FUNCTIONS!*/
uint8_t sine(uint32_t wavetable_position, uint32_t frequency, uint32_t amplitude){
  uint32_t t = 2*frequency*wavetable_position;
  uint32_t x = (PI*t);
  x = x >> wavetable_size_log;
  uint32_t value = (sin(x)+1) * 255;
  uint8_t result = value >> amplitude;
  return result;
}

uint8_t triangle(uint32_t wavetable_position, uint32_t frequency, uint32_t amplitude){
  uint8_t result;
  uint32_t slope = (256 / wavetable_size) / frequency;
  uint32_t waveform_position = (wavetable_position * frequency) % wavetable_size;
  if (waveform_position < (wavetable_size >> 1)){
    result = slope * waveform_position;
  }
  else{
    result = (1/slope) * waveform_position;
  }
  result = result >> amplitude;
  return result;
}

uint8_t square(uint32_t wavetable_position, uint32_t frequency, uint32_t amplitude){
  uint8_t result;
  double x = (wavetable_position*frequency) % wavetable_size;
  if (x < (wavetable_size >> 1)) {result = 0;}
  else {result = 255;}
  result = result >> amplitude;
  return result;
}

uint8_t saw(uint32_t wavetable_position, uint32_t frequency, uint32_t amplitude){
  uint8_t result;
  uint32_t slope = (256 / wavetable_size) / frequency;
  uint32_t waveform_position = (wavetable_position * frequency) % wavetable_size;
  result = slope * waveform_position;
  result = result >> amplitude;
  return result;
}

uint8_t get_waveform_value(uint32_t waveform, uint32_t wavetable_position, uint32_t frequency, uint32_t amplitude){
  uint8_t result;
  if (waveform == 0) { result = sine(wavetable_position, frequency, amplitude); }
  else if (waveform == 1) { result = triangle(wavetable_position, frequency, amplitude); }
  else if (waveform == 2) { result = square(wavetable_position, frequency, amplitude); }
  else if (waveform == 3) { result = saw(wavetable_position, frequency, amplitude); }
  return result;
}

uint32_t mod_mix_block(uint32_t index){ //returns 16-bit value
  //block to iterate through operators and sum up values for each channel at the current playback index (which is the position in the waveform)
  uint32_t channel_waveforms[8];
  for (int i=0; i<8; i++){
    if (operator_toggles[i] == 1) {
      bool bitshift;
      if (channel_waveforms[operator_channel[i]] != 0) {bitshift = 1;}
      channel_waveforms[operator_channel[i]] += get_waveform_value(operator_waveform[i], playback_index, operator_frequency[i], operator_amplitude[i]);;
      if (bitshift == 1) { channel_waveforms[operator_channel[i]] = channel_waveforms[operator_channel[i]] >> 1; }
    }
  }

  //block to iterate through channel waveforms and multiply them together for final audio output value, skipping channels with no operators on them (value 0)
  uint64_t value = 1;
  int bitshifts = -12;
  for (int i=0; i<8; i++){
    if (channel_waveforms[i] != 0) {
      value *= channel_waveforms[i];
      bitshifts += 8;
    }
  }
  return value >> bitshifts;
}

uint32_t fx_block(uint32_t input_value){
  uint32_t result = input_value;
  if (bitcrush_toggle == 1) {
    result = result >> bitcrush_value;
    result = result << bitcrush_value;
  }
  if (reverb_toggle == 1){
    uint32_t samples_per_millisecond = 2.048 * output_frequency;
    uint32_t samples_of_delay = (reverb_value * samples_per_millisecond) % wavetable_size;
    uint32_t samples_to_add = wavetable_size - samples_of_delay;
    uint32_t reverb_index = (playback_index + samples_to_add) % wavetable_size;
    uint32_t wet_signal = mod_mix_block(reverb_index);
    result += wet_signal;
    result = result >> 1;
  }
  return result;
}

void set_mux_select_lines(char line_number){ //set MUX select lines based on desired line number (0 to 7)
  bool select3 = line_number / 4; //returns 1 if line_number is >=4, setting the MSB of the select lines
  bool select2 = (line_number-4*select3) / 2; //returns 1 if line_number is 2, 3, 6, or 7
  bool select1 = line_number % 2; //returns 1 if line number is odd
  digitalWrite(MUX_SELECT_3, select3);
  digitalWrite(MUX_SELECT_2, select2);
  digitalWrite(MUX_SELECT_1, select1);
}

void update_operator_settings(void){ //set MUX select lines and read operator settings from associated pin for each of the 8 operators
  for (int i=0; i<8; i++){
    set_mux_select_lines(i);
    operator_toggles[i] = digitalRead(TOGGLE_PIN);
    operator_waveform[i] = analogRead(WF_PIN) >> 7; //values between 0 and 7
    operator_frequency[i] = analogRead(FREQ_PIN) >> 5; //values between 0 and 31
    operator_amplitude[i] = analogRead(AMP_PIN) >> 7; //values between 0 and 7
    operator_channel[i] = analogRead(CHANNEL_PIN) >> 7; //values between 0 and 7
  }
}

void update_fx_and_playback_settings(void){
  bitcrush_toggle = digitalRead(BITCRUSH_TOGGLE_PIN);
  reverb_toggle = digitalRead(REVERB_TOGGLE_PIN);
  bitcrush_value = analogRead(BITCRUSH_VALUE_PIN) >> 7; //values between 0 and 7
  reverb_value = analogRead(REVERB_VALUE_PIN) >> 4; //values between 0 and 63
  output_frequency = analogRead(OUTPUT_FREQ_PIN) >> 3; //values between 0 and 127
  playback_rate = playback_rates[output_frequency];
}

void update_user_settings(void){
  update_operator_settings();
  update_fx_and_playback_settings();
}


void setup() {
  for (int i=0;i<63;i++){
    float freq = frequency_table[i];
    playback_rates[2*i] = 976563 / (wavetable_size * freq);
    freq = (frequency_table[i] + frequency_table[i+1]) / 2;
    playback_rates[(2*i)+1] = 976563 / (wavetable_size * freq);
  }
  playback_rates[126] = 976563 / (wavetable_size * frequency_table[63]);
  playback_rates[127] = 976563 / (wavetable_size * frequency_table[63]);

  update_user_settings();

  settingsUpdate.begin(set_update_flag, 5000);
  audioUpdate.begin(inc_playback_index, 9.6);
}

void loop() {
  if (update_settings_flag == 1){
      update_user_settings();
      update_settings_flag = 0;
    }
  if (playback_counter >= playback_rate){
    playback_index++;
    if (playback_index == wavetable_size) { playback_index = 0; }
    playback_counter = 0;
    mod_mix_value = mod_mix_block(playback_index);
    output_value = fx_block(mod_mix_value);
  }
  digitalWrite(OUTPUT_PIN, output_value);
}
