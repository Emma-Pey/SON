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