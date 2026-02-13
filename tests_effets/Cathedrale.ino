#include <Audio.h>
#include <Wire.h>
#include <SPI.h>

// --- Composants ---
AudioInputI2S            micInput;
AudioMixer4              mixer1;
AudioEffectDelay         delay1;
AudioFilterStateVariable filter1;
AudioEffectReverb        reverb1;
AudioOutputI2S           audioOutput;

// --- Connexions ---
AudioConnection          patch1(micInput, 0, mixer1, 0);    // Micro vers mix (canal 0)
AudioConnection          patch2(mixer1, 0, delay1, 0);     // Mix vers délai
AudioConnection          patch3(delay1, 0, filter1, 0);    // Sortie délai vers filtre
AudioConnection          patch4(filter1, 0, mixer1, 1);    // Sortie filtre vers feedback (canal 1)
AudioConnection          patch5(mixer1, 0, reverb1, 0);    // Mix total vers réverb
AudioConnection          patch6(reverb1, 0, audioOutput, 0);
AudioConnection          patch7(reverb1, 0, audioOutput, 1);

AudioControlSGTL5000     sgtl5000_1;

const int potPin = A0;

void setup() {
  AudioMemory(180); // On augmente encore pour la réverb + délai

  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.volume(0.5);
  sgtl5000_1.micGain(25);

  // --- Réglages précis du feedback ---
  mixer1.gain(0, 1.0);  // Volume direct
  mixer1.gain(1, 0.45); // Baisse de l'echo

  // --- Réglage Réverb ---
  reverb1.reverbTime(0.5); // Temps de réverbération court pour lisser le son

  filter1.frequency(1000); 
  filter1.resonance(1.5);
}

void loop() {
  int potValue = analogRead(potPin);

  // 1. Réglage du temps de l'écho (50ms à 500ms)
  int delayTime = map(potValue, 0, 1023, 50, 500);
  delay1.delay(0, delayTime);

  // 2. Réglage de la profondeur de la réverb (0.0 à 1.0)
  // On utilise enfin revSize !
  float revSize = (float)potValue / 1023.0;
  reverb1.reverbTime(revSize); 

  // Optionnel : Affichage pour voir ce qui se passe
  if (potValue % 100 == 0) { // N'affiche pas à chaque cycle pour ne pas ralentir
    Serial.print("Echo: "); Serial.print(delayTime);
    Serial.print("ms | Reverb: "); Serial.println(revSize);
  }

  delay(20);
}