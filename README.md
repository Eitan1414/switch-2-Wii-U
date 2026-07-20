# Switch2 Mode pour Wii U

Switch2 Mode est un lanceur graphique pour Wii U sous Aroma. Il affiche les jeux,
les logiciels Wii U et les homebrews dans une interface horizontale, reprend les
collections creees dans le menu Wii U et utilise la video fournie comme introduction.
Le lecteur de collections fonctionne en lecture seule et ne modifie aucun fichier de
la NAND.

## Contenu

- `Switch2Mode.wuhb` : application graphique et lanceur de jeux, logiciels et homebrews.
- `Switch2ModePlugin.wps` : relance automatiquement le mode lorsqu'il est actif.
- `Boot_WiiU_Switch2_16x9_Corrige_30FPS.mp4` : video source corrigee fournie par l'utilisateur.

## Installation

Extraire `Switch2Mode_SD.zip` a la racine de la carte SD :

```text
SD:/wiiu/apps/Switch2Mode.wuhb
SD:/wiiu/environments/aroma/plugins/Switch2ModePlugin.wps
```

Redemarrer Aroma, ouvrir **Switch2 Mode**, puis choisir **Activer et lancer**.
Le mode se rouvrira ensuite automatiquement au demarrage et apres la fermeture
d'un jeu. L'intro complete n'est jouee qu'au premier lancement suivant le
demarrage d'Aroma.

Maintenir **B** pendant l'arrivee sur le menu Wii U ignore le lancement
automatique pour cette fois. Le mode sans echec d'Aroma reste disponible avec
`L + Haut + Minus` pendant le demarrage.

## Collections du menu Wii U

Switch2 Mode lit l'organisation du profil Wii U actif dans
`BaristaAccountSaveFile.dat`. Il utilise correctement le repertoire utilisateur
`8000000X`, comme le menu Wii U et Homebrew on Wii U Menu.

Les collections creees directement depuis le menu Wii U sont affichees avec :

- leur nom ;
- leur couleur ;
- les jeux ranges a l'interieur ;
- l'ordre des jeux dans chaque collection.

Lorsque **Homebrew on Wii U Menu** redirige l'organisation vers la carte SD,
Switch2 Mode utilise automatiquement cette copie active. Sinon, il lit la
sauvegarde native du menu dans la MLC. Cette lecture est strictement en lecture
seule : l'application ne deplace aucun jeu et ne reecrit jamais la sauvegarde du
menu Wii U.

## Jeux, logiciels et homebrews

Les vues rapides comprennent maintenant :

- `Tous les jeux` : titres utilisateur installes ;
- `Logiciels Wii U` : applications systeme et logiciels installes reconnus ;
- `Homebrews` : fichiers `.wuhb` et `.rpx` trouves dans `SD:/wiiu/apps`, y compris
  dans les sous-dossiers ;
- `Favoris` et `Recents` : vues propres a Switch2 Mode.

Les mises a jour et contenus additionnels ne sont pas listes comme applications.
Le menu Wii U lui-meme est exclu pour eviter une entree inutile dans le lanceur.

## Textes, animations et sons

Les noms ne sont plus coupes selon un simple nombre de caracteres. Leur largeur
reelle est mesuree avec la police Wii U :

- deux lignes maximum dans les cartes ;
- points de suspension automatiques dans les barres et titres ;
- largeur limitee dans les boutons, le dock et l'ecran de lancement.

L'ouverture d'une collection joue une animation de tiroir avec des cartes qui en
sortent. La fermeture joue l'effet inverse. Des signatures sonores distinctes sont
utilisees pour l'ouverture et la fermeture.

Une musique de fond originale et discrete est generee directement par
l'application. Aucun fichier musical Nintendo n'est inclus. La musique baisse et
s'arrete pendant l'introduction et le lancement d'un titre.

## Commandes

- Gauche / Droite : parcourir les jeux, collections et vues rapides.
- A : ouvrir une collection ou lancer l'element selectionne.
- B : refermer une collection ou ouvrir le panneau du mode.
- X ou Plus : ouvrir les options.
- Y : ajouter ou retirer un jeu ou logiciel des favoris Switch2 Mode.
- Bas : passer de la liste a la barre d'actions circulaires.
- Haut : revenir de la barre d'actions aux elements.

La barre inferieure reprend les raccourcis Miiverse, Nintendo eShop, navigateur
Internet, notifications, liste d'amis et gestion des telechargements. L'icone verte
fournie correspond a **Miiverse**, le sac orange a **Nintendo eShop**, et le dossier
beige sert de base visuelle aux collections. La Liste d'amis conserve son propre
pictogramme separe.

## UTheme / StyleMiiU

Le menu Wii U original et ses themes ne sont jamais remplaces. Une image
personnelle peut etre placee dans :

```text
SD:/wiiu/switch2mode/background.png
```

## Compilation

Le workflow GitHub Actions compile automatiquement l'application et le plugin,
puis produit `Switch2Mode_SD.zip`.

```bash
docker build -t switch2mode-builder .
docker run --rm -v "$PWD":/project switch2mode-builder ./scripts/build.sh
```

## Limites de cette version alpha

- Les collections Wii U restent en lecture seule.
- Certains logiciels systeme internes peuvent refuser un lancement direct selon
  la region ou la configuration de la console.
- Les homebrews sans icone exploitable utilisent une icone de remplacement.
- La compilation GitHub ne remplace pas un test sur une vraie Wii U.
