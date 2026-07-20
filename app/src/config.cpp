#include "config.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::vector<uint64_t> parseIds(const std::string& value) {
    std::vector<uint64_t> result;
    std::stringstream stream(value);
    std::string token;
    while (std::getline(stream, token, ',')) {
        token = trim(token);
        if (token.empty()) {
            continue;
        }
        uint64_t id = 0;
        const auto parsed = std::from_chars(token.data(), token.data() + token.size(), id, 16);
        if (parsed.ec == std::errc{}) {
            result.push_back(id);
        }
    }
    return result;
}

std::string idsToString(const std::vector<uint64_t>& values) {
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            stream << ',';
        }
        stream << std::setw(16) << values[i];
    }
    return stream.str();
}

std::string readWholeFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        return {};
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

std::string extractJsonString(const std::string& json, const std::string& key) {
    const std::string marker = "\"" + key + "\"";
    auto position = json.find(marker);
    if (position == std::string::npos) {
        return {};
    }
    position = json.find(':', position + marker.size());
    if (position == std::string::npos) {
        return {};
    }
    position = json.find('"', position + 1);
    if (position == std::string::npos) {
        return {};
    }
    const auto end = json.find('"', position + 1);
    if (end == std::string::npos) {
        return {};
    }
    return json.substr(position + 1, end - position - 1);
}

} // namespace

ModeConfig loadConfig() {
    ModeConfig config;
    std::ifstream file(CONFIG_PATH);
    std::string line;
    while (std::getline(file, line)) {
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            continue;
        }
        const auto key = trim(line.substr(0, separator));
        const auto value = trim(line.substr(separator + 1));
        if (key == "enabled") {
            config.enabled = value == "1" || value == "true";
        } else if (key == "intro") {
            config.introEnabled = value == "1" || value == "true";
        } else if (key == "background") {
            if (value == "white") config.background = BackgroundMode::White;
            else if (value == "custom") config.background = BackgroundMode::Custom;
            else config.background = BackgroundMode::UTheme;
        } else if (key == "favorites") {
            config.favorites = parseIds(value);
        } else if (key == "recent") {
            config.recent = parseIds(value);
        }
    }
    return config;
}

bool saveConfig(const ModeConfig& config) {
    std::error_code error;
    std::filesystem::create_directories(MODE_ROOT, error);
    std::ofstream file(CONFIG_PATH, std::ios::trunc);
    if (!file) {
        return false;
    }
    const char* background = "utheme";
    if (config.background == BackgroundMode::White) background = "white";
    if (config.background == BackgroundMode::Custom) background = "custom";
    file << "enabled=" << (config.enabled ? 1 : 0) << '\n';
    file << "intro=" << (config.introEnabled ? 1 : 0) << '\n';
    file << "background=" << background << '\n';
    file << "favorites=" << idsToString(config.favorites) << '\n';
    file << "recent=" << idsToString(config.recent) << '\n';
    return static_cast<bool>(file);
}

bool containsTitle(const std::vector<uint64_t>& values, uint64_t titleId) {
    return std::find(values.begin(), values.end(), titleId) != values.end();
}

void toggleFavorite(ModeConfig& config, uint64_t titleId) {
    const auto found = std::find(config.favorites.begin(), config.favorites.end(), titleId);
    if (found == config.favorites.end()) {
        config.favorites.push_back(titleId);
    } else {
        config.favorites.erase(found);
    }
}

void rememberRecent(ModeConfig& config, uint64_t titleId) {
    config.recent.erase(std::remove(config.recent.begin(), config.recent.end(), titleId), config.recent.end());
    config.recent.insert(config.recent.begin(), titleId);
    if (config.recent.size() > 12) {
        config.recent.resize(12);
    }
}

std::string backgroundModeName(BackgroundMode mode) {
    switch (mode) {
        case BackgroundMode::White: return "Blanc";
        case BackgroundMode::UTheme: return "Theme UTheme";
        case BackgroundMode::Custom: return "Image personnelle";
    }
    return "Blanc";
}

BackgroundMode nextBackgroundMode(BackgroundMode mode, int direction) {
    int value = static_cast<int>(mode);
    value = (value + (direction >= 0 ? 1 : 2)) % 3;
    return static_cast<BackgroundMode>(value);
}

std::filesystem::path findUThemeBackground() {
    const auto configPath = SD_ROOT / "wiiu/environments/aroma/plugins/config/style-mii-u.json";
    const auto json = readWholeFile(configPath);
    auto themeName = extractJsonString(json, "enabledThemes");
    const auto separator = themeName.find('|');
    if (separator != std::string::npos) {
        themeName = themeName.substr(0, separator);
    }
    themeName = trim(themeName);
    if (themeName.empty()) {
        return {};
    }

    const auto root = SD_ROOT / "wiiu/themes" / themeName;
    static constexpr const char* candidates[] = {
        "preview-launcher.webp", "preview-launcher.png", "preview-launcher.jpg",
        "preview-collage.webp", "preview-collage.png", "preview-collage.jpg"
    };
    for (const auto* candidate : candidates) {
        const auto path = root / candidate;
        std::error_code error;
        if (std::filesystem::exists(path, error)) {
            return path;
        }
    }
    return {};
}

