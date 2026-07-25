#pragma once

#include "package.h"
#include <string>

bool executeInstall(const Package& pkg);
bool executeRemove(const std::string& name, PackageFormat format);
bool executeAptUpdate();
bool executeSnapUpdate();
bool executeFlatpakUpdate();
