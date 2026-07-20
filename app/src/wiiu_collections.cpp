#include "wiiu_collections.hpp"

#include <nn/act.h>
#include <sysapp/title.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace {

constexpr std::size_t ENTRY_SIZE = 16;
constexpr std::size_t ROOT_ENTRY_COUNT = 300;
constexpr std::size_t ROOT_NAND_OFFSET = 0x000000;
constexpr std::size_t ROOT_USB_OFFSET = 0x0012C0;
constexpr std::size_t FIRST_FOLDER_OFFSET = 0x002D24;
constexpr std::size_t FOLDER_ENTRY_COUNT = 60;
constexpr std::size_t FOLDER_BANK_SIZE = FOLDER_ENTRY_COUNT * ENTRY_SIZE;
constexpr std::size_t FOLDER_INFO_SIZE = 56;
constexpr std::size_t FOLDER_STRIDE = FOLDER_BANK_SIZE * 2 + FOLDER_INFO_SIZE;
constexpr std::size_t FOLDER_NAME_UNITS = 17;
constexpr std::size_t FOLDER_COLOR_OFFSET = 0x32;

constexpr uint8_t MENU_ITEM_NAND = 0x01;
constexpr uint8_t MENU_ITEM_USB = 0x02;
constexpr uint8_t MENU_ITEM_FOLDER = 0x10;

struct RawMenuEntry {
    uint64_t titleId = 0;
    uint32_t lowId = 0;
    uint8_t type = 0;
};

uint16_t readBe16(const std::vector<uint8_t>& data, std::size_t offset) {
    if (offset + 2 > data.size()) return 0;
    return static_cast<uint16_t>((static_cast<uint16_t>(data[offset]) << 8) |
                                 static_cast<uint16_t>(data[offset + 1]));
}

uint32_t readBe32(const std::vector<uint8_t>& data, std::size_t offset) {
    if (offset + 4 > data.size()) return 0;
    return (static_cast<uint32_t>(data[offset]) << 24) |
           (static_cast<uint32_t>(data[offset + 1]) << 16) |
           (static_cast<uint32_t>(data[offset + 2]) << 8) |
           static_cast<uint32_t>(data[offset + 3]);
}

RawMenuEntry readEntry(const std::vector<uint8_t>& data, std::size_t offset) {
    RawMenuEntry entry;
    if (offset + ENTRY_SIZE > data.size()) return entry;
    const uint32_t high = readBe32(data, offset);
    entry.lowId = readBe32(data, offset + 4);
    entry.titleId = (static_cast<uint64_t>(high) << 32) | entry.lowId;
    entry.type = data[offset + 0x0B];
    return entry;
}

void appendUtf8(std::string& output, uint32_t codePoint) {
    if (codePoint <= 0x7F) {
        output.push_back(static_cast<char>(codePoint));
    } else if (codePoint <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
        output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else if (codePoint <= 0xFFFF) {
        output.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
        output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else {
        output.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
        output.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    }
}

std::string decodeFolderName(const std::vector<uint8_t>& data, std::size_t offset) {
    std::string result;
    for (std::size_t i = 0; i < FOLDER_NAME_UNITS; ++i) {
        uint16_t unit = readBe16(data, offset + i * 2);
        if (unit == 0) break;

        uint32_t codePoint = unit;
        if (unit >= 0xD800 && unit <= 0xDBFF && i + 1 < FOLDER_NAME_UNITS) {
            const uint16_t low = readBe16(data, offset + (i + 1) * 2);
            if (low >= 0xDC00 && low <= 0xDFFF) {
                codePoint = 0x10000 +
                            ((static_cast<uint32_t>(unit - 0xD800) << 10) |
                             static_cast<uint32_t>(low - 0xDC00));
                ++i;
            } else {
                codePoint = 0xFFFD;
            }
        } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
            codePoint = 0xFFFD;
        }
        appendUtf8(result, codePoint);
    }
    return result;
}

bool readBinaryFile(const std::filesystem::path& path, std::vector<uint8_t>& output) {
    FILE* file = std::fopen(path.string().c_str(), "rb");
    if (!file) return false;

    if (std::fseek(file, 0, SEEK_END) != 0) {
        std::fclose(file);
        return false;
    }
    const long length = std::ftell(file);
    if (length <= 0 || std::fseek(file, 0, SEEK_SET) != 0) {
        std::fclose(file);
        return false;
    }

    output.resize(static_cast<std::size_t>(length));
    const std::size_t read = std::fread(output.data(), 1, output.size(), file);
    std::fclose(file);
    if (read != output.size()) {
        output.clear();
        return false;
    }
    return true;
}

std::string hex8(uint32_t value) {
    std::array<char, 9> buffer{};
    std::snprintf(buffer.data(), buffer.size(), "%08X", value);
    return buffer.data();
}

bool fileExists(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_regular_file(path, error);
}

std::filesystem::path findAromaRedirect(uint32_t persistentId) {
    const std::filesystem::path base =
        "fs:/vol/external01/wiiu/homebrew_on_menu_plugin";
    const std::string user = hex8(persistentId);
    std::error_code error;

    if (std::filesystem::is_directory(base, error)) {
        for (const auto& directory : std::filesystem::directory_iterator(base, error)) {
            if (error) break;
            if (!directory.is_directory(error)) continue;
            const auto candidate = directory.path() / "save" / user /
                                   "BaristaAccountSaveFile.dat";
            if (fileExists(candidate)) return candidate;
        }
    }

    const auto legacy = base / "save" / user / "BaristaAccountSaveFile.dat";
    return fileExists(legacy) ? legacy : std::filesystem::path{};
}

std::filesystem::path findNativeSave(uint32_t persistentId) {
    const uint64_t menuTitleId =
        _SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_WII_U_MENU);
    if ((menuTitleId >> 32) != 0x00050010ULL) return {};

    return std::filesystem::path("fs:/vol/storage_mlc01/usr/save/00050010") /
           hex8(static_cast<uint32_t>(menuTitleId)) / "user" /
           hex8(persistentId) / "BaristaAccountSaveFile.dat";
}

void appendUnique(std::vector<uint64_t>& output, uint64_t titleId) {
    if (titleId == 0) return;
    if (std::find(output.begin(), output.end(), titleId) == output.end()) {
        output.push_back(titleId);
    }
}

uint64_t readPairedTitle(const std::vector<uint8_t>& data,
                         std::size_t nandOffset,
                         std::size_t usbOffset) {
    const RawMenuEntry nandEntry = readEntry(data, nandOffset);
    if (nandEntry.type == MENU_ITEM_NAND && nandEntry.titleId != 0) {
        return nandEntry.titleId;
    }

    const RawMenuEntry usbEntry = readEntry(data, usbOffset);
    if (usbEntry.type == MENU_ITEM_USB && usbEntry.titleId != 0) {
        return usbEntry.titleId;
    }
    return 0;
}

} // namespace

WiiUMenuLayout loadWiiUMenuLayout() {
    WiiUMenuLayout layout;
    const uint32_t persistentId = nn::act::GetPersistentId();
    if (persistentId == 0) return layout;

    std::filesystem::path source = findAromaRedirect(persistentId);
    if (!source.empty()) {
        layout.loadedFromAromaRedirect = true;
    } else {
        source = findNativeSave(persistentId);
    }
    if (source.empty() || !fileExists(source)) return layout;

    std::vector<uint8_t> data;
    if (!readBinaryFile(source, data) || data.size() < ROOT_USB_OFFSET + ROOT_ENTRY_COUNT * ENTRY_SIZE) {
        return layout;
    }

    std::vector<uint32_t> folderIds;
    for (std::size_t slot = 0; slot < ROOT_ENTRY_COUNT; ++slot) {
        const std::size_t nandOffset = ROOT_NAND_OFFSET + slot * ENTRY_SIZE;
        const std::size_t usbOffset = ROOT_USB_OFFSET + slot * ENTRY_SIZE;
        const RawMenuEntry nandEntry = readEntry(data, nandOffset);

        if (nandEntry.type == MENU_ITEM_FOLDER &&
            nandEntry.lowId >= 1 && nandEntry.lowId <= 60) {
            if (std::find(folderIds.begin(), folderIds.end(), nandEntry.lowId) == folderIds.end()) {
                folderIds.push_back(nandEntry.lowId);
            }
            continue;
        }

        appendUnique(layout.rootTitleIds,
                     readPairedTitle(data, nandOffset, usbOffset));
    }

    for (uint32_t folderId : folderIds) {
        const std::size_t folderOffset =
            FIRST_FOLDER_OFFSET + static_cast<std::size_t>(folderId - 1) * FOLDER_STRIDE;
        const std::size_t infoOffset = folderOffset + FOLDER_BANK_SIZE * 2;
        if (infoOffset + FOLDER_INFO_SIZE > data.size()) continue;

        WiiUMenuCollection collection;
        collection.id = folderId;
        collection.name = decodeFolderName(data, infoOffset);
        collection.color = data[infoOffset + FOLDER_COLOR_OFFSET] & 0x07;
        if (collection.name.empty()) {
            collection.name = "Collection Wii U " + std::to_string(folderId);
        }

        for (std::size_t slot = 0; slot < FOLDER_ENTRY_COUNT; ++slot) {
            const std::size_t nandOffset = folderOffset + slot * ENTRY_SIZE;
            const std::size_t usbOffset = folderOffset + FOLDER_BANK_SIZE + slot * ENTRY_SIZE;
            appendUnique(collection.titleIds,
                         readPairedTitle(data, nandOffset, usbOffset));
        }
        layout.collections.push_back(std::move(collection));
    }

    layout.loaded = true;
    return layout;
}
