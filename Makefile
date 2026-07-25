CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -I src
LDFLAGS  := -static

DEPS_DIR  := .deps
CLI11_HPP := $(DEPS_DIR)/CLI11.hpp
JSON_HPP  := $(DEPS_DIR)/json.hpp

SRCS := src/main.cpp src/cli.cpp src/query.cpp src/scoring.cpp src/executor.cpp src/state.cpp
OBJS := $(SRCS:.cpp=.o)
TARGET := unipack

.PHONY: all clean deps

all: deps $(TARGET)

$(DEPS_DIR):
	mkdir -p $(DEPS_DIR)

$(CLI11_HPP): | $(DEPS_DIR)
	curl -sL https://github.com/CLIUtils/CLI11/releases/download/v2.4.2/CLI11.hpp -o $@

$(JSON_HPP): | $(DEPS_DIR)
	curl -sL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp -o $@

deps: $(CLI11_HPP) $(JSON_HPP)

$(TARGET): $(SRCS) deps
	$(CXX) $(CXXFLAGS) -I $(DEPS_DIR) $(SRCS) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(DEPS_DIR)
