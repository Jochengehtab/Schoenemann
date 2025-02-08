CXX = clang++

EXE = null

FLAGS = -Wall -Wextra -Wpedantic -Wshadow -pedantic

EVALFILE = quantised.bin

# Append .exe to the binary name on Windows
ifeq ($(OS),Windows_NT)
	EXE := $(EXE).exe
endif

SOURCES = schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cpp datagen.cpp

all:
	$(CXX) $(FLAGS) -pthread -std=c++20 -march=native -O3 -funroll-loops -DEVALFILE=\"$(EVALFILE)\" $(SOURCES) -o $(EXE)

test:
	$(CXX) $(FLAGS) -pthread -std=c++20 -fsanitize=address -g3 -DEVALFILE=\"$(EVALFILE)\" $(SOURCES) -o $(EXE)

release:
	$(CXX) $(FLAGS) -pthread -std=c++20 -O3 -funroll-loops -DEVALFILE=\"$(EVALFILE)\" $(SOURCES) -o $(EXE)