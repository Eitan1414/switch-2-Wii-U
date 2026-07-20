#pragma once

#include "common.hpp"

#include <SDL2/SDL.h>

#include <string>

SDL_Texture* makeTextTexture(AppContext& context, TTF_Font* font,
                              const std::string& text, SDL_Color color,
                              int* width = nullptr, int* height = nullptr);
std::string fitTextToWidth(TTF_Font* font, const std::string& text, int maxWidth);
void drawText(AppContext& context, TTF_Font* font, const std::string& text,
              int x, int y, SDL_Color color, bool centered = false);
void drawTextFitted(AppContext& context, TTF_Font* font, const std::string& text,
                    int x, int y, int maxWidth, SDL_Color color,
                    bool centered = false);
void drawTextWrapped(AppContext& context, TTF_Font* font, const std::string& text,
                     const SDL_Rect& area, SDL_Color color, int maxLines = 2,
                     bool centered = true);
void drawRoundedPanel(AppContext& context, const SDL_Rect& rect,
                      SDL_Color fill, SDL_Color border, int borderWidth = 2);
void drawCircle(AppContext& context, int centerX, int centerY, int radius, SDL_Color color);
void drawButtonHint(AppContext& context, const std::string& button,
                    const std::string& label, int x, int y);
void drawFolderIcon(AppContext& context, const SDL_Rect& rect, SDL_Color color);
void drawPlaceholderIcon(AppContext& context, const SDL_Rect& rect,
                         const std::string& name, SDL_Color color);
void drawSystemDock(AppContext& context, int selectedIndex = -1);
