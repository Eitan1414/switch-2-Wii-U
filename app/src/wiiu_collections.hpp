#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct WiiUMenuCollection {
    uint32_t id = 0;
    std::string name;
    uint8_t color = 0;
    std::vector<uint64_t> titleIds;
};

struct WiiUMenuLayout {
    bool loaded = false;
    bool loadedFromAromaRedirect = false;
    std::vector<WiiUMenuCollection> collections;
    std::vector<uint64_t> rootTitleIds;
};

// Lit uniquement l'organisation du profil actif. Aucun octet du fichier
// BaristaAccountSaveFile.dat n'est modifie.
WiiUMenuLayout loadWiiUMenuLayout();
