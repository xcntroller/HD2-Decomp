CXX ?= c++
WINDOWS_CXX ?= i686-w64-mingw32-g++
CPPFLAGS := -I..
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
BUILD_DIR := ../build/networking
SOURCES := command_tokens.cpp launcher_config.cpp server_commands.cpp

.PHONY: all windows clean
all: $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
windows: $(SOURCES:%.cpp=$(BUILD_DIR)/windows-%.o)

$(BUILD_DIR):
	mkdir -p "$@"
$(BUILD_DIR)/%.o: %.cpp $(wildcard *.h) | $(BUILD_DIR)
	"$(CXX)" $(CPPFLAGS) $(CXXFLAGS) -c "$<" -o "$@"
$(BUILD_DIR)/windows-%.o: %.cpp $(wildcard *.h) | $(BUILD_DIR)
	"$(WINDOWS_CXX)" $(CPPFLAGS) $(CXXFLAGS) -c "$<" -o "$@"
clean:
	rm -f $(SOURCES:%.cpp=$(BUILD_DIR)/%.o) $(SOURCES:%.cpp=$(BUILD_DIR)/windows-%.o)
