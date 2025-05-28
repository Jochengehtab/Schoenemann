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

#include "helper.h"

#include <cassert>
#include <chrono>
#include <thread>

void Helper::transpositionTableTest(const tt &transpositionTable) {
    Board board;
    // Set up a unique position
    board.setFen("3N4/2p5/5K2/k1PB3p/3Pr3/1b5p/6p1/5nB1 w - - 0 1");
    const std::uint64_t key = board.hash();

    // Store some placeholder information
    transpositionTable.storeHash(key, 2, LOWER_BOUND, transpositionTable.scoreToTT(200, 1),
                                 uci::uciToMove(board, "d5e4"), 1);

    // Try to get the information out of the table
    const Hash *entry = transpositionTable.getHash(key);

    assert(entry != nullptr);

    const std::uint64_t hashedKey = entry->key;
    assert(hashedKey == key);

    const std::uint8_t hashedDepth = entry->depth;
    assert(hashedDepth == 2);

    const short hashedType = entry->type;
    assert(hashedType == LOWER_BOUND);

    const int hashedScore = entry->score;
    assert(hashedScore == 200);

    const Move hashedMove = entry->move;
    assert(hashedMove == uci::uciToMove(board, "d5e4"));
}

// Print the uci info
void Helper::uciPrint() {
    std::cout << "id name Schoenemann" << std::endl
            << "option name Hash type spin default 64 min 1 max 4096" << std::endl
            << "option name Threads type spin default 1 min 1 max 1" << std::endl;
}

void Helper::runBenchmark(Search* search, Board &board) {
    // Setting up the clock
    const std::chrono::time_point start = std::chrono::steady_clock::now();

    // Resting the nodes
    search->nodes = 0;
    search->setTimeInfinite();

    // Looping over all bench positions
    for (const std::string &test: testStrings) {
        board.setFen(test);
        search->pvs(-EVAL_INFINITE, EVAL_INFINITE, benchDepth, 0, board);
    }

    const std::chrono::time_point end = std::chrono::steady_clock::now();

    // Calculates the total time used
    const std::chrono::duration<double, std::milli> timeElapsed = end - start;
    const int timeInMs = static_cast<int>(timeElapsed.count());

    // calculates the Nodes per Second
    const int NPS = static_cast<int>(search->nodes / timeElapsed.count() * 1000);

    // Prints out the final bench
    std::cout << "Time  : " << timeInMs << " ms\nNodes : " << search->nodes << "\nNPS   : " << NPS << std::endl;

    board.setFen(STARTPOS);
}

void Helper::handleSetPosition(Board &board, std::istringstream &is, std::string &token) {
    board.setFen(STARTPOS);
    std::string fen;
    std::vector<std::string> moves;
    bool isFen = false;
    while (is >> token) {
        if (token == "fen") {
            isFen = true;
            while (is >> token && token != "moves") {
                fen += token + " ";
            }
            fen = fen.substr(0, fen.size() - 1);
            board.setFen(fen);
        } else if (token != "moves" && isFen) {
            moves.push_back(token);
        } else if (token == "startpos") {
            board.setFen(STARTPOS);
            isFen = true;
        }
    }

    for (const std::string &move: moves) {
        board.makeMove(uci::uciToMove(board, move));
    }
}

void Helper::handleGo(Search &search, Time &timeManagement, Board &board, std::istringstream &is, std::string &token) {
    int number[4];
    bool hasTime = false;
    search.shouldStop = false;

    is >> token;
    if (!is.good()) {
        std::thread t1([&] { search.iterativeDeepening(board, true); });
        t1.detach();
    }
    while (is.good()) {
        if (token == "wtime") {
            is >> token;
            number[0] = std::stoi(token);
            hasTime = true;
        } else if (token == "btime") {
            is >> token;
            number[1] = std::stoi(token);
            hasTime = true;
        } else if (token == "winc") {
            is >> token;
            number[2] = std::stoi(token);
        } else if (token == "binc") {
            is >> token;
            number[3] = std::stoi(token);
        } else if (token == "depth") {
            is >> token;
            std::thread t1([&] { search.pvs(-EVAL_INFINITE, EVAL_INFINITE, std::stoi(token), 0, board); });
            t1.detach();

            std::cout << "bestmove " << uci::moveToUci(search.rootBestMove) << std::endl;
        } else if (token == "nodes") {
            is >> token;
            search.nodeLimit = std::stoi(token);
            std::thread t1([&] { search.iterativeDeepening(board, true); });
            t1.detach();
        } else if (token == "movetime") {
            is >> token;
            timeManagement.timeLeft = std::stoi(token);
            std::thread t1([&] { search.iterativeDeepening(board, false); });
            t1.detach();
        }
        if (!(is >> token)) {
            break;
        }
    }
    if (hasTime) {
        if (board.sideToMove() == Color::WHITE) {
            timeManagement.timeLeft = number[0];
            timeManagement.increment = number[2];
        } else {
            timeManagement.timeLeft = number[1];
            timeManagement.increment = number[3];
        }

        std::thread t1([&] { search.iterativeDeepening(board, false); });
        t1.detach();
    }
}
