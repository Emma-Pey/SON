#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include "AudioEffectVarispeed.h"

const int myInput = AUDIO_INPUT_MIC;   // ou AUDIO_INPUT_LINEIN

AudioInputI2S          audioInput;
AudioEffectVarispeed   accel;
AudioOutputI2S         audioOutput;

AudioConnection patchCord1(audioInput, 0, accel, 0);
AudioConnection patchCord2(accel, 0, audioOutput, 0);
AudioConnection patchCord3(accel, 0, audioOutput, 1);

AudioControlSGTL5000 sgtl5000_1;

void setup() {
  Serial.begin(115200);     // <-- indispensable pour Serial.println
  delay(200);

  AudioMemory(80);

  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);

  if (myInput == AUDIO_INPUT_MIC) {
    sgtl5000_1.micGain(50); // 0..63
  }

  pinMode(A0, INPUT);
}

void loop() {
  int v = analogRead(A0);

  // Accélération seulement : de 1.0 à 2.5
  float speed = 1.0f + (v / 1023.0f) * 1.5f;

  accel.setSpeed(speed);

  Serial.println(v);   // <-- correction

  delay(5);
}
