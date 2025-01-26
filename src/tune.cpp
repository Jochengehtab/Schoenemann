#include "tune.h"

#include <sstream>
#include <vector>

// Tuner taken from Obsidian
// Thx gabe

std::vector<EngineParam *> tuningParams;

void registerParam(EngineParam *param)
{
  tuningParams.push_back(param);
}

EngineParam *findParam(std::string name)
{
  for (int i = 0; i < static_cast<int>(tuningParams.size()); i++)
  {
    if (tuningParams.at(i)->name == name)
    {
      return tuningParams.at(i);
    }
  }
  return nullptr;
}

std::string paramsToUci()
{
  std::ostringstream ss;

  for (int i = 0; i < static_cast<int>(tuningParams.size()); i++)
  {
    EngineParam *p = tuningParams.at(i);

    ss << "option name " << p->name << " type spin default " << p->value << " min -999999999 max 999999999\n";
  }

  return ss.str();
}

std::string paramsToSpsaInput()
{
  std::ostringstream ss;

  for (int i = 0; i < static_cast<int>(tuningParams.size()); i++)
  {
    EngineParam *p = tuningParams.at(i);

    ss << p->name
       << ", " << "int"
       << ", " << double(p->value)
       << ", " << double(p->min)
       << ", " << double(p->max)
       << ", " << std::max(0.5, double(p->max - p->min) / 20.0)
       << ", " << 0.002
       << "\n";
  }

  return ss.str();
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
