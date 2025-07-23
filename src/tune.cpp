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

#include "tune.h"

#include <vector>

std::vector<EngineParameter *> engineParameter;

EngineParameter *findEngineParameterByName(const std::string &name) {
    // Loop over the whole vector
    for (EngineParameter *e: engineParameter) {
        if (e->name == name) {
            return e;
        }
    }
    return nullptr;
}

void addEngineParameter(EngineParameter *parameter) {
    engineParameter.push_back(parameter);
}

std::string engineParameterToUCI() {
    std::stringstream stream;
    for (EngineParameter *e: engineParameter) {
        if (e->max == e->min) {
            continue;
        }

        stream << "option name " << e->name << " type spin default " << e->value << " min -999999999 max 999999999\n";
    }
    return stream.str();
}

std::string engineParameterToSpsaInput() {
    std::stringstream stream;
    for (EngineParameter *e: engineParameter) {
        if (e->max == e->min) {
            continue;
        }
        stream << e->name << ", int, " << static_cast<double>(e->value) << ", " << static_cast<double>(e->min) << ", "
                << static_cast<double>(e->max) << ", " << std::max(0.5, static_cast<double>(e->max - e->min) / 20.0) <<
                ", " << 0.002 << "\n";
    }
    return stream.str();
}


DEFINE_PARAM(seePawn, 100, 100, 180);
DEFINE_PARAM(seeKnight, 300, 200, 350);
DEFINE_PARAM(seeBishop, 300, 280, 400);
DEFINE_PARAM(seeRook, 500, 500, 700);
DEFINE_PARAM(seeQueen, 900, 840, 1240);
DEFINE_PARAM(seeEmpty, 0, 0, 0);

DEFINE_PARAM(piecePawn, 73, 53, 93);
DEFINE_PARAM(pieceKnight, 258, 208, 288);
DEFINE_PARAM(pieceBishop, 217, 208, 308);
DEFINE_PARAM(pieceRook, 476, 430, 500);
DEFINE_PARAM(pieceQueen, 569, 530, 700);
DEFINE_PARAM(pieceKing, 15000, 15000, 15000);
DEFINE_PARAM(pieceEmpty, 0, 0, 0);

#ifdef DO_TUNING

EngineParameter* SEE_PIECE_VALUES[7] = {
    &seePawn, &seeKnight, &seeBishop, &seeRook, &seeQueen, &seeEmpty, &seeEmpty
};

EngineParameter* PIECE_VALUES[7] = {
    &piecePawn, &pieceKnight, &pieceBishop, &pieceRook, &pieceQueen, &pieceKing, &pieceEmpty
};

#else

int *SEE_PIECE_VALUES[7] = {
    const_cast<int *>(&seePawn), const_cast<int *>(&seeKnight), const_cast<int *>(&seeBishop),
    const_cast<int *>(&seeRook), const_cast<int *>(&seeQueen), const_cast<int *>(&seeEmpty),
    const_cast<int *>(&seeEmpty)
};

int *PIECE_VALUES[7] = {
    const_cast<int *>(&piecePawn), const_cast<int *>(&pieceKnight), const_cast<int *>(&pieceBishop),
    const_cast<int *>(&pieceRook), const_cast<int *>(&pieceQueen), const_cast<int *>(&pieceKing),
    const_cast<int *>(&pieceEmpty)
};

#endif
