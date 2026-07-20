# Switch2 Mode pour Wii U

Switch2 Mode est un lanceur graphique pour Wii U sous Aroma. Il affiche les jeux
horizontalement, propose des dossiers, des transitions glissees et utilise la
video fournie comme introduction. Le projet ne modifie aucun fichier de la NAND.

## Contenu

- `Switch2Mode.wuhb` : application graphique et lanceur de jeux.
- `Switch2ModePlugin.wps` : relance automatiquement le mode lorsqu'il est actif.
- `Boot_WiiU_Switch2_16x9_Corrige_30FPS.mp4` : video source corrigee fournie par l'utilisateur.

## Installation

Apres compilation, extraire `Switch2Mode_SD.zip` a la racine de la carte SD :

```text
SD:/wiiu/apps/Switch2Mode.wuhb
SD:/wiiu/environments/aroma/plugins/Switch2ModePlugin.wps
```

Redemarrer Aroma, ouvrir **Switch2 Mode**, puis choisir **Activer et lancer**.
Le mode se rouvrira ensuite automatiquement au demarrage et apres la fermeture
d'un jeu. L'intro complete n'est jouee qu'au premier lancement suivant le
demarrage d'Aroma ; elle n'est donc pas repetee apres chaque jeu.

Maintenir **B** pendant l'arrivee sur le menu Wii U ignore le lancement
automatique pour cette fois. Le mode sans echec d'Aroma reste egalement
disponible avec `L + Haut + Minus` pendant le demarrage.

## Commandes

- Gauche / Droite : parcourir les jeux et dossiers.
- A : ouvrir ou lancer.
- B : fermer un dossier ou ouvrir le panneau du mode.
- X : ouvrir les options.
- Y : ajouter ou retirer le jeu selectionne des favoris.
- Plus : ouvrir les options.
- Bas : passer de la liste des jeux a la barre d'actions circulaires.
- Haut : revenir de la barre d'actions aux jeux.

La barre inferieure reprend les six raccourcis classiques de la Wii U dans des
cercles et avec une selection animee inspiree de la Switch 2 : Miiverse,
Nintendo eShop, navigateur Internet, notifications, liste d'amis et gestion des
telechargements. Chaque bouton est relie a l'application systeme correspondante.
Miiverse et les fonctions en ligne de l'eShop dependent des services encore
disponibles sur la console (Nintendo ou remplacement communautaire).

La barre superieure affiche le nom du profil actif, l'etat Wi-Fi, l'heure et le
niveau de batterie reel du GamePad. Cinq sons originaux accompagnent le
deplacement, la validation, le retour, l'ouverture des dossiers et le lancement.

Au lancement d'un jeu ou d'un homebrew, un carillon numerique ascendant original
accompagne son icone qui se recentre et s'agrandit. Le menu s'assombrit, puis un
fondu noir masque le passage vers le titre. Le son est synchronise sur les 0,86
seconde de la transition et ne reprend aucun fichier audio Nintendo.

## UTheme / StyleMiiU

Le menu Wii U original et ses themes ne sont jamais remplaces. Dans le lanceur,
le mode `UTheme` cherche le theme selectionne dans la configuration StyleMiiU,
puis utilise `preview-launcher.webp`, `.png` ou `.jpg` comme arriere-plan. Si le
theme n'en fournit pas, le fond blanc est utilise. Une image personnelle peut
etre placee dans :

```text
SD:/wiiu/switch2mode/background.png
```

## Compilation

Le workflow GitHub Actions compile automatiquement l'application et le plugin,
puis produit `Switch2Mode_SD.zip`. Pour compiler avec Docker :

```bash
docker build -t switch2mode-builder .
docker run --rm -v "$PWD":/project switch2mode-builder ./scripts/build.sh
```

La video MP4 corrigee est transformee pendant la preparation des ressources en
276 images WebP de 960x540 a 30 images/s et une piste Ogg. Cela evite d'embarquer
un decodeur H.264 lourd dans l'application Wii U.

## Limites de cette premiere version

- Les dossiers fournis sont `Tous les jeux`, `Favoris` et `Recents`.
- Les titres Wii U installes et les homebrews `.wuhb`/`.rpx` du dossier
  `SD:/wiiu/apps` sont listes.
- Les apercus UTheme servent de fond lorsque le theme ne fournit pas directement
  une texture de fond exploitable.
