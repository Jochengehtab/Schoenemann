CXX = clang++

EXE = null

FLAGS = -Wall -Wextra -Wpedantic -Wshadow -pedantic -pthread -std=c++20

EVALFILE = quantised.bin

# Append .exe to the binary name on Windows
ifeq ($(OS),Windows_NT)
	EXE := $(EXE).exe
endif

SOURCES = schoenemann.cpp search.cpp time.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cpp datagen.cpp history.cpp

all:
	$(CXX) $(FLAGS) -march=native -O3 -funroll-loops -DEVALFILE=\"$(EVALFILE)\" $(SOURCES) -o $(EXE)

test:
# Possible santizier: address, undefined, leak, thread, memory
	$(CXX) $(FLAGS) -fsanitize=memory -g3 -fno-omit-frame-pointer -DEVALFILE=\"$(EVALFILE)\" $(SOURCES) -o $(EXE)

release:
	$(CXX) $(FLAGS) -O3 -funroll-loops -DEVALFILE=\"$(EVALFILE)\" $(SOURCES) -o $(EXE)