CXX ?= g++
PYTHON ?= python3
CPPFLAGS += -I.
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
BUILD_DIR ?= build
WINDOWS_CXX ?= i686-w64-mingw32-g++
WINDOWS_CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror -static -municode

.PHONY: all test engine-window

all: $(BUILD_DIR)/chunk_reader_tests $(BUILD_DIR)/scan_scene_chunks $(BUILD_DIR)/model_resource_key_tests $(BUILD_DIR)/model_resource_key $(BUILD_DIR)/frame_kind_tests $(BUILD_DIR)/model_load_policy_tests $(BUILD_DIR)/model_file_header_tests $(BUILD_DIR)/inspect_model_header $(BUILD_DIR)/model_first_table_tests $(BUILD_DIR)/scan_model_first_table $(BUILD_DIR)/graphics_settings_tests

$(BUILD_DIR):
	mkdir -p "$@"

$(BUILD_DIR)/chunk_reader_tests: io/chunk_reader.cpp tests/chunk_reader_tests.cpp io/chunk_reader.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) io/chunk_reader.cpp tests/chunk_reader_tests.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/scan_scene_chunks: io/chunk_reader.cpp tools/scan_scene_chunks.cpp io/chunk_reader.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) io/chunk_reader.cpp tools/scan_scene_chunks.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/model_resource_key_tests: scene/model_resource_key.cpp tests/model_resource_key_tests.cpp scene/model_resource_key.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) scene/model_resource_key.cpp tests/model_resource_key_tests.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/model_resource_key: scene/model_resource_key.cpp tools/model_resource_key.cpp scene/model_resource_key.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) scene/model_resource_key.cpp tools/model_resource_key.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/frame_kind_tests: scene/frame_kind.cpp tests/frame_kind_tests.cpp scene/frame_kind.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) scene/frame_kind.cpp tests/frame_kind_tests.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/model_load_policy_tests: scene/model_load_policy.cpp tests/model_load_policy_tests.cpp scene/model_load_policy.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) scene/model_load_policy.cpp tests/model_load_policy_tests.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/model_file_header_tests: scene/model_file_header.cpp tests/model_file_header_tests.cpp scene/model_file_header.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) scene/model_file_header.cpp tests/model_file_header_tests.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/inspect_model_header: scene/model_file_header.cpp tools/inspect_model_header.cpp scene/model_file_header.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) scene/model_file_header.cpp tools/inspect_model_header.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/model_first_table_tests: scene/model_first_table.cpp tests/model_first_table_tests.cpp scene/model_first_table.h scene/model_file_header.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) scene/model_first_table.cpp tests/model_first_table_tests.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/scan_model_first_table: scene/model_file_header.cpp scene/model_first_table.cpp tools/scan_model_first_table.cpp scene/model_file_header.h scene/model_first_table.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) scene/model_file_header.cpp scene/model_first_table.cpp tools/scan_model_first_table.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/graphics_settings_tests: tests/graphics_settings_tests.cpp host/graphics_settings.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/graphics_settings_tests.cpp $(LDFLAGS) -o "$@"

engine-window: $(BUILD_DIR)/engine_window.exe

$(BUILD_DIR)/engine_window.exe: host/engine_window.cpp host/graphics_settings.h | $(BUILD_DIR)
	"$(WINDOWS_CXX)" $(CPPFLAGS) $(WINDOWS_CXXFLAGS) host/engine_window.cpp -o "$@"

test: all
	"$(BUILD_DIR)/chunk_reader_tests"
	"$(BUILD_DIR)/model_resource_key_tests"
	"$(BUILD_DIR)/frame_kind_tests"
	"$(BUILD_DIR)/model_load_policy_tests"
	"$(BUILD_DIR)/model_file_header_tests"
	"$(BUILD_DIR)/model_first_table_tests"
	"$(BUILD_DIR)/graphics_settings_tests"
	$(PYTHON) -B tests/compare_scene_scan.py --scanner "$(BUILD_DIR)/scan_scene_chunks"
	$(PYTHON) -B tests/compare_model_resource_keys.py --normalizer "$(BUILD_DIR)/model_resource_key"
	$(PYTHON) -B tests/compare_model_headers.py --reader "$(BUILD_DIR)/inspect_model_header"
	$(PYTHON) -B tests/compare_model_tables.py --scanner "$(BUILD_DIR)/scan_model_first_table"
