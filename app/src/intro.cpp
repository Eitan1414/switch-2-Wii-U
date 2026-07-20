#include "intro.hpp"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <algorithm>
#include <cstdio>
#include <string>

bool playIntro(AppContext& context) {
    constexpr uint64_t durationMs = 9200;
    constexpr int frameRate = 30;
    constexpr int frameCount = 276;

    stopBackgroundMusic(250);
    SDL_Delay(260);

    Mix_Music* music = Mix_LoadMUS("fs:/vol/content/intro/intro.ogg");
    if (music) {
        Mix_VolumeMusic(MIX_MAX_VOLUME);
        Mix_PlayMusic(music, 1);
    }

    SDL_Texture* frameTexture = nullptr;
    int loadedFrame = -1;
    const uint64_t start = SDL_GetTicks64();
    bool skipped = false;

    while (!skipped) {
        const auto elapsed = SDL_GetTicks64() - start;
        if (elapsed >= durationMs) break;

        const int frame = std::min(frameCount, static_cast<int>(elapsed * frameRate / 1000) + 1);
        if (frame != loadedFrame) {
            char path[128];
            std::snprintf(path, sizeof(path), "fs:/vol/content/intro/frames/frame_%04d.webp", frame);
            SDL_Texture* next = IMG_LoadTexture(context.renderer, path);
            if (next) {
                SDL_DestroyTexture(frameTexture);
                frameTexture = next;
                loadedFrame = frame;
            }
        }

        SDL_SetRenderDrawColor(context.renderer, 3, 7, 13, 255);
        SDL_RenderClear(context.renderer);
        if (frameTexture) {
            SDL_Rect target{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(context.renderer, frameTexture, nullptr, &target);
        }
        SDL_RenderPresent(context.renderer);

        const auto input = pollInput(context);
        skipped = input.accept || input.back || input.quit;
        SDL_Delay(2);
    }

    Mix_HaltMusic();
    Mix_FreeMusic(music);
    SDL_DestroyTexture(frameTexture);
    startBackgroundMusic(context);
    return !skipped;
}
