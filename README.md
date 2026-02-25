# SON
## dépendances 
TeensyVariablePlayback ?
## A faire 
- refaire les soudures du micro OK
- multiplexeur boutons
- (encodeur rotatif) * 2 effets + volume
- carte SD OK
- boutons tactiles
- ajouter un écran

## Encodeur cliquable
- cliquer pour sélectionner l'effet (affichage en serial)
- Effets :
  - pitch
  - echo (cathedrale?)
  - accélaration
  - noise gate
- suppression du bruit de fond sur un potentiomètre séparé ?
- ça garde en mémoire les paramètres de l'effet pour pouvoir les ajouter

## Boutons
- enregistrer/arrêter + son/led
- (son enregistré : sélectionner le bouton en restant appuyé) * 9
- boutons à son prédéfini


## Poster
- quels problèmes on a cherché à résoudre (ex : trop de boutons et pas assez de pins sur le teensy; jouer plusieurs sons en même temps grâce à 3 mixer de 4 voies (on peut faire un petit schéma), l'enregistrement qui commence avec du vide parce qu'on n'a pas encore parlé, problème de mémoire RAM du teensy avec faust noise gate => trouvé une librairie sur github qui le fait en utilisant moins de mémoire, bruit de fond quand silence (noise gate)
- comment on les a résolus

## Objectif du projet

Nous réalisons une soundboard (table de sons) basée sur un microcontrôleur Teensy 4.0 et sa carte audio (audio shield). Le système permet :
- de jouer des sons prédéfinis via des boutons,
- d’enregistrer un son au micro sur une carte SD,
- puis de relire cet enregistrement dans un casque,
- avec un réglage de volume et une sélection d’effets audio.

## Matériel utilisé (composants)
- 1 casque audio (headphones)
- 1 Teensy 4.0 + audio shield
- 10 boutons
- 2 multiplexeurs
- 1 breadboard
- 1 résistance
- des câbles jumper
- 1 carte SD

## Fonctionnement global
1) Lecture de sons prédéfinis
- 9 boutons sont dédiés à des sons fixes :
- 1 bouton = 1 son
- un appui déclenche immédiatement la lecture du son associé (dans le casque)

2) Bouton d’enregistrement / lecture
- Un 10ᵉ bouton sert à gérer l’enregistrement et la lecture :
- Appui long (> 2,5 s) :
  - lance l’enregistrement via le micro,
  - le fichier est stocké sur la carte SD,
  - l’enregistrement s’arrête automatiquement après 3 s.
- Appui court :
  - lance la lecture de l’enregistrement (dans les écouteurs).

## Contrôle et effets (multiplexeurs)
- Multiplexeur n°1 : contrôle du volume (réglage du niveau sonore).
- Multiplexeur n°2 : sélection d’un effet audio, parmi :

## Shift pitch (changement de hauteur)
- Echo
- Variateur d’accélération (effet de variation du “speed”/tempo)

## Choix techniques et difficultés abordées
- Nous avons décidé d’acheter des boutons tactiles.
-Nous avons étudié comment les faire fonctionner correctement (déclenchement fiable, gestion des appuis, etc.).
- Nous avons aussi réfléchi à la réduction des bruits de fond (parasites / bruit micro / bruit électronique) afin d’améliorer la qualité audio.

## Partie code (à mettre sur le poster) — ce qu’on peut décrire sans tout montrer
Dans le programme, on gère notamment :
- la détection des appuis (différencier appui court / appui long, temporisation)
- le mapping des 9 boutons vers 9 fichiers sons / buffers
- l’enregistrement audio (acquisition micro → écriture sur SD → arrêt auto à 3 s)
- la lecture audio (fichiers SD → sortie casque)
- le contrôle du volume via le multiplexeur 1
- la sélection/activation des effets via le multiplexeur 2