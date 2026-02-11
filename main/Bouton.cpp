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
    //patchCord3 = new AudioConnection(noise, varispeed);
    //patchCord4 = new AudioConnection(varispeed, reverb);
    //patchCord5 = new AudioConnection(reverb, bitcrusher);
    //patchCord6 = new AudioConnection(bitcrusher, mixerDryWet, 0);
}

void Bouton::begin() {
    pinMode(pin, INPUT);
    sprintf(filename, "BUTTON%d.RAW", num); //créer le nom du fichier
}

void Bouton::update() {

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

void Bouton::setGain(float g) {
    gain = constrain(g, 0.0, 1.0);
    //mixerDryWet.gain(0, gain);
}
