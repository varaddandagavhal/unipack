#include "executor.h"
#include "colors.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <cstdlib>

static int runCommand(const std::vector<std::string>& args) {
    if (args.empty()) return -1;
    std::string cmd;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) cmd += " ";
        if (args[i].find(' ') != std::string::npos) {
            cmd += "\"" + args[i] + "\"";
        } else {
            cmd += args[i];
        }
    }
    return system(cmd.c_str());
}

#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

static int runCommand(const std::vector<std::string>& args) {
    if (args.empty()) return -1;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        std::vector<char*> argv;
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        _exit(127);
    }

    close(pipefd[1]);

    char buffer[4096];
    ssize_t count;
    while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[count] = '\0';
        std::cout << buffer << std::flush;
    }
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
#endif

bool executeInstall(const Package& pkg) {
    std::cout << color::bold << color::green << "[installing] " << pkg.name
              << " via " << formatToString(pkg.format) << color::reset << std::endl;

    switch (pkg.format) {
        case PackageFormat::APT:
            return runCommand({"sudo", "apt", "install", "-y", pkg.name}) == 0;
        case PackageFormat::SNAP:
            return runCommand({"sudo", "snap", "install", pkg.name}) == 0;
        case PackageFormat::FLATPAK:
            return runCommand({"flatpak", "install", "-y", "flathub", pkg.name}) == 0;
    }
    return false;
}

bool executeRemove(const std::string& name, PackageFormat format) {
    std::cout << color::bold << color::red << "[removing] " << name
              << " via " << formatToString(format) << color::reset << std::endl;

    switch (format) {
        case PackageFormat::APT:
            return runCommand({"sudo", "apt", "remove", "-y", name}) == 0;
        case PackageFormat::SNAP:
            return runCommand({"sudo", "snap", "remove", name}) == 0;
        case PackageFormat::FLATPAK:
            return runCommand({"flatpak", "uninstall", "-y", name}) == 0;
    }
    return false;
}

bool executeAptUpdate() {
    std::cout << color::bold << color::blue << "[apt] Checking for updates..." << color::reset << std::endl;
    return runCommand({"sudo", "apt", "update"}) == 0;
}

bool executeSnapUpdate() {
    std::cout << color::bold << color::blue << "[snap] Checking for updates..." << color::reset << std::endl;
    return runCommand({"sudo", "snap", "refresh"}) == 0;
}

bool executeFlatpakUpdate() {
    std::cout << color::bold << color::blue << "[flatpak] Checking for updates..." << color::reset << std::endl;
    return runCommand({"flatpak", "update", "-y"}) == 0;
}
