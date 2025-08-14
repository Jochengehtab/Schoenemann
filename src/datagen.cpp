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

#include "datagen.h"
#include "consts.h"
#include "helper.h"
#include "see.h"
#include "timeman.h"
#include "NNUE/nnue.h"
#include <random>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

// Global resources shared across all threads
extern std::mutex outputFileMutex;
extern std::atomic<std::uint64_t> totalPositionsGenerated;

void generate(int threadId, std::ofstream &outputFile) {
    // Each thread gets its own set of chess objects to prevent data races.
    tt transpositionTable(16);
    TimeManagement timeManagement;
    Network net;
    const auto search =
            std::make_unique<Search>(timeManagement, transpositionTable, net);
    SearchParams params;
    params.minimal = true;
    Board board(&net);
    board.setFen(STARTPOS);
    search->initLMR();

    // Seed the random number generator uniquely for each thread
    std::random_device rd;
    std::mt19937 gen(rd() + threadId);

    while (true) {
        board.setFen(STARTPOS);
        bool exitEarly = false;

        // Play a few random moves to get a variety of starting positions
        for (int i = 0; i < 10; i++) {
            Movelist moveList;
            movegen::legalmoves(moveList, board);

            if (auto [fst, snd] = board.isGameOver(); snd != GameResult::NONE || moveList.empty()) {
                exitEarly = true;
                break;
            }

            std::uniform_int_distribution dis(0, moveList.size() - 1);
            Move move = moveList[dis(gen)];

            board.makeMove(move);
        }

        if (exitEarly) {
            continue;
        }

        std::vector<std::string> outputLines;
        std::string resultString = "none";

        // Play out a game for a maximum of 500 moves
        for (int i = 0; i < 500; i++) {
            if (auto [fst, snd] = board.isGameOver(); snd != GameResult::NONE) {
                if (snd == GameResult::DRAW) resultString = "0.5";
                else resultString = snd == GameResult::LOSE && board.sideToMove() == Color::BLACK ? "1.0" : "0.0";
                break;
            }

            search->nodeLimit = 1000;
            search->iterativeDeepening(board, params);
            Move bestMove = search->rootBestMove;

            // Skip noisy positions to generate cleaner data
            if (bestMove.typeOf() == Move::PROMOTION || board.inCheck() || board.isCapture(bestMove) || std::abs(
                    search->currentScore) >= 10000) {
                board.makeMove(bestMove);
                continue;
            }

            int score = board.sideToMove() == Color::WHITE ? search->currentScore : -search->currentScore;
            outputLines.push_back(board.getFen() + " | " + std::to_string(score) + " | ");

            board.makeMove(bestMove);
        }

        if (resultString == "none") {
            continue;
        }

        if (!outputLines.empty()) {
            std::string finalOutput;
            for (const auto &line: outputLines) {
                finalOutput += line + resultString + "\n";
            }

            // Lock the mutex to ensure exclusive access to the file
            std::lock_guard guard(outputFileMutex);
            outputFile << finalOutput;

            // Atomically update the global counter
            totalPositionsGenerated += outputLines.size();
        }
    }
}
