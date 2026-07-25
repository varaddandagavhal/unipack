#include "query.h"
#include "colors.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <array>
#include <memory>
#include <sstream>
#include <algorithm>
#include <cctype>

#ifdef UNIPACK_USE_CPR
#include <cpr/cpr.h>
#else
#include <cstdlib>
#endif

using json = nlohmann::json;

static std::string trim(const std::string& s) {
    auto wsfront = std::find_if_not(s.begin(), s.end(), [](unsigned char c){ return std::isspace(c); });
    auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c){ return std::isspace(c); }).base();
    return (wsback <= wsfront) ? std::string() : std::string(wsfront, wsback);
}

static std::string execAndCapture(const std::string& cmd) {
    std::array<char, 256> buffer{};
    std::string result;
    auto pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

#ifndef UNIPACK_USE_CPR
static std::string httpGet(const std::string& url) {
    std::string cmd = "curl -s \"" + url + "\"";
    return execAndCapture(cmd);
}
#endif

PackageList queryApt(const std::string& query) {
    PackageList results;
    std::string cmd = "apt-cache search " + query + " 2>/dev/null";
    std::string output = execAndCapture(cmd);
    std::istringstream stream(output);
    std::string line;
    while (std::getline(stream, line)) {
        auto pos = line.find(" - ");
        if (pos != std::string::npos) {
            Package pkg;
            pkg.name = trim(line.substr(0, pos));
            pkg.description = trim(line.substr(pos + 3));
            pkg.format = PackageFormat::APT;
            pkg.source = "Ubuntu APT";

            std::string verCmd = "apt-cache policy " + pkg.name + " 2>/dev/null | grep 'Installed:' | head -1";
            std::string verOut = execAndCapture(verCmd);
            auto colon = verOut.find(':');
            if (colon != std::string::npos) {
                pkg.version = trim(verOut.substr(colon + 1));
            } else {
                std::string verCmd2 = "apt-cache policy " + pkg.name + " 2>/dev/null | grep 'Candidate:' | head -1";
                std::string verOut2 = execAndCapture(verCmd2);
                auto colon2 = verOut2.find(':');
                if (colon2 != std::string::npos) {
                    pkg.version = trim(verOut2.substr(colon2 + 1));
                } else {
                    pkg.version = "unknown";
                }
            }

            results.push_back(std::move(pkg));
        }
    }
    return results;
}

PackageList querySnap(const std::string& query) {
    PackageList results;
#ifdef UNIPACK_USE_CPR
    try {
        cpr::Response r = cpr::Get(
            cpr::Url{"https://api.snapcraft.io/v2/snaps/search"},
            cpr::Parameters{{"q", query}},
            cpr::Header{{"Snap-Device-Series", "16"}, {"User-Agent", "UniPack/1.0"}}
        );
        if (r.status_code != 200) return results;
        json data = json::parse(r.text);
        for (const auto& item : data["results"]) {
            Package pkg;
            pkg.name = item["name"];
            pkg.version = item["version"];
            pkg.description = item["summary"];
            pkg.source = "Snapcraft";
            pkg.format = PackageFormat::SNAP;
            results.push_back(std::move(pkg));
        }
    } catch (const std::exception& e) {
        std::cerr << color::dim << "  [snap] API error: " << e.what() << color::reset << std::endl;
    }
#else
    std::string url = "https://api.snapcraft.io/v2/snaps/search?q=" + query;
    std::string response = httpGet(url);
    if (response.empty()) return results;
    try {
        json data = json::parse(response);
        for (const auto& item : data["results"]) {
            Package pkg;
            pkg.name = item["name"];
            pkg.version = item["version"];
            pkg.description = item["summary"];
            pkg.source = "Snapcraft";
            pkg.format = PackageFormat::SNAP;
            results.push_back(std::move(pkg));
        }
    } catch (...) {}
#endif
    return results;
}

PackageList queryFlatpak(const std::string& query) {
    PackageList results;
#ifdef UNIPACK_USE_CPR
    try {
        cpr::Response r = cpr::Get(
            cpr::Url{"https://flathub.org/api/v2/appstream"},
            cpr::Parameters{{"search", query}}
        );
        if (r.status_code != 200) return results;
        json data = json::parse(r.text);
        for (const auto& item : data) {
            Package pkg;
            pkg.name = item["id"];
            std::string id = item["id"];
            auto dot = id.rfind('.');
            if (dot != std::string::npos) pkg.name = id.substr(dot + 1);
            pkg.description = item.value("summary", "");
            pkg.source = "Flathub";
            pkg.format = PackageFormat::FLATPAK;
            auto versions = item["releases"];
            pkg.version = (!versions.empty()) ? versions[0].value("version", "stable") : "stable";
            results.push_back(std::move(pkg));
        }
    } catch (const std::exception& e) {
        std::cerr << color::dim << "  [flatpak] API error: " << e.what() << color::reset << std::endl;
    }
#else
    std::string url = "https://flathub.org/api/v2/appstream?search=" + query;
    std::string response = httpGet(url);
    if (response.empty()) return results;
    try {
        json data = json::parse(response);
        for (const auto& item : data) {
            Package pkg;
            pkg.name = item["id"];
            std::string id = item["id"];
            auto dot = id.rfind('.');
            if (dot != std::string::npos) pkg.name = id.substr(dot + 1);
            pkg.description = item.value("summary", "");
            pkg.source = "Flathub";
            pkg.format = PackageFormat::FLATPAK;
            auto versions = item["releases"];
            pkg.version = (!versions.empty()) ? versions[0].value("version", "stable") : "stable";
            results.push_back(std::move(pkg));
        }
    } catch (...) {}
#endif
    return results;
}

PackageList queryAptExact(const std::string& name) {
    PackageList results;
    std::string cmd = "dpkg-query -W -f='${Package} ${Version}\\n' " + name + " 2>/dev/null";
    std::string output = execAndCapture(cmd);
    if (!output.empty()) {
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line)) {
            auto space = line.find(' ');
            if (space != std::string::npos) {
                Package pkg;
                pkg.name = trim(line.substr(0, space));
                pkg.version = trim(line.substr(space + 1));
                pkg.format = PackageFormat::APT;
                pkg.source = "Ubuntu APT";
                results.push_back(std::move(pkg));
            }
        }
    }
    if (results.empty()) results = queryApt(name);
    return results;
}

PackageList querySnapExact(const std::string& name) {
    PackageList results;
#ifdef UNIPACK_USE_CPR
    try {
        cpr::Response r = cpr::Get(
            cpr::Url{"https://api.snapcraft.io/v2/snaps/info/" + name},
            cpr::Header{{"Snap-Device-Series", "16"}, {"User-Agent", "UniPack/1.0"}}
        );
        if (r.status_code != 200) return results;
        json data = json::parse(r.text);
        const auto& snap = data["snap"];
        Package pkg;
        pkg.name = snap["name"];
        pkg.description = snap["summary"];
        pkg.source = "Snapcraft";
        pkg.format = PackageFormat::SNAP;
        auto channels = data["channel-map"];
        pkg.version = (!channels.empty() && channels[0].contains("version")) ? channels[0]["version"] : "unknown";
        results.push_back(std::move(pkg));
    } catch (const std::exception& e) {
        std::cerr << color::dim << "  [snap] API error: " << e.what() << color::reset << std::endl;
    }
#else
    std::string url = "https://api.snapcraft.io/v2/snaps/info/" + name;
    std::string response = httpGet(url);
    if (response.empty()) return results;
    try {
        json data = json::parse(response);
        const auto& snap = data["snap"];
        Package pkg;
        pkg.name = snap["name"];
        pkg.description = snap["summary"];
        pkg.source = "Snapcraft";
        pkg.format = PackageFormat::SNAP;
        auto channels = data["channel-map"];
        pkg.version = (!channels.empty() && channels[0].contains("version")) ? channels[0]["version"] : "unknown";
        results.push_back(std::move(pkg));
    } catch (...) {}
#endif
    return results;
}

PackageList queryFlatpakExact(const std::string& name) {
    PackageList results;
#ifdef UNIPACK_USE_CPR
    try {
        cpr::Response r = cpr::Get(cpr::Url{"https://flathub.org/api/v2/appstream/" + name});
        if (r.status_code == 200) {
            json item = json::parse(r.text);
            Package pkg;
            pkg.name = name;
            pkg.description = item.value("summary", "");
            pkg.source = "Flathub";
            pkg.format = PackageFormat::FLATPAK;
            auto versions = item["releases"];
            pkg.version = (!versions.empty()) ? versions[0].value("version", "stable") : "stable";
            results.push_back(std::move(pkg));
            return results;
        }
        results = queryFlatpak(name);
    } catch (const std::exception& e) {
        std::cerr << color::dim << "  [flatpak] API error: " << e.what() << color::reset << std::endl;
    }
#else
    std::string url = "https://flathub.org/api/v2/appstream/" + name;
    std::string response = httpGet(url);
    if (!response.empty()) {
        try {
            json item = json::parse(response);
            Package pkg;
            pkg.name = name;
            pkg.description = item.value("summary", "");
            pkg.source = "Flathub";
            pkg.format = PackageFormat::FLATPAK;
            auto versions = item["releases"];
            pkg.version = (!versions.empty()) ? versions[0].value("version", "stable") : "stable";
            results.push_back(std::move(pkg));
            return results;
        } catch (...) {}
    }
    results = queryFlatpak(name);
#endif
    return results;
}

PackageList searchAll(const std::string& query) {
    PackageList results;
    auto aptResults = queryApt(query);
    results.insert(results.end(), aptResults.begin(), aptResults.end());
    auto snapResults = querySnap(query);
    results.insert(results.end(), snapResults.begin(), snapResults.end());
    auto flatpakResults = queryFlatpak(query);
    results.insert(results.end(), flatpakResults.begin(), flatpakResults.end());
    return results;
}

PackageList searchAllExact(const std::string& name) {
    PackageList results;
    auto aptResults = queryAptExact(name);
    results.insert(results.end(), aptResults.begin(), aptResults.end());
    auto snapResults = querySnapExact(name);
    results.insert(results.end(), snapResults.begin(), snapResults.end());
    auto flatpakResults = queryFlatpakExact(name);
    results.insert(results.end(), flatpakResults.begin(), flatpakResults.end());
    return results;
}
