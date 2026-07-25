#include "cli.h"
#include "colors.h"
#include <CLI/CLI11.hpp>
#include <iostream>
#include <cstdlib>

CliOptions parseCli(int argc, char** argv) {
    CLI::App app{"UniPack - Unified Linux Package Manager"};

    app.require_subcommand(0, 1);
    app.set_version_flag("-v,--version", "UniPack version 1.0.0");

    CliOptions opts;

    auto* search = app.add_subcommand("search", "Search for packages across all backends");
    search->add_option("query", opts.query, "Search term")->required();

    auto* install = app.add_subcommand("install", "Install the best-format package");
    install->add_option("app", opts.query, "Application name")->required();
    install->add_option("--prefer", opts.preferFormat, "Preferred format (apt, snap, flatpak)");

    auto* remove = app.add_subcommand("remove", "Remove an installed package");
    remove->add_option("app", opts.query, "Application name")->required();

    auto* update = app.add_subcommand("update", "Update all installed packages");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        std::exit(app.exit(e));
    }

    if (search->parsed())         opts.command = "search";
    else if (install->parsed())   opts.command = "install";
    else if (remove->parsed())    opts.command = "remove";
    else if (update->parsed())    opts.command = "update";

    if (opts.command.empty()) {
        std::cerr << "Error: a subcommand is required." << std::endl;
        std::exit(1);
    }

    return opts;
}
