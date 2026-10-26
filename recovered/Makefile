CXX ?= g++
PYTHON ?= python3
CPPFLAGS += -I.
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
BUILD_DIR ?= build

.PHONY: all test

all: $(BUILD_DIR)/chunk_reader_tests $(BUILD_DIR)/scan_scene_chunks

$(BUILD_DIR):
	mkdir -p "$@"

$(BUILD_DIR)/chunk_reader_tests: io/chunk_reader.cpp tests/chunk_reader_tests.cpp io/chunk_reader.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) io/chunk_reader.cpp tests/chunk_reader_tests.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/scan_scene_chunks: io/chunk_reader.cpp tools/scan_scene_chunks.cpp io/chunk_reader.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) io/chunk_reader.cpp tools/scan_scene_chunks.cpp $(LDFLAGS) -o "$@"

test: all
	"$(BUILD_DIR)/chunk_reader_tests"
	$(PYTHON) -B tests/compare_scene_scan.py --scanner "$(BUILD_DIR)/scan_scene_chunks"
