#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "Bouton.h"

#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

#define MAX_VOICES 15

const char KEY_ENC_LEFT  = 't';
const char KEY_ENC_RIGHT = 'u';
const char KEY_ENC_CLICK = 'q';
const char KEY_RECORD    = 'z';
const char KEY_VOL_UP    = 's';
const char KEY_VOL_DOWN  = 'r';

char keyMap[MAX_VOICES] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o'};

float volValue = 0.5;
int choiceButton = 0;
int mode = 0;
char filename[20];
File frec;

Bouton boutons[MAX_VOICES] = {
    Bouton(0), Bouton(1), Bouton(2), Bouton(3), Bouton(4), 
    Bouton(5), Bouton(6), Bouton(7), Bouton(8), Bouton(9),
    Bouton(10), Bouton(11), Bouton(12), Bouton(13), Bouton(14)
};

// --- CHAINE AUDIO (Mixers & Connections) ---
// Note : Assure-toi que ton fichier "Bouton.h" et tes patchcords sont bien reliés 
// comme dans ton code original pour que le son sorte.
AudioInputI2S      i2s2;
AudioRecordQueue   queue1;
AudioOutputI2S     i2s1;
AudioControlSGTL5000 sgtl5000_1;
AudioConnection    patchCord1(i2s2, 0, queue1, 0);
AudioMixer4 mixerA, mixerB, mixerC, mixerD, mixerE;

AudioConnection patchCordA0(boutons[0].bitcrusher, 0, mixerA, 0); //A MODIFIER ? ici, remplacer [0].blabla par le dernier effet ajouté dans Bouton
AudioConnection patchCordA1(boutons[1].bitcrusher, 0, mixerA, 1); // A DECOMMENTER POUR PLUS DE VOICES A LA FOIS
AudioConnection patchCordA2(boutons[2].bitcrusher, 0, mixerA, 2);
AudioConnection patchCordA3(boutons[3].bitcrusher, 0, mixerA, 3);

AudioConnection patchCordB0(boutons[4].bitcrusher, 0, mixerB, 0);
AudioConnection patchCordB1(boutons[5].bitcrusher, 0, mixerB, 1);
AudioConnection patchCordB2(boutons[6].bitcrusher, 0, mixerB, 2);
AudioConnection patchCordB3(boutons[7].bitcrusher, 0, mixerB, 3);

AudioConnection patchCordC0(boutons[8].bitcrusher, 0, mixerC, 0);
AudioConnection patchCordC1(boutons[9].bitcrusher, 0, mixerC, 1);
AudioConnection patchCordC2(boutons[10].bitcrusher, 0, mixerC, 2);
AudioConnection patchCordC3(boutons[11].bitcrusher, 0, mixerC, 3);

AudioConnection patchCordD0(boutons[12].bitcrusher, 0, mixerD, 0);
AudioConnection patchCordD1(boutons[13].bitcrusher, 0, mixerD, 1);
AudioConnection patchCordD2(boutons[14].bitcrusher, 0, mixerD, 2);

AudioConnection patchCordE0(mixerA, 0, mixerE, 0); 
AudioConnection patchCordE1(mixerB, 0, mixerE, 1);
AudioConnection patchCordE2(mixerC, 0, mixerE, 2); 
AudioConnection patchCordE3(mixerD, 0, mixerE, 3);

AudioConnection          patchCordOutL(mixerE, 0, i2s1, 0);
AudioConnection          patchCordOutR(mixerE, 0, i2s1, 1);

void setup() {
  Serial.begin(9600);

  AudioMemory(120);
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.volume(1.0);  // Volume max à la source
  sgtl5000_1.micGain(40);

  // Initialiser les gains des mixers avec le volume actuel
  updateMixerGains();

  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) { Serial.println("Erreur SD"); delay(500); }
  }

  for (int i = 0; i < MAX_VOICES; i++) {
    boutons[i].begin();
  }

  Serial.println("Soundboard prête");
  Serial.print("Volume initial: ");
  Serial.println(volValue);
}

void loop() {
  if (Serial.available()) {
    char key = Serial.read();
    handleInput(key);
  }

  if (mode == 1) continueRecording();
  if (mode == 2) continuePlaying();
}

void updateMixerGains() {
  for (int i = 0; i < 4; i++) {
    mixerA.gain(i, volValue);
    mixerB.gain(i, volValue);
    mixerC.gain(i, volValue);
    mixerD.gain(i, volValue);
    mixerE.gain(i, volValue);
  }
}

void handleInput(char key) {
  Serial.print("Touche: ");
  Serial.println(key);

  if (key == KEY_ENC_LEFT) {
    Serial.println("Effet -");
    boutons[choiceButton].changeEffectAmount(-1);
  }
  else if (key == KEY_ENC_RIGHT) {
    Serial.println("Effet +");
    boutons[choiceButton].changeEffectAmount(1);
  }
  else if (key == KEY_ENC_CLICK) {
    Serial.print("Effet: ");
    Serial.println(boutons[choiceButton].getEffectName());
    boutons[choiceButton].nextEffect();
  }
  else if (key == KEY_VOL_UP) {
    volValue = constrain(volValue + 0.1, 0.0, 1.0);
    updateMixerGains();
    Serial.print("Volume: ");
    Serial.println(volValue);
  }
  else if (key == KEY_VOL_DOWN) {
    volValue = constrain(volValue - 0.1, 0.0, 1.0);
    updateMixerGains();
    Serial.print("Volume: ");
    Serial.println(volValue);
  }
  else if (key == KEY_RECORD) {
    if (mode == 0) {
      Serial.println("REC START");
      startRecording();
    }
    else if (mode == 1) {
      Serial.println("REC STOP");
      stopRecording();
    }
  }
  else {
    for (int i = 0; i < MAX_VOICES; i++) {
      if (key == keyMap[i]) {
        choiceButton = i;
        sprintf(filename, "BUTTON%d.WAV", i);
        if (mode == 1) stopRecording();
        startPlaying(i);
      }
    }
  }
}

void startRecording() {
  sprintf(filename, "BUTTON%d.WAV", choiceButton);
  Serial.print("REC FILE: ");
  Serial.println(filename);
  if (SD.exists(filename)) SD.remove(filename);
  frec = SD.open(filename, FILE_WRITE);
  if (frec) {
    queue1.begin();
    mode = 1;
    Serial.println("Recording...");
  } else {
    Serial.println("FILE ERROR");
  }
}

void stopRecording() {
  Serial.println("REC STOP");
  queue1.end();
  while (queue1.available() > 0) {
    frec.write((byte*)queue1.readBuffer(), 256);
    queue1.freeBuffer();
  }
  frec.close();
  mode = 0;
  Serial.println("Saved");
}

void startPlaying(int i) {
  if (!SD.exists(filename)) {
    Serial.print("FILE NOT FOUND: ");
    Serial.println(filename);
    return;
  }
  Serial.print("PLAY: ");
  Serial.println(filename);
  // if this button is still playing, stop it first to avoid clicks
  if (boutons[i].playRaw.isPlaying()) {
    boutons[i].playRaw.stop();
    delay(2);                 // allow hardware to settle
  }
  // mute output briefly to suppress transients
  updateMixerGains();
  boutons[i].play();
  mode = 2;
  
}

void continuePlaying() {
  bool anyPlaying = false;
  for (int v = 0; v < MAX_VOICES; v++) {
    if (boutons[v].playRaw.isPlaying()) anyPlaying = true;
  }
  if (!anyPlaying) mode = 0;
}

void continueRecording() {
  if (queue1.available() >= 2) {
    byte buffer[512];
    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer+256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    frec.write(buffer, 512);
  }
}