#pragma once

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

#define DO_TUNING

#ifdef DO_TUNING

// The # turns parameterName into a string

#define DEFINE_PARAM_S(parameterName, startValue, step) EngineParameter parameterName(#parameterName, startValue, step)

#define DEFINE_PARAM_B(parameterName, startValue, minValue, maxValue) EngineParameter parameterName(#parameterName, startValue, minValue, maxValue)

#else

#define DEFINE_PARAM_S(parameterName, startValue, step) constexpr int parameterName = startValue

#define DEFINE_PARAM_B(parameterName, startValue, minValue, maxValue) constexpr int parameterName = startValue

#endif