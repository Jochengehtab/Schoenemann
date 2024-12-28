EXE := null
CC ?= null

# Append .exe to the binary name on Windows

ifeq ($(OS),Windows_NT)

    override EXE := $(EXE).exe

endif

all:
	cd Schoenemann && clang++ -Wall -O3 -std=c++20 -funroll-loops -flto -DIS_64BIT -DNDEBUG schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp main.cpp moveorder.cpp see.cpp tune.cpp tuneconsts.cpp datagen.cpp -o ../src/$(EXE)
t:
	cd ../Schoenemann && clang++ -Wall -O0 -std=c++20 schoenemann.cpp search.cpp main.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp tune.cpp tuneconsts.cpp datagen.cpp -o $(EXE)