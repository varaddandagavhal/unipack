#pragma once

#include <string>
#include <iostream>

namespace color {

inline const char* reset   = "\033[0m";
inline const char* bold    = "\033[1m";
inline const char* dim     = "\033[2m";

inline const char* red     = "\033[31m";
inline const char* green   = "\033[32m";
inline const char* yellow  = "\033[33m";
inline const char* blue    = "\033[34m";
inline const char* magenta = "\033[35m";
inline const char* cyan    = "\033[36m";
inline const char* white   = "\033[37m";

inline void disable() {
    auto noop = [](const char*) {};
    (void)noop;
}

inline std::string styled(const char* c, const std::string& text) {
    return c + text + reset;
}

} // namespace color
