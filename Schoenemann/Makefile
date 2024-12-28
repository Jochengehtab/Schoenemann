EXE := null
CC ?= null
 

# Append .exe to the binary name on Windows

ifeq ($(OS),Windows_NT)

    override EXE := $(EXE).exe

endif

all:
	cd ../Schoenemann && clang++ -Wall -std=c++20 -O3 -march=native -fuse-ld=lld -funroll-loops -O3 -flto -fno-exceptions -DIS_64BIT -DNDEBUG schoenemann.cpp search.cpp main.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cpp tuneconsts.cpp datagen.cpp -o $(EXE)
test:
# Possible santizier: address, undefined, leak, thread, memory
	cd ../Schoenemann && clang++ -Wall -Wextra -Wpedantic -Wshadow -std=c++20 -fsanitize=address -g3 schoenemann.cpp search.cpp main.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cpp tuneconsts.cpp datagen.cpp -o $(EXE)