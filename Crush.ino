#include <Audio.h>
#include <Wire.h>
#include <SPI.h>

// --- Chaîne Audio ---
AudioInputI2S            micInput;
AudioEffectBitcrusher    bitcrusher1;
AudioOutputI2S           audioOutput;

AudioConnection          patchCord1(micInput, 0, bitcrusher1, 0);
AudioConnection          patchCord2(bitcrusher1, 0, audioOutput, 0);
AudioConnection          patchCord3(bitcrusher1, 0, audioOutput, 1);

AudioControlSGTL5000     sgtl5000_1;

// --- Broche du potentiomètre ---
const int potPin = A0; 

void setup() {
  Serial.begin(9600);
  AudioMemory(12);

  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.volume(0.5);
  sgtl5000_1.micGain(25);

  // Valeurs de départ
  bitcrusher1.bits(16);
  bitcrusher1.sampleRate(44100);
}

void loop() {
  // Lecture du potentiomètre (0 à 1023)
  int potValue = analogRead(potPin);

  // --- 1. Réglage du Sample Rate (Fréquence) ---
  // On passe de 44100 Hz (normal) à 3000 Hz (écrasé)
  int rate = map(potValue, 0, 1023, 44100, 3000);
  bitcrusher1.sampleRate(rate);

  // --- 2. Réglage des Bits (Résolution) ---
  // On passe de 16 bits (propre) à 8 bits (typique crush)
  // map(valeur, entree_min, entree_max, sortie_min, sortie_max)
  int bits = map(potValue, 0, 1023, 16, 8);
  bitcrusher1.bits(bits);

  // Affichage pour le debug
  Serial.print("Rate: "); Serial.print(rate);
  Serial.print(" | Bits: "); Serial.println(bits);

  delay(20); // Petite pause pour la stabilité
}