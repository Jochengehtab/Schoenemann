/*
  This file is part of the Schoenemann chess engine written by Jochengehtab

  Copyright (C) 2024-2025 Jochengehtab

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TUNE_H
#define TUNE_H

#include <iostream>
#include <string>
#include <sstream>
#include <utility>

struct EngineParameter;

EngineParameter *findEngineParameterByName(const std::string &name);

void addEngineParameter(EngineParameter *parameter);

// UCI Stuff
std::string engineParameterToUCI();

std::string engineParameterToSpsaInput();

struct EngineParameter {
    std::string name;
    int value;
    int min;
    int max;

    operator int() const {
        return value;
    }

    EngineParameter(std::string parameterName, const int startValue, const int minValue, const int maxValue)
        : name(std::move(parameterName)), value(startValue), min(minValue), max(maxValue) {
        // TODO proper value checking
        if (this->max < this->min) {
            std::cout << "Max Value " << this->max << " is smaller than the Min" << this->min << " value" << std::endl;
        }

        addEngineParameter(this);
    }
};

//#define DO_TUNING

#ifdef DO_TUNING

// The # turns parameterName into a string
#define DEFINE_PARAM(parameterName, startValue, minValue, maxValue) EngineParameter parameterName(#parameterName, startValue, minValue, maxValue)
extern EngineParameter *SEE_PIECE_VALUES[7];
extern EngineParameter *PIECE_VALUES[7];

#else

#define DEFINE_PARAM(parameterName, startValue, minValue, maxValue) constexpr int parameterName = startValue
extern int *SEE_PIECE_VALUES[7];
extern int *PIECE_VALUES[7];

#endif

#endif
