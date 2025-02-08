CXX ?= clang++

# Name of the executable
EXE = Schoenemann

EVALFILE = quantised.bin

# Append .exe to the binary name on Windows
ifeq ($(OS),Windows_NT)
	EXE := $(EXE).exe
endif

SOURCES = schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cpp datagen.cpp

all:
	$(CXX) -Wall -Wextra -Wpedantic -Wshadow -std=c++20 -march=native -O3 -funroll-loops -DEVALFILE=\"$(EVALFILE)\" $(SOURCES) -o $(EXE)

test:
	$(CXX) -Wall -Wextra -Wpedantic -Wshadow -std=c++20 -fsanitize=address -g3 -DEVALFILE=\"$(EVALFILE)\" $(SOURCES) -o $(EXE)

release:
	$(CXX) -Wall -std=c++20 -O3 -funroll-loops -DEVALFILE=\"$(EVALFILE)\" $(SOURCES) -o $(EXE)