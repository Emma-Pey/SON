#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include "AudioEffectVarispeed.h"

const int myInput = AUDIO_INPUT_MIC;  

AudioInputI2S          audioInput;
AudioEffectVarispeed   varispeed;
AudioOutputI2S         audioOutput;

AudioConnection patchCord1(audioInput, 0, varispeed, 0);
AudioConnection patchCord2(varispeed, 0, audioOutput, 0);
AudioConnection patchCord3(varispeed, 0, audioOutput, 1);

AudioControlSGTL5000 sgtl5000_1;

void setup() {
  Serial.begin(115200);
  delay(200);

  AudioMemory(80);

  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);

  if (myInput == AUDIO_INPUT_MIC) {
    sgtl5000_1.micGain(50);   // ajuste 20..55 selon ton micro
  }

  pinMode(A0, INPUT);
}

void loop() {
  int v = analogRead(A0);

  // IMPORTANT: division float
  // Accélération seulement : 1.0 -> 1.5 (recommandé en live mic)
  float speed = 1.0f + (v / 1023.0f) * 0.5f;

  // Si tu veux tenter plus fort (avec + de sauts possibles), remplace par:
  // float speed = 1.0f + (v / 1023.0f) * 1.5f; // 1.0 -> 2.5

  varispeed.setSpeed(speed);

  // Debug
  Serial.print("A0=");
  Serial.print(v);
  Serial.print("  speed=");
  Serial.println(speed, 3);

  delay(10);
}

