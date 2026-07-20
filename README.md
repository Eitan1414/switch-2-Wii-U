# Switch2 Mode pour Wii U

Switch2 Mode est un lanceur graphique pour Wii U sous Aroma. Il affiche les jeux
horizontalement, reprend les collections creees dans le menu Wii U, propose des
transitions glissees et utilise la video fournie comme introduction. Le lecteur
de collections fonctionne en lecture seule et ne modifie aucun fichier de la NAND.

## Contenu

- `Switch2Mode.wuhb` : application graphique et lanceur de jeux.
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
`BaristaAccountSaveFile.dat`. La version 0.7.1 utilise correctement le repertoire
utilisateur `8000000X`, comme le menu Wii U et Homebrew on Wii U Menu.

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

Les elements **Tous les jeux**, **Favoris** et **Recents** sont des vues rapides
propres a Switch2 Mode. Ce ne sont pas les collections creees par l'utilisateur
sur le menu Wii U.

## Animations et sons des tiroirs

L'ouverture d'une collection lance maintenant une animation dediee : le couvercle
du tiroir se souleve et des cartes de jeux en sortent. La fermeture joue l'effet
en sens inverse.

Les tiroirs utilisent aussi deux signatures sonores distinctes :

- ouverture : glissement spatialise et carillon ;
- fermeture : retour spatialise et glissement inverse.

Ces effets reutilisent les sons originaux du projet et ne contiennent aucun
fichier audio Nintendo.

## Commandes

- Gauche / Droite : parcourir les jeux, les collections et les vues rapides.
- A : ouvrir une collection ou lancer le jeu selectionne.
- B : refermer une collection ou ouvrir le panneau du mode.
- X : ouvrir les options.
- Y : ajouter ou retirer le jeu selectionne des favoris Switch2 Mode.
- Plus : ouvrir les options.
- Bas : passer de la liste des jeux a la barre d'actions circulaires.
- Haut : revenir de la barre d'actions aux jeux.

La barre inferieure reprend les six raccourcis classiques de la Wii U dans des
cercles et avec une selection animee inspiree de la Switch 2 : Miiverse,
Nintendo eShop, navigateur Internet, notifications, liste d'amis et gestion des
telechargements.

La barre superieure affiche le nom du profil actif, l'etat Wi-Fi, l'heure et le
niveau de batterie reel du GamePad.

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

- Les collections Wii U sont importees en lecture seule : leur modification se
  fait toujours depuis le menu Wii U original.
- Les jeux Wii U installes sont relies aux collections grace a leur Title ID.
- Les homebrews `.wuhb`/`.rpx` autonomes de `SD:/wiiu/apps` restent visibles a
  l'accueil lorsqu'ils ne possedent pas d'identifiant correspondant dans le menu.
