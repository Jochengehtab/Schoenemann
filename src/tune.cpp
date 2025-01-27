#include "tune.h"

std::vector<EngineParameter *> engineParameter;

EngineParameter *findEngineParameterByName(std::string name)
{
    // Loop over the whole vector
    for (EngineParameter *e : engineParameter)
    {
        if (e->name == name)
        {
            return e;
        }
    }
    return nullptr;
}

void addEngineParameter(EngineParameter *parameter)
{
    engineParameter.push_back(parameter);
}

std::string engineParameterToUCI()
{
    std::stringstream stream;
    for (EngineParameter *e : engineParameter)
    {
        stream << "option name " << e->name << " type spin default " << e->value << " min -999999999 max 999999999\n";
    }
    return stream.str();
}

std::string engineParameterToSpsaInput()
{
    std::stringstream stream;
    for (EngineParameter *e : engineParameter)
    {
        stream << e->name << ", " << "int" << ", " << double(e->value) << ", " << double(e->min) << ", " << double(e->max) << ", " << std::max(0.5, double(e->max - e->min) / 20.0) << ", " << 0.002 << "\n";
    }
    return stream.str();
}


DEFINE_PARAM_S(seePawn, 140, 10);
DEFINE_PARAM_S(seeKnight, 287, 30);
DEFINE_PARAM_S(seeBishop, 348, 30);
DEFINE_PARAM_S(seeRook, 565, 50);
DEFINE_PARAM_S(seeQueen, 1045, 90);

int SEE_PIECE_VALUES[7] = {seePawn, seeKnight, seeBishop, seeRook, seeQueen, 0, 0};

DEFINE_PARAM_S(piecePawn, 73, 10);
DEFINE_PARAM_S(pieceKnight, 258, 20);
DEFINE_PARAM_S(pieceBishop, 217, 20);
DEFINE_PARAM_S(pieceRook, 476, 50);
DEFINE_PARAM_S(pieceQueen, 569, 90);

int PIECE_VALUES[7] = {piecePawn, pieceKnight, pieceBishop, pieceRook, pieceQueen, 15000, 0};