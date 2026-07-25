#pragma once

#include "package.h"
#include <string>
#include <optional>

void scorePackage(Package& pkg);
void scorePackages(PackageList& pkgs);
Package selectBest(PackageList& pkgs, const std::optional<std::string>& preferFormat = std::nullopt);
