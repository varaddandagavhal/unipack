#include "cli.h"
#include "colors.h"
#include "package.h"
#include "query.h"
#include "scoring.h"
#include "executor.h"
#include "state.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

static void printTableHeader() {
    std::cout << color::bold << color::white
              << std::left << std::setw(24) << "Name"
              << std::setw(14) << "Format"
              << std::setw(14) << "Version"
              << std::setw(18) << "Source"
              << std::setw(8) << "Score"
              << "Description"
              << color::reset << std::endl;
    std::cout << std::string(120, '-') << std::endl;
}

static void printPackage(const Package& pkg, bool showScore = true) {
    const char* fmtColor = color::white;
    switch (pkg.format) {
        case PackageFormat::APT:     fmtColor = color::cyan;    break;
        case PackageFormat::SNAP:    fmtColor = color::yellow;  break;
        case PackageFormat::FLATPAK: fmtColor = color::magenta; break;
    }

    std::string desc = pkg.description;
    if (desc.length() > 50) desc = desc.substr(0, 47) + "...";

    std::cout << std::left << std::setw(24) << color::bold + pkg.name + color::reset
              << fmtColor << std::setw(14) << formatToString(pkg.format) << color::reset
              << std::setw(14) << pkg.version
              << std::setw(18) << pkg.source;

    if (showScore) {
        if (pkg.score >= 50)
            std::cout << color::green;
        else if (pkg.score >= 30)
            std::cout << color::yellow;
        else
            std::cout << color::red;
        std::cout << std::setw(8) << pkg.score << color::reset;
    } else {
        std::cout << std::setw(8) << "";
    }

    std::cout << desc << std::endl;
}

static int cmdSearch(const std::string& query) {
    std::cout << color::bold << color::blue
              << "Searching for '" << query << "' across all backends..."
              << color::reset << std::endl << std::endl;

    auto results = searchAll(query);
    if (results.empty()) {
        std::cout << color::yellow << "No results found." << color::reset << std::endl;
        return 0;
    }

    scorePackages(results);
    std::sort(results.begin(), results.end(),
        [](const Package& a, const Package& b) { return a.score > b.score; });

    printTableHeader();
    for (const auto& pkg : results) {
        printPackage(pkg);
    }

    std::cout << std::endl
              << color::dim << "Found " << results.size() << " result(s)"
              << color::reset << std::endl;

    return 0;
}

static int cmdInstall(const std::string& name, const std::optional<std::string>& prefer) {
    std::cout << color::bold << color::blue
              << "Finding best format for '" << name << "'..."
              << color::reset << std::endl;

    auto results = searchAllExact(name);
    if (results.empty()) {
        std::cout << color::red << "Error: '" << name << "' not found in any repository."
                  << color::reset << std::endl;
        return 1;
    }

    scorePackages(results);
    auto best = selectBest(results, prefer);

    printTableHeader();
    for (const auto& pkg : results) {
        printPackage(pkg);
    }

    std::cout << std::endl;
    std::cout << color::bold << color::green
              << "Selected: " << best.name
              << " via " << formatToString(best.format)
              << " (score: " << best.score << ")"
              << color::reset << std::endl << std::endl;

    bool success = executeInstall(best);
    if (success) {
        StateManager state;
        state.trackInstall(best.name, best.format);
        std::cout << color::bold << color::green
                  << "Successfully installed " << best.name << color::reset << std::endl;
    } else {
        std::cout << color::bold << color::red
                  << "Failed to install " << best.name << color::reset << std::endl;
        return 1;
    }
    return 0;
}

static int cmdRemove(const std::string& name) {
    StateManager state;
    auto tracked = state.getAllTracked();

    auto it = tracked.find(name);
    if (it == tracked.end()) {
        std::cout << color::red << "Error: '" << name
                  << "' is not tracked by UniPack." << color::reset << std::endl;
        std::cout << color::dim
                  << "Try specifying the format manually (not yet supported)."
                  << color::reset << std::endl;
        return 1;
    }

    bool success = executeRemove(name, it->second);
    if (success) {
        state.trackRemove(name);
        std::cout << color::bold << color::green
                  << "Successfully removed " << name << color::reset << std::endl;
    } else {
        std::cout << color::bold << color::red
                  << "Failed to remove " << name << color::reset << std::endl;
        return 1;
    }
    return 0;
}

static int cmdUpdate() {
    StateManager state;
    auto tracked = state.getAllTracked();

    if (tracked.empty()) {
        std::cout << color::yellow
                  << "No packages are tracked by UniPack." << color::reset << std::endl;
        std::cout << color::dim
                  << "Running updates for all backends anyway..." << color::reset << std::endl;
    }

    bool hasApt = false, hasSnap = false, hasFlatpak = false;
    for (const auto& [_, fmt] : tracked) {
        switch (fmt) {
            case PackageFormat::APT:     hasApt = true;     break;
            case PackageFormat::SNAP:    hasSnap = true;    break;
            case PackageFormat::FLATPAK: hasFlatpak = true; break;
        }
    }

    bool allOk = true;

    if (hasApt || tracked.empty()) {
        allOk = executeAptUpdate() && allOk;
    }
    if (hasSnap || tracked.empty()) {
        allOk = executeSnapUpdate() && allOk;
    }
    if (hasFlatpak || tracked.empty()) {
        allOk = executeFlatpakUpdate() && allOk;
    }

    if (allOk) {
        std::cout << color::bold << color::green
                  << "All updates completed." << color::reset << std::endl;
    } else {
        std::cout << color::bold << color::red
                  << "Some updates failed." << color::reset << std::endl;
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    auto opts = parseCli(argc, argv);

    if (opts.command == "search") {
        return cmdSearch(opts.query);
    } else if (opts.command == "install") {
        return cmdInstall(opts.query, opts.preferFormat);
    } else if (opts.command == "remove") {
        return cmdRemove(opts.query);
    } else if (opts.command == "update") {
        return cmdUpdate();
    }

    return 0;
}
