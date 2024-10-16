#pragma once

#include <iostream>
#include <algorithm>

#include "tt.h"
#include "search.h"

struct Tunable {
    explicit Tunable(std::string _name, double _value, int _divisor)
        : name(std::move(_name)),
          value(_value),
          max(value * _divisor * 2),
          divisor(_divisor),
          step(max / 15 == 0 ? 1 : max / 15) {}

    void updateValue(double newValue) {
        value = newValue / divisor;
    }

    std::string name;
    double value;
    int max;
    int divisor;
    int step;
};

extern std::vector<Tunable *> tunables;
extern std::array<std::array<uint8_t, 218>, 150> reductions;

const std::string STARTPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int timeLeft = 0;
int increment = 0;

// Previously consts.h

//Bench depth 
const int benchDepth = 7;

const int NO_VALUE = 50000;

const int hashMoveScore = 2147483647, promotion = 1000000000, goodCapture = 1500000000, badCapture = -500000000;

const std::string testStrings[] = {
		STARTPOS,
		"r2q4/pp1k1pp1/2p1r1np/5p2/2N5/1P5Q/5PPP/3RR1K1 b - -",
		"5k2/1qr2pp1/2Np1n1r/QB2p3/2R4p/3PPRPb/PP2P2P/6K1 w - -",
		"r2r2k1/2p2pp1/p1n4p/1qbnp3/2Q5/1PPP1RPP/3NN2K/R1B5 b - -",
		"8/3k4/p6Q/pq6/3p4/1P6/P3p1P1/6K1 w - -",
		"8/8/k7/2B5/P1K5/8/8/1r6 w - -",
		"8/8/8/p1k4p/P2R3P/2P5/1K6/5q2 w - -",
		"rnbq1k1r/ppp1ppb1/5np1/1B1pN2p/P2P1P2/2N1P3/1PP3PP/R1BQK2R w KQ -",
		"4r3/6pp/2p1p1k1/4Q2n/1r2Pp2/8/6PP/2R3K1 w - -",
		"8/3k2p1/p2P4/P5p1/8/1P1R1P2/5r2/3K4 w - -",
		"r5k1/1bqnbp1p/r3p1p1/pp1pP3/2pP1P2/P1P2N1P/1P2NBP1/R2Q1RK1 b - -",
		"r1bqk2r/1ppnbppp/p1np4/4p1P1/4PP2/3P1N1P/PPP5/RNBQKBR1 b Qkq -",
		"5nk1/6pp/8/pNpp4/P7/1P1Pp3/6PP/6K1 w - -",
		"2r2rk1/1p2npp1/1q1b1nbp/p2p4/P2N3P/BPN1P3/4BPP1/2RQ1RK1 w - -",
		"8/2b3p1/4knNp/2p4P/1pPp1P2/1P1P1BPK/8/8 w - -"
};

extern tt transpositionTabel;
class Search;
extern Search* searcher;

/*
void calculateReductions() {
    for(int depth = 0; depth < 150; depth++) {
        for(int move = 0; move < 218; move++) {
            reductions[depth][move] = uint8_t(std::clamp(lmrBase.value + log(depth) * log(move) * lmrMultiplier.value, -32678.0, 32678.0));
        }
    }
}
*/

Tunable rfpDepthCondition("RFP_DepthCondition", 6, 1);
Tunable rfpMultiplier("RFP_Multiplier", 70, 1);

Tunable iirDepthCondition("IIR_DepthCondition", 4, 1);

Tunable fpDepthCondition("FP_DepthCondition", 0, 1);
Tunable fpBase("FP_Base", 347, 1);
Tunable fpMultiplier("FP_Multiplier", 44, 1);

Tunable pcbAddition("PCB_Addition", 390, 1);
Tunable pcbDepth("PCB_Depth", 2, 1);

//Tunable lmpBase("LMP_Base", 3, 1);

Tunable nmpDivisor("NMP_Divisor", 3, 1);
Tunable nmpAdder("NMP_Adder", 3, 1);
Tunable nmpDepthCondition("NMP_DepthCondition", 5, 1);

//Tunable lmrBase("LMR_Base", 0.97, 100);
//Tunable lmrMultiplier("LMR_Multiplier", 0.54, 100);

Tunable mvvPawn("MVV_Pawn", 100, 1);
Tunable mvvKnight("MVV_Knight", 302, 1);
Tunable mvvBishop("MVV_Bishop", 320, 1);
Tunable mvvRook("MVV_Rook", 500, 1);
Tunable mvvQueen("MVV_Queen", 900, 1);
Tunable blank("blank", 0, 1);


// const int SEE_PIECE_VALUES[7] = {100, 300, 300, 500, 900, 0, 0};
Tunable seePawn("SEE_Pawn", 100, 1);
Tunable seeKnight("SEE_Knight", 300, 1);
Tunable seeBishop("SEE_Bishop", 300, 1);
Tunable seeRook("SEE_Rook", 500, 1);
Tunable seeQueen("SEE_Queen", 900, 1);

// Declaration of pointers to tunables
std::array<Tunable *, 7> MVV_values = {
    &mvvPawn,
    &mvvKnight,
    &mvvBishop,
    &mvvRook,
    &mvvQueen,
    &blank,
    &blank,
};

std::array<Tunable *, 7> SEE_values = {
    &seePawn,
    &seeKnight,
    &seeBishop,
    &seeRook,
    &seeQueen,
    &blank,
    &blank,
};

std::vector<Tunable *> tunables = {
    &nmpAdder,
    &pcbAddition,
    &pcbDepth,
    &rfpDepthCondition,
    &rfpMultiplier,
    &iirDepthCondition,
    &fpDepthCondition,
    &fpBase,
    &fpMultiplier,
    &nmpDivisor,
    &nmpDepthCondition,
    &mvvPawn,
    &mvvKnight,
    &mvvBishop,
    &mvvRook,
    &mvvQueen,
    &seePawn,
    &seeKnight,
    &seeBishop,
    &seeRook,
    &seeQueen,
};


void outputTunables() {
    for(Tunable *tunable : tunables) {
        std::cout << "option name " << tunable->name << " type spin default " << std::to_string(static_cast<int>(tunable->value * tunable->divisor)) << " min " << "0" << " max " << tunable->max << std::endl;
    }
}
void outputTunableJSON() {
    std::cout << "{\n";
    for(Tunable *tunable : tunables) {
        std::cout << "   \"" << tunable->name << "\": {\n";
        std::cout << "      \"value\": " << std::to_string(static_cast<int>(tunable->value * tunable->divisor)) << "," << std::endl;
        std::cout << "      \"min_value\": " << "0" << "," << std::endl;
        std::cout << "      \"max_value\": " << tunable->max << "," << std::endl;
        std::cout << "      \"step\": " << tunable->step << std::endl;
        std::cout << "   },\n";
    }
    std::cout << "}\n";
}
void outputTunableOpenBench() {
    for(Tunable *tunable : tunables) {
        std::cout << tunable->name << ", int, " << std::to_string(static_cast<int>(tunable->value * tunable->divisor)) << ", 0, " << tunable->max << ", " << tunable->step << ", 0.002" << std::endl;
    }
}
void adjustTunable(const std::string &name, const int &value) {
    for(Tunable *tunable : tunables) {
        if(tunable->name == name) {
            tunable->updateValue(value);
            /*
            if(name == "LMR_Base" || name == "LMR_Multiplier") {
                calculateReductions();
            }
            */
            return;
        }
    }
    std::cout << "This tunable: '" << name << "' does not exists!" << std::endl;
}
void readTunable(const std::string &name) {
    for(Tunable *tunable : tunables) {
        if(tunable->name == name) {
            std::cout << "value: " << tunable->value << std::endl;
            return;
        }
    }
    std::cout << "No Such Tunable\n";
}
void readTunables() {
    for(Tunable *tunable : tunables) {
        std::cout << "name: " << tunable->name << std::endl;
        std::cout << "value: " << tunable->value << std::endl;
        std::cout << "min: " << "0" << std::endl;
        std::cout << "max: " << tunable->max << std::endl;
        std::cout << "divisor: " << tunable->divisor << std::endl;
        std::cout << "step: " << tunable->step << std::endl;
        }
}