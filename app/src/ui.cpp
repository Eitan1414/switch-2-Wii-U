#include "ui.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <vector>

namespace {

void fillCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; ++y) {
        const int width = static_cast<int>(std::sqrt(static_cast<float>(radius * radius - y * y)));
        SDL_RenderDrawLine(renderer, centerX - width, centerY + y, centerX + width, centerY + y);
    }
}

void fillRoundedRect(SDL_Renderer* renderer, const SDL_Rect& rect, int radius, SDL_Color color) {
    radius = std::clamp(radius, 0, std::min(rect.w, rect.h) / 2);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect middle{rect.x + radius, rect.y, rect.w - radius * 2, rect.h};
    SDL_Rect vertical{rect.x, rect.y + radius, rect.w, rect.h - radius * 2};
    SDL_RenderFillRect(renderer, &middle);
    SDL_RenderFillRect(renderer, &vertical);
    fillCircle(renderer, rect.x + radius, rect.y + radius, radius, color);
    fillCircle(renderer, rect.x + rect.w - radius - 1, rect.y + radius, radius, color);
    fillCircle(renderer, rect.x + radius, rect.y + rect.h - radius - 1, radius, color);
    fillCircle(renderer, rect.x + rect.w - radius - 1, rect.y + rect.h - radius - 1, radius, color);
}

void drawThickLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2,
                   int thickness, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const int half = std::max(0, thickness / 2);
    if (std::abs(x2 - x1) >= std::abs(y2 - y1)) {
        for (int offset = -half; offset <= half; ++offset) {
            SDL_RenderDrawLine(renderer, x1, y1 + offset, x2, y2 + offset);
        }
    } else {
        for (int offset = -half; offset <= half; ++offset) {
            SDL_RenderDrawLine(renderer, x1 + offset, y1, x2 + offset, y2);
        }
    }
}

int textWidth(TTF_Font* font, const std::string& text) {
    if (!font || text.empty()) return 0;
    int width = 0;
    int height = 0;
    return TTF_SizeUTF8(font, text.c_str(), &width, &height) == 0 ? width : 0;
}

void removeLastUtf8Character(std::string& text) {
    if (text.empty()) return;
    std::size_t position = text.size() - 1;
    while (position > 0 &&
           (static_cast<unsigned char>(text[position]) & 0xC0) == 0x80) {
        --position;
    }
    text.erase(position);
}

std::vector<std::string> wrapText(TTF_Font* font, const std::string& text,
                                  int maxWidth, int maxLines) {
    std::vector<std::string> lines;
    if (!font || text.empty() || maxWidth <= 0 || maxLines <= 0) return lines;

    std::istringstream stream(text);
    std::vector<std::string> words;
    std::string word;
    while (stream >> word) words.push_back(word);
    if (words.empty()) {
        lines.push_back(fitTextToWidth(font, text, maxWidth));
        return lines;
    }

    std::string current;
    for (std::size_t index = 0; index < words.size(); ++index) {
        if (static_cast<int>(lines.size()) == maxLines - 1) {
            std::string remainder = current;
            for (; index < words.size(); ++index) {
                if (!remainder.empty()) remainder += ' ';
                remainder += words[index];
            }
            lines.push_back(fitTextToWidth(font, remainder, maxWidth));
            return lines;
        }

        const std::string candidate = current.empty() ? words[index]
                                                       : current + " " + words[index];
        if (textWidth(font, candidate) <= maxWidth) {
            current = candidate;
            continue;
        }

        if (!current.empty()) {
            lines.push_back(current);
            current.clear();
            --index;
        } else {
            lines.push_back(fitTextToWidth(font, words[index], maxWidth));
        }
    }

    if (!current.empty() && static_cast<int>(lines.size()) < maxLines) {
        lines.push_back(current);
    }
    return lines;
}

void drawPerson(SDL_Renderer* renderer, int centerX, int centerY, int scale,
                SDL_Color color, bool smiling) {
    const int head = std::max(2, scale * 3);
    fillCircle(renderer, centerX, centerY - scale * 5, head, color);

    SDL_Rect body{centerX - scale * 2, centerY - scale * 2,
                  scale * 4 + 1, scale * 7};
    fillRoundedRect(renderer, body, std::max(1, scale), color);
    drawThickLine(renderer, centerX - scale * 2, centerY,
                  centerX - scale * 4, centerY + scale * 3,
                  std::max(1, scale), color);
    drawThickLine(renderer, centerX + scale * 2, centerY,
                  centerX + scale * 4, centerY + scale * 3,
                  std::max(1, scale), color);
    drawThickLine(renderer, centerX - scale, centerY + scale * 4,
                  centerX - scale * 2, centerY + scale * 8,
                  std::max(1, scale), color);
    drawThickLine(renderer, centerX + scale, centerY + scale * 4,
                  centerX + scale * 2, centerY + scale * 8,
                  std::max(1, scale), color);

    if (smiling) {
        const SDL_Color green{0, 198, 22, color.a};
        fillCircle(renderer, centerX - scale, centerY - scale * 6, 1, green);
        fillCircle(renderer, centerX + scale, centerY - scale * 6, 1, green);
        drawThickLine(renderer, centerX - scale, centerY - scale * 4,
                      centerX + scale, centerY - scale * 4,
                      1, green);
    }
}

void drawMiiverseGlyph(AppContext& context, int centerX, int centerY, bool active) {
    const int lift = active ? 1 : 0;
    const SDL_Color green{0, 205, 25, static_cast<Uint8>(active ? 255 : 225)};
    const SDL_Color white{255, 255, 255, static_cast<Uint8>(active ? 255 : 235)};

    fillCircle(context.renderer, centerX, centerY - 7 - lift, active ? 15 : 14, green);
    fillCircle(context.renderer, centerX - 14, centerY + 2 - lift, active ? 13 : 12, green);
    fillCircle(context.renderer, centerX + 14, centerY + 2 - lift, active ? 13 : 12, green);
    fillCircle(context.renderer, centerX - 8, centerY + 11 - lift, active ? 12 : 11, green);
    fillCircle(context.renderer, centerX + 8, centerY + 11 - lift, active ? 12 : 11, green);

    drawPerson(context.renderer, centerX, centerY - 2 - lift, active ? 2 : 1, white, true);
    drawPerson(context.renderer, centerX - 14, centerY + 5 - lift, 1, white, false);
    drawPerson(context.renderer, centerX + 14, centerY + 5 - lift, 1, white, false);
}

void drawEshopGlyph(AppContext& context, int centerX, int centerY, bool active) {
    const SDL_Color orange{255, 131, 0, static_cast<Uint8>(active ? 255 : 225)};
    const int size = active ? 1 : 0;
    const int left = centerX - 16 - size;
    const int right = centerX + 16 + size;
    const int top = centerY - 8 - size;
    const int bottom = centerY + 15 + size;
    const int thickness = active ? 4 : 3;

    drawThickLine(context.renderer, left, top, right, top, thickness, orange);
    drawThickLine(context.renderer, left, top, left + 2, bottom, thickness, orange);
    drawThickLine(context.renderer, right, top, right - 2, bottom, thickness, orange);
    drawThickLine(context.renderer, left + 2, bottom, right - 2, bottom, thickness, orange);

    drawThickLine(context.renderer, centerX - 7, top,
                  centerX - 7, centerY - 14 - size, thickness, orange);
    drawThickLine(context.renderer, centerX + 7, top,
                  centerX + 7, centerY - 14 - size, thickness, orange);
    drawThickLine(context.renderer, centerX - 7, centerY - 14 - size,
                  centerX + 7, centerY - 14 - size, thickness, orange);
}

void drawDockGlyph(AppContext& context, int kind, int centerX, int centerY, bool active) {
    if (kind == 0) {
        drawMiiverseGlyph(context, centerX, centerY, active);
        return;
    }
    if (kind == 1) {
        drawEshopGlyph(context, centerX, centerY, active);
        return;
    }

    const SDL_Color color = active ? SDL_Color{0, 168, 218, 255} : SDL_Color{55, 60, 70, 255};
    SDL_SetRenderDrawColor(context.renderer, color.r, color.g, color.b, color.a);
    if (kind == 2) {
        for (int degree = 0; degree < 360; degree += 6) {
            const float angle = static_cast<float>(degree) * 3.14159265f / 180.0f;
            SDL_RenderDrawPoint(context.renderer,
                                centerX + static_cast<int>(std::cos(angle) * 14),
                                centerY + static_cast<int>(std::sin(angle) * 14));
        }
        SDL_RenderDrawLine(context.renderer, centerX - 14, centerY, centerX + 14, centerY);
        SDL_RenderDrawLine(context.renderer, centerX, centerY - 14, centerX, centerY + 14);
        SDL_Rect meridian{centerX - 7, centerY - 14, 14, 28};
        SDL_RenderDrawRect(context.renderer, &meridian);
    } else if (kind == 3) {
        SDL_RenderDrawLine(context.renderer, centerX, centerY - 14, centerX - 7, centerY - 10);
        SDL_RenderDrawLine(context.renderer, centerX - 7, centerY - 10, centerX - 11, centerY + 7);
        SDL_RenderDrawLine(context.renderer, centerX, centerY - 14, centerX + 7, centerY - 10);
        SDL_RenderDrawLine(context.renderer, centerX + 7, centerY - 10, centerX + 11, centerY + 7);
        SDL_RenderDrawLine(context.renderer, centerX - 13, centerY + 10, centerX + 13, centerY + 10);
        SDL_RenderDrawLine(context.renderer, centerX - 11, centerY + 7, centerX - 13, centerY + 10);
        SDL_RenderDrawLine(context.renderer, centerX + 11, centerY + 7, centerX + 13, centerY + 10);
        fillCircle(context.renderer, centerX, centerY + 14, 3, color);
    } else if (kind == 4) {
        fillCircle(context.renderer, centerX - 7, centerY - 7, 6, color);
        fillCircle(context.renderer, centerX + 8, centerY - 5, 5, color);
        SDL_Rect leftBody{centerX - 16, centerY + 1, 18, 12};
        SDL_Rect rightBody{centerX + 2, centerY + 2, 15, 10};
        SDL_RenderFillRect(context.renderer, &leftBody);
        SDL_RenderFillRect(context.renderer, &rightBody);
    } else {
        SDL_RenderDrawLine(context.renderer, centerX, centerY - 14, centerX, centerY + 6);
        SDL_RenderDrawLine(context.renderer, centerX - 7, centerY, centerX, centerY + 7);
        SDL_RenderDrawLine(context.renderer, centerX + 7, centerY, centerX, centerY + 7);
        SDL_RenderDrawLine(context.renderer, centerX - 13, centerY + 8, centerX - 13, centerY + 14);
        SDL_RenderDrawLine(context.renderer, centerX - 13, centerY + 14, centerX + 13, centerY + 14);
        SDL_RenderDrawLine(context.renderer, centerX + 13, centerY + 14, centerX + 13, centerY + 8);
    }
}

void drawDrawerTransition(AppContext& context) {
    if (context.drawerEffect == 0 || context.drawerEffectStarted == 0) return;

    constexpr float durationMs = 460.0f;
    const uint64_t elapsed = SDL_GetTicks64() - context.drawerEffectStarted;
    const float raw = std::clamp(static_cast<float>(elapsed) / durationMs, 0.0f, 1.0f);
    if (raw >= 1.0f) {
        context.drawerEffect = 0;
        context.drawerEffectStarted = 0;
        return;
    }

    const float eased = easeOutCubic(raw);
    const float openAmount = context.drawerEffect > 0 ? eased : 1.0f - eased;
    const float pulse = std::sin(raw * 3.14159265f);

    SDL_SetRenderDrawBlendMode(context.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(context.renderer, 12, 18, 28, static_cast<Uint8>(72.0f * pulse));
    SDL_Rect screen{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(context.renderer, &screen);

    const SDL_Color accent{0, 174, 221, 255};
    const int centerX = SCREEN_WIDTH / 2;
    const int bodyY = 350;
    const int lidLift = static_cast<int>(openAmount * 72.0f);
    const int cardRise = static_cast<int>(openAmount * 142.0f);
    const int cardSpread = static_cast<int>(openAmount * 88.0f);

    for (int i = -1; i <= 1; ++i) {
        const int cardX = centerX - 37 + i * cardSpread;
        const int cardY = bodyY + 22 - cardRise + std::abs(i) * 12;
        SDL_Rect card{cardX, cardY, 74, 92};
        drawRoundedPanel(context, card,
                         {255, 255, 255, static_cast<Uint8>(210 + 40 * openAmount)},
                         {accent.r, accent.g, accent.b, 255}, 3);
        SDL_Rect inner{card.x + 9, card.y + 9, card.w - 18, card.w - 18};
        SDL_SetRenderDrawColor(context.renderer,
                               static_cast<Uint8>(accent.r + (i + 1) * 12),
                               static_cast<Uint8>(std::min(255, accent.g + 20 + (i + 1) * 8)),
                               static_cast<Uint8>(std::min(255, accent.b + (i + 1) * 6)), 255);
        SDL_RenderFillRect(context.renderer, &inner);
    }

    SDL_Rect body{centerX - 190, bodyY, 380, 150};
    drawRoundedPanel(context, body, {247, 250, 253, 252}, {accent.r, accent.g, accent.b, 255}, 6);

    SDL_Rect front{body.x + 18, body.y + 40, body.w - 36, body.h - 58};
    SDL_SetRenderDrawColor(context.renderer, accent.r, accent.g, accent.b, 235);
    SDL_RenderFillRect(context.renderer, &front);
    SDL_SetRenderDrawColor(context.renderer, 255, 255, 255, 85);
    SDL_Rect shine{front.x + 16, front.y + 14, front.w - 32, 8};
    SDL_RenderFillRect(context.renderer, &shine);

    SDL_Rect lid{centerX - 204, bodyY - 26 - lidLift, 408, 58};
    drawRoundedPanel(context, lid, {255, 255, 255, 252}, {accent.r, accent.g, accent.b, 255}, 5);
    SDL_Rect handle{centerX - 46, lid.y + 18, 92, 15};
    SDL_SetRenderDrawColor(context.renderer, accent.r, accent.g, accent.b, 255);
    SDL_RenderFillRect(context.renderer, &handle);

    drawTextFitted(context, context.fontSmall,
                   context.drawerEffect > 0 ? "OUVERTURE DU TIROIR" : "FERMETURE DU TIROIR",
                   centerX, 535, 520, {255, 255, 255, 255}, true);
}

} // namespace

SDL_Texture* makeTextTexture(AppContext& context, TTF_Font* font,
                             const std::string& text, SDL_Color color,
                             int* width, int* height) {
    if (!font || text.empty()) return nullptr;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return nullptr;
    if (width) *width = surface->w;
    if (height) *height = surface->h;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(context.renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

std::string fitTextToWidth(TTF_Font* font, const std::string& text, int maxWidth) {
    if (!font || text.empty() || maxWidth <= 0) return {};
    if (textWidth(font, text) <= maxWidth) return text;

    const std::string ellipsis = "...";
    if (textWidth(font, ellipsis) > maxWidth) return {};

    std::string result = text;
    while (!result.empty() && textWidth(font, result + ellipsis) > maxWidth) {
        removeLastUtf8Character(result);
    }
    return result.empty() ? ellipsis : result + ellipsis;
}

void drawText(AppContext& context, TTF_Font* font, const std::string& text,
              int x, int y, SDL_Color color, bool centered) {
    int width = 0;
    int height = 0;
    SDL_Texture* texture = makeTextTexture(context, font, text, color, &width, &height);
    if (!texture) return;
    SDL_Rect target{x - (centered ? width / 2 : 0), y, width, height};
    SDL_RenderCopy(context.renderer, texture, nullptr, &target);
    SDL_DestroyTexture(texture);
}

void drawTextFitted(AppContext& context, TTF_Font* font, const std::string& text,
                    int x, int y, int maxWidth, SDL_Color color, bool centered) {
    drawText(context, font, fitTextToWidth(font, text, maxWidth), x, y, color, centered);
}

void drawTextWrapped(AppContext& context, TTF_Font* font, const std::string& text,
                     const SDL_Rect& area, SDL_Color color, int maxLines,
                     bool centered) {
    const auto lines = wrapText(font, text, area.w, maxLines);
    if (lines.empty()) return;

    const int lineHeight = std::max(1, TTF_FontLineSkip(font));
    const int totalHeight = static_cast<int>(lines.size()) * lineHeight;
    int y = area.y + std::max(0, (area.h - totalHeight) / 2);
    for (const auto& line : lines) {
        const int x = centered ? area.x + area.w / 2 : area.x;
        drawText(context, font, line, x, y, color, centered);
        y += lineHeight;
    }
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
    drawTextFitted(context, context.fontSmall, button, x + 16, y + 2, 28,
                   {255, 255, 255, 255}, true);
    const int availableWidth = std::max(0, SCREEN_WIDTH - (x + 42) - 8);
    drawTextFitted(context, context.fontSmall, label, x + 42, y + 2,
                   availableWidth, {45, 48, 54, 255});
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
        drawTextFitted(context, context.fontSmall, labels[i], centerX, 651, 78,
                       active ? SDL_Color{0, 143, 191, 255} : SDL_Color{82, 87, 96, 255}, true);
    }

    drawDrawerTransition(context);
}

void drawFolderIcon(AppContext& context, const SDL_Rect& rect, SDL_Color color) {
    const SDL_Color beige{244, 216, 153, 255};
    const SDL_Color highlight{255, 235, 188, 190};
    const SDL_Color shadow{
        static_cast<Uint8>(std::max(40, static_cast<int>(color.r) - 35)),
        static_cast<Uint8>(std::max(40, static_cast<int>(color.g) - 35)),
        static_cast<Uint8>(std::max(40, static_cast<int>(color.b) - 35)),
        255
    };

    SDL_Rect tab{rect.x + 18, rect.y + 11, rect.w / 2, 34};
    SDL_Rect body{rect.x + 7, rect.y + 34, rect.w - 14, rect.h - 43};
    fillRoundedRect(context.renderer, tab, 10, beige);
    fillRoundedRect(context.renderer, body, 13, beige);

    SDL_SetRenderDrawColor(context.renderer, shadow.r, shadow.g, shadow.b, shadow.a);
    for (int i = 0; i < 4; ++i) {
        SDL_Rect outline{body.x + i, body.y + i, body.w - i * 2, body.h - i * 2};
        SDL_RenderDrawRect(context.renderer, &outline);
    }
    SDL_Rect tabOutline{tab.x, tab.y, tab.w, tab.h};
    SDL_RenderDrawRect(context.renderer, &tabOutline);

    SDL_SetRenderDrawColor(context.renderer, highlight.r, highlight.g, highlight.b, highlight.a);
    SDL_Rect shine{body.x + 13, body.y + 13, body.w - 26, 8};
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
    drawTextFitted(context, context.fontLarge, initials,
                   rect.x + rect.w / 2, rect.y + rect.h / 2 - 28,
                   rect.w - 20, {255, 255, 255, 255}, true);
}
