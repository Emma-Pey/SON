//A FAIRE : 
//// - tester avec les 9 boutons (des trucs à décommenter pour que ça fonctionne)
//// - ajouter les effets
//// - enlever le potentiomètre et mettre un bouton à la place

// Ce que ça fait : sélection d'un bouton par appui long sur le bouton, enregistrement d'un son en tournant le potentiomètre, rejouer le son quand on appuie dessus

#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "Bouton.h"

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7   // Teensy 4 ignores this, uses pin 11
#define SDCARD_SCK_PIN   14  // Teensy 4 ignores this, uses pin 13

const int myInput = AUDIO_INPUT_MIC;

#define MAX_VOICES 2 // A MODIFIER POUR FAIRE JOUER PLUS de boutons

//const int nbButtons = MAX_VOICES; 
int buttonPins[MAX_VOICES] = {0,1}; // A MODIFIER AVEC LES NUMEROS DE PINS DE TOUS LES BOUTONS

Bouton boutons[MAX_VOICES] = {
    Bouton(0, 0),
    Bouton(1, 1)
};

AudioInputI2S            i2s2;           //xy=105,63
AudioRecordQueue         queue1;         //xy=281,63
AudioPlaySdRaw playRaw[MAX_VOICES]; //plusieurs players pour joeur plusieurs sons en même temps
AudioOutputI2S           i2s1;           //xy=470,120

AudioConnection          patchCord1(i2s2, 0, queue1, 0);


// mixers
AudioMixer4 mixerA; // voies 0..3
AudioMixer4 mixerB; // voies 4..7
AudioMixer4 mixerC; // mix final (voies 0..3)

// connection pour chaque player
AudioConnection patchCordA0(boutons[0].playRaw, 0, mixerA, 0);
AudioConnection patchCordA1(boutons[1].playRaw, 0, mixerA, 1);

// A DECOMMENTER POUR PLUS DE VOICES A LA FOIS
// AudioConnection patchCordA2(playRaw[2], 0, mixerA, 2);
// AudioConnection patchCordA3(playRaw[3], 0, mixerA, 3);

// AudioConnection patchCordB0(playRaw[4], 0, mixerB, 0);
// AudioConnection patchCordB1(playRaw[5], 0, mixerB, 1);
// AudioConnection patchCordB2(playRaw[6], 0, mixerB, 2);
// AudioConnection patchCordB3(playRaw[7], 0, mixerB, 3);

// mixer final : on mixe A+B + la voix 8
AudioConnection patchCordC0(mixerA, 0, mixerC, 0); //inutile pour 2 voix
//AudioConnection patchCordC1(mixerB, 0, mixerC, 1);
//AudioConnection patchCordC2(playRaw[8], 0, mixerC, 2);

AudioConnection          patchCordOutL(mixerC, 0, i2s1, 0);
AudioConnection          patchCordOutR(mixerC, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=265,212

// Variables de gestion
//bool buttonState[nbButtons];
//bool lastButtonState[nbButtons];
//unsigned long pressStartTime[nbButtons];
//bool longPressTriggered[nbButtons];

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

  // Configure the pushbutton pins
  for (int i=0; i < MAX_VOICES ; i++) {
    boutons[i].begin();
  }

  // Audio connections require memory, and the record queue
  // uses this memory to buffer incoming audio.
  AudioMemory(60);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);
  sgtl5000_1.micGain(40);

  //gain des mixers
  for (int i = 0; i < 4; i++) {
    mixerA.gain(i, 0.5);
    mixerB.gain(i, 0.5);
    mixerC.gain(i, 0.5);
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

  //regarder tous les boutons 
  for (int i = 0; i < MAX_VOICES; i++) {

    boutons[i].state = digitalRead(buttonPins[i]);

    // Bouton pressé (transition LOW -> HIGH)
    if (boutons[i].lastState == LOW && boutons[i].state == HIGH) {
      boutons[i].pressStartTime = millis();
      boutons[i].longPressTriggered = false;
    }

    // Bouton maintenu : sélectionner le bouton si > 3s
    if (boutons[i].state == HIGH && !boutons[i].longPressTriggered) {
      if (millis() - boutons[i].pressStartTime >= longPressDuration) {
        choiceButton = i;   // on stocke l'index du bouton
        boutons[i].longPressTriggered = true;

        Serial.print("Bouton sélectionné : ");
        Serial.println(choiceButton);
        sprintf(filename, "BUTTON%d.RAW", choiceButton); //on change le nom du fichier selon ce qui est sélectionné
        Serial.print("Filename : ");
        Serial.println(filename);
      }
    }

    // Bouton relâché : jouer un son si nécessaire
    if (boutons[i].lastState == HIGH && boutons[i].state == LOW) {
      boutons[i].longPressTriggered = false;
      if (millis() - boutons[i].pressStartTime < longPressDuration) { // on a juste cliqué sur le bouton
        //Serial.println("Play Button Pressed");
        if (mode == 1) {
          //Serial.println("stopRecording()");
          stopRecording();
        }
        startPlaying(i);
      }
    }

    boutons[i].lastState = boutons[i].state;
  }

  // Potentiomètre enregistreur
  if (analogRead(A0)>600) { // on met un gap pour pas que ça enregistre et stoppe l'enregistrement à cause des fulctuations
    if (mode == 2) {//on est en train de jouer
      //Serial.println("stopPlaying()");
      stopPlaying();    
    }
    if (mode == 0) {
      //Serial.println("startRecording()");
      startRecording();
    }
  }
  if (analogRead(A0)<500) {
    if (mode == 1) {
      //Serial.println("stopRecording()");
      stopRecording();
    }
  }

  // If we're playing or recording, carry on...
  if (mode == 1) {
    continueRecording();
  }
  if (mode == 2) {
    continuePlaying();
  }
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
  Serial.println(i);
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
