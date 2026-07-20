#pragma once

#include "common.hpp"

#include <SDL2/SDL.h>

#include <cstddef>
#include <string>

enum class CategoryArtwork {
    Collection,
    AllGames,
    Software,
    Homebrew,
    Favorites,
    Recent
};

void drawAmbientBackground(AppContext& context, SDL_Color accent, uint64_t ticks);
void drawCardShadow(AppContext& context, const SDL_Rect& card, bool selected);
void drawCategoryArtwork(AppContext& context, const SDL_Rect& rect,
                         CategoryArtwork artwork, SDL_Color accent,
                         bool selected, uint64_t ticks);
void drawTypeBadge(AppContext& context, const SDL_Rect& card,
                   const std::string& label, SDL_Color accent);
void drawCountBadge(AppContext& context, const SDL_Rect& card,
                    std::size_t count, SDL_Color accent);
void drawPositionIndicator(AppContext& context, std::size_t selected,
                           std::size_t total, int centerX, int y);
