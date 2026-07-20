#include "ui_extras.hpp"

#include "ui.hpp"

#include <algorithm>
#include <array>
#include <cmath>

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

void glow(AppContext& context, int cx, int cy, int radius, SDL_Color accent, bool selected) {
    if (!selected) return;
    for (int layer = 5; layer >= 1; --layer) {
        circle(context.renderer, cx, cy, radius + layer * 5,
               {accent.r, accent.g, accent.b,
                static_cast<Uint8>(5 + (6 - layer) * 5)});
    }
}

void allGamesIcon(AppContext& context, SDL_Rect rect, SDL_Color accent,
                  int lift, bool selected) {
    const int cx = rect.x + rect.w / 2;
    const int cy = rect.y + rect.h / 2 + lift;
    glow(context, cx, cy, rect.w / 3, accent, selected);

    SDL_Rect console{cx - rect.w / 5, cy - rect.h / 3,
                     rect.w * 2 / 5, rect.h / 2};
    rounded(context.renderer, console, 9, {238, 248, 255, 250});
    SDL_Rect screen{console.x + 7, console.y + 7, console.w - 14, console.h - 18};
    rounded(context.renderer, screen, 5, tone(accent, 18, 240));
    SDL_Rect stand{cx - rect.w / 9, console.y + console.h - 3,
                   rect.w * 2 / 9, 9};
    rounded(context.renderer, stand, 4, {225, 239, 251, 250});

    SDL_Rect pad{cx - rect.w / 3, cy + rect.h / 8,
                 rect.w * 2 / 3, rect.h / 3};
    rounded(context.renderer, pad, 18, {245, 251, 255, 252});
    SDL_Rect padScreen{cx - rect.w / 9, pad.y + 7,
                       rect.w * 2 / 9, pad.h - 14};
    rounded(context.renderer, padScreen, 5, tone(accent, 35, 230));
    circle(context.renderer, pad.x + 20, pad.y + pad.h / 2, 6, accent);
    circle(context.renderer, pad.x + pad.w - 20, pad.y + pad.h / 2 - 5, 4, accent);
    circle(context.renderer, pad.x + pad.w - 11, pad.y + pad.h / 2 + 5, 4, accent);
}

void collectionIcon(AppContext& context, SDL_Rect rect, SDL_Color accent,
                    int lift, bool selected) {
    const int cx = rect.x + rect.w / 2;
    const int cy = rect.y + rect.h / 2 + lift;
    glow(context, cx, cy, rect.w / 3, accent, selected);

    SDL_Rect tab{rect.x + rect.w / 8, rect.y + rect.h / 5 + lift,
                 rect.w * 2 / 5, rect.h / 6};
    rounded(context.renderer, tab, 9, {238, 211, 150, 255});
    SDL_Rect outer{rect.x + rect.w / 12, rect.y + rect.h / 4 + lift,
                   rect.w * 5 / 6, rect.h * 7 / 12};
    rounded(context.renderer, outer, 14,
            {accent.r, accent.g, accent.b, 245});
    SDL_Rect body{outer.x + 5, outer.y + 5, outer.w - 10, outer.h - 10};
    rounded(context.renderer, body, 11, {246, 220, 164, 255});
    SDL_Rect shine{body.x + 12, body.y + 10, body.w - 24, 7};
    rounded(context.renderer, shine, 4, {255, 248, 220, 150});
}

void softwareIcon(AppContext& context, SDL_Rect rect, SDL_Color accent,
                  int lift, bool selected) {
    const SDL_Color orange{255, 133, 0, 255};
    const int cx = rect.x + rect.w / 2;
    const int cy = rect.y + rect.h / 2 + lift;
    glow(context, cx, cy, rect.w / 3, orange, selected);

    SDL_Rect bag{rect.x + rect.w / 7, rect.y + rect.h / 3 + lift,
                 rect.w * 5 / 7, rect.h / 2};
    rounded(context.renderer, bag, 11, {255, 151, 25, 252});
    SDL_Rect inner{bag.x + 7, bag.y + 7, bag.w - 14, bag.h - 14};
    rounded(context.renderer, inner, 8, {37, 48, 68, 225});
    const int handleY = bag.y - rect.h / 6;
    thickLine(context.renderer, cx - rect.w / 6, bag.y + 2,
              cx - rect.w / 6, handleY + 8, 7, orange);
    thickLine(context.renderer, cx + rect.w / 6, bag.y + 2,
              cx + rect.w / 6, handleY + 8, 7, orange);
    thickLine(context.renderer, cx - rect.w / 6, handleY,
              cx + rect.w / 6, handleY, 7, orange);
    (void)accent;
}

void homebrewIcon(AppContext& context, SDL_Rect rect, SDL_Color accent,
                  int lift, bool selected) {
    const int cx = rect.x + rect.w / 2;
    const int cy = rect.y + rect.h / 2 + lift;
    glow(context, cx, cy, rect.w / 3, accent, selected);
    const SDL_Color glass = tone(accent, 65);
    thickLine(context.renderer, cx - 15, cy - 45, cx + 15, cy - 45, 5, glass);
    thickLine(context.renderer, cx - 9, cy - 45, cx - 9, cy - 17, 5, glass);
    thickLine(context.renderer, cx + 9, cy - 45, cx + 9, cy - 17, 5, glass);
    SDL_Rect flask{cx - 43, cy - 18, 86, 67};
    rounded(context.renderer, flask, 28, tone(accent, 62, 245));
    SDL_Rect liquid{flask.x + 7, flask.y + 31, flask.w - 14, flask.h - 38};
    rounded(context.renderer, liquid, 13, tone(accent, -5, 238));
    circle(context.renderer, cx - 14, cy + 20, 7, {255, 255, 255, 188});
    circle(context.renderer, cx + 17, cy + 31, 4, {255, 255, 255, 198});
    circle(context.renderer, cx + 7, cy + 7, 3, {255, 255, 255, 170});
}

void starIcon(AppContext& context, SDL_Rect rect, SDL_Color accent,
              int lift, bool selected) {
    const int cx = rect.x + rect.w / 2;
    const int cy = rect.y + rect.h / 2 + lift;
    glow(context, cx, cy, rect.w / 3, accent, selected);
    std::array<SDL_Point, 10> points{};
    for (int index = 0; index < 10; ++index) {
        const float angle = -1.5707963f + static_cast<float>(index) * 0.6283185f;
        const float radius = (index % 2 == 0) ? rect.w * 0.30f : rect.w * 0.13f;
        points[static_cast<std::size_t>(index)] = {
            cx + static_cast<int>(std::cos(angle) * radius),
            cy + static_cast<int>(std::sin(angle) * radius)
        };
    }
    SDL_SetRenderDrawBlendMode(context.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(context.renderer, accent.r, accent.g, accent.b, 255);
    for (int y = cy - rect.h / 3; y <= cy + rect.h / 3; ++y) {
        std::array<int, 20> intersections{};
        int count = 0;
        for (int edge = 0; edge < 10; ++edge) {
            const SDL_Point a = points[static_cast<std::size_t>(edge)];
            const SDL_Point b = points[static_cast<std::size_t>((edge + 1) % 10)];
            if ((a.y <= y && b.y > y) || (b.y <= y && a.y > y)) {
                const float ratio = static_cast<float>(y - a.y) / static_cast<float>(b.y - a.y);
                intersections[static_cast<std::size_t>(count++)] =
                    a.x + static_cast<int>((b.x - a.x) * ratio);
            }
        }
        std::sort(intersections.begin(), intersections.begin() + count);
        for (int index = 0; index + 1 < count; index += 2) {
            SDL_RenderDrawLine(context.renderer,
                               intersections[static_cast<std::size_t>(index)], y,
                               intersections[static_cast<std::size_t>(index + 1)], y);
        }
    }
}

void clockIcon(AppContext& context, SDL_Rect rect, SDL_Color accent,
               int lift, bool selected) {
    const int cx = rect.x + rect.w / 2;
    const int cy = rect.y + rect.h / 2 + lift;
    const int radius = std::max(28, rect.w / 3);
    glow(context, cx, cy, radius, accent, selected);
    circle(context.renderer, cx, cy, radius, tone(accent, 30, 250));
    circle(context.renderer, cx, cy, radius - 7, {239, 248, 255, 245});
    thickLine(context.renderer, cx, cy, cx, cy - radius / 2, 4, accent);
    thickLine(context.renderer, cx, cy, cx + radius / 2, cy + radius / 4, 4, accent);
    circle(context.renderer, cx, cy, 5, accent);
}

} // namespace

void drawCategoryArtwork(AppContext& context, const SDL_Rect& rect,
                         CategoryArtwork artwork, SDL_Color accent,
                         bool selected, uint64_t ticks) {
    const float phase = static_cast<float>(ticks % 10000ULL) / 1000.0f;
    const int lift = selected ? static_cast<int>(std::sin(phase * 2.2f) * 4.0f) : 0;

    rounded(context.renderer, rect, std::max(12, rect.w / 12),
            {25, 62, 124, static_cast<Uint8>(selected ? 208 : 166)});
    SDL_Rect inner{rect.x + 6, rect.y + 6, rect.w - 12, rect.h - 12};
    rounded(context.renderer, inner, std::max(10, inner.w / 13),
            {224, 241, 255, static_cast<Uint8>(selected ? 225 : 190)});
    SDL_Rect shine{inner.x + 12, inner.y + 8, inner.w - 24, 7};
    rounded(context.renderer, shine, 4, {255, 255, 255, 90});

    switch (artwork) {
        case CategoryArtwork::Collection:
            collectionIcon(context, inner, accent, lift, selected);
            break;
        case CategoryArtwork::AllGames:
            allGamesIcon(context, inner, accent, lift, selected);
            break;
        case CategoryArtwork::Software:
            softwareIcon(context, inner, accent, lift, selected);
            break;
        case CategoryArtwork::Homebrew:
            homebrewIcon(context, inner, accent, lift, selected);
            break;
        case CategoryArtwork::Favorites:
            starIcon(context, inner, {255, 180, 20, 255}, lift, selected);
            break;
        case CategoryArtwork::Recent:
            clockIcon(context, inner, accent, lift, selected);
            break;
    }
}
