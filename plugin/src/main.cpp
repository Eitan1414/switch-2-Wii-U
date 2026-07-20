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
WUPS_PLUGIN_DESCRIPTION("Relance le lanceur Switch2 Mode depuis le menu Wii U.");
WUPS_PLUGIN_VERSION("0.7.2");
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

void launchAfterMenuStarts() {
    launchThreadActive = true;
    OSSleepTicks(OSMillisecondsToTicks(900));

    if (!isEnabled() || bypassHeld()) {
        launchThreadActive = false;
        return;
    }

    FILE* file = std::fopen(APP_FILE_PATH, "rb");
    if (!file) {
        launchThreadActive = false;
        return;
    }
    std::fclose(file);

    RPXLoader_LaunchHomebrew(APP_LAUNCH_PATH);
    launchThreadActive = false;
}

} // namespace

INITIALIZE_PLUGIN() {
    RPXLoader_InitLibrary();
}

ON_APPLICATION_START() {
    if (isMenu(OSGetTitleID()) && firstMenuStart) {
        firstMenuStart = false;
        if (isEnabled()) {
            if (FILE* pending = std::fopen(INTRO_PENDING_PATH, "wb")) {
                std::fputs("1\n", pending);
                std::fclose(pending);
            }
        }
    }

    bool expected = false;
    if (isMenu(OSGetTitleID()) && isEnabled() &&
        launchThreadActive.compare_exchange_strong(expected, true)) {
        std::thread(launchAfterMenuStarts).detach();
    }
}

DEINITIALIZE_PLUGIN() {
    RPXLoader_DeInitLibrary();
}
