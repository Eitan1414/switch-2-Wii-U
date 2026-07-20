#include <wups.h>

#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <coreinit/title.h>
#include <rpxloader/rpxloader.h>
#include <vpad/input.h>

#include <atomic>
#include <cstdio>
#include <fstream>
#include <string>
#include <thread>

WUPS_PLUGIN_NAME("Switch2 Mode Bootstrap");
WUPS_PLUGIN_DESCRIPTION("Utilise Switch2 Mode comme ecran d'accueil principal.");
WUPS_PLUGIN_VERSION("0.8.1");
WUPS_PLUGIN_AUTHOR("Eitan1414");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();

namespace {

constexpr uint64_t MENU_JPN = 0x0005001010040000ULL;
constexpr uint64_t MENU_USA = 0x0005001010040100ULL;
constexpr uint64_t MENU_EUR = 0x0005001010040200ULL;
constexpr const char* CONFIG_PATH = "fs:/vol/external01/wiiu/switch2mode/config.ini";
constexpr const char* INTRO_PENDING_PATH = "fs:/vol/external01/wiiu/switch2mode/intro.pending";
constexpr const char* APP_FILE_PATH = "fs:/vol/external01/wiiu/apps/Switch2Mode.wuhb";
constexpr const char* APP_LAUNCH_PATH = "wiiu/apps/Switch2Mode.wuhb";

constexpr int BOOT_GUARD_STEPS = 12;
constexpr int BOOT_GUARD_STEP_MS = 25;
constexpr int LAUNCH_RETRY_COUNT = 8;
constexpr int LAUNCH_RETRY_DELAY_MS = 120;

std::atomic_bool launchThreadActive{false};
bool firstMenuStart = true;

bool isMenu(uint64_t titleId) {
    return titleId == MENU_JPN || titleId == MENU_USA || titleId == MENU_EUR;
}

bool isEnabled() {
    std::ifstream file(CONFIG_PATH);
    std::string line;
    while (std::getline(file, line)) {
        if (line == "enabled=1" || line == "enabled=true") return true;
    }
    return false;
}

bool bypassHeld() {
    VPADStatus status{};
    VPADReadError error = VPAD_READ_SUCCESS;
    const int32_t read = VPADRead(VPAD_CHAN_0, &status, 1, &error);
    return read > 0 && error == VPAD_READ_SUCCESS && (status.hold & VPAD_BUTTON_B);
}

bool waitForFastBootWindow() {
    for (int step = 0; step < BOOT_GUARD_STEPS; ++step) {
        if (!isEnabled() || bypassHeld()) return false;
        OSSleepTicks(OSMillisecondsToTicks(BOOT_GUARD_STEP_MS));
    }
    return true;
}

bool appExists() {
    FILE* file = std::fopen(APP_FILE_PATH, "rb");
    if (!file) return false;
    std::fclose(file);
    return true;
}

void launchPrimaryShell() {
    if (!waitForFastBootWindow() || !appExists()) {
        launchThreadActive = false;
        return;
    }

    for (int attempt = 0; attempt < LAUNCH_RETRY_COUNT; ++attempt) {
        if (!isEnabled() || bypassHeld()) break;

        if (RPXLoader_LaunchHomebrew(APP_LAUNCH_PATH) == RPX_LOADER_RESULT_SUCCESS) {
            launchThreadActive = false;
            return;
        }

        OSSleepTicks(OSMillisecondsToTicks(LAUNCH_RETRY_DELAY_MS));
    }

    launchThreadActive = false;
}

} // namespace

INITIALIZE_PLUGIN() {
    RPXLoader_InitLibrary();
}

ON_APPLICATION_START() {
    const bool menuStarted = isMenu(OSGetTitleID());
    if (!menuStarted) return;

    if (firstMenuStart) {
        firstMenuStart = false;
        if (isEnabled()) {
            if (FILE* pending = std::fopen(INTRO_PENDING_PATH, "wb")) {
                std::fputs("1\n", pending);
                std::fclose(pending);
            }
        }
    }

    bool expected = false;
    if (isEnabled() && launchThreadActive.compare_exchange_strong(expected, true)) {
        std::thread(launchPrimaryShell).detach();
    }
}

DEINITIALIZE_PLUGIN() {
    RPXLoader_DeInitLibrary();
}
