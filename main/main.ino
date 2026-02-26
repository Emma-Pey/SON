#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "Bouton.h"

// --- CONFIGURATION SD ---
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

#define MAX_VOICES 15
#define VOL_PIN A0 

// --- MAPPING DES TOUCHES LOUPEDECK ---
// Remplace ces caractères par ceux envoyés par ton Loupedeck
const char KEY_ENC_LEFT  = 't'; // Exemple : Molette vers la gauche
const char KEY_ENC_RIGHT = 'u'; // Exemple : Molette vers la droite
const char KEY_ENC_CLICK = 'q'; // Exemple : Clic sur la molette
const char KEY_RECORD    = 'z';

// Touches pour déclencher les sons (1 à 9)
char keyMap[MAX_VOICES] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i','j', 'k', 'l', 'm', 'n', 'o'};

// --- VARIABLES ---
float volValue = 0.25;
int choiceButton = 0; // Le bouton actuellement sélectionné pour les effets
int mode = 0;         // 0=idle, 1=recording, 2=playing
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
  sgtl5000_1.volume(0.5);
  sgtl5000_1.micGain(40);

  //gain des mixers
  for (int i = 0; i < 4; i++) {
    mixerA.gain(i, volValue); 
    mixerB.gain(i, volValue); 
    mixerC.gain(i, volValue);
    mixerD.gain(i, volValue);
    mixerE.gain(i, volValue);
  }

  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) { Serial.println("Erreur SD"); delay(500); }
  }

  for (int i=0; i < MAX_VOICES ; i++) {
    boutons[i].begin();
  }
  
  Serial.println("Soundboard Prête (Attente touches Loupedeck)");
}

void loop() {
  // 1. GESTION DU VOLUME (Potentiomètre physique toujours actif)
  readPhysicalVolume();

  // 2. LECTURE DES TOUCHES (Entrées Loupedeck / Clavier)
  if (Serial.available()) {
    char key = Serial.read();
    handleInput(key);
  }

  // 3. FLUX AUDIO
  if (mode == 1) continueRecording();
  if (mode == 2) continuePlaying();
}

void handleInput(char key) {
  
  // --- GESTION DE LA MOLETTE (EFFETS) ---
  if (key == KEY_ENC_LEFT) {
    boutons[choiceButton].changeEffectAmount(-1);
    Serial.print("Effet - sur bouton "); Serial.println(choiceButton);
  } 
  else if (key == KEY_ENC_RIGHT) {
    boutons[choiceButton].changeEffectAmount(1);
    Serial.print("Effet + sur bouton "); Serial.println(choiceButton);
  }
  else if (key == KEY_ENC_CLICK) {
    boutons[choiceButton].nextEffect();
    Serial.print("Changement effet: "); Serial.println(boutons[choiceButton].getEffectName());
  }

  // --- GESTION ENREGISTREMENT ---
  else if (key == KEY_RECORD) {
    if (mode == 0) startRecording();
    else if (mode == 1) stopRecording();
  }

  // --- GESTION DECLENCHEMENT SONS ---
  else {
    for (int i = 0; i < MAX_VOICES; i++) {
      if (key == keyMap[i]) {
        choiceButton = i; // Sélectionne ce bouton pour la molette d'effets
        sprintf(filename, "BUTTON%d.WAV", i);
        
        if (mode == 1) stopRecording();
        startPlaying(i);
        Serial.print("Lecture: "); Serial.println(filename);
      }
    }
  }
}

// --- FONCTIONS AUDIO REPRISES DE TON CODE ---

void readPhysicalVolume() {
  int potValue = analogRead(VOL_PIN);
  float vol = potValue / 1023.0 * 0.6; 
  if (abs(vol - volValue) >= 0.05) {
    volValue = vol;
    // On ajuste les gains des mixers ici
    mixerC.gain(0, volValue);
  }
}

void startRecording() {
  sprintf(filename, "BUTTON%d.RAW", choiceButton);
  if (SD.exists(filename)) SD.remove(filename);
  frec = SD.open(filename, FILE_WRITE);
  if (frec) {
    queue1.begin();
    mode = 1;
    Serial.println("Enregistrement en cours...");
  }
}

void stopRecording() {
  queue1.end();
  while (queue1.available() > 0) {
    frec.write((byte*)queue1.readBuffer(), 256);
    queue1.freeBuffer();
  }
  frec.close();
  mode = 0;
  Serial.println("Enregistrement fini.");
}

void startPlaying(int i) {
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