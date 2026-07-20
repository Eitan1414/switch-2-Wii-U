# Switch2 Mode pour Wii U

Switch2 Mode est un lanceur graphique pour Wii U sous Aroma. Il affiche les jeux,
les logiciels Wii U et les homebrews, reprend les collections du menu Wii U et
utilise la video fournie comme introduction.

## Installation

Extraire `Switch2Mode_SD.zip` a la racine de la carte SD :

```text
SD:/wiiu/apps/Switch2Mode.wuhb
SD:/wiiu/environments/aroma/plugins/Switch2ModePlugin.wps
```

Redemarrer Aroma, ouvrir **Switch2 Mode**, puis choisir **Activer et lancer**.
Maintenir **B** a l'arrivee sur le menu Wii U ignore le lancement automatique.

## Interface v0.8.0

La v0.8.0 reprend le style bleu lumineux valide :

- cartes translucides avec effet verre ;
- halo bleu autour de la selection ;
- ombres et reflets ;
- particules lumineuses legeres ;
- dock inferieur translucide ;
- indicateur de position ;
- animation legere des illustrations ;
- textes limites a leur zone.

Les categories utilisent une console et un GamePad pour `Tous les jeux`, un sac
orange pour `Logiciels Wii U`, un dossier beige avec contour colore pour les
collections, une fiole pour `Homebrews`, une etoile pour `Favoris` et une horloge
pour `Recents`.

L'icone verte de la barre inferieure correspond a **Miiverse**. Le sac orange
correspond a **Nintendo eShop**.

## Compatibilite UTheme / StyleMiiU

La compatibilite UTheme est conservee et reste le mode de fond par defaut.
Le nouveau rendu ne remplace pas le theme : une couche bleue translucide est
dessinee par-dessus afin que l'image UTheme reste visible sous les cartes, les
halos et les particules.

Une image personnelle peut toujours etre placee dans :

```text
SD:/wiiu/switch2mode/background.png
```

Le panneau permet toujours de choisir le fond standard, UTheme ou le fond
personnalise.

## Collections Wii U

Switch2 Mode lit `BaristaAccountSaveFile.dat` en lecture seule. Les collections
sont affichees avec leur nom, leur couleur, leurs jeux et leur ordre. Le repertoire
de profil `8000000X` et la redirection Homebrew on Wii U Menu sont pris en charge.

## Jeux, logiciels et homebrews

- `Tous les jeux` : titres utilisateur installes.
- `Logiciels Wii U` : applications et logiciels reconnus.
- `Homebrews` : fichiers `.wuhb` et `.rpx` trouves recursivement dans `SD:/wiiu/apps`.
- `Favoris` et `Recents` : vues propres a Switch2 Mode.

## Sons et animations

Les tiroirs possedent une animation et des sons distincts pour l'ouverture et la
fermeture. Une musique de fond originale est generee par l'application et s'arrete
pendant l'introduction et le lancement d'un titre.

## Limites alpha

- Les collections restent en lecture seule.
- Certains logiciels systeme peuvent refuser un lancement direct.
- Un test sur une vraie Wii U reste necessaire.
