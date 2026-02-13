# SON
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
- quels problèmes on a cherché à résoudre (ex : trop de boutons et pas assez de pins sur le teensy; jouer plusieurs sons en même temps grâce à 3 mixer de 4 voies (on peut faire un petit schéma, l'enregistrement qui commence avec du vide parce qu'on n'a pas encore parlé, problème de mémoire RAM du teensy avec faust noise gate => trouvé une librairie sur github qui le fait en utilisant moins de mémoire)
- comment on les a résolus
