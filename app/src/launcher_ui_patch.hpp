#pragma once

#include "ui.hpp"
#include "ui_extras.hpp"

#include <string>

inline CategoryArtwork categoryArtworkForName(const std::string& name) {
    if (name == "Tous les jeux") return CategoryArtwork::AllGames;
    if (name == "Logiciels Wii U") return CategoryArtwork::Software;
    if (name == "Homebrews") return CategoryArtwork::Homebrew;
    if (name == "Favoris") return CategoryArtwork::Favorites;
    if (name == "Recents") return CategoryArtwork::Recent;
    return CategoryArtwork::Collection;
}

#define drawFolderIcon(contextValue, rectValue, colorValue)                         \
    drawCategoryArtwork((contextValue), (rectValue),                               \
                        categoryArtworkForName(entry.name), (colorValue),           \
                        selected, now)

#define drawRoundedPanel(contextValue, rectValue, fillValue, borderValue, widthValue) \
    do {                                                                              \
        drawCardShadow((contextValue), (rectValue), (widthValue) >= 5);                \
        drawRoundedPanel((contextValue), (rectValue), (fillValue),                    \
                         (borderValue), (widthValue));                                  \
    } while (false)

#define drawSystemDock(contextValue, selectedIndexValue)                            \
    do {                                                                             \
        drawPositionIndicator((contextValue), selection, visible.size(), 640, 585); \
        drawSystemDock((contextValue), (selectedIndexValue));                       \
    } while (false)
