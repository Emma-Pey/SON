//A FAIRE : 
//// - potentiomètre pour le son (à retester)

#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <cmath> //abs
#include <Encoder.h> // librairie encodeur optimisée pour teensy
#include "Bouton.h"


// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7   // Teensy 4 ignores this, uses pin 11
#define SDCARD_SCK_PIN   14  // Teensy 4 ignores this, uses pin 13

#define S0 0 //A MODIFIER : les numéros des pins du multiplexeur
#define S1 1
#define S2 2
#define S3 3
#define SIG_PIN 4

#define MAX_VOICES 9 // A MODIFIER POUR FAIRE JOUER PLUS de boutons, 1 bouton pour l'instant
#define EFFECTS_PIN 16 // A MODIFIER POUR LE PIN du bouton de l'encodeur 
#define ENC_PIN_A 17 //A MODIFIER AVEC LES NUMEROS DE PIN DE L'ENCODEUR
#define ENC_PIN_B 22

#define VOL_PIN A0 // A MODIFIER SI PAS CELUI LA
float volValue=0.25;

// bouton enregistrement
bool lastStateREC = LOW; 
unsigned long lastClickTimeREC = 0;
unsigned long debounceDelay = 50; // 50ms pour filtrer les parasites
const int BUTTON_REC = 9;    // bouton enregistrement (le 10e)
bool actionDeclencheeREC = false;

const int myInput = AUDIO_INPUT_MIC;

// encodeur
bool lastStateEnc = HIGH; //bouton enc
unsigned long lastClickTimeEnc = 0; // pour debounce l'encodeur
Encoder myEnc(ENC_PIN_A, ENC_PIN_B);
long oldPos = -999;

// boutons 0-8
int buttonPins[MAX_VOICES] = {1}; // A MODIFIER AVEC LES NUMEROS DE PINS DE TOUS LES BOUTONS
Bouton boutons[MAX_VOICES] = { // A MODIFIER POUR AJOUTER TOUS LES BOUTONS (on ne peut pas faire de boucle for dans l'init)
    Bouton(0), 
    Bouton(1),
    Bouton(2),
    Bouton(3),
    Bouton(4),
    Bouton(5),
    Bouton(6),
    Bouton(7),
    Bouton(8),
    };

// audio
AudioInputI2S            i2s2;           //xy=105,63
AudioRecordQueue         queue1;         //xy=281,63
AudioOutputI2S           i2s1;           //xy=470,120

AudioConnection          patchCord1(i2s2, 0, queue1, 0);


// mixers
AudioMixer4 mixerA; // voies 0..3
AudioMixer4 mixerB; // voies 4..7 // A DECOMMENTER POUR PLUS DE BOUTONS
AudioMixer4 mixerC; // mix final (voies 0..3)

// connection pour chaque player
AudioConnection patchCordA0(boutons[0].bitcrusher, 0, mixerA, 0); //A MODIFIER ? ici, remplacer [0].blabla par le dernier effet ajouté dans Bouton
AudioConnection patchCordA1(boutons[1].bitcrusher, 0, mixerA, 1); // A DECOMMENTER POUR PLUS DE VOICES A LA FOIS

AudioConnection patchCordA2(boutons[2].bitcrusher, 0, mixerA, 2);
AudioConnection patchCordA3(boutons[3].bitcrusher, 0, mixerA, 3);

AudioConnection patchCordB0(boutons[4].bitcrusher, 0, mixerB, 0);
AudioConnection patchCordB1(boutons[5].bitcrusher, 0, mixerB, 1);
AudioConnection patchCordB2(boutons[6].bitcrusher, 0, mixerB, 2);
AudioConnection patchCordB3(boutons[7].bitcrusher, 0, mixerB, 3);

// mixer final : on mixe A+B + la voix 8
AudioConnection patchCordC0(mixerA, 0, mixerC, 0); //inutile pour 2 voix
AudioConnection patchCordC1(mixerB, 0, mixerC, 1);
AudioConnection patchCordC2(boutons[8].bitcrusher, 0, mixerC, 2);

AudioConnection          patchCordOutL(mixerC, 0, i2s1, 0);
AudioConnection          patchCordOutR(mixerC, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=265,212

const unsigned long longPressDuration = 2000; // 2 secondes

// Remember which mode we're doing
int mode = 0;  // 0=stopped, 1=recording, 2=playing 
int choiceButton = 0; //0..8=le  bouton sélectionné

// The file where data is recorded
File frec;
char filename[20];

void setup() {
  Serial.begin(9600);
  sprintf(filename, "BUTTON%d.RAW", choiceButton);

  // Multiplexeur
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(SIG_PIN, INPUT_PULLUP);

  //encodeur 
  pinMode(ENC_PIN_A,INPUT_PULLUP);
  pinMode(ENC_PIN_B,INPUT_PULLUP);

  pinMode(EFFECTS_PIN, INPUT_PULLUP); // on aura plus besoin de ça avec le MUX
  // Configure the pushbutton pins
  for (int i=0; i < MAX_VOICES ; i++) {
    boutons[i].begin();
  }

  // Audio connections require memory, and the record queue
  // uses this memory to buffer incoming audio.
  AudioMemory(200);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);
  sgtl5000_1.micGain(40);

  //gain des mixers
  for (int i = 0; i < 4; i++) {
    mixerA.gain(i, volValue); 
    mixerB.gain(i, volValue); 
    mixerC.gain(i, volValue);
  }

  // Initialize the SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here if no SD card, but print a message
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

void loop() {
  //volume 
  int potValue = analogRead(VOL_PIN);
  //Serial.println(potValue);
  float vol = potValue/1023.0*0.5; // entre 0 et 0.5
  //Serial.println(vol);
  if (abs(vol-volValue)>=0.1) {
    volValue = vol;
    //Serial.println(volValue);
    for (int i = 0; i < 4; i++) {
      mixerA.gain(i, volValue); 
      mixerB.gain(i, volValue); 
      mixerC.gain(i, volValue);
    }
  }
  //regarder tous les boutons 
  for (int i = 0; i < MAX_VOICES; i++) {

    boutons[i].state = readMux(i); //digitalRead(boutons[i].pin)

    // Bouton pressé (transition LOW -> HIGH)
    if (boutons[i].lastState == LOW && boutons[i].state == HIGH) {
      boutons[i].pressStartTime = millis();
      boutons[i].longPressTriggered = false;
    }

    // Bouton maintenu : sélectionner le bouton si > 2s
    if (boutons[i].state == HIGH && !boutons[i].longPressTriggered) {
      if (millis() - boutons[i].pressStartTime >= longPressDuration) {
        choiceButton = boutons[i].num;  // on stocke l'index du bouton
        boutons[i].longPressTriggered = true;

        Serial.print("Bouton sélectionné : ");
        Serial.println(choiceButton);
        sprintf(filename, "BUTTON%d.RAW", choiceButton); //on change le nom du fichier selon ce qui est sélectionné
        Serial.print("Filename : ");
        Serial.println(filename);
      }
    }

    // Bouton relâché : jouer un son si < 2s
    if (boutons[i].lastState == HIGH && boutons[i].state == LOW) {
      boutons[i].longPressTriggered = false;
      if (millis() - boutons[i].pressStartTime < longPressDuration) { // on a juste cliqué sur le bouton
        //Serial.println("Play Button Pressed");
        if (mode == 1) { // pas forcément besoin de ça ? on peut record et play en même temps ?
          //Serial.println("stopRecording()");
          stopRecording();
        }
        startPlaying(i);
      }
    }

    //si c'est le bouton sélectionné on modifie l'effet
    if (i == choiceButton) { 
      bool stateEnc = digitalRead(EFFECTS_PIN);
      if (lastStateEnc == LOW && stateEnc == HIGH) { 
        if (millis() - lastClickTimeEnc > debounceDelay) { // debounce
      //if (encButton.fallingEdge()) {
          //lastClickTimeEnc = millis();
          boutons[i].nextEffect();
          Serial.println(boutons[i].getEffectName());
        }
      }
      long newPos = myEnc.read() / 4;  // 1 pas par cran
      // mémoriser l'ancienne position
      static long oldPos = 0;
      long delta = newPos - oldPos;
      oldPos = newPos;
      if (delta != 0) {
        //Serial.println("Tourne détecté");
        boutons[i].changeEffectAmount(delta);
      }
      lastStateEnc = stateEnc;
    }
    boutons[i].lastState = boutons[i].state;
  }

  // --- PARTIE ENREGISTREMENT (Strictement comme les autres boutons) ---
  bool stateREC = readMux(BUTTON_REC);

  // 1. Détection du front montant (Appui)
  if (stateREC == HIGH && lastStateREC == LOW) {
      lastClickTimeREC = millis(); // On enregistre le moment exact de l'appui
      actionDeclencheeREC = false;   // On autorise une nouvelle action
      //Serial.println("Doigt posé sur REC");
  }

  // 2. Détection de l'appui stable
  if (stateREC == HIGH && !actionDeclencheeREC) {
      if (millis() - lastClickTimeREC >= 50) { // Un mini "debounce" de 50ms
          actionDeclencheeREC = true; // On verrouille pour ne le faire qu'une fois
          
          //Serial.println("Action REC déclenchée !");
          if (mode == 0) {startRecording(); //Serial.println(mode);
          }
          else if (mode == 1) stopRecording();
          else if (mode == 2) {
              stopPlaying();
              startRecording();
          }
      }
  }

  // 3. Détection du relâchement (Optionnel, pour reset le verrou)
  if (stateREC == LOW && lastStateREC == HIGH) {
      actionDeclencheeREC = false;
      //Serial.println("Doigt retiré de REC");
  }

  lastStateREC = stateREC;

  // If we're playing or recording, carry on...
  if (mode == 1) {
    continueRecording();
  }
  if (mode == 2) {
    continuePlaying();
  }
}

void selectChannel(int channel) {
  digitalWrite(S0, channel & 0x01);
  digitalWrite(S1, (channel >> 1) & 0x01);
  digitalWrite(S2, (channel >> 2) & 0x01);
  digitalWrite(S3, (channel >> 3) & 0x01);
}

int readMux(int channel) {
  selectChannel(channel);
  delayMicroseconds(5); // petit temps de stabilisation
  return digitalRead(SIG_PIN); // peut être ajouter un ! devant le digitalread (logique bizarre, même avec le input pullup)
}

void startRecording() {
  Serial.println("startRecording :");
  Serial.println(filename);
  if (SD.exists(filename)) { //le bouton sélectionné (0..8.RAW)
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove(filename); //le bouton sélectionné (0..8.RAW)
  }
  frec = SD.open(filename, FILE_WRITE); // open le bouton sélectionné (0..8.RAW)
  if (frec) {
    queue1.begin();
    mode = 1;
    //Serial.println(mode);
  }
}

void continueRecording() {
  if (queue1.available() >= 2) {
    // Serial.println("continue recording");
    byte buffer[512];
    // Fetch 2 blocks from the audio library and copy
    // into a 512 byte buffer.  The Arduino SD library
    // is most efficient when full 512 byte sector size
    // writes are used.
    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer+256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    // write all 512 bytes to the SD card
    frec.write(buffer, 512);
  }
}

void stopRecording() {
  Serial.println("stopRecording");
  queue1.end();
  if (mode == 1) {
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    frec.close();
  }
  mode = 0;
}

void startPlaying(int i) { 
  Serial.println("startPlaying : ");
  Serial.println(boutons[i].filename);
  boutons[i].play();
  mode = 2;
}

void continuePlaying() {
  bool anyPlaying = false;

  for (int v = 0; v < MAX_VOICES; v++) {
    if (boutons[v].playRaw.isPlaying()) {
      anyPlaying = true;
      break;
    }
  }
  if (!anyPlaying) {
    mode = 0;
  }
}

void stopPlaying() {
  Serial.println("stopPlaying");

  for (int v = 0; v < MAX_VOICES; v++) {
    if (boutons[v].playRaw.isPlaying()) {
      boutons[v].playRaw.stop();
    }
  }
  mode = 0;
}
