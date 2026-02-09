#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

AudioPlaySdWav           playWav1;
AudioOutputI2S           audioOutput;
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;

const int buttonPin = 0; 

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT); // Toujours avec ta résistance externe

  AudioMemory(12);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6); 

  SPI.setMOSI(7);
  SPI.setSCK(14);
  if (!(SD.begin(10))) {
    Serial.println("Erreur : Carte SD non détectée");
  }
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH) {
    // LE BOUTON EST APPUYÉ
    if (playWav1.isPlaying() == false) {
      Serial.println("Début de lecture");
      playWav1.play("1.WAV");
    }
  } 
  else {
    // LE BOUTON EST RELÂCHÉ
    if (playWav1.isPlaying() == true) {
      Serial.println("Arrêt du son");
      playWav1.stop(); 
    }
  }
  
  // Très court délai pour la stabilité de lecture des boutons
  delay(10); 
}