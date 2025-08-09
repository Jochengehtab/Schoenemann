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

#include "consts.h"
#include "chess.hpp"

using namespace chess;

struct alignas(16) Hash {
    std::uint64_t key; // 8 Byte
    Move move = Move::NO_MOVE; // 2 Byte
    std::int16_t score; // 2 Byte
    std::int16_t eval; // 2 Byte
    std::int8_t depth; // 1 Byte
    Bound type; // 1 Byte

    void setEntry(const std::uint64_t _key, const std::uint8_t _depth, const Bound _type,
                  const std::int16_t _score, const Move _move,
                  const std::int16_t _eval) {
        key = _key;
        depth = _depth;
        type = _type;
        score = _score;
        move = _move;
        eval = _eval;
    }
};

class tt {
public:
    explicit tt(std::uint64_t MB);

    ~tt();

    [[nodiscard]] Hash *getHash(std::uint64_t zobristKey) const noexcept;

    void setSize(std::uint64_t MB);

    void clear() const;

    void storeHash(std::uint64_t key, int depth, Bound type, int score,
                   Move move,
                   int eval) const noexcept;

    [[nodiscard]] int estimateHashfull() const noexcept;

    // Adjust a potential mate score for the tt
    static int scoreToTT(const int score, const int ply) {
        return score >= EVAL_MATE
                   ? score + ply
                   : score <= -EVAL_MATE
                         ? score - ply
                         : score;
    }

    // Adjust a potential mate score from the tt
    static int scoreFromTT(const int score, const int ply) {
        return score >= EVAL_MATE
                   ? score - ply
                   : score <= -EVAL_MATE
                         ? score + ply
                         : score;
    }

private:
    std::uint64_t size{};
    Hash *table{};

    void init(std::uint64_t MB);
};

#endif
