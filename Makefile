EXE = null

all:
	$(info i am here)
	cd Schoenemann && g++ -Wall -O3 -std=c++20 schoenemann.cpp search.cpp timeman.cpp helper.cpp tt.cpp main.cpp moveorder.cpp see.cpp -o $(EXE).exe