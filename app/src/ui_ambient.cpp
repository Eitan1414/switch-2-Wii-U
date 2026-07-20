#include "ui_extras.hpp"

#include "ui.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

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

SDL_Color tone(SDL_Color color, int amount, Uint8 alpha) {
    return {
        static_cast<Uint8>(std::clamp(static_cast<int>(color.r) + amount, 0, 255)),
        static_cast<Uint8>(std::clamp(static_cast<int>(color.g) + amount, 0, 255)),
        static_cast<Uint8>(std::clamp(static_cast<int>(color.b) + amount, 0, 255)),
        alpha
    };
}

} // namespace

void drawAmbientBackground(AppContext& context, SDL_Color accent, uint64_t ticks) {
    const float time = static_cast<float>(ticks % 120000ULL) / 1000.0f;
    for (int index = 0; index < 4; ++index) {
        const float phase = time * (0.10f + index * 0.02f) + index * 1.7f;
        const int x = 170 + index * 315 + static_cast<int>(std::sin(phase) * 42.0f);
        const int y = 190 + (index % 2) * 235 + static_cast<int>(std::cos(phase) * 28.0f);
        circle(context.renderer, x, y, 92 + index * 13, tone(accent, 50, 11));
    }
}

void drawCardShadow(AppContext& context, const SDL_Rect& card, bool selected) {
    const int layers = selected ? 7 : 4;
    for (int layer = layers; layer >= 1; --layer) {
        SDL_Rect shadow{card.x - layer, card.y + layer * 2,
                        card.w + layer * 2, card.h + layer};
        rounded(context.renderer, shadow, selected ? 16 : 12,
                {12, 18, 28, static_cast<Uint8>(5 + layer * (selected ? 3 : 2))});
    }
}

void drawTypeBadge(AppContext& context, const SDL_Rect& card,
                   const std::string& label, SDL_Color accent) {
    SDL_Rect badge{card.x + 10, card.y + 10, std::min(112, card.w - 20), 27};
    rounded(context.renderer, badge, 13, tone(accent, -14, 230));
    drawTextFitted(context, context.fontSmall, label,
                   badge.x + badge.w / 2, badge.y - 1,
                   badge.w - 12, {255, 255, 255, 255}, true);
}

void drawCountBadge(AppContext& context, const SDL_Rect& card,
                    std::size_t count, SDL_Color accent) {
    char text[24];
    std::snprintf(text, sizeof(text), "%zu", count);
    const int width = count >= 100 ? 48 : count >= 10 ? 40 : 34;
    SDL_Rect badge{card.x + card.w - width - 10, card.y + 10, width, 28};
    rounded(context.renderer, badge, 14, tone(accent, -18, 236));
    drawTextFitted(context, context.fontSmall, text,
                   badge.x + badge.w / 2, badge.y,
                   badge.w - 8, {255, 255, 255, 255}, true);
}

void drawPositionIndicator(AppContext& context, std::size_t selected,
                           std::size_t total, int centerX, int y) {
    if (total <= 1) return;
    const std::size_t dots = std::min<std::size_t>(total, 9);
    const std::size_t active = total > dots
                                   ? std::min(dots - 1, selected * dots / total)
                                   : selected;
    const int startX = centerX - static_cast<int>((dots - 1) * 18) / 2;
    for (std::size_t index = 0; index < dots; ++index) {
        circle(context.renderer, startX + static_cast<int>(index) * 18, y,
               index == active ? 5 : 3,
               index == active ? SDL_Color{0, 168, 218, 235}
                               : SDL_Color{122, 130, 143, 110});
    }
}
