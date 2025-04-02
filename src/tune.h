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
#include <vector>
#include <string>
#include <sstream>

// Forward declaration of the struct
struct EngineParameter;

EngineParameter *findEngineParameterByName(std::string name);
void addEngineParameter(EngineParameter *parameter);

// UCI Stuff
std::string engineParameterToUCI();
std::string engineParameterToSpsaInput();

struct EngineParameter
{
    // The order of this values is importatnt
    std::string name;
    int value;
    int min;
    int max;

    operator int()
    {
        return value;
    }

    EngineParameter(std::string parameterName, int startValue, int step)
    : name(parameterName), value(startValue)
    {
        this->max = startValue + 15 * step;
        this->min = startValue - 15 * step;

        if (this->max < this->min)
        {
            std::cout << "Max Value is smaller than the Min value" << std::endl;
        }

        addEngineParameter(this);
    }

    EngineParameter(std::string parameterName, int startValue, int minValue, int maxValue)
    : name(parameterName), value(startValue), min(minValue), max(maxValue)
    {
        if (this->max < this->min)
        {
            std::cout << "Max Value is smaller than the Min value" << std::endl;
        }

        addEngineParameter(this);
    }
};

extern int SEE_PIECE_VALUES[7];
extern int PIECE_VALUES[7];

// #define DO_TUNING

#ifdef DO_TUNING

// The # turns parameterName into a string

#define DEFINE_PARAM_S(parameterName, startValue, step) EngineParameter parameterName(#parameterName, startValue, step)

#define DEFINE_PARAM_B(parameterName, startValue, minValue, maxValue) EngineParameter parameterName(#parameterName, startValue, minValue, maxValue)

#else

#define DEFINE_PARAM_S(parameterName, startValue, step) constexpr int parameterName = startValue

#define DEFINE_PARAM_B(parameterName, startValue, minValue, maxValue) constexpr int parameterName = startValue

#endif

#endif