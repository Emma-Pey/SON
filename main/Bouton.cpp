#include "Bouton.h"

Bouton::Bouton(int number, int buttonPin)
: num(number), pin(buttonPin)
{
    state = LOW;
    lastState = LOW;
    pressStartTime = 0;
    longPressTriggered = false;
    gain = 1.0;

    // Création dynamique des connexions
    //patchCord1 = new AudioConnection(playRaw, pitch);
    //patchCord2 = new AudioConnection(pitch, noise);
    //patchCord3 = new AudioConnection(noise, bitcrusher);

    //patchCord3 = new AudioConnection(noise, varispeed);
    //patchCord4 = new AudioConnection(varispeed, reverb);
    //patchCord5 = new AudioConnection(reverb, bitcrusher);

    patchCord7 = new AudioConnection(mixer1, 0, delay1, 0);
    patchCord8 = new AudioConnection(delay1, 0, filter1, 0);
    patchCord9 = new AudioConnection(filter1, 0, mixer1, 1);
    patchCord10 = new AudioConnection(mixer1, 0, reverb1, 0);


}

void Bouton::begin() {
    pinMode(pin, INPUT);
    sprintf(filename, "BUTTON%d.RAW", num); //créer le nom du fichier

    //initialisation pour le noise gate, il faut mettre les autres aussi ?
    noise.gate(
        -40.0f,   // threshold plus bas
        0.8f,    // attack 80 ms
        0.8f,     // release 800 ms
        12.0f     // hysteresis large
    );
    //sans les trucs juste après, ça clique
    noise.compression(0.0f);      // désactive comp
    noise.limit(0.0f);            // désactive limiter
    noise.makeupGain(0.0f);       // pas d'auto gain

    bitcrusher.sampleRate(44100);
    bitcrusher.bits(16);

    // --- Réglages précis du feedback ---
    mixer1.gain(0, 1.0);  // Volume direct
    mixer1.gain(1, 0.45); // Baisse de l'echo

    // --- Réglage Réverb ---
    reverb1.reverbTime(0.5); // Temps de réverbération court pour lisser le son

    filter1.frequency(1000); 
    filter1.resonance(1.5);
}

void Bouton::update() {//pas utile ?

    state = digitalRead(pin);

    if (state == LOW && lastState == HIGH) {
        pressStartTime = millis();
        longPressTriggered = false;
    }

    if (state == LOW && !longPressTriggered) {
        if (millis() - pressStartTime > 800) {
            longPressTriggered = true;
            // action long press
        }
    }

    lastState = state;
}

void Bouton::play() {
  playRaw.stop(); // on l'arrête
  playRaw.play(filename); // on le relance
}


// Effets
void Bouton::setGain(float g) {
    gain = constrain(g, 0.0, 1.0);
    //mixerDryWet.gain(0, gain);
}


void Bouton::nextEffect() {
  currentEffect++;
  if (currentEffect >= EFFECT_COUNT) currentEffect = 0;
}

void Bouton::setEffectAmount(float value) {//value entre 0 et 1023
    switch(currentEffect) {
        case EFFECT_PITCH:
            pitch.setParamValue("shift (semitones)", value*24/1023-12); // -12 - 12
            break;

        /// à ajouter après (modifier les calculs pour notre cas):

        case EFFECT_NOISE:
            //noise.setParamValue("Threshold",value*120/1023-120.0); // exemple
            noise.gate(value*120/1023-120.0);
            //Serial.println(value*120/1023-120.0);
            break;

    //     case EFFECT_VARISPEED:
    //       varispeed.playbackRate(value * 2.0f); // 0..2
    //       break;

         case EFFECT_REVERB:
            // 1. Réglage du temps de l'écho (50ms à 500ms)
            int delayTime = map(value, 0, 1023, 50, 500);
            delay1.delay(0, delayTime);

            // 2. Réglage de la profondeur de la réverb (0.0 à 1.0)
            // On utilise enfin revSize !
            float revSize = (float)value / 1023.0;
            reverb1.reverbTime(revSize); 

            break;

        case EFFECT_BITCRUSHER:
            int rate = map(value, 0, 1023, 44100, 3000);
            bitcrusher.sampleRate(rate);
            int bits = map(value, 0, 1023, 16, 8);
            bitcrusher.bits(bits);
            break;
        
        
    }
}

const char* Bouton::getEffectName(){
    static const char* names[] = {
      "PITCH", "NOISE", "BITCRUSHER"// ,"VARISPEED", "REVERB", 
    };
    return names[currentEffect];
}


