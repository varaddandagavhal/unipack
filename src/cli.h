#pragma once

#include <string>
#include <optional>

struct CliOptions {
    std::string command;
    std::string query;
    std::optional<std::string> preferFormat;
    bool verbose = false;
};

CliOptions parseCli(int argc, char** argv);
