#pragma once

#include "common.hpp"
#include "config.hpp"

enum class LauncherResult {
    Quit,
    ReturnToControl,
    DisableAndExit
};

LauncherResult runLauncher(AppContext& context, ModeConfig& config);
bool runControlPanel(AppContext& context, ModeConfig& config, bool firstLaunch);

