CXX ?= g++
PYTHON ?= python3
CPPFLAGS += -I.
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
BUILD_DIR ?= build
WINDOWS_CXX ?= i686-w64-mingw32-g++
WINDOWS_CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror -static -municode

.PHONY: all test engine-window

all: $(BUILD_DIR)/chunk_reader_tests $(BUILD_DIR)/scan_scene_chunks $(BUILD_DIR)/model_resource_key_tests $(BUILD_DIR)/model_resource_key $(BUILD_DIR)/frame_kind_tests $(BUILD_DIR)/model_load_policy_tests $(BUILD_DIR)/model_file_header_tests $(BUILD_DIR)/inspect_model_header $(BUILD_DIR)/model_first_table_tests $(BUILD_DIR)/scan_model_first_table $(BUILD_DIR)/graphics_settings_tests $(BUILD_DIR)/cursor_geometry_tests $(BUILD_DIR)/bitmap_font_tests $(BUILD_DIR)/menu_text_geometry_tests

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

$(BUILD_DIR)/cursor_geometry_tests: tests/cursor_geometry_tests.cpp host/cursor_geometry.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/cursor_geometry_tests.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/bitmap_font_tests: tests/bitmap_font_tests.cpp host/bitmap_font.cpp host/bitmap_font.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) host/bitmap_font.cpp tests/bitmap_font_tests.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/menu_text_geometry_tests: tests/menu_text_geometry_tests.cpp host/menu_text_geometry.h frontend/text_object.h host/bitmap_font.h host/cursor_geometry.h host/frame_name.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/menu_text_geometry_tests.cpp $(LDFLAGS) -o "$@"

$(BUILD_DIR)/engine_window.exe: host/engine_window.cpp host/graphics_settings.h host/menu_scene.h host/menu_group_probe.h host/menu_cursor.h host/cursor_geometry.h host/menu_text.h host/menu_text_geometry.h host/menu_button_visuals.h frontend/button_focus.h frontend/button_input.h frontend/control_events.h frontend/control_state.h frontend/control_runtime.h frontend/text_control_event.h frontend/text_object.h frontend/text_control_runtime.h host/menu_text_object.h frontend/control_initialization.h frontend/control_factory.h frontend/control_definition_ids.h frontend/container_event.h frontend/defined_container_events.h host/menu_control.h frontend/control_events.cpp frontend/menu_event_queue.h frontend/screen_stack.h frontend/screen_registry.h frontend/screen_lifecycle.h frontend/screen_event.h frontend/defined_screen_events.h host/menu_screen_events.h frontend/menu_event_queue.cpp frontend/menu_click_sound.h frontend/pointer_buttons.h frontend/pointer_click.h frontend/pointer_click_timing.h frontend/pointer_update.h frontend/pointer_cursor.h frontend/cursor_frame.h frontend/pointer_dispatch.h frontend/pointer_wheel.h host/pointer_dispatch_probe.h host/engine_mouse.h host/engine_sound.h frontend/sound_startup.h frontend/menu_definition.h frontend/defined_screen.h frontend/defined_controls.h input/mouse_poll.h frontend/pointer_motion.h frontend/pointer_pick.h frontend/screen_focus.h frontend/screen_input.h frontend/screen_key.h frontend/frontend_key.h frontend/screen_target.h host/bitmap_font.h host/bitmap_font.cpp host/frame_name.h | $(BUILD_DIR)
	"$(WINDOWS_CXX)" $(CPPFLAGS) $(WINDOWS_CXXFLAGS) host/engine_window.cpp host/bitmap_font.cpp frontend/control_events.cpp frontend/menu_event_queue.cpp -o "$@"

$(BUILD_DIR)/engine_window.exe: frontend/container_input.h frontend/defined_control_tree.h host/menu_control_tree.h
$(BUILD_DIR)/engine_window.exe: host/menu_screen_product.h frontend/runtime_confirmation.h
$(BUILD_DIR)/engine_window.exe: host/menu_screen_factory.h frontend/screen_construction.h frontend/owned_screens.h
$(BUILD_DIR)/engine_window.exe: frontend/application_events.h

test: all
	"$(BUILD_DIR)/chunk_reader_tests"
	"$(BUILD_DIR)/model_resource_key_tests"
	"$(BUILD_DIR)/frame_kind_tests"
	"$(BUILD_DIR)/model_load_policy_tests"
	"$(BUILD_DIR)/model_file_header_tests"
	"$(BUILD_DIR)/model_first_table_tests"
	"$(BUILD_DIR)/graphics_settings_tests"
	"$(BUILD_DIR)/cursor_geometry_tests"
	"$(BUILD_DIR)/bitmap_font_tests"
	"$(BUILD_DIR)/menu_text_geometry_tests"
	$(PYTHON) -B tests/compare_scene_scan.py --scanner "$(BUILD_DIR)/scan_scene_chunks"
	$(PYTHON) -B tests/compare_model_resource_keys.py --normalizer "$(BUILD_DIR)/model_resource_key"
	$(PYTHON) -B tests/compare_model_headers.py --reader "$(BUILD_DIR)/inspect_model_header"
	$(PYTHON) -B tests/compare_model_tables.py --scanner "$(BUILD_DIR)/scan_model_first_table"
