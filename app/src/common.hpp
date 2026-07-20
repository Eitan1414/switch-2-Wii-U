#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

inline constexpr int SCREEN_WIDTH = 1280;
inline constexpr int SCREEN_HEIGHT = 720;

inline const std::filesystem::path SD_ROOT = "fs:/vol/external01";
inline const std::filesystem::path MODE_ROOT = SD_ROOT / "wiiu/switch2mode";
inline const std::filesystem::path CONFIG_PATH = MODE_ROOT / "config.ini";
inline const std::filesystem::path INTRO_PENDING_PATH = MODE_ROOT / "intro.pending";
inline const std::filesystem::path CUSTOM_BACKGROUND_PATH = MODE_ROOT / "background.png";
inline const std::filesystem::path APPS_PATH = SD_ROOT / "wiiu/apps";

struct AppContext {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* fontSmall = nullptr;
    TTF_Font* fontMedium = nullptr;
    TTF_Font* fontLarge = nullptr;
    Mix_Chunk* soundMove = nullptr;
    Mix_Chunk* soundAccept = nullptr;
    Mix_Chunk* soundBack = nullptr;
    Mix_Chunk* soundFolder = nullptr;
    Mix_Chunk* soundLaunch = nullptr;
    Mix_Chunk* backgroundLoop = nullptr;
    std::vector<int16_t> backgroundPcm;
    int backgroundChannel = -1;
    std::vector<SDL_GameController*> controllers;
    std::string profileName = "Profil Wii U";
    bool wifiConnected = false;
    int batteryLevel = 4;
    uint64_t lastStatusRefresh = 0;
    bool acInitialized = false;
    bool actInitialized = false;

    int drawerEffect = 0;
    uint64_t drawerEffectStarted = 0;
    bool drawerOpen = false;
};

inline AppContext* ACTIVE_AUDIO_CONTEXT = nullptr;

struct InputState {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool accept = false;
    bool back = false;
    bool x = false;
    bool y = false;
    bool plus = false;
    bool quit = false;
};

InputState pollInput(AppContext& ctx);
void refreshSystemStatus(AppContext& ctx);
void playMoveSound(AppContext& ctx);
void playAcceptSound(AppContext& ctx);
void playBackSound(AppContext& ctx);
void playFolderSound(AppContext& ctx);
void playLaunchSound(AppContext& ctx);
void startBackgroundMusic(AppContext& ctx);
void stopBackgroundMusic(AppContext& ctx, int fadeMs = 0);
inline void stopBackgroundMusic(int fadeMs = 0) {
    if (ACTIVE_AUDIO_CONTEXT) stopBackgroundMusic(*ACTIVE_AUDIO_CONTEXT, fadeMs);
}
float easeOutCubic(float value);
