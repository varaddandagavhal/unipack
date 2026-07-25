#pragma once

#include "package.h"

PackageList queryApt(const std::string& query);
PackageList querySnap(const std::string& query);
PackageList queryFlatpak(const std::string& query);
PackageList searchAll(const std::string& query);

PackageList queryAptExact(const std::string& name);
PackageList querySnapExact(const std::string& name);
PackageList queryFlatpakExact(const std::string& name);
PackageList searchAllExact(const std::string& name);
