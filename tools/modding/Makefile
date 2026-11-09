CXX ?= c++
CXXFLAGS ?= -O1 -g -std=c++17 -Wall -Wextra -Wpedantic
OUT ?= /tmp/hd2-inspect
$(OUT): inspect.cpp $(wildcard ../../recovered/game/mission_script*.h)
	$(CXX) $(CXXFLAGS) -I../../recovered inspect.cpp -o $(OUT)
.PHONY: all
all: $(OUT)

.PHONY: gui
gui: $(OUT)
	python3 gui.py --inspector "$(OUT)"
