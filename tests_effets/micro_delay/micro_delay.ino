// le programme joue en direct le son lu avec un écho
// crée 3 échos
// AudioEffectDelayExternal à utiliser si on veut des échos à l'infini

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


const int myInput = AUDIO_INPUT_MIC;

AudioInputI2S          audioInput;         // audio shield: mic or line-in
AudioOutputI2S         audioOutput;        // audio shield: headphones & line-out


AudioEffectDelay         delay1;         //xy=393,238
AudioMixer4              mixer1;         //xy=532,205
AudioOutputI2S           i2s1;           //xy=611,61
AudioConnection          patchCord1(audioInput, delay1);
AudioConnection          patchCord4(delay1, 0, mixer1, 0);
AudioConnection          patchCord5(delay1, 1, mixer1, 1);
AudioConnection          patchCord6(delay1, 2, mixer1, 2);
AudioConnection          patchCord7(delay1, 3, mixer1, 3);
AudioConnection          patchCord8(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=195,272
// GUItool: end automatically generated code

void setup() {
  // allocate enough memory for the delay
  AudioMemory(120);
  
  // enable the audio shield
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  
  // configure a sine wave for the chirp
  // the original is turned on/off by an envelope effect
  // and output directly on the left channel
  //sine1.frequency(1000);
  //sine1.amplitude(0.5);

  // create 3 delay taps, which connect through a
  // mixer to the right channel output
  delay1.delay(0, 110);
  delay1.delay(1, 220);
  delay1.delay(2, 330);
}

void loop() {
}
