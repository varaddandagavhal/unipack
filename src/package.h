#pragma once

#include <string>
#include <vector>
#include <cstdint>

enum class PackageFormat : uint8_t {
    APT,
    SNAP,
    FLATPAK
};

inline const char* formatToString(PackageFormat f) {
    switch (f) {
        case PackageFormat::APT:     return "apt";
        case PackageFormat::SNAP:    return "snap";
        case PackageFormat::FLATPAK: return "flatpak";
    }
    return "unknown";
}

struct Package {
    std::string name;
    std::string version;
    std::string description;
    std::string source;
    PackageFormat format;
    int score = 0;
};

using PackageList = std::vector<Package>;
