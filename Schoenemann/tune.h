#pragma once

#include <iostream>
#include <cstdint>
#include <cassert>
#include <string_view>
#include <array>
#include <bit>
#include <ranges>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include <fstream>
#include <thread>
#include <memory>
#include <cstring>
#include <atomic>
#include <span>
#include <chrono>
#include <iostream>
#include <cmath>
#include <cassert>

// nicknaming st

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

Tunable seePawn("SEE_Pawn", 117, 1);
Tunable seeKnight("SEE_Knight", 370, 1);
Tunable seeBishop("SEE_Bishop", 422, 1);
Tunable seeRook("SEE_Rook", 606, 1);
Tunable seeQueen("SEE_Queen", 1127, 1);

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