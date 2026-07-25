#pragma once

#include "package.h"
#include <string>
#include <unordered_map>

class StateManager {
public:
    StateManager();

    void trackInstall(const std::string& appName, PackageFormat format);
    void trackRemove(const std::string& appName);
    std::unordered_map<std::string, PackageFormat> getAllTracked() const;
    std::string getStatePath() const;

private:
    std::string stateFile_;
    std::unordered_map<std::string, PackageFormat> installed_;
    void load();
    void save() const;
};
