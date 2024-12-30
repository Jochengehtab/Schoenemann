EXE := null
CC ?= null
 

# Append .exe to the binary name on Windows

ifeq ($(OS),Windows_NT)

    override EXE := $(EXE).exe

endif

all:
	clang++ -Wall -std=c++20 -march=native -O3 -funroll-loops -DNDEBUG schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cpp tuneconsts.cpp datagen.cpp -o $(EXE)
test:
# Possible santizier: address, undefined, leak, thread, memory
	clang++ -Wall -Wextra -Wpedantic -Wshadow -std=c++20 -fsanitize=address -g3 schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cpp tuneconsts.cpp datagen.cpp -o $(EXE)