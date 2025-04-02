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

#ifndef TT_H
#define TT_H

#include <iostream>
#include <cstring>

#include "chess.hpp"

using namespace chess;

const std::uint8_t EXACT = 0;       // Exact bound
const std::uint8_t UPPER_BOUND = 1; // Upper bound
const std::uint8_t LOWER_BOUND = 2; // Lower bound
const std::int16_t infinity = 32767;

struct Hash
{
    std::uint64_t key;  // The zobrist Key of the position
    std::int16_t depth; // The current depth
    std::int8_t type;   // Ether EXACT, UPPER_BOUND or LOWER_BOUND
    int score;          // The current score
    int eval;           // The static eval
    Move move = Move::NO_MOVE;          // The bestmove that we currently have

    void setEntry(std::uint64_t _key, std::int16_t _depth, std::uint8_t _type, int _score, Move _move, int _eval)
    {
        key = _key;
        depth = _depth;
        type = _type;
        score = _score;
        move = _move;
        eval = _eval;
    }
};

class tt
{
public:
    tt(std::uint64_t MB);
    ~tt();

    Hash *getHash(std::uint64_t zobristKey) noexcept;

    void setSize(std::uint64_t MB);
    void clear();
    void storeEvaluation(std::uint64_t key, std::int16_t depth, std::uint8_t type, int score, Move move, int eval) noexcept;

    int estimateHashfull() const noexcept;
    int scoreToTT(int score, std::int16_t ply)
    {
        return score >= infinity    ? score + ply
               : score <= -infinity ? score - ply
                                    : score;
    }

    int scoreFromTT(int score, std::int16_t ply)
    {
        return score >= infinity    ? score - ply
               : score <= -infinity ? score + ply
                                    : score;
    }

    bool checkForMoreInformation(std::uint8_t type, int ttScore, int score)
    {
        std::uint8_t tempType;
        if (ttScore >= score)
        {
            tempType = LOWER_BOUND;
        }
        else
        {
            tempType = UPPER_BOUND;
        }

        return type & tempType;
    }

private:
    std::uint64_t size;
    Hash *table;
    void init(std::uint64_t MB);
};

#endif