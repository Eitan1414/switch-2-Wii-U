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

#define drawRoundedPanel(contextValue, rectValue, ...)                              \
    drawGlassPanel((contextValue), (rectValue), __VA_ARGS__)

#define drawSystemDock(contextValue, selectedIndexValue)                            \
    do {                                                                             \
        drawGlassDock((contextValue));                                               \
        drawPositionIndicator((contextValue), selection, visible.size(), 640, 575); \
        drawSystemDock((contextValue), (selectedIndexValue));                       \
    } while (false)
