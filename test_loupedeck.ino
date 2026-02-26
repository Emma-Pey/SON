#include "USBHost_t36.h"

USBHost myusb;
KeyboardController keyboard1(myusb);

void setup() {
  Serial.begin(9600);
  myusb.begin();
  keyboard1.attachRawPress(OnRawPress); // Déclenche une fonction direct à l'appui
  Serial.println("Pret : Branchez le Loupedeck sur le port USB Host du Teensy");
}

void loop() {
  myusb.Task(); // Gère la pile USB
}

void OnRawPress(uint8_t keycode) {
  Serial.print("Touche direct (Keycode): ");
  Serial.println(keycode);
  
  // Ici, le Teensy réagit à la microseconde près !
}