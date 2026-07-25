#include "state.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <iostream>

using json = nlohmann::json;
namespace fs = std::filesystem;

StateManager::StateManager() {
    const char* home = std::getenv("HOME");
    if (!home) home = std::getenv("USERPROFILE");
    if (!home) home = ".";
    stateFile_ = std::string(home) + "/.config/unipack/installed.json";
    load();
}

void StateManager::load() {
    installed_.clear();
    std::ifstream f(stateFile_);
    if (!f.is_open()) return;

    try {
        json data = json::parse(f);
        for (auto& [key, val] : data.items()) {
            std::string fmt = val;
            if (fmt == "apt")         installed_[key] = PackageFormat::APT;
            else if (fmt == "snap")   installed_[key] = PackageFormat::SNAP;
            else if (fmt == "flatpak") installed_[key] = PackageFormat::FLATPAK;
        }
    } catch (...) {}
}

void StateManager::save() const {
    try {
        fs::create_directories(fs::path(stateFile_).parent_path());
    } catch (...) {}

    json data = json::object();
    for (const auto& [name, fmt] : installed_) {
        data[name] = formatToString(fmt);
    }

    std::ofstream f(stateFile_);
    if (f.is_open()) {
        f << data.dump(2);
    }
}

void StateManager::trackInstall(const std::string& appName, PackageFormat format) {
    installed_[appName] = format;
    save();
}

void StateManager::trackRemove(const std::string& appName) {
    installed_.erase(appName);
    save();
}

std::unordered_map<std::string, PackageFormat> StateManager::getAllTracked() const {
    return installed_;
}

std::string StateManager::getStatePath() const {
    return stateFile_;
}
