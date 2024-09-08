EXE ?= null
CC ?= null

all:
	cd Schoenemann && clang++ -Wall -O3 -std=c++20 schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp main.cpp moveorder.cpp see.cpp -o ../src/$(EXE).exe
t:
	cd ../Schoenemann && clang++ -Wall -O0 -std=c++20 schoenemann.cpp search.cpp main.cpp timeman.cpp helper.cpp tt.cpp moveorder.cpp see.cpp -o $(EXE).exe
