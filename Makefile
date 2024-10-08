EXE := null
CC ?= null

 

# Append .exe to the binary name on Windows

ifeq ($(OS),Windows_NT)

    override EXE := $(EXE).exe

endif

all:
	$(info The compiler is $(CC))
	cd Schoenemann && $(CC) -Wall -O3 -std=c++20 schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp main.cpp moveorder.cpp see.cpp -o ../src/$(EXE).exe
t:
	cd ../Schoenemann && $(CC) -Wall -O0 -std=c++20 schoenemann.cpp search.cpp main.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp -o $(EXE).exe
