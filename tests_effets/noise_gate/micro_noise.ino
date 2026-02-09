//A TESTER 
// le programme joue en direct le son tordu en fonction de la valeur lue sur le potentiomètre A0
//
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "noise_gate.h" //généré par Faust

const int myInput = AUDIO_INPUT_MIC;

AudioInputI2S          audioInput;         // audio shield: mic or line-in
AudioOutputI2S         audioOutput;        // audio shield: headphones & line-out


noise_gate d;
AudioOutputI2S           i2s1;           //xy=611,61
AudioConnection          patchCord2(audioInput, d);
AudioConnection          patchCord8(d, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=195,272

void setup() {
  Serial.begin(9600);
  // allocate enough memory for the delay
  AudioMemory(120);
  
  // enable the audio shield
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);
  sgtl5000_1.micGain(50);

  d.setParamValue("Threshold", -20);

}

void loop() {
  int driveValue = analogRead(A0);
  Serial.println(driveValue);
  d.setParamValue("Threshold", driveValue*120/1023-120.0); //entre -20 et -50
  delay(100);
}
