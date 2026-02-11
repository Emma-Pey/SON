//A FAIRE : 
//// - tester avec les 9 boutons ( des trucs à décommenter pour que ça fonctionne)
//// - ajouter les effets

// Ce que ça fait : sélection d'un bouton par appui long sur le bouton, enregistrement d'un son en tournant le potentiomètre, rejouer le son quand on appuie dessus

#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define MAX_VOICES 2 // A MODIFIER POUR FAIRE JOUER PLUS de boutons

const int nbButtons = MAX_VOICES; 

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=105,63
//AudioAnalyzePeak         peak1;          //xy=278,108
AudioRecordQueue         queue1;         //xy=281,63
//AudioPlaySdRaw           playRaw1;       //xy=302,157
AudioPlaySdRaw playRaw[MAX_VOICES]; //plusieurs players pour joeur plusieurs sons en même temps
AudioOutputI2S           i2s1;           //xy=470,120

AudioConnection          patchCord1(i2s2, 0, queue1, 0);
//AudioConnection          patchCord2(i2s2, 0, peak1, 0);


// mixers
AudioMixer4 mixerA; // voies 0..3
AudioMixer4 mixerB; // voies 4..7
AudioMixer4 mixerC; // mix final (voies 0..3)

// connection pour chaque player
AudioConnection patchCordA0(playRaw[0], 0, mixerA, 0);
AudioConnection patchCordA1(playRaw[1], 0, mixerA, 1);

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
// GUItool: end automatically generated code

// For a stereo recording version, see this forum thread:
// https://forum.pjrc.com/threads/46150?p=158388&viewfull=1#post158388

// A much more advanced sound recording and data logging project:
// https://github.com/WMXZ-EU/microSoundRecorder
// https://github.com/WMXZ-EU/microSoundRecorder/wiki/Hardware-setup
// https://forum.pjrc.com/threads/52175?p=185386&viewfull=1#post185386

// Bounce objects to easily and reliably read the buttons
//Bounce buttonRecord = Bounce(0, 8);
//Bounce buttonStop =   Bounce(1, 8);  // 8 = 8 ms debounce time
//Bounce buttonPlay =   Bounce(0, 8);


int buttonPins[nbButtons] = {0,1}; // A MODIFIER AVEC LES NUMEROS DE PINS DE TOUS LES BOUTONS

// Variables de gestion
bool buttonState[nbButtons];
bool lastButtonState[nbButtons];
unsigned long pressStartTime[nbButtons];
bool longPressTriggered[nbButtons];

const unsigned long longPressDuration = 2000; // 2 secondes

// which input on the audio shield will be used?
//const int myInput = AUDIO_INPUT_LINEIN;
const int myInput = AUDIO_INPUT_MIC;


// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7   // Teensy 4 ignores this, uses pin 11
#define SDCARD_SCK_PIN   14  // Teensy 4 ignores this, uses pin 13

// Use these with the Teensy 3.5 & 3.6 & 4.1 SD card
//#define SDCARD_CS_PIN    BUILTIN_SDCARD
//#define SDCARD_MOSI_PIN  11  // not actually used
//#define SDCARD_SCK_PIN   13  // not actually used

// Use these for the SD+Wiz820 or other adaptors
//#define SDCARD_CS_PIN    4
//#define SDCARD_MOSI_PIN  11
//#define SDCARD_SCK_PIN   13


// Remember which mode we're doing
int mode = 0;  // 0=stopped, 1=recording, 2=playing
int choiceButton = 0; //0..8=les  boutons sélectionnés
char filename[20];

// The file where data is recorded
File frec;

void setup() {
  Serial.begin(9600);
  sprintf(filename, "BUTTON%d.RAW", choiceButton);

  // Configure the pushbutton pins
  for (int i=0; i < nbButtons ; i++) {
    pinMode(buttonPins[i], INPUT);
  }
  //pinMode(1, INPUT_PULLUP);
  //pinMode(2, INPUT_PULLUP);

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
  // First, read the buttons
  //buttonRecord.update();
  //buttonStop.update();
  //buttonPlay.update();


  //regarder tous les boutons pour voir celui qui est sélectionné
  for (int i = 0; i < nbButtons; i++) {

    buttonState[i] = digitalRead(buttonPins[i]);

    // Bouton pressé (transition LOW -> HIGH)
    if (lastButtonState[i] == LOW && buttonState[i] == HIGH) {
      //Serial.println("Bouton pressé");  
      pressStartTime[i] = millis();
      longPressTriggered[i] = false;
    }

    // Bouton maintenu
    if (buttonState[i] == HIGH && !longPressTriggered[i]) {
      if (millis() - pressStartTime[i] >= longPressDuration) {
        choiceButton = i;   // on stocke l'index du bouton
        longPressTriggered[i] = true;

        Serial.print("Bouton sélectionné : ");
        Serial.println(choiceButton);
        sprintf(filename, "BUTTON%d.RAW", choiceButton); //on change le nom du fichier selon ce qui est sélectionné
        Serial.print("Filename : ");
        Serial.println(filename);
      }
    }

    // Bouton relâché
    if (lastButtonState[i] == HIGH && buttonState[i] == LOW) {
      longPressTriggered[i] = false;
      if (millis() - pressStartTime[i] < longPressDuration) { // on a juste cliqué sur le bouton
        //Serial.println("Play Button Pressed");
        if (mode == 1) {
          //Serial.println("stopRecording()");
          stopRecording();
        }    
        // if (mode == 0) {
        //   //Serial.println("startPlaying()");
        //   startPlaying();   
        // } 
        startPlaying(i);
      }
    }

    lastButtonState[i] = buttonState[i];
  }



  // Respond to button presses
  if (analogRead(A0)>600) { // on met un gap pour pas que ça enregistre et stoppe l'enregistrement à cause des fulctuations
    //Serial.println("Potentiomètre enregistrement tourné côté Record");
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
    //Serial.println("Potentiomètre enregistrement tourné côté Stop");
    if (mode == 1) {
      //Serial.println("stopRecording()");
      stopRecording();
    }
    // if (mode == 2) {//on est en train de jouer
    //   //Serial.println("stopPlaying()");
    //   stopPlaying();
    // }  
  }

  // // il faut faire une boucle sur les boutons ici
  // if (digitalRead(0)==HIGH) {
  //   //Serial.println("Play Button Pressed");
  //   if (mode == 1) {
  //     //Serial.println("stopRecording()");
  //     stopRecording();
  //   }    
  //   if (mode == 0) {
  //     //Serial.println("startPlaying()");
  //     startPlaying();   
  //   } 
  // }

  // If we're playing or recording, carry on...
  if (mode == 1) {
    continueRecording();
  }
  if (mode == 2) {
    continuePlaying();
  }
  //Serial.println(mode);
  // when using a microphone, continuously adjust gain
  //if (myInput == AUDIO_INPUT_MIC) adjustMicLevel();
  //delay(100);
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
    //elapsedMicros usec = 0;
    frec.write(buffer, 512);
    // Uncomment these lines to see how long SD writes
    // are taking.  A pair of audio blocks arrives every
    // 5802 microseconds, so hopefully most of the writes
    // take well under 5802 us.  Some will take more, as
    // the SD library also must write to the FAT tables
    // and the SD card controller manages media erase and
    // wear leveling.  The queue1 object can buffer
    // approximately 301700 us of audio, to allow time
    // for occasional high SD card latency, as long as
    // the average write time is under 5802 us.
    //Serial.print("SD write, us=");
    //Serial.println(usec);
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


void startPlaying(int i) { //modifier pour que ça change la liste des sons joués
  char name[20]; //nom du fichier
  sprintf(name, "BUTTON%d.RAW", i);  

  playRaw[i].stop(); // on l'arrête
  Serial.println("startPlaying : ");
  Serial.println(i);
  playRaw[i].play(name); // on le relance
  //playRaw1.play(name); 
  mode = 2;
  //delay(200);
}

// void continuePlaying() {
//   if (!playRaw.isPlaying()) { //playRaw1
//     playRaw.stop(); //playRaw1
//     mode = 0;
//   }
// }

void continuePlaying() {
  bool anyPlaying = false;

  for (int v = 0; v < MAX_VOICES; v++) {
    if (playRaw[v].isPlaying()) {
      anyPlaying = true;
      break;
    }
  }

  if (!anyPlaying) {
    mode = 0;
  }
}


// void stopPlaying() {
//   Serial.println("stopPlaying");
//   if (mode == 2) playRaw.stop(); //playRaw1
//   mode = 0;
// }

void stopPlaying() {
  Serial.println("stopPlaying");

  for (int v = 0; v < MAX_VOICES; v++) {
    if (playRaw[v].isPlaying()) {
      playRaw[v].stop();
    }
  }
  mode = 0;
}


