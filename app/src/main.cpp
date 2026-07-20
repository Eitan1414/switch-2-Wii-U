#include "common.hpp"
#include "config.hpp"
#include "intro.hpp"
#include "launcher.hpp"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <coreinit/memory.h>
#include <mocha/mocha.h>
#include <nn/ac.h>
#include <nn/act.h>
#include <rpxloader/rpxloader.h>
#include <sysapp/launch.h>
#include <vpad/input.h>

#include <algorithm>
#include <cmath>

InputState pollInput(AppContext& context) {
    InputState input;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) input.quit = true;
        if (event.type == SDL_CONTROLLERDEVICEADDED) {
            if (auto* controller = SDL_GameControllerOpen(event.cdevice.which)) {
                context.controllers.push_back(controller);
            }
        }
        if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
            auto* controller = SDL_GameControllerFromInstanceID(event.cdevice.which);
            const auto found = std::find(context.controllers.begin(), context.controllers.end(), controller);
            if (found != context.controllers.end()) {
                SDL_GameControllerClose(*found);
                context.controllers.erase(found);
            }
        }
        if (event.type == SDL_CONTROLLERBUTTONDOWN) {
            switch (event.cbutton.button) {
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT: input.left = true; break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: input.right = true; break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP: input.up = true; break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN: input.down = true; break;
                case SDL_CONTROLLER_BUTTON_A: input.accept = true; break;
                case SDL_CONTROLLER_BUTTON_B: input.back = true; break;
                case SDL_CONTROLLER_BUTTON_X: input.x = true; break;
                case SDL_CONTROLLER_BUTTON_Y: input.y = true; break;
                case SDL_CONTROLLER_BUTTON_START: input.plus = true; break;
                default: break;
            }
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_LEFT: input.left = true; break;
                case SDLK_RIGHT: input.right = true; break;
                case SDLK_UP: input.up = true; break;
                case SDLK_DOWN: input.down = true; break;
                case SDLK_RETURN: input.accept = true; break;
                case SDLK_ESCAPE: input.back = true; break;
                case SDLK_x: input.x = true; break;
                case SDLK_y: input.y = true; break;
                default: break;
            }
        }
    }
    return input;
}

void refreshSystemStatus(AppContext& context) {
    const uint64_t now = SDL_GetTicks64();
    if (context.lastStatusRefresh != 0 && now - context.lastStatusRefresh < 1000) return;
    context.lastStatusRefresh = now;

    VPADStatus status{};
    VPADReadError readError = VPAD_READ_SUCCESS;
    if (VPADRead(VPAD_CHAN_0, &status, 1, &readError) > 0 && readError == VPAD_READ_SUCCESS) {
        context.batteryLevel = std::clamp(static_cast<int>(status.battery), 0, 6);
    }

    if (context.acInitialized) {
        BOOL connected = FALSE;
        if (NNResult_IsSuccess(ACIsApplicationConnected(&connected))) {
            context.wifiConnected = connected == TRUE;
        }
    }
}

void playMoveSound(AppContext& context) {
    if (context.soundMove) Mix_PlayChannel(-1, context.soundMove, 0);
}

void playAcceptSound(AppContext& context) {
    if (context.soundAccept) Mix_PlayChannel(-1, context.soundAccept, 0);
}

void playBackSound(AppContext& context) {
    if (context.soundBack) Mix_PlayChannel(-1, context.soundBack, 0);
}

void playFolderSound(AppContext& context) {
    if (context.soundFolder) Mix_PlayChannel(-1, context.soundFolder, 0);
}

void playLaunchSound(AppContext& context) {
    if (context.soundLaunch) Mix_PlayChannel(-1, context.soundLaunch, 0);
}

float easeOutCubic(float value) {
    value = std::clamp(value, 0.0f, 1.0f);
    const float inverse = 1.0f - value;
    return 1.0f - inverse * inverse * inverse;
}

namespace {

TTF_Font* openSystemFont(int size) {
    void* fontData = nullptr;
    uint32_t fontSize = 0;
    if (!OSGetSharedData(OS_SHAREDDATATYPE_FONT_STANDARD, 0, &fontData, &fontSize)) {
        return nullptr;
    }
    SDL_RWops* stream = SDL_RWFromConstMem(fontData, static_cast<int>(fontSize));
    return stream ? TTF_OpenFontRW(stream, 1, size) : nullptr;
}

bool initialize(AppContext& context) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) != 0) return false;
    if ((IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_WEBP) & IMG_INIT_WEBP) == 0) return false;
    if (TTF_Init() != 0) return false;
    Mix_Init(MIX_INIT_OGG);
    Mix_OpenAudioDevice(48000, MIX_DEFAULT_FORMAT, 2, 1024, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);

    context.window = SDL_CreateWindow("Switch2 Mode", SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
                                      SDL_WINDOW_SHOWN);
    context.renderer = SDL_CreateRenderer(context.window, -1,
                                           SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!context.window || !context.renderer) return false;

    context.fontSmall = openSystemFont(24);
    context.fontMedium = openSystemFont(32);
    context.fontLarge = openSystemFont(44);
    context.soundMove = Mix_LoadWAV("fs:/vol/content/sound/move.ogg");
    context.soundAccept = Mix_LoadWAV("fs:/vol/content/sound/accept.ogg");
    context.soundBack = Mix_LoadWAV("fs:/vol/content/sound/back.ogg");
    context.soundFolder = Mix_LoadWAV("fs:/vol/content/sound/folder.ogg");
    context.soundLaunch = Mix_LoadWAV("fs:/vol/content/sound/launch.ogg");

    context.acInitialized = NNResult_IsSuccess(ACInitialize());
    context.actInitialized = nn::act::Initialize().IsSuccess();
    if (context.actInitialized) {
        int16_t miiName[nn::act::MiiNameSize]{};
        if (nn::act::GetMiiName(miiName).IsSuccess()) {
            std::string name;
            for (std::size_t i = 0; i < nn::act::MiiNameSize && miiName[i] != 0; ++i) {
                const auto character = static_cast<uint16_t>(miiName[i]);
                name += character < 128 ? static_cast<char>(character) : '?';
            }
            if (!name.empty()) context.profileName = name;
        }
    }
    context.lastStatusRefresh = 0;
    refreshSystemStatus(context);

    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            if (auto* controller = SDL_GameControllerOpen(i)) context.controllers.push_back(controller);
        }
    }
    return true;
}

void finalize(AppContext& context) {
    for (auto* controller : context.controllers) SDL_GameControllerClose(controller);
    if (context.fontSmall) TTF_CloseFont(context.fontSmall);
    if (context.fontMedium) TTF_CloseFont(context.fontMedium);
    if (context.fontLarge) TTF_CloseFont(context.fontLarge);
    if (context.soundMove) Mix_FreeChunk(context.soundMove);
    if (context.soundAccept) Mix_FreeChunk(context.soundAccept);
    if (context.soundBack) Mix_FreeChunk(context.soundBack);
    if (context.soundFolder) Mix_FreeChunk(context.soundFolder);
    if (context.soundLaunch) Mix_FreeChunk(context.soundLaunch);
    if (context.actInitialized) nn::act::Finalize();
    if (context.acInitialized) ACFinalize();
    SDL_DestroyRenderer(context.renderer);
    SDL_DestroyWindow(context.window);
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

} // namespace

int main() {
    Mocha_InitLibrary();
    Mocha_MountFS("storage_mlc", nullptr, "/vol/storage_mlc01");
    Mocha_MountFS("storage_usb", nullptr, "/vol/storage_usb01");
    RPXLoader_InitLibrary();

    AppContext context;
    if (!initialize(context)) {
        SYSLaunchMenu();
        return 1;
    }

    auto config = loadConfig();
    const bool wasEnabledAtLaunch = config.enabled;
    std::error_code pendingError;
    const bool introWasPending = std::filesystem::exists(INTRO_PENDING_PATH, pendingError);
    std::filesystem::remove(INTRO_PENDING_PATH, pendingError);
    bool enterLauncher = config.enabled;
    if (!config.enabled) {
        enterLauncher = runControlPanel(context, config, true);
    }

    bool shouldPlayIntro = config.introEnabled &&
                           (introWasPending || (!wasEnabledAtLaunch && config.enabled));
    while (enterLauncher) {
        if (shouldPlayIntro) playIntro(context);
        shouldPlayIntro = false;
        const auto result = runLauncher(context, config);
        if (result == LauncherResult::ReturnToControl) {
            enterLauncher = runControlPanel(context, config, false);
            continue;
        }
        break;
    }

    finalize(context);
    RPXLoader_DeInitLibrary();
    Mocha_DeInitLibrary();
    return 0;
}
