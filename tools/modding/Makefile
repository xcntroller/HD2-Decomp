CXX ?= c++
CXXFLAGS ?= -O1 -g -std=c++17 -Wall -Wextra -Wpedantic
OUT ?= /tmp/hd2-inspect
TRACE_DIR ?= /tmp/hd2-inspect-trace
HEADERS := $(wildcard ../../recovered/game/mission_script*.h)
$(OUT): inspect.cpp trace_support.h prepare_trace.py trace-contract.json $(HEADERS)
	python3 prepare_trace.py "$(TRACE_DIR)/game"
	$(CXX) $(CXXFLAGS) -I"$(TRACE_DIR)" -I. -I../../recovered inspect.cpp -o "$(OUT)"
.PHONY: all gui
all: $(OUT)
gui: $(OUT)
	python3 gui.py --inspector "$(OUT)"
