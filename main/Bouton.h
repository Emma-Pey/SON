#ifndef BOUTON_H
#define BOUTON_H

//#include <Arduino.h>
#include <Audio.h>
// #include "Pitch.h"
// #include "noise_gate.h"
// #include "Reverb.h"

class Bouton {
public:
    // ======== Données ========
    int num;
    int pin;

    char filename[20];

    bool state;
    bool lastState;

    unsigned long pressStartTime;
    bool longPressTriggered;

    float gain;   // 0.0 → 1.0

    // ======== Audio Objects ========
    AudioPlaySdRaw playRaw;

    //Pitch pitch;
    //noise_gate noise;
    //AudioEffectVarispeed varispeed;
    //Reverb reverb;
    //AudioEffectBitcrusher bitcrusher;

    //AudioMixer4 mixerDryWet;

    // ======== Connexions ========
    // AudioConnection *patchCord1;
    // AudioConnection *patchCord2;
    // AudioConnection *patchCord3;
    // AudioConnection *patchCord4;
    // AudioConnection *patchCord5;
    // AudioConnection *patchCord6;

    // ======== Constructeur ========
    Bouton(int number, int buttonPin);

    void begin();
    void update();
    void play();

    void setGain(float g);
    float getOutput();

private:
    float outputLevel;
};

#endif
