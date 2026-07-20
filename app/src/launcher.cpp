#include "launcher.hpp"

#include "intro.hpp"
#include "ui.hpp"
#include "wiiu_collections.hpp"

#include <SDL2/SDL_image.h>

#include <coreinit/mcp.h>
#include <malloc.h>
#include <nn/acp/title.h>
#include <rpxloader/rpxloader.h>
#include <sysapp/launch.h>
#include <sysapp/title.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

namespace {

enum class EntryKind {
    FolderWiiU,
    FolderAll,
    FolderSoftware,
    FolderHomebrew,
    FolderFavorites,
    FolderRecent,
    InstalledTitle,
    SystemSoftware,
    Homebrew
};

struct Entry {
    EntryKind kind = EntryKind::InstalledTitle;
    uint64_t titleId = 0;
    std::string name;
    std::filesystem::path launchPath;
    std::filesystem::path iconPath;
    SDL_Texture* icon = nullptr;
    uint32_t folderId = 0;
    uint8_t folderColor = 0;
    std::vector<uint64_t> folderTitleIds;
};

struct BackgroundTexture {
    SDL_Texture* texture = nullptr;
    ~BackgroundTexture() { SDL_DestroyTexture(texture); }
};

bool isFolderEntry(const Entry& entry) {
    return entry.kind == EntryKind::FolderWiiU ||
           entry.kind == EntryKind::FolderAll ||
           entry.kind == EntryKind::FolderSoftware ||
           entry.kind == EntryKind::FolderHomebrew ||
           entry.kind == EntryKind::FolderFavorites ||
           entry.kind == EntryKind::FolderRecent;
}

bool isTitleEntry(const Entry& entry) {
    return entry.kind == EntryKind::InstalledTitle ||
           entry.kind == EntryKind::SystemSoftware;
}

Entry makeSmartFolder(EntryKind kind, const std::string& name) {
    Entry entry;
    entry.kind = kind;
    entry.name = name;
    return entry;
}

SDL_Color wiiUFolderColor(uint8_t color) {
    static constexpr std::array<SDL_Color, 8> colors{{
        {0, 174, 221, 255},
        {38, 184, 104, 255},
        {241, 195, 44, 255},
        {239, 137, 40, 255},
        {230, 66, 72, 255},
        {233, 104, 169, 255},
        {132, 91, 205, 255},
        {132, 139, 151, 255}
    }};
    return colors[color % colors.size()];
}

bool isMenuTitle(uint64_t titleId) {
    return titleId == 0x0005001010040000ULL ||
           titleId == 0x0005001010040100ULL ||
           titleId == 0x0005001010040200ULL;
}

EntryKind classifyInstalledTitle(uint64_t titleId) {
    const uint64_t type = titleId >> 32;
    if (type == 0x00050000ULL) return EntryKind::InstalledTitle;
    if (type == 0x00050010ULL || type == 0x00050030ULL) {
        return EntryKind::SystemSoftware;
    }
    return EntryKind::FolderAll;
}

bool isLaunchableInstalledTitle(uint64_t titleId) {
    if (isMenuTitle(titleId)) return false;
    const EntryKind kind = classifyInstalledTitle(titleId);
    return kind == EntryKind::InstalledTitle || kind == EntryKind::SystemSoftware;
}

std::string chooseTitleName(const ACPMetaXml& metadata) {
    if (metadata.shortname_fr[0] != '\0') return metadata.shortname_fr;
    if (metadata.shortname_en[0] != '\0') return metadata.shortname_en;
    if (metadata.longname_fr[0] != '\0') return metadata.longname_fr;
    return metadata.longname_en;
}

std::string lowerExtension(std::filesystem::path path) {
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
    return extension;
}

std::vector<Entry> loadInstalledEntries() {
    std::vector<Entry> result;
    const int handle = MCP_Open();
    const int count = handle >= 0 ? MCP_TitleCount(handle) : 0;
    if (handle >= 0 && count > 0) {
        std::vector<MCPTitleListType> titles(static_cast<std::size_t>(count));
        uint32_t outCount = static_cast<uint32_t>(count);
        if (MCP_TitleList(handle, &outCount, titles.data(),
                          titles.size() * sizeof(MCPTitleListType)) >= 0) {
            for (uint32_t index = 0; index < outCount; ++index) {
                const auto& title = titles[index];
                if (!isLaunchableInstalledTitle(title.titleId)) continue;

                Entry entry;
                entry.kind = classifyInstalledTitle(title.titleId);
                entry.titleId = title.titleId;

                auto* metadata = static_cast<ACPMetaXml*>(memalign(0x40, sizeof(ACPMetaXml)));
                if (metadata) {
                    std::memset(metadata, 0, sizeof(ACPMetaXml));
                    if (ACPGetTitleMetaXml(title.titleId, metadata) == ACP_RESULT_SUCCESS) {
                        entry.name = chooseTitleName(*metadata);
                    }
                    free(metadata);
                }

                if (entry.name.empty()) {
                    char fallback[40];
                    std::snprintf(fallback, sizeof(fallback),
                                  entry.kind == EntryKind::SystemSoftware
                                      ? "Logiciel %08x"
                                      : "Jeu %08x",
                                  static_cast<uint32_t>(title.titleId));
                    entry.name = fallback;
                }

                char metaDirectory[256]{};
                if (ACPGetTitleMetaDirByTitleListType(title, metaDirectory,
                                                      sizeof(metaDirectory)) == ACP_RESULT_SUCCESS) {
                    entry.iconPath = std::filesystem::path(metaDirectory) / "iconTex.tga";
                }
                result.push_back(std::move(entry));
            }
        }
    }
    if (handle >= 0) MCP_Close(handle);

    std::error_code error;
    if (std::filesystem::exists(APPS_PATH, error)) {
        const auto options = std::filesystem::directory_options::skip_permission_denied;
        std::filesystem::recursive_directory_iterator iterator(APPS_PATH, options, error);
        const std::filesystem::recursive_directory_iterator end;
        while (!error && iterator != end) {
            const auto file = *iterator;
            iterator.increment(error);
            if (error) break;

            std::error_code fileError;
            if (!file.is_regular_file(fileError) || fileError) continue;
            const auto extension = lowerExtension(file.path());
            if (extension != ".wuhb" && extension != ".rpx") continue;
            if (file.path().filename() == "Switch2Mode.wuhb") continue;

            Entry entry;
            entry.kind = EntryKind::Homebrew;
            entry.name = file.path().stem().string();
            entry.launchPath = file.path();
            result.push_back(std::move(entry));
        }
    }

    std::sort(result.begin(), result.end(), [](const Entry& left, const Entry& right) {
        if (left.kind != right.kind) return static_cast<int>(left.kind) < static_cast<int>(right.kind);
        return left.name < right.name;
    });
    return result;
}

std::vector<Entry*> filterEntries(std::vector<Entry>& all, const Entry& folder,
                                  const ModeConfig& config) {
    std::vector<Entry*> result;
    for (auto& entry : all) {
        if (!isTitleEntry(entry) && entry.kind != EntryKind::Homebrew) continue;

        if (folder.kind == EntryKind::FolderAll && entry.kind != EntryKind::InstalledTitle) continue;
        if (folder.kind == EntryKind::FolderSoftware && entry.kind != EntryKind::SystemSoftware) continue;
        if (folder.kind == EntryKind::FolderHomebrew && entry.kind != EntryKind::Homebrew) continue;
        if (folder.kind == EntryKind::FolderFavorites &&
            (!isTitleEntry(entry) || !containsTitle(config.favorites, entry.titleId))) continue;
        if (folder.kind == EntryKind::FolderRecent &&
            (!isTitleEntry(entry) || !containsTitle(config.recent, entry.titleId))) continue;
        if (folder.kind == EntryKind::FolderWiiU &&
            (entry.kind != EntryKind::InstalledTitle ||
             !containsTitle(folder.folderTitleIds, entry.titleId))) continue;

        result.push_back(&entry);
    }

    const std::vector<uint64_t>* order = nullptr;
    if (folder.kind == EntryKind::FolderRecent) order = &config.recent;
    if (folder.kind == EntryKind::FolderWiiU) order = &folder.folderTitleIds;
    if (order) {
        std::stable_sort(result.begin(), result.end(), [order](const Entry* left, const Entry* right) {
            const auto leftIndex = std::find(order->begin(), order->end(), left->titleId);
            const auto rightIndex = std::find(order->begin(), order->end(), right->titleId);
            return leftIndex < rightIndex;
        });
    }
    return result;
}

SDL_Texture* loadBackground(AppContext& context, const ModeConfig& config) {
    std::filesystem::path path;
    if (config.background == BackgroundMode::Custom) {
        path = CUSTOM_BACKGROUND_PATH;
    } else if (config.background == BackgroundMode::UTheme) {
        path = findUThemeBackground();
    }
    if (path.empty()) return nullptr;
    return IMG_LoadTexture(context.renderer, path.string().c_str());
}

void drawBaseBackground(AppContext& context, SDL_Texture* background) {
    SDL_SetRenderDrawColor(context.renderer, 247, 248, 251, 255);
    SDL_RenderClear(context.renderer);
    if (!background) return;

    SDL_Rect screen{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_SetTextureColorMod(background, 245, 245, 245);
    SDL_SetTextureAlphaMod(background, 120);
    SDL_RenderCopy(context.renderer, background, nullptr, &screen);
    SDL_SetRenderDrawBlendMode(context.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(context.renderer, 255, 255, 255, 145);
    SDL_RenderFillRect(context.renderer, &screen);
}

void drawTopBar(AppContext& context, const std::string& subtitle) {
    refreshSystemStatus(context);
    drawText(context, context.fontMedium, "Wii U", 58, 32, {18, 22, 29, 255});
    drawText(context, context.fontSmall, "SWITCH 2 MODE", 154, 42, {12, 158, 207, 255});
    if (!subtitle.empty()) {
        drawTextFitted(context, context.fontSmall, subtitle, 58, 94, 760,
                       {76, 80, 88, 255});
    }

    drawCircle(context, 883, 49, 21, {0, 170, 220, 255});
    const std::string initial = context.profileName.empty()
                                    ? "U"
                                    : std::string(1, context.profileName.front());
    drawText(context, context.fontSmall, initial, 883, 34,
             {255, 255, 255, 255}, true);
    drawTextFitted(context, context.fontSmall, context.profileName, 914, 37, 108,
                   {44, 48, 56, 255});

    const SDL_Color wifiColor = context.wifiConnected
                                    ? SDL_Color{44, 48, 56, 255}
                                    : SDL_Color{170, 174, 182, 255};
    SDL_SetRenderDrawColor(context.renderer, wifiColor.r, wifiColor.g,
                           wifiColor.b, wifiColor.a);
    for (int bar = 0; bar < 3; ++bar) {
        SDL_Rect signal{1028 + bar * 7, 57 - bar * 6, 5, 7 + bar * 6};
        SDL_RenderFillRect(context.renderer, &signal);
    }
    if (!context.wifiConnected) {
        SDL_RenderDrawLine(context.renderer, 1027, 35, 1050, 64);
    }

    std::time_t current = std::time(nullptr);
    std::tm* local = std::localtime(&current);
    char clock[16]{};
    if (local) std::strftime(clock, sizeof(clock), "%H:%M", local);
    drawTextFitted(context, context.fontMedium, clock, 1070, 32, 90,
                   {30, 34, 40, 255});

    SDL_Rect batteryOutline{1182, 37, 54, 25};
    SDL_Rect batteryTip{1237, 44, 5, 11};
    SDL_SetRenderDrawColor(context.renderer, 44, 48, 56, 255);
    SDL_RenderDrawRect(context.renderer, &batteryOutline);
    SDL_RenderFillRect(context.renderer, &batteryTip);
    const int batteryWidth = std::clamp(context.batteryLevel, 0, 6) * 46 / 6;
    SDL_Rect batteryFill{1186, 41, batteryWidth, 17};
    const SDL_Color batteryColor = context.batteryLevel <= 1
                                       ? SDL_Color{235, 66, 78, 255}
                                       : SDL_Color{44, 48, 56, 255};
    SDL_SetRenderDrawColor(context.renderer, batteryColor.r, batteryColor.g,
                           batteryColor.b, batteryColor.a);
    SDL_RenderFillRect(context.renderer, &batteryFill);
}

SDL_Color accentForIndex(std::size_t index) {
    static constexpr std::array<SDL_Color, 6> colors{{
        {0, 181, 226, 255}, {245, 62, 79, 255}, {100, 91, 220, 255},
        {20, 184, 128, 255}, {243, 150, 32, 255}, {37, 115, 209, 255}
    }};
    return colors[index % colors.size()];
}

SDL_Color accentForEntry(const Entry& entry, std::size_t index) {
    if (entry.kind == EntryKind::FolderWiiU) return wiiUFolderColor(entry.folderColor);
    if (entry.kind == EntryKind::FolderSoftware || entry.kind == EntryKind::SystemSoftware) {
        return {91, 102, 218, 255};
    }
    if (entry.kind == EntryKind::FolderHomebrew || entry.kind == EntryKind::Homebrew) {
        return {26, 174, 126, 255};
    }
    return accentForIndex(index);
}

void ensureIcon(AppContext& context, Entry& entry) {
    if (!entry.icon && !entry.iconPath.empty()) {
        entry.icon = IMG_LoadTexture(context.renderer, entry.iconPath.string().c_str());
    }
}

void destroyEntries(std::vector<Entry>& entries) {
    for (auto& entry : entries) {
        SDL_DestroyTexture(entry.icon);
        entry.icon = nullptr;
    }
}

bool launchEntry(Entry& entry, ModeConfig& config) {
    if (isTitleEntry(entry)) {
        rememberRecent(config, entry.titleId);
        saveConfig(config);
        SYSLaunchTitle(entry.titleId);
        return true;
    }
    if (entry.kind == EntryKind::Homebrew) {
        saveConfig(config);
        auto path = entry.launchPath.string();
        constexpr const char* prefix = "fs:/vol/external01/";
        if (path.rfind(prefix, 0) == 0) {
            path.erase(0, std::char_traits<char>::length(prefix));
        }
        return RPXLoader_LaunchHomebrew(path.c_str()) == RPX_LOADER_RESULT_SUCCESS;
    }
    return false;
}

enum class SystemShortcutResult {
    Failed,
    OverlayOpened,
    TitleLaunching
};

SystemShortcutResult launchSystemShortcut(int index) {
    switch (index) {
        case 0:
            return SYSSwitchTo(SYSAPP_PFID_MIIVERSE) >= 0
                       ? SystemShortcutResult::OverlayOpened
                       : SystemShortcutResult::Failed;
        case 1:
            return SYSSwitchTo(SYSAPP_PFID_ESHOP) >= 0
                       ? SystemShortcutResult::OverlayOpened
                       : SystemShortcutResult::Failed;
        case 2:
            return SYSSwitchTo(SYSAPP_PFID_BROWSER) >= 0
                       ? SystemShortcutResult::OverlayOpened
                       : SystemShortcutResult::Failed;
        case 3: {
            const uint64_t titleId =
                _SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_NOTIFICATIONS);
            if (titleId == 0 || !SYSCheckTitleExists(titleId)) {
                return SystemShortcutResult::Failed;
            }
            SYSLaunchTitle(titleId);
            return SystemShortcutResult::TitleLaunching;
        }
        case 4:
            return SYSSwitchTo(SYSAPP_PFID_FRIENDLIST) >= 0
                       ? SystemShortcutResult::OverlayOpened
                       : SystemShortcutResult::Failed;
        case 5:
            return SYSSwitchTo(SYSAPP_PFID_DOWNLOAD_MANAGEMENT) >= 0
                       ? SystemShortcutResult::OverlayOpened
                       : SystemShortcutResult::Failed;
        default:
            return SystemShortcutResult::Failed;
    }
}

void playLaunchTransition(AppContext& context, SDL_Texture* background,
                          Entry& entry, SDL_Color accent) {
    ensureIcon(context, entry);
    stopBackgroundMusic(650);

    constexpr float durationMs = 860.0f;
    const uint64_t startedAt = SDL_GetTicks64();
    float progress = 0.0f;

    while (progress < 1.0f) {
        const uint64_t now = SDL_GetTicks64();
        progress = std::min(1.0f,
                            static_cast<float>(now - startedAt) / durationMs);
        const float zoom = easeOutCubic(std::min(1.0f, progress * 1.3f));
        (void) pollInput(context);

        drawBaseBackground(context, background);
        drawTopBar(context, "Lancement");

        SDL_SetRenderDrawBlendMode(context.renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(context.renderer, 10, 14, 22,
                               static_cast<Uint8>(105.0f * zoom));
        SDL_Rect screen{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(context.renderer, &screen);

        const int cardSize = 214 + static_cast<int>(zoom * 94.0f);
        const int lift = static_cast<int>(zoom * 32.0f);
        SDL_Rect card{SCREEN_WIDTH / 2 - cardSize / 2,
                      SCREEN_HEIGHT / 2 - cardSize / 2 - lift,
                      cardSize, cardSize};
        drawRoundedPanel(context, card, {255, 255, 255, 252},
                         {accent.r, accent.g, accent.b, 255}, 6);

        SDL_Rect iconRect{card.x + 13, card.y + 13, card.w - 26, card.h - 26};
        if (entry.icon) {
            SDL_RenderCopy(context.renderer, entry.icon, nullptr, &iconRect);
        } else {
            drawPlaceholderIcon(context, iconRect, entry.name, accent);
        }

        drawTextFitted(context, context.fontMedium, entry.name,
                       SCREEN_WIDTH / 2, card.y + card.h + 28, 760,
                       {255, 255, 255, 255}, true);

        if (progress > 0.62f) {
            const float fade = (progress - 0.62f) / 0.38f;
            SDL_SetRenderDrawColor(context.renderer, 0, 0, 0,
                                   static_cast<Uint8>(255.0f * fade));
            SDL_RenderFillRect(context.renderer, &screen);
        }

        SDL_RenderPresent(context.renderer);
        if (progress < 1.0f) SDL_Delay(16);
    }
}

} // namespace

bool runControlPanel(AppContext& context, ModeConfig& config, bool firstLaunch) {
    startBackgroundMusic(context);
    int selection = 0;
    uint64_t previous = SDL_GetTicks64();
    float animation = 0.0f;
    bool running = true;

    while (running) {
        const uint64_t now = SDL_GetTicks64();
        const float dt = std::min(0.05f,
                                  static_cast<float>(now - previous) / 1000.0f);
        previous = now;
        animation = std::min(1.0f, animation + dt * 2.5f);

        const auto input = pollInput(context);
        if (input.quit) return false;
        if (input.back) {
            playBackSound(context);
            return config.enabled;
        }
        if (input.left) {
            selection = (selection + 2) % 3;
            playMoveSound(context);
        }
        if (input.right) {
            selection = (selection + 1) % 3;
            playMoveSound(context);
        }
        if (input.y) {
            config.introEnabled = !config.introEnabled;
            playAcceptSound(context);
        }
        if (input.x) {
            config.background = nextBackgroundMode(config.background, 1);
            playMoveSound(context);
        }

        if (input.accept) {
            playAcceptSound(context);
            if (selection == 0) {
                config.enabled = true;
                saveConfig(config);
                return true;
            }
            if (selection == 1) {
                playIntro(context);
                previous = SDL_GetTicks64();
            }
            if (selection == 2) {
                config.enabled = false;
                saveConfig(config);
                stopBackgroundMusic(300);
                SYSLaunchMenu();
                return false;
            }
        }

        SDL_SetRenderDrawColor(context.renderer, 246, 248, 252, 255);
        SDL_RenderClear(context.renderer);
        drawTopBar(context, firstLaunch ? "Configuration initiale"
                                        : "Panneau du mode");

        drawTextFitted(context, context.fontLarge,
                       "Une nouvelle facon de parcourir la Wii U",
                       640, 142, 1140, {18, 22, 30, 255}, true);
        drawTextFitted(context, context.fontSmall,
                       config.enabled ? "Le mode est actif"
                                      : "Le mode est actuellement desactive",
                       640, 205, 900,
                       config.enabled ? SDL_Color{0, 154, 112, 255}
                                      : SDL_Color{100, 105, 114, 255}, true);

        const std::array<std::string, 3> labels{
            "Activer et lancer", "Voir l'intro", "Desactiver et quitter"
        };
        const std::array<std::string, 3> descriptions{
            "Ouvre le lanceur horizontal",
            "Relit ta video de demarrage",
            "Retourne au menu Wii U"
        };

        for (int index = 0; index < 3; ++index) {
            const float delay = std::max(0.0f, animation - index * 0.12f);
            const float eased = easeOutCubic(std::min(1.0f, delay * 1.35f));
            const int x = 110 + index * 360 +
                          static_cast<int>((1.0f - eased) * 300.0f);
            SDL_Rect card{x, 285, 320, 190};
            const bool selected = index == selection;
            drawRoundedPanel(context, card,
                             selected ? SDL_Color{255, 255, 255, 250}
                                      : SDL_Color{237, 240, 246, 235},
                             selected ? SDL_Color{0, 172, 220, 255}
                                      : SDL_Color{206, 211, 220, 255},
                             selected ? 5 : 2);
            SDL_Rect labelArea{card.x + 16, card.y + 34, card.w - 32, 62};
            drawTextWrapped(context, context.fontMedium, labels[index], labelArea,
                            {25, 29, 36, 255}, 2, true);
            SDL_Rect descriptionArea{card.x + 18, card.y + 100, card.w - 36, 62};
            drawTextWrapped(context, context.fontSmall, descriptions[index],
                            descriptionArea, {95, 100, 110, 255}, 2, true);
        }

        drawTextFitted(context, context.fontSmall,
                       "Intro : " + std::string(config.introEnabled ? "active" : "inactive") +
                       "     Fond : " + backgroundModeName(config.background),
                       640, 535, 1000, {69, 74, 84, 255}, true);
        drawButtonHint(context, "A", "Choisir", 80, 645);
        drawButtonHint(context, "X", "Changer le fond", 260, 645);
        drawButtonHint(context, "Y", "Activer/desactiver l'intro", 520, 645);
        drawButtonHint(context, "B", "Retour", 890, 645);
        SDL_RenderPresent(context.renderer);
    }
    return false;
}

LauncherResult runLauncher(AppContext& context, ModeConfig& config) {
    startBackgroundMusic(context);
    auto entries = loadInstalledEntries();
    const WiiUMenuLayout menuLayout = loadWiiUMenuLayout();

    std::vector<Entry> folders;
    folders.reserve(menuLayout.collections.size() + 5);
    for (const auto& collection : menuLayout.collections) {
        Entry folder;
        folder.kind = EntryKind::FolderWiiU;
        folder.folderId = collection.id;
        folder.name = collection.name;
        folder.folderColor = collection.color;
        folder.folderTitleIds = collection.titleIds;
        folders.push_back(std::move(folder));
    }
    folders.push_back(makeSmartFolder(EntryKind::FolderAll, "Tous les jeux"));
    folders.push_back(makeSmartFolder(EntryKind::FolderSoftware, "Logiciels Wii U"));
    folders.push_back(makeSmartFolder(EntryKind::FolderHomebrew, "Homebrews"));
    folders.push_back(makeSmartFolder(EntryKind::FolderFavorites, "Favoris"));
    folders.push_back(makeSmartFolder(EntryKind::FolderRecent, "Recents"));

    std::vector<uint64_t> assignedTitleIds;
    for (const auto& collection : menuLayout.collections) {
        for (uint64_t titleId : collection.titleIds) {
            if (!containsTitle(assignedTitleIds, titleId)) {
                assignedTitleIds.push_back(titleId);
            }
        }
    }

    std::vector<Entry*> rootEntries;
    if (menuLayout.loaded) {
        for (uint64_t titleId : menuLayout.rootTitleIds) {
            const auto found = std::find_if(entries.begin(), entries.end(),
                                            [titleId](const Entry& entry) {
                return entry.kind == EntryKind::InstalledTitle &&
                       entry.titleId == titleId;
            });
            if (found != entries.end()) rootEntries.push_back(&*found);
        }
        for (auto& entry : entries) {
            if (entry.kind != EntryKind::InstalledTitle) continue;
            if (std::find(rootEntries.begin(), rootEntries.end(), &entry) != rootEntries.end()) continue;
            if (!containsTitle(assignedTitleIds, entry.titleId)) rootEntries.push_back(&entry);
        }
    } else {
        for (auto& entry : entries) {
            if (entry.kind == EntryKind::InstalledTitle) rootEntries.push_back(&entry);
        }
    }

    BackgroundTexture background{loadBackground(context, config)};
    bool inFolder = false;
    Entry* currentFolder = nullptr;
    std::size_t rootFolderSelection = 0;
    std::vector<Entry*> visible;
    std::size_t selection = 0;
    bool dockActive = false;
    int dockSelection = 4;
    float scroll = 0.0f;
    float enterAnimation = 0.0f;
    uint64_t previous = SDL_GetTicks64();
    bool running = true;

    auto rebuildVisible = [&]() {
        visible.clear();
        if (inFolder && currentFolder) {
            visible = filterEntries(entries, *currentFolder, config);
        } else {
            for (auto& folder : folders) visible.push_back(&folder);
            for (Entry* entry : rootEntries) visible.push_back(entry);
        }
        if (selection >= visible.size()) {
            selection = visible.empty() ? 0 : visible.size() - 1;
        }
        scroll = static_cast<float>(selection) * 220.0f;
        enterAnimation = 0.0f;
    };
    rebuildVisible();

    while (running) {
        const uint64_t now = SDL_GetTicks64();
        const float dt = std::min(0.05f,
                                  static_cast<float>(now - previous) / 1000.0f);
        previous = now;
        enterAnimation = std::min(1.0f, enterAnimation + dt * 2.8f);

        const auto input = pollInput(context);
        if (input.quit) {
            destroyEntries(entries);
            return LauncherResult::Quit;
        }
        if (input.down && !dockActive) {
            dockActive = true;
            playMoveSound(context);
        }
        if (input.up && dockActive) {
            dockActive = false;
            playMoveSound(context);
        }
        if (input.left) {
            if (dockActive && dockSelection > 0) {
                --dockSelection;
                playMoveSound(context);
            } else if (!dockActive && selection > 0) {
                --selection;
                playMoveSound(context);
            }
        }
        if (input.right) {
            if (dockActive && dockSelection < 5) {
                ++dockSelection;
                playMoveSound(context);
            } else if (!dockActive && selection + 1 < visible.size()) {
                ++selection;
                playMoveSound(context);
            }
        }
        if (input.plus || input.x || (input.back && !inFolder)) {
            playBackSound(context);
            destroyEntries(entries);
            return LauncherResult::ReturnToControl;
        }
        if (input.back && inFolder) {
            playBackSound(context);
            inFolder = false;
            currentFolder = nullptr;
            selection = rootFolderSelection;
            rebuildVisible();
        }

        if (!dockActive && !visible.empty() && input.y) {
            Entry* selected = visible[selection];
            if (isTitleEntry(*selected) && selected->titleId != 0) {
                toggleFavorite(config, selected->titleId);
                playAcceptSound(context);
                saveConfig(config);
                if (inFolder && currentFolder &&
                    currentFolder->kind == EntryKind::FolderFavorites) {
                    rebuildVisible();
                }
            }
        }

        if (dockActive && input.accept) {
            playAcceptSound(context);
            const SystemShortcutResult result = launchSystemShortcut(dockSelection);
            if (result == SystemShortcutResult::TitleLaunching) {
                destroyEntries(entries);
                return LauncherResult::Quit;
            }
            if (result == SystemShortcutResult::Failed) playBackSound(context);
        } else if (!visible.empty() && input.accept) {
            Entry* selected = visible[selection];
            if (isFolderEntry(*selected)) {
                currentFolder = selected;
                rootFolderSelection = selection;
                playFolderSound(context);
                inFolder = true;
                selection = 0;
                rebuildVisible();
            } else {
                playLaunchSound(context);
                playLaunchTransition(context, background.texture, *selected,
                                     accentForEntry(*selected, selection));
                if (launchEntry(*selected, config)) {
                    running = false;
                } else {
                    playBackSound(context);
                    startBackgroundMusic(context);
                    previous = SDL_GetTicks64();
                }
            }
        }

        const float targetScroll = static_cast<float>(selection) * 220.0f;
        scroll += (targetScroll - scroll) * std::min(1.0f, dt * 12.0f);

        drawBaseBackground(context, background.texture);
        const std::string subtitle = inFolder && currentFolder
                                         ? currentFolder->name
                                         : "Accueil";
        drawTopBar(context, subtitle);

        if (visible.empty()) {
            const std::string message = currentFolder &&
                                        currentFolder->kind == EntryKind::FolderWiiU
                                            ? "Cette collection est vide"
                                            : "Cette vue est vide";
            drawTextFitted(context, context.fontLarge, message,
                           640, 330, 1000, {64, 68, 76, 255}, true);
        }

        const float slideIn = (1.0f - easeOutCubic(enterAnimation)) * 1280.0f;
        for (std::size_t index = 0; index < visible.size(); ++index) {
            Entry& entry = *visible[index];
            const float xFloat = 520.0f + static_cast<float>(index) * 220.0f -
                                 scroll + slideIn;
            const int x = static_cast<int>(xFloat);
            if (x < -210 || x > SCREEN_WIDTH + 20) continue;

            const bool selected = index == selection && !dockActive;
            SDL_Rect card{x, selected ? 220 : 242,
                          selected ? 204 : 184,
                          selected ? 288 : 258};
            drawRoundedPanel(context, card,
                             selected ? SDL_Color{255, 255, 255, 250}
                                      : SDL_Color{244, 246, 250, 235},
                             selected ? SDL_Color{0, 174, 221, 255}
                                      : SDL_Color{194, 200, 210, 220},
                             selected ? 6 : 2);

            SDL_Rect iconRect{card.x + 12, card.y + 12,
                              card.w - 24, card.w - 24};
            const SDL_Color accent = accentForEntry(entry, index);
            if (isFolderEntry(entry)) {
                drawFolderIcon(context, iconRect, accent);
            } else {
                ensureIcon(context, entry);
                if (entry.icon) {
                    SDL_RenderCopy(context.renderer, entry.icon, nullptr, &iconRect);
                } else {
                    drawPlaceholderIcon(context, iconRect, entry.name, accent);
                }
            }

            SDL_Rect nameArea{card.x + 10, card.y + card.h - 74,
                              card.w - 20, 64};
            drawTextWrapped(context,
                            selected ? context.fontMedium : context.fontSmall,
                            entry.name, nameArea, {31, 35, 43, 255}, 2, true);

            if (entry.titleId && containsTitle(config.favorites, entry.titleId)) {
                drawText(context, context.fontMedium, "*", card.x + card.w - 28,
                         card.y + 5, {245, 67, 86, 255}, true);
            }
        }

        if (!visible.empty()) {
            drawTextFitted(context, context.fontSmall, visible[selection]->name,
                           640, 555, 900, {47, 51, 59, 255}, true);
        }
        drawSystemDock(context, dockActive ? dockSelection : -1);
        drawButtonHint(context, "A", "Ouvrir", 60, 682);
        drawButtonHint(context, "B", inFolder ? "Fermer" : "Panneau", 210, 682);
        drawButtonHint(context, "Y", "Favori", 1020, 682);
        drawButtonHint(context, "X", "Options", 1150, 682);
        SDL_RenderPresent(context.renderer);
    }

    destroyEntries(entries);
    return LauncherResult::Quit;
}
