EXE := null
CC ?= null

 

# Append .exe to the binary name on Windows

ifeq ($(OS),Windows_NT)

    override EXE := $(EXE).exe

endif

all:
	cd ../Schoenemann && g++ -Wall -std=c++20 -O3 schoenemann.cpp search.cpp main.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp -o $(EXE).exe
test:
	cd ../Schoenemann && g++ -Wall -std=c++20 -O0 schoenemann.cpp search.cpp main.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp -o $(EXE).exe