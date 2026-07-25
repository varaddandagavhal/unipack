#include "scoring.h"
#include <algorithm>

void scorePackage(Package& pkg) {
    int score = 0;

    switch (pkg.format) {
        case PackageFormat::FLATPAK:
            score += 50;
            score += 10;
            break;
        case PackageFormat::APT:
            score += 40;
            score += 15;
            break;
        case PackageFormat::SNAP:
            score += 10;
            score -= 5;
            break;
    }

    if (pkg.version != "unknown" && !pkg.version.empty()) {
        score += 5;
    }

    if (!pkg.description.empty()) {
        score += 3;
    }

    pkg.score = score;
}

void scorePackages(PackageList& pkgs) {
    for (auto& pkg : pkgs) {
        scorePackage(pkg);
    }
}

Package selectBest(PackageList& pkgs, const std::optional<std::string>& preferFormat) {
    if (pkgs.empty()) {
        return Package{};
    }

    if (preferFormat) {
        PackageFormat preferred;
        if (*preferFormat == "apt")         preferred = PackageFormat::APT;
        else if (*preferFormat == "snap")   preferred = PackageFormat::SNAP;
        else if (*preferFormat == "flatpak") preferred = PackageFormat::FLATPAK;
        else return pkgs.front();

        for (auto& pkg : pkgs) {
            if (pkg.format == preferred) {
                pkg.score += 100;
            }
        }
    }

    auto best = std::max_element(pkgs.begin(), pkgs.end(),
        [](const Package& a, const Package& b) {
            return a.score < b.score;
        });

    return *best;
}
