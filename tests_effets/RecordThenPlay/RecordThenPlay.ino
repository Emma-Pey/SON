#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>

// ================== Réglages ==================
const int  myInput = AUDIO_INPUT_MIC;     // ou AUDIO_INPUT_LINEIN
const char* FILENAME = "RECORD.RAW";

const int BUTTON_PIN = 0;                // bouton externe vers GND
const int LED_PIN    = 13;

const uint32_t LONG_PRESS_MS      = 3000;  // seuil appui long
const uint32_t RECORD_DURATION_MS = 3000;  // durée d'enregistrement
// =============================================

Bounce button(BUTTON_PIN, 15);

// ================== Audio ==================
AudioPlaySdRaw     playRaw1;
AudioInputI2S      i2sIn;
AudioRecordQueue   queue1;
AudioMixer4        mixer1;
AudioOutputI2S     i2sOut;

AudioConnection patchCord1(playRaw1, 0, mixer1, 0);  // SD -> mixer
AudioConnection patchCord2(i2sIn,   0, queue1, 0);   // entrée -> record queue
AudioConnection patchCord3(mixer1,  0, i2sOut, 0);   // -> L
AudioConnection patchCord4(mixer1,  0, i2sOut, 1);   // -> R

AudioControlSGTL5000 sgtl5000_1;

// ================== SD / fichier ==================
File frec;

// ================== Etat ==================
enum State { IDLE_NO_REC, RECORDING, READY, PLAYING };
State state = IDLE_NO_REC;

bool hasRecording = false;               // devient true uniquement après 1 enregistrement terminé

uint32_t pressStartMs = 0;
bool longTriggeredThisPress = false;

uint32_t recordStartMs = 0;

// SD (Audio Shield)
#define SDCARD_CS_PIN 10
#if defined(__IMXRT1062__)   // Teensy 4.x
  #define SDCARD_MOSI_PIN 11
  #define SDCARD_SCK_PIN  13
#else                        // Teensy 3.x
  #define SDCARD_MOSI_PIN 7
  #define SDCARD_SCK_PIN  14
#endif

// ---------- Enregistrement ----------
void startRecording()
{
  // Stop lecture si en cours (sinon conflit)
  if (state == PLAYING) {
    playRaw1.stop();
  }

  // Écrase l'ancien
  if (SD.exists(FILENAME)) SD.remove(FILENAME);

  frec = SD.open(FILENAME, FILE_WRITE);
  if (!frec) {
    // échec => on reste "pas d'enregistrement"
    hasRecording = false;
    state = IDLE_NO_REC;
    digitalWrite(LED_PIN, LOW);
    return;
  }

  queue1.begin();
  recordStartMs = millis();
  state = RECORDING;

  // LED ON exactement au moment où l'enregistrement démarre (à 3s pile)
  digitalWrite(LED_PIN, HIGH);
}

void stopRecording(bool success)
{
  // vider ce qui reste dans la queue
  while (queue1.available() > 0) {
    frec.write((byte*)queue1.readBuffer(), 256); // 128 samples * 2 bytes
    queue1.freeBuffer();
  }

  queue1.end();
  frec.close();

  digitalWrite(LED_PIN, LOW);

  hasRecording = success && SD.exists(FILENAME);
  state = hasRecording ? READY : IDLE_NO_REC;
}

void serviceRecording()
{
  // écrire au fil de l'eau
  while (queue1.available() > 0) {
    frec.write((byte*)queue1.readBuffer(), 256);
    queue1.freeBuffer();
  }

  // stop auto après 3s
  if (millis() - recordStartMs >= RECORD_DURATION_MS) {
    stopRecording(true);
  }
}

// ---------- Lecture ----------
void startPlaybackFromBeginning()
{
  if (!hasRecording) return;                 // avant 1er appui long : appui court ne fait rien
  if (!SD.exists(FILENAME)) {                // sécurité
    hasRecording = false;
    state = IDLE_NO_REC;
    return;
  }

  // relancer depuis le début
  playRaw1.stop();
  delay(2);
  playRaw1.play(FILENAME);

  // attendre un tout petit moment que le player démarre vraiment
  uint32_t t0 = millis();
  while (!playRaw1.isPlaying() && (millis() - t0) < 200) {
    delay(1);
  }

  if (playRaw1.isPlaying()) {
    state = PLAYING;
  } else {
    // si ça n'a pas démarré, on reste prêt
    state = READY;
  }
}

void stopPlayback()
{
  playRaw1.stop();
  state = READY;
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  AudioMemory(120);

  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.6);

  if (myInput == AUDIO_INPUT_MIC) {
    sgtl5000_1.micGain(45);  // ajuste 20..55 selon ton micro
  }

  mixer1.gain(0, 1.0f); // SD playback -> sortie

  // SD init
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);

  bool ok = SD.begin(SDCARD_CS_PIN);
#ifdef BUILTIN_SDCARD
  if (!ok) ok = SD.begin(BUILTIN_SDCARD);
#endif
  if (!ok) {
    while (1) delay(200);
  }

  // AU DEMARRAGE : aucun enregistrement n'est considéré -> on efface le fichier s'il existe
  if (SD.exists(FILENAME)) SD.remove(FILENAME);
  hasRecording = false;
  state = IDLE_NO_REC;
}

void loop()
{
  button.update();

  // Début d'appui
  if (button.fallingEdge()) {
    pressStartMs = millis();
    longTriggeredThisPress = false;
  }

  // Appui maintenu : déclenchement EXACT à 3s
  if (button.read() == LOW && !longTriggeredThisPress) {
    if (millis() - pressStartMs >= LONG_PRESS_MS) {
      longTriggeredThisPress = true;

      // Déclenche l'enregistrement (une seule fois par appui)
      if (state != RECORDING) {
        startRecording();
      }
    }
  }

  // Fin d'appui : si ce n'était PAS un long press => short press
  if (button.risingEdge()) {
    if (!longTriggeredThisPress) {
      // short press :
      // - si ça joue -> stop
      // - sinon -> play depuis le début (mais seulement après un enregistrement)
      if (state == PLAYING) {
        stopPlayback();
      } else if (state != RECORDING) {
        startPlaybackFromBeginning();
      }
    }
    // si long press : on ne fait rien au relâchement
  }

  // Enregistrement en cours (continue + stop auto après 3s)
  if (state == RECORDING) {
    serviceRecording();
  }

  // Fin lecture : repasser READY (et appui court rejoue)
  if (state == PLAYING && !playRaw1.isPlaying()) {
    state = READY;
  }
}


