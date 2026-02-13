#ifndef BOUTON_H
#define BOUTON_H

//#include <Arduino.h>
#include <Audio.h>
#include "Pitch.h"
#include "effect_dynamics.h" //noise gate
// #include "Reverb.h"

enum EffectType {
  EFFECT_PITCH, 
  EFFECT_NOISE,
   //EFFECT_VARISPEED,
//   EFFECT_REVERB,
  EFFECT_BITCRUSHER,
  EFFECT_COUNT};

class Bouton {
public:
    // ======== Données ========
    int num;
    int pin;

    char filename[20];

    int currentEffect;

    bool state;
    bool lastState;

    unsigned long pressStartTime;
    bool longPressTriggered;

    float gain;   // 0.0 → 1.0 ?

    // ======== Audio Objects ========
    AudioPlaySdRaw playRaw;

    Pitch pitch;
    //noise_gate noise; // ne fonctionne pas
    //AudioEffectDynamics noise;
    //AudioEffectVarispeed varispeed;
    //Reverb reverb;
    //AudioEffectBitcrusher bitcrusher;

    AudioMixer4              mixer1;
    AudioEffectDelay         delay1;
    AudioFilterStateVariable filter1;
    AudioEffectReverb        reverb1;
    //AudioMixer4 mixerDryWet;

    // ======== Connexions ========
    //AudioConnection *patchCord1;
    //AudioConnection *patchCord2;
    //AudioConnection *patchCord3;
  
    // AudioConnection *patchCord4;
    // AudioConnection *patchCord5;
    // AudioConnection *patchCord6;

    AudioConnection *patchCord7;
    AudioConnection *patchCord8;
    AudioConnection *patchCord9;
    AudioConnection *patchCord10;

    
    // ======== Constructeur ========
    Bouton(int number, int buttonPin);

    void begin();
    void update(); //pas utile?
    void play();

    void setGain(float g);

    void nextEffect();
    void setEffectAmount(float value);

    float getOutput();

    const char* getEffectName();

private:
    float outputLevel;
};

#endif
