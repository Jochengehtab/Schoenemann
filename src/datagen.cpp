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
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <cassert>

// Global resources shared across all threads
extern std::mutex outputFileMutex;
extern std::atomic<std::uint64_t> totalPositionsGenerated;

void generate(int threadId, std::ofstream &outputFile, std::uint64_t positionAmount) {
    tt transpositionTable(16);
    TimeManagement timeManagement;
    Network net;
    const auto search =
            std::make_unique<Search>(timeManagement, transpositionTable, net);
    SearchParams params;
    params.minimal = true;
    Board board(&net);
    search->initLMR();

    std::random_device rd;
    std::mt19937 gen(rd() + threadId);

    // The persistent buffer for batching writes.
    std::vector<std::string> writeBuffer;

    // Pre-allocate memory
    writeBuffer.reserve(5120);

    while (totalPositionsGenerated < positionAmount) {
        board.setFen(STARTPOS);
        bool exitEarly = false;

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

        // Temporary storage for the current game
        std::vector<std::pair<std::string, int> > currentGameData;
        std::string resultString = "none";

        // Play out the game
        for (int i = 0; i < 500; i++) {
            if (auto [fst, snd] = board.isGameOver(); snd != GameResult::NONE) {
                if (snd == GameResult::DRAW) resultString = "0.5";
                else resultString = snd == GameResult::LOSE && board.sideToMove() == Color::BLACK ? "1.0" : "0.0";
                break;
            }

            search->nodeLimit = 5000;
            search->iterativeDeepening(board, params);
            Move bestMove = search->rootBestMove;

            if (bestMove.typeOf() == Move::PROMOTION || board.inCheck() || board.isCapture(bestMove) || std::abs(
                    search->currentScore) >= 10000) {
                board.makeMove(bestMove);
                continue;
            }

            int score = board.sideToMove() == Color::WHITE ? search->currentScore : -search->currentScore;

            // Store FEN and score in the temporary container
            currentGameData.emplace_back(board.getFen(), score);

            board.makeMove(bestMove);
        }

        // Discard incomplete games
        if (resultString == "none") {
            continue;
        }

        // Append the result
        for (const auto &[fst, snd]: currentGameData) {
            std::string line = fst;
            line.append(" | ");
            line.append(std::to_string(snd));
            line.append(" | ");
            line.append(resultString);
            writeBuffer.push_back(line);
        }
        totalPositionsGenerated += currentGameData.size();

        // Check if the persistent buffer is full enough to write
        if (writeBuffer.size() >= 5000) {
            std::lock_guard guard(outputFileMutex);
            for (const auto &line: writeBuffer) {
                outputFile << line << "\n";
            }
            outputFile.flush();

            // Clear the buffer for the next batch
            writeBuffer.clear();
        }
    }

    // After the loop, write any remaining data in the buffer.
    if (!writeBuffer.empty()) {
        std::lock_guard guard(outputFileMutex);
        for (const auto& line : writeBuffer) {
            outputFile << line << "\n";
        }
        outputFile.flush();
        writeBuffer.clear();
    }
}