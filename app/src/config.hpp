#pragma once

#include "common.hpp"

#include <cstdint>
#include <string>
#include <vector>

enum class BackgroundMode {
    White,
    UTheme,
    Custom
};

struct ModeConfig {
    bool enabled = false;
    bool introEnabled = true;
    BackgroundMode background = BackgroundMode::UTheme;
    std::vector<uint64_t> favorites;
    std::vector<uint64_t> recent;
};

ModeConfig loadConfig();
bool saveConfig(const ModeConfig& config);
bool containsTitle(const std::vector<uint64_t>& values, uint64_t titleId);
void toggleFavorite(ModeConfig& config, uint64_t titleId);
void rememberRecent(ModeConfig& config, uint64_t titleId);
std::string backgroundModeName(BackgroundMode mode);
BackgroundMode nextBackgroundMode(BackgroundMode mode, int direction);
std::filesystem::path findUThemeBackground();

