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

#ifndef SEARCH_H
#define SEARCH_H

#include "timeman.h"
#include "tt.h"
#include "moveorder.h"
#include "search_fwd.h"
#include <memory>
#include <limits>
#include <atomic>

struct alignas(8) SearchParams {
    bool isInfinite = false;
    int depth = MAX_PLY;
};

class Search {
public:
    Search(TimeManagement &timeManagement,
           tt &transpositionTabel,
           Network &net) : reductions{}, stack{}, timeManagement(timeManagement),
                           transpositionTable(transpositionTabel), history(),
                           net(net) {
    }

    Move rootBestMove = Move::NULL_MOVE;
    Move previousBestMove = Move::NULL_MOVE;

    std::atomic<bool> shouldStop{false};

    std::uint64_t nodeLimit = NO_NODE_LIMIT;
    std::uint64_t nodes = 0;

    int timeForMove = 0;
    int currentScore = 0;
    int previousBestScore = 0;

    std::uint8_t reductions[MAX_PLY][MAX_MOVES];
    SearchStack stack[MAX_PLY];

    static int scaleOutput(int rawEval, const Board &board);

    [[nodiscard]] std::string scoreToUci() const;
    [[nodiscard]] int evaluate(const Board &board) const;

    int pvs(int alpha, int beta, int depth, int ply, Board &board, bool cutNode);
    int qs(int alpha, int beta, Board &board, int ply);

    void updatePv(int ply, const Move &move);
    void iterativeDeepening(Board &board, const SearchParams &params);
    void initLMR();
    void resetHistory();

private:
    TimeManagement &timeManagement;
    tt &transpositionTable;
    History history;
    Network &net;

    std::chrono::steady_clock::time_point start;

    std::unique_ptr<RootMove[]> rootMoveList;
    int rootMoveListSize = 0;
    static constexpr std::uint64_t NO_NODE_LIMIT = std::numeric_limits<std::uint64_t>::max();

    static bool isDraw(const Board &board);

    [[nodiscard]] bool shouldExit(const Board &board, int ply) const;

    [[nodiscard]] std::string getPVLine() const;
};

#endif
