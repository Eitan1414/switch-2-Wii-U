#include "ui_extras.hpp"

#include "ui.hpp"

#include <algorithm>
#include <cmath>

namespace {

void circle(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; ++y) {
        const int width = static_cast<int>(std::sqrt(static_cast<float>(radius * radius - y * y)));
        SDL_RenderDrawLine(renderer, cx - width, cy + y, cx + width, cy + y);
    }
}

void rounded(SDL_Renderer* renderer, SDL_Rect rect, int radius, SDL_Color color) {
    radius = std::clamp(radius, 0, std::min(rect.w, rect.h) / 2);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect horizontal{rect.x + radius, rect.y, rect.w - radius * 2, rect.h};
    SDL_Rect vertical{rect.x, rect.y + radius, rect.w, rect.h - radius * 2};
    SDL_RenderFillRect(renderer, &horizontal);
    SDL_RenderFillRect(renderer, &vertical);
    circle(renderer, rect.x + radius, rect.y + radius, radius, color);
    circle(renderer, rect.x + rect.w - radius - 1, rect.y + radius, radius, color);
    circle(renderer, rect.x + radius, rect.y + rect.h - radius - 1, radius, color);
    circle(renderer, rect.x + rect.w - radius - 1, rect.y + rect.h - radius - 1, radius, color);
}

SDL_Color tone(SDL_Color color, int amount, Uint8 alpha = 255) {
    return {
        static_cast<Uint8>(std::clamp(static_cast<int>(color.r) + amount, 0, 255)),
        static_cast<Uint8>(std::clamp(static_cast<int>(color.g) + amount, 0, 255)),
        static_cast<Uint8>(std::clamp(static_cast<int>(color.b) + amount, 0, 255)),
        alpha
    };
}

void thickLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2,
               int thickness, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const int half = thickness / 2;
    for (int offset = -half; offset <= half; ++offset) {
        SDL_RenderDrawLine(renderer, x1 + offset, y1, x2 + offset, y2);
        SDL_RenderDrawLine(renderer, x1, y1 + offset, x2, y2 + offset);
    }
}

void gridIcon(AppContext& context, SDL_Rect rect, SDL_Color accent, int lift) {
    const int gap = std::max(5, rect.w / 18);
    const int size = (rect.w - gap * 3) / 2;
    for (int index = 0; index < 4; ++index) {
        SDL_Rect tile{rect.x + gap + (index % 2) * (size + gap),
                      rect.y + gap + (index / 2) * (size + gap) + ((index % 2) ? lift : -lift),
                      size, size};
        rounded(context.renderer, tile, std::max(5, size / 7),
                tone(accent, 35 - index * 12, 240));
    }
}

void softwareIcon(AppContext& context, SDL_Rect rect, SDL_Color accent, int lift) {
    SDL_Rect window{rect.x + 10, rect.y + 14 + lift, rect.w - 20, rect.h - 28};
    rounded(context.renderer, window, 12, tone(accent, 60, 245));
    SDL_Rect bar{window.x, window.y, window.w, std::max(18, window.h / 5)};
    rounded(context.renderer, bar, 10, accent);
    SDL_SetRenderDrawColor(context.renderer, accent.r, accent.g, accent.b, 210);
    const int cellW = (window.w - 30) / 2;
    for (int index = 0; index < 4; ++index) {
        SDL_Rect cell{window.x + 10 + (index % 2) * (cellW + 10),
                      bar.y + bar.h + 10 + (index / 2) * 31,
                      cellW, 21};
        SDL_RenderFillRect(context.renderer, &cell);
    }
}

void homebrewIcon(AppContext& context, SDL_Rect rect, SDL_Color accent, int lift) {
    const int cx = rect.x + rect.w / 2;
    const int cy = rect.y + rect.h / 2 + lift;
    const SDL_Color glass = tone(accent, 55);
    thickLine(context.renderer, cx - 15, cy - 45, cx + 15, cy - 45, 5, glass);
    thickLine(context.renderer, cx - 9, cy - 45, cx - 9, cy - 17, 5, glass);
    thickLine(context.renderer, cx + 9, cy - 45, cx + 9, cy - 17, 5, glass);
    SDL_Rect flask{cx - 43, cy - 18, 86, 67};
    rounded(context.renderer, flask, 28, tone(accent, 55, 245));
    SDL_Rect liquid{flask.x + 7, flask.y + 31, flask.w - 14, flask.h - 38};
    rounded(context.renderer, liquid, 13, tone(accent, 0, 235));
    circle(context.renderer, cx - 14, cy + 20, 7, {255, 255, 255, 180});
    circle(context.renderer, cx + 17, cy + 31, 4, {255, 255, 255, 190});
}

void heartIcon(AppContext& context, SDL_Rect rect, SDL_Color accent, int lift) {
    const int cx = rect.x + rect.w / 2;
    const int cy = rect.y + rect.h / 2 + lift;
    const int radius = std::max(22, rect.w / 5);
    circle(context.renderer, cx - radius / 2, cy - radius / 3, radius / 2 + 3, accent);
    circle(context.renderer, cx + radius / 2, cy - radius / 3, radius / 2 + 3, accent);
    SDL_SetRenderDrawColor(context.renderer, accent.r, accent.g, accent.b, accent.a);
    for (int row = 0; row < radius; ++row) {
        SDL_RenderDrawLine(context.renderer, cx - radius + row, cy - radius / 3 + row,
                          cx + radius - row, cy - radius / 3 + row);
    }
}

void clockIcon(AppContext& context, SDL_Rect rect, SDL_Color accent, int lift) {
    const int cx = rect.x + rect.w / 2;
    const int cy = rect.y + rect.h / 2 + lift;
    const int radius = std::max(28, rect.w / 3);
    circle(context.renderer, cx, cy, radius, tone(accent, 55, 245));
    circle(context.renderer, cx, cy, radius - 6, {255, 255, 255, 235});
    thickLine(context.renderer, cx, cy, cx, cy - radius / 2, 4, accent);
    thickLine(context.renderer, cx, cy, cx + radius / 2, cy + radius / 4, 4, accent);
    circle(context.renderer, cx, cy, 5, accent);
}

} // namespace

void drawCategoryArtwork(AppContext& context, const SDL_Rect& rect,
                         CategoryArtwork artwork, SDL_Color accent,
                         bool selected, uint64_t ticks) {
    const float phase = static_cast<float>(ticks % 10000ULL) / 1000.0f;
    const int lift = selected ? static_cast<int>(std::sin(phase * 2.1f) * 3.0f) : 0;
    rounded(context.renderer, rect, std::max(12, rect.w / 12),
            tone(accent, 75, selected ? 242 : 224));
    SDL_Rect inner{rect.x + 9, rect.y + 9, rect.w - 18, rect.h - 18};

    switch (artwork) {
        case CategoryArtwork::Collection: drawFolderIcon(context, inner, accent); break;
        case CategoryArtwork::AllGames: gridIcon(context, inner, accent, lift); break;
        case CategoryArtwork::Software: softwareIcon(context, inner, accent, lift); break;
        case CategoryArtwork::Homebrew: homebrewIcon(context, inner, accent, lift); break;
        case CategoryArtwork::Favorites: heartIcon(context, inner, accent, lift); break;
        case CategoryArtwork::Recent: clockIcon(context, inner, accent, lift); break;
    }
}
