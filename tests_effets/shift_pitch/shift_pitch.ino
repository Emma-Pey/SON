
// le programme joue en direct le son shifté en fonction de la valeur lue sur le potentiomètre A0

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "Pitch.h" //généré par Faust

const int myInput = AUDIO_INPUT_MIC;

AudioInputI2S          audioInput;         // audio shield: mic or line-in
AudioOutputI2S         audioOutput;        // audio shield: headphones & line-out


Pitch pitch;
AudioMixer4              mixer1;         //xy=532,205
AudioOutputI2S           i2s1;           //xy=611,61
AudioConnection          patchCord2(audioInput, pitch);
AudioConnection          patchCord4(pitch, 0, mixer1, 0); //delay à la place de e
AudioConnection          patchCord5(pitch, 1, mixer1, 1);
AudioConnection          patchCord6(pitch, 2, mixer1, 2);
AudioConnection          patchCord7(pitch, 3, mixer1, 3);
AudioConnection          patchCord8(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=195,272

void setup() {
  // allocate enough memory for the delay
  AudioMemory(120);
  
  // enable the audio shield
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);
  sgtl5000_1.micGain(50);

  pitch.setParamValue("shift (semitones)", 10);
}

void loop() {
  int pitchValue = analogRead(A0);
  pitch.setParamValue("shift (semitones)", pitchValue*24/1023-12); //entre 12 et -12
}
