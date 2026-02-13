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
    patchCord1 = new AudioConnection(playRaw, pitch);
    patchCord2 = new AudioConnection(pitch, noise);

    //patchCord3 = new AudioConnection(noise, varispeed);
    //patchCord4 = new AudioConnection(varispeed, reverb);
    //patchCord5 = new AudioConnection(reverb, bitcrusher);
    //patchCord6 = new AudioConnection(bitcrusher, mixerDryWet, 0);
}

void Bouton::begin() {
    pinMode(pin, INPUT);
    sprintf(filename, "BUTTON%d.RAW", num); //créer le nom du fichier

    //initialisation
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

    //     case EFFECT_REVERB:
    //       reverb.setRoomSize(value); // 0..1
    //       break;

    //     case EFFECT_BITCRUSHER:
    //       bitcrusher.bits(16 - value * 12); // 16..4 bits
    //       break;
    //   
    }
}

const char* Bouton::getEffectName(){
    static const char* names[] = {
      "PITCH", "NOISE", "VARISPEED", "REVERB", "BITCRUSHER"
    };
    return names[currentEffect];
}


