#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// --- Chaîne Audio Simple ---
AudioPlaySdWav           playWav1;
AudioOutputI2S           audioOutput;

// Connexion directe du lecteur SD vers les sorties Gauche et Droite
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);

AudioControlSGTL5000     sgtl5000_1;

const int buttonPin = 0;
bool lastButtonState = LOW; 

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT); // Assure-toi d'avoir ta résistance de tirage vers le bas (Pull-down)

  AudioMemory(8); // Pas besoin de beaucoup de mémoire sans écho
  
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6);

  // Initialisation SD (Shield Audio Rev D)
  SPI.setMOSI(7);
  SPI.setSCK(14);
  if (!(SD.begin(10))) {
    Serial.println("Erreur : Carte SD non détectée");
  }
}

void loop() {
  bool currentButtonState = digitalRead(buttonPin);

  // Détection du front montant (le moment où on appuie)
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    Serial.println("Lecture du son...");
    playWav1.play("SON.WAV");
    
    delay(50); // Anti-rebond
  }

  lastButtonState = currentButtonState;
}