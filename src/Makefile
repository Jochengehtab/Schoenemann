EXE := null
CC := clang++
EVALFILE := quantised.bin
 

# Append .exe to the binary name on Windows

ifeq ($(OS),Windows_NT)

    override EXE := $(EXE).exe

endif

all:
	$(CC) -Wall -Wextra -Wpedantic -Wshadow -std=c++20 -march=native -O3 -funroll-loops -DEVALFILE=\"$(EVALFILE)\" schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cpp datagen.cpp -o $(EXE)
test:
# Possible santizier: address, undefined, leak, thread, memory
	$(CC) -Wall -Wextra -Wpedantic -Wshadow -std=c++20 -fsanitize=address -g3 -DEVALFILE=\"$(EVALFILE)\" schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cpp datagen.cpp -o $(EXE)
release:
	$(CC) -Wall -std=c++20 -O3 -funroll-loops -DEVALFILE=\"$(EVALFILE)\" schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cppdatagen.cpp -o Schoenemann