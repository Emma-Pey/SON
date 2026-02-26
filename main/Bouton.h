#include <_Teensy.h>

#ifndef BOUTON_H
#define BOUTON_H

//#include <Arduino.h>
#include <Audio.h>
#include "Pitch.h"
#include "effect_dynamics.h" //noise gate
#include <TeensyVariablePlayback.h>

enum EffectType {
  EFFECT_PITCH, 
  EFFECT_NOISE,
  EFFECT_VARISPEED,
  //EFFECT_REVERB,
  EFFECT_BITCRUSHER,
  EFFECT_COUNT};

class Bouton {
public:
    // ======== Données ========
    int num; // il n'y aura plus besoin de ces deux là au final, on regarde juste l'indice des boutons dans la liste avec le multiplexeur
    //int pin;

    char filename[20];

    int currentEffect;
    float effectValues[5] = {512, 512, 512, 512, 512}; 
    //const int pickupThreshold = 20; // plus besoin de ces deux-là avec l'encodeur
    //bool potLocked = true; ///////////

    bool state;
    bool lastState;

    unsigned long pressStartTime;
    bool longPressTriggered;

    float basePitch;       // pitch manuel de l'utilisateur
    float speedCompensation; // compensation liée à la vitesse

    // ======== Audio Objects ========
    AudioPlaySdResmp playRaw;

    Pitch pitch;
    AudioEffectDynamics noise;
    // AudioMixer4              mixer1;
    // AudioEffectDelay         delay1;
    // AudioFilterStateVariable filter1;
    // AudioEffectReverb        reverb1;
    AudioEffectBitcrusher bitcrusher;

    // ======== Connexions ========
    AudioConnection *patchCord1;
    AudioConnection *patchCord2;
    AudioConnection *patchCord3;
    AudioConnection *patchCord4;
    
    AudioConnection *patchCord5;
    AudioConnection *patchCord6;

    AudioConnection *patchCord7;
    AudioConnection *patchCord8;
    AudioConnection *patchCord9;
    AudioConnection *patchCord10;
    AudioConnection *patchCord11;



    // ======== Constructeur ========
    Bouton(int number); //, int buttonPin

    void begin();
    void update(); //pas utile?
    void play();

    void nextEffect();
    void changeEffectAmount(int delta);

    float getOutput();

    const char* getEffectName();

private:
    float outputLevel;
};

#endif
