#include "ui_extras.hpp"

#include "ui.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

namespace {

void circle(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    if (radius <= 0) return;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; ++y) {
        const int width = static_cast<int>(std::sqrt(static_cast<float>(radius * radius - y * y)));
        SDL_RenderDrawLine(renderer, cx - width, cy + y, cx + width, cy + y);
    }
}

void rounded(SDL_Renderer* renderer, SDL_Rect rect, int radius, SDL_Color color) {
    if (rect.w <= 0 || rect.h <= 0) return;
    radius = std::clamp(radius, 0, std::min(rect.w, rect.h) / 2);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect horizontal{rect.x + radius, rect.y, rect.w - radius * 2, rect.h};
    SDL_Rect vertical{rect.x, rect.y + radius, rect.w, rect.h - radius * 2};
    if (horizontal.w > 0) SDL_RenderFillRect(renderer, &horizontal);
    if (vertical.h > 0) SDL_RenderFillRect(renderer, &vertical);
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

SDL_Color glassTone(SDL_Color fill, bool selected) {
    const int red = (static_cast<int>(fill.r) + 410) / 3;
    const int green = (static_cast<int>(fill.g) + 445) / 3;
    const int blue = (static_cast<int>(fill.b) + 510) / 3;
    return {
        static_cast<Uint8>(std::clamp(red, 0, 255)),
        static_cast<Uint8>(std::clamp(green, 0, 255)),
        static_cast<Uint8>(std::clamp(blue, 0, 255)),
        static_cast<Uint8>(selected ? 238 : 211)
    };
}

} // namespace

void drawAmbientBackground(AppContext& context, SDL_Color accent, uint64_t ticks) {
    const float time = static_cast<float>(ticks % 120000ULL) / 1000.0f;

    // Couche bleue translucide : elle colore le fond standard mais laisse
    // toujours apparaître les images UTheme ou le fond personnalisé dessous.
    SDL_SetRenderDrawBlendMode(context.renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect content{0, 106, SCREEN_WIDTH, 472};
    SDL_SetRenderDrawColor(context.renderer, 18, 67, 145, 68);
    SDL_RenderFillRect(context.renderer, &content);

    for (int band = 0; band < 8; ++band) {
        SDL_Rect strip{0, 106 + band * 59, SCREEN_WIDTH, 60};
        SDL_SetRenderDrawColor(context.renderer,
                               static_cast<Uint8>(15 + band * 2),
                               static_cast<Uint8>(55 + band * 3),
                               static_cast<Uint8>(125 + band * 5),
                               static_cast<Uint8>(18 + band * 2));
        SDL_RenderFillRect(context.renderer, &strip);
    }

    for (int index = 0; index < 5; ++index) {
        const float phase = time * (0.09f + index * 0.018f) + index * 1.61f;
        const int x = 105 + index * 280 + static_cast<int>(std::sin(phase) * 54.0f);
        const int y = 175 + (index % 2) * 235 + static_cast<int>(std::cos(phase * 0.8f) * 34.0f);
        circle(context.renderer, x, y, 82 + index * 11, tone(accent, 60, 12));
        circle(context.renderer, x, y, 44 + index * 7, tone(accent, 90, 8));
    }

    // Petites particules lumineuses, calculées sans texture ni allocation.
    for (int index = 0; index < 22; ++index) {
        const float speed = 7.0f + static_cast<float>((index % 5) * 2);
        const float travel = std::fmod(time * speed + static_cast<float>(index * 79),
                                      static_cast<float>(SCREEN_WIDTH + 80));
        const int x = static_cast<int>(travel) - 40;
        const int baseY = 128 + (index * 83) % 420;
        const int y = baseY + static_cast<int>(std::sin(time * 0.55f + index) * 13.0f);
        const int radius = 1 + index % 3;
        circle(context.renderer, x, y, radius,
               {235, 247, 255, static_cast<Uint8>(36 + (index % 4) * 17)});
    }
}

void drawCardShadow(AppContext& context, const SDL_Rect& card, bool selected) {
    const int layers = selected ? 8 : 5;
    for (int layer = layers; layer >= 1; --layer) {
        SDL_Rect shadow{card.x - layer, card.y + layer * 2,
                        card.w + layer * 2, card.h + layer};
        rounded(context.renderer, shadow, selected ? 19 : 15,
                {5, 17, 39, static_cast<Uint8>(4 + layer * (selected ? 4 : 3))});
    }
}

void drawGlassPanel(AppContext& context, const SDL_Rect& rect,
                    SDL_Color fill, SDL_Color border, int borderWidth) {
    const uint64_t ticks = SDL_GetTicks64();
    const uint64_t frameBucket = ticks / 8ULL;
    static uint64_t decoratedFrame = std::numeric_limits<uint64_t>::max();
    if (frameBucket != decoratedFrame) {
        decoratedFrame = frameBucket;
        drawAmbientBackground(context, border, ticks);
    }

    const bool selected = borderWidth >= 5;
    drawCardShadow(context, rect, selected);

    if (selected) {
        for (int glow = 5; glow >= 1; --glow) {
            SDL_Rect aura{rect.x - glow * 3, rect.y - glow * 3,
                          rect.w + glow * 6, rect.h + glow * 6};
            rounded(context.renderer, aura, 22 + glow,
                    {border.r, border.g, border.b,
                     static_cast<Uint8>(5 + (6 - glow) * 7)});
        }
    }

    const int radius = selected ? 21 : 17;
    rounded(context.renderer, rect, radius,
            {border.r, border.g, border.b,
             static_cast<Uint8>(selected ? 235 : 145)});

    const int inset = std::max(2, std::min(6, borderWidth));
    SDL_Rect inner{rect.x + inset, rect.y + inset,
                   rect.w - inset * 2, rect.h - inset * 2};
    rounded(context.renderer, inner, std::max(8, radius - inset),
            glassTone(fill, selected));

    SDL_Rect shine{inner.x + 13, inner.y + 10,
                   std::max(0, inner.w - 26), std::max(5, inner.h / 13)};
    rounded(context.renderer, shine, std::max(3, shine.h / 2),
            {255, 255, 255, static_cast<Uint8>(selected ? 92 : 60)});

    SDL_Rect lowerTint{inner.x + 8, inner.y + inner.h * 3 / 4,
                       std::max(0, inner.w - 16), std::max(0, inner.h / 4 - 8)};
    rounded(context.renderer, lowerTint, 10,
            {35, 98, 186, static_cast<Uint8>(selected ? 28 : 17)});
}

void drawGlassDock(AppContext& context) {
    SDL_Rect shadow{348, 588, 584, 98};
    rounded(context.renderer, shadow, 28, {4, 13, 31, 58});
    SDL_Rect panel{352, 582, 576, 96};
    rounded(context.renderer, panel, 27, {221, 239, 255, 174});
    SDL_Rect shine{panel.x + 22, panel.y + 9, panel.w - 44, 8};
    rounded(context.renderer, shine, 4, {255, 255, 255, 75});
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
        if (index == active) {
            circle(context.renderer, startX + static_cast<int>(index) * 18, y,
                   8, {53, 185, 255, 38});
        }
        circle(context.renderer, startX + static_cast<int>(index) * 18, y,
               index == active ? 5 : 3,
               index == active ? SDL_Color{19, 165, 231, 245}
                               : SDL_Color{222, 239, 255, 145});
    }
}
