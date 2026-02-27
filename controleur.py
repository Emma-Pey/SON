# controleur2.py  -- renvoyez ce fichier sur votre Teensy
import sys
import serial
import keyboard
import serial.tools.list_ports

# ------------------------------------------------------------------
# liste rapide des ports dispo (pour vous aider à choisir)
ports = [p.device for p in serial.tools.list_ports.comports()]
print("ports série détectés :", ports)
port = input("Port série à utiliser (ex: COM3 ou /dev/ttyACM0) : ").strip()
try:
    ser = serial.Serial(port, 9600, timeout=0.1)
except Exception as e:
    print("ouverture du port échouée :", e)
    sys.exit(1)

print("Ouvert", port, "- appuyez sur ESC pour quitter.")

# touches autorisées
allowed = set(
    list("abcdefghijklmnopqrstuvwxyz")  # sons + record + vol + encodeur
)

def on_event(evt):
    if evt.event_type != keyboard.KEY_DOWN:
        return
    key = evt.name
    if key == "esc":
        print("fin du programme")
        ser.close()
        sys.exit(0)
    # certaines touches reviennent comme 'space', 'enter'… on les ignore
    if len(key) == 1:
        ch = key.lower()
        if ch in allowed:
            try:
                ser.write(ch.encode("ascii"))
            except Exception as err:
                print("Erreur d'envoi :", err)
            else:
                print("envoyé :", repr(ch))
        # else on ne fait rien

# hook global
keyboard.hook(on_event)

# boucle vide, l'événement clavier fait tout le travail
try:
    while True:
        pass
except KeyboardInterrupt:
    ser.close()
    print("arrêt par Ctrl‑C")