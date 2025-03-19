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

const short EXACT = 0;       // Exact bound
const short UPPER_BOUND = 1; // Upper bound
const short LOWER_BOUND = 2; // Lower bound
const short infinity = 32767;

struct Hash
{
    std::uint64_t key; // The zobrist Key of the position
    short depth;       // The current depth
    short type;        // Ether EXACT, UPPER_BOUND or LOWER_BOUND
    int score;         // The current score
    Move move;         // The bestmove that we currently have
    int eval;          // The static eval

    void setEntry(std::uint64_t _key, std::uint8_t _depth, std::uint8_t _type, int _score, Move _move, int _eval)
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
    tt(const tt &other) = delete;
    tt &operator=(const tt &other) = delete;
    ~tt();

    void storeEvaluation(std::uint64_t key, std::uint8_t depth, std::uint8_t type, int score, Move move, int eval);

    Hash *getHash(std::uint64_t zobristKey);

    std::uint64_t getSize() const;

    void setSize(std::uint64_t MB);

    void clear();

    int estimateHashfull() const;

    int scoreToTT(int score, int ply)
    {
        return score >= infinity    ? score + ply
               : score <= -infinity ? score - ply
                                    : score;
    }

    int scoreFromTT(int score, int ply)
    {
        return score >= infinity    ? score - ply
               : score <= -infinity ? score + ply
                                    : score;
    }

    bool checkForMoreInformation(short type, int ttScore, int score)
    {
        short tempType;
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