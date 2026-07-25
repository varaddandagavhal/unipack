cmake_minimum_required(VERSION 3.20)
project(UniPack VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.4.2
)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.11.0
)

FetchContent_MakeAvailable(cli11 nlohmann_json cpr)

set(SOURCES
  src/main.cpp
  src/cli.cpp
  src/query.cpp
  src/scoring.cpp
  src/executor.cpp
  src/state.cpp
)

set(HEADERS
  src/colors.h
  src/package.h
  src/cli.h
  src/query.h
  src/scoring.h
  src/executor.h
  src/state.h
)

add_executable(unipack ${SOURCES} ${HEADERS})

target_include_directories(unipack PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(unipack
  PRIVATE
    CLI11::CLI11
    nlohmann_json::nlohmann_json
    cpr::cpr
)

target_compile_definitions(unipack PRIVATE UNIPACK_USE_CPR)

target_compile_options(unipack PRIVATE -Wall -Wextra -Wpedantic)
