void setup() {
  Serial.begin(9600);
  while (!Serial); // Attend l'ouverture du moniteur
  Serial.println("--- TESTEUR DIRECT IDE ARDUINO ---");
  Serial.println("Configurez le Loupedeck pour envoyer: Touche + Entree");
}

void loop() {
  if (Serial.available() > 0) {
    char key = Serial.read();

    // On ignore les caractères de fin de ligne pour ne pas polluer l'affichage
    if (key != '\n' && key != '\r') {
      Serial.print("Action detectee : [");
      Serial.print(key);
      Serial.print("] - Code ASCII : ");
      Serial.println((int)key);
      
      // Simulation de réaction
      if (key == 'q') Serial.println("  -> Molette GAUCHE");
      if (key == 'd') Serial.println("  -> Molette DROITE");
    }
  }
}