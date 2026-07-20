#include "ui.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace {

void fillCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; ++y) {
        const int width = static_cast<int>(std::sqrt(static_cast<float>(radius * radius - y * y)));
        SDL_RenderDrawLine(renderer, centerX - width, centerY + y, centerX + width, centerY + y);
    }
}

void drawDockGlyph(AppContext& context, int kind, int centerX, int centerY, bool active) {
    const SDL_Color color = active ? SDL_Color{0, 168, 218, 255} : SDL_Color{55, 60, 70, 255};
    SDL_SetRenderDrawColor(context.renderer, color.r, color.g, color.b, color.a);
    if (kind == 0) { // Miiverse : bulle de discussion et silhouettes
        SDL_Rect bubble{centerX - 15, centerY - 12, 30, 20};
        SDL_RenderDrawRect(context.renderer, &bubble);
        SDL_RenderDrawLine(context.renderer, centerX - 8, centerY + 8,
                           centerX - 12, centerY + 14);
        fillCircle(context.renderer, centerX - 5, centerY - 4, 3, color);
        fillCircle(context.renderer, centerX + 6, centerY - 4, 3, color);
        SDL_RenderDrawLine(context.renderer, centerX - 10, centerY + 4,
                           centerX + 11, centerY + 4);
    } else if (kind == 1) { // eShop
        SDL_Rect bag{centerX - 12, centerY - 7, 24, 20};
        SDL_RenderDrawRect(context.renderer, &bag);
        SDL_Rect handle{centerX - 6, centerY - 13, 12, 8};
        SDL_RenderDrawRect(context.renderer, &handle);
    } else if (kind == 2) { // Navigateur Internet : globe
        for (int degree = 0; degree < 360; degree += 6) {
            const float angle = static_cast<float>(degree) * 3.14159265f / 180.0f;
            SDL_RenderDrawPoint(context.renderer,
                                centerX + static_cast<int>(std::cos(angle) * 14),
                                centerY + static_cast<int>(std::sin(angle) * 14));
        }
        SDL_RenderDrawLine(context.renderer, centerX - 14, centerY,
                           centerX + 14, centerY);
        SDL_RenderDrawLine(context.renderer, centerX, centerY - 14,
                           centerX, centerY + 14);
        SDL_Rect meridian{centerX - 7, centerY - 14, 14, 28};
        SDL_RenderDrawRect(context.renderer, &meridian);
    } else if (kind == 3) { // Notifications : cloche
        SDL_RenderDrawLine(context.renderer, centerX, centerY - 14,
                           centerX - 7, centerY - 10);
        SDL_RenderDrawLine(context.renderer, centerX - 7, centerY - 10,
                           centerX - 11, centerY + 7);
        SDL_RenderDrawLine(context.renderer, centerX, centerY - 14,
                           centerX + 7, centerY - 10);
        SDL_RenderDrawLine(context.renderer, centerX + 7, centerY - 10,
                           centerX + 11, centerY + 7);
        SDL_RenderDrawLine(context.renderer, centerX - 13, centerY + 10,
                           centerX + 13, centerY + 10);
        SDL_RenderDrawLine(context.renderer, centerX - 11, centerY + 7,
                           centerX - 13, centerY + 10);
        SDL_RenderDrawLine(context.renderer, centerX + 11, centerY + 7,
                           centerX + 13, centerY + 10);
        fillCircle(context.renderer, centerX, centerY + 14, 3, color);
    } else if (kind == 4) { // Liste d'amis : deux personnes
        fillCircle(context.renderer, centerX - 7, centerY - 7, 6, color);
        fillCircle(context.renderer, centerX + 8, centerY - 5, 5, color);
        SDL_Rect leftBody{centerX - 16, centerY + 1, 18, 12};
        SDL_Rect rightBody{centerX + 2, centerY + 2, 15, 10};
        SDL_RenderFillRect(context.renderer, &leftBody);
        SDL_RenderFillRect(context.renderer, &rightBody);
    } else { // Gestion des telechargements : fleche et bac
        SDL_RenderDrawLine(context.renderer, centerX, centerY - 14,
                           centerX, centerY + 6);
        SDL_RenderDrawLine(context.renderer, centerX - 7, centerY,
                           centerX, centerY + 7);
        SDL_RenderDrawLine(context.renderer, centerX + 7, centerY,
                           centerX, centerY + 7);
        SDL_RenderDrawLine(context.renderer, centerX - 13, centerY + 8,
                           centerX - 13, centerY + 14);
        SDL_RenderDrawLine(context.renderer, centerX - 13, centerY + 14,
                           centerX + 13, centerY + 14);
        SDL_RenderDrawLine(context.renderer, centerX + 13, centerY + 14,
                           centerX + 13, centerY + 8);
    }
}

} // namespace

SDL_Texture* makeTextTexture(AppContext& context, TTF_Font* font,
                             const std::string& text, SDL_Color color,
                             int* width, int* height) {
    if (!font || text.empty()) {
        return nullptr;
    }
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        return nullptr;
    }
    if (width) *width = surface->w;
    if (height) *height = surface->h;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(context.renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void drawText(AppContext& context, TTF_Font* font, const std::string& text,
              int x, int y, SDL_Color color, bool centered) {
    int width = 0;
    int height = 0;
    SDL_Texture* texture = makeTextTexture(context, font, text, color, &width, &height);
    if (!texture) {
        return;
    }
    SDL_Rect target{x - (centered ? width / 2 : 0), y, width, height};
    SDL_RenderCopy(context.renderer, texture, nullptr, &target);
    SDL_DestroyTexture(texture);
}

void drawRoundedPanel(AppContext& context, const SDL_Rect& rect,
                      SDL_Color fill, SDL_Color border, int borderWidth) {
    SDL_SetRenderDrawBlendMode(context.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(context.renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(context.renderer, &rect);
    SDL_SetRenderDrawColor(context.renderer, border.r, border.g, border.b, border.a);
    for (int i = 0; i < borderWidth; ++i) {
        SDL_Rect outline{rect.x + i, rect.y + i, rect.w - i * 2, rect.h - i * 2};
        SDL_RenderDrawRect(context.renderer, &outline);
    }
}

void drawCircle(AppContext& context, int centerX, int centerY, int radius, SDL_Color color) {
    fillCircle(context.renderer, centerX, centerY, radius, color);
}

void drawButtonHint(AppContext& context, const std::string& button,
                    const std::string& label, int x, int y) {
    fillCircle(context.renderer, x + 16, y + 16, 16, {28, 31, 38, 230});
    drawText(context, context.fontSmall, button, x + 16, y + 2, {255, 255, 255, 255}, true);
    drawText(context, context.fontSmall, label, x + 42, y + 2, {45, 48, 54, 255});
}

void drawSystemDock(AppContext& context, int selectedIndex) {
    static constexpr const char* labels[] = {
        "Miiverse", "eShop", "Navigateur", "Notifications", "Liste d'amis", "Telecharg."
    };
    constexpr int startX = 430;
    constexpr int spacing = 86;
    constexpr int centerY = 620;
    for (int i = 0; i < 6; ++i) {
        const int centerX = startX + i * spacing;
        const bool active = i == selectedIndex;
        fillCircle(context.renderer, centerX, centerY, active ? 29 : 27,
                   active ? SDL_Color{255, 255, 255, 250} : SDL_Color{237, 240, 246, 245});
        SDL_SetRenderDrawColor(context.renderer,
                               active ? 0 : 198,
                               active ? 168 : 204,
                               active ? 218 : 214,
                               255);
        for (int radius = active ? 28 : 26; radius <= (active ? 30 : 28); ++radius) {
            for (int degree = 0; degree < 360; degree += 3) {
                const float angle = static_cast<float>(degree) * 3.14159265f / 180.0f;
                SDL_RenderDrawPoint(context.renderer,
                                    centerX + static_cast<int>(std::cos(angle) * radius),
                                    centerY + static_cast<int>(std::sin(angle) * radius));
            }
        }
        drawDockGlyph(context, i, centerX, centerY, active);
        drawText(context, context.fontSmall, labels[i], centerX, 651,
                 active ? SDL_Color{0, 143, 191, 255} : SDL_Color{82, 87, 96, 255}, true);
    }
}

void drawFolderIcon(AppContext& context, const SDL_Rect& rect, SDL_Color color) {
    SDL_SetRenderDrawColor(context.renderer, color.r, color.g, color.b, color.a);
    SDL_Rect tab{rect.x + 18, rect.y + 12, rect.w / 2, 28};
    SDL_Rect body{rect.x + 8, rect.y + 34, rect.w - 16, rect.h - 44};
    SDL_RenderFillRect(context.renderer, &tab);
    SDL_RenderFillRect(context.renderer, &body);
    SDL_SetRenderDrawColor(context.renderer, 255, 255, 255, 65);
    SDL_Rect shine{body.x + 10, body.y + 10, body.w - 20, 8};
    SDL_RenderFillRect(context.renderer, &shine);
}

void drawPlaceholderIcon(AppContext& context, const SDL_Rect& rect,
                         const std::string& name, SDL_Color color) {
    SDL_SetRenderDrawColor(context.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(context.renderer, &rect);
    std::string initials;
    for (const char character : name) {
        if (initials.empty() || (character != ' ' && name.find(' ') != std::string::npos)) {
            if (character != ' ') initials += static_cast<char>(std::toupper(character));
        }
        if (initials.size() == 2) break;
    }
    if (initials.empty()) initials = "U";
    drawText(context, context.fontLarge, initials,
             rect.x + rect.w / 2, rect.y + rect.h / 2 - 28,
             {255, 255, 255, 255}, true);
}
