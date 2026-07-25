# UniPack Project Context

## Build Commands
- **CMake**: `cmake -B build && cmake --build build`
- **Makefile**: `make` (uses curl CLI instead of cpr)
- **Clean**: `make clean` or `rm -rf build`

## Dependencies
- CLI11 (header-only, via FetchContent or direct download)
- nlohmann/json (header-only, via FetchContent or direct download)
- cpr (optional, used when `UNIPACK_USE_CPR` is defined — CMake build only)
- libcurl (only for CMake/cpr build)
- curl CLI (for Makefile build fallback)

## Code Style
- C++17, no exceptions in hot paths
- snake_case for functions/variables, PascalCase for classes
- No comments in production code
- ANSI color constants in colors.h
- POSIX executor uses fork/pipe/execvp/waitpid (Linux) or system() (Windows fallback)

## Key Architecture
- `Package` struct: name, version, description, source, format, score
- `PackageFormat` enum: APT, SNAP, FLATPAK
- Query layer: apt-cache (local), Snapcraft API (REST), Flathub API (REST)
- Scoring: heuristic with +50 flatpak, +40 apt, +10 snap, -5 snap penalty
- Executor: POSIX fork/pipe/execvp on Linux, system() on Windows
- State: JSON file at ~/.config/unipack/installed.json

## Lint
The project compiles cleanly with `-Wall -Wextra -Wpedantic`.

## Test
No test framework configured. Manual testing on Linux recommended.
