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
    patchCord4 = new AudioConnection(noise, mixer1);
    //patchCord5 = new AudioConnection(varispeed, mixer1); 

    patchCord7 = new AudioConnection(mixer1, 0, delay1, 0);
    patchCord8 = new AudioConnection(delay1, 0, filter1, 0);
    patchCord9 = new AudioConnection(filter1, 0, mixer1, 1);
    patchCord10 = new AudioConnection(mixer1, 0, reverb1, 0);
    patchCord11 = new AudioConnection(reverb1, 0, mixer1, 0);

    patchCord6 = new AudioConnection(mixer1, bitcrusher);


}

void Bouton::begin() {
    pinMode(pin, INPUT);
    sprintf(filename, "BUTTON%d.RAW", num); //créer le nom du fichier

    // Speed
    playRaw.setPlaybackRate(1.0f); // 1.0 = normal, >1 = plus rapide, <1 = plus lent
    playRaw.enableInterpolation(true);
    playRaw.setPlaybackRate(1.0);

    basePitch = 0.0;  
    speedCompensation = 0.0; 

    // Noise Gate
    noise.gate(
        -40.0f,   // threshold plus bas
        0.8f,    // attack 80 ms
        0.8f,     // release 800 ms
        12.0f     // hysteresis large
    );
    noise.compression(0.0f);      // désactive comp
    noise.limit(0.0f);            // désactive limiter
    noise.makeupGain(0.0f);       // pas d'auto gain

    // Bitcrusher
    bitcrusher.sampleRate(44100);
    bitcrusher.bits(16);

    // Reverb
    mixer1.gain(0, 1.0);  // Volume direct
    mixer1.gain(1, 0.0); // Baisse de l'echo
    mixer1.gain(2, 0.0);  // reverb OFF au départ
    reverb1.reverbTime(0.6); // Temps de réverbération court pour lisser le son
    filter1.frequency(1000); 
    filter1.resonance(1.5);
}

void Bouton::update() {//pas utile parce qu'on le fait dans le main? mieux vaut le faire ici ou pas ?

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
  playRaw.playRaw(filename,1); // on le relance
}

// Effets
void Bouton::setGain(float g) {
    gain = constrain(g, 0.0, 1.0);
}

void Bouton::nextEffect() {
  currentEffect++;
  potLocked = true;  // On relock le potentiomètre
  if (currentEffect >= EFFECT_COUNT) currentEffect = 0;
}

void Bouton::setEffectAmount(float value) {//value entre 0 et 1023

    int currentValue = effectValues[currentEffect];

    // Si le pot n’a pas encore rattrapé la valeur mémorisée
    if (potLocked) {
        if (abs(value - currentValue) < pickupThreshold) {
            potLocked = false; // On déverrouille
        } else {
            return; // On ignore le mouvement
        }
    }
    effectValues[currentEffect] = value;
    switch(currentEffect) {
        case EFFECT_PITCH:
            basePitch = value*24/1023-12; //-12 à 12
            pitch.setParamValue("shift (semitones)", basePitch + speedCompensation); 
            break;

        case EFFECT_NOISE:
            noise.gate(value*120/1023-120.0);
            break;

        case EFFECT_VARISPEED: {
            double speed = 0.2 + (value / 1023.0) * 2.8;
            playRaw.setPlaybackRate(speed);

            speedCompensation = -log(speed) / log(2.0) * 12.0;
            pitch.setParamValue("shift (semitones)", speedCompensation + basePitch); //compensation en prenant en compte la valeur actuelle du pitch
            //Serial.println(speedCompensation);
            break;
        }

        case EFFECT_REVERB: {
            // 1. Réglage du temps de l'écho (0ms à 500ms)
            int delayTime = map(value, 0, 1023, 0, 500);
            delay1.delay(0, delayTime);

            // 2. Réglage du niveau de la réverb (0.0 à 1.0)
            float wet = (float)value / 1023.0;  
            mixer1.gain(2, wet);   // niveau reverb progressif
            mixer1.gain(1, wet*0.5);   // niveau delay progressif limité à 0.7

            break;
        }
        
        case EFFECT_BITCRUSHER: {
            int rate = map(value, 0, 1023, 44100, 3000);
            bitcrusher.sampleRate(rate);
            int bits = map(value, 0, 1023, 16, 8);
            bitcrusher.bits(bits);
            break;
        }
        
    }    
}

const char* Bouton::getEffectName(){
    static const char* names[] = {
      "PITCH", "NOISE","VARISPEED", "REVERB", "BITCRUSHER"
    };
    return names[currentEffect];
}


