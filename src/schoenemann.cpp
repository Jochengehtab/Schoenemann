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

#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <cstring>
#include <vector>
#include <fstream>
#include <mutex>
#include <atomic>
#include <iomanip>

#include "consts.h"
#include "helper.h"
#include "NNUE/nnue.h"
#include "datagen.h"
#include "tune.h"
#include "search.h"
#include "tt.h"
#include "timeman.h"
#include "see.h"


// Define global shared resources for multithreading
std::mutex outputFileMutex;
std::atomic<std::uint64_t> totalPositionsGenerated(0);

int main(int argc, char *argv[]) {
    std::uint32_t transpositionTableSize = 16;

    tt transpositionTable(transpositionTableSize);
    TimeManagement timeManagement;
    Network net;
    SearchParams params;

    const std::unique_ptr<Search> search =
            std::make_unique<Search>(timeManagement, transpositionTable, net);

    // The main board
    Board board(&net);

    // UCI-Command stuff
    std::string token, cmd;

    // Reset the board
    board.setFen(STARTPOS);

    // Disable FRC (Fisher-Random-Chess)
    board.set960(false);

    // Init the LMR
    search->initLMR();

    transpositionTable.setSize(transpositionTableSize);
    timeManagement.reset();
    search->resetHistory();

    std::thread searchThread;

    // Helper function for stoping the search
    auto stopSearch = [&]() {
        if (searchThread.joinable()) {
            search->shouldStop = true;
            searchThread.join();
        }
    };

    if (argc > 1 && std::strcmp(argv[1], "bench") == 0) {
        Helper::runBenchmark(search.get(), board, params);
        return 0;
    }

    // Main UCI-Loop
    do {
        if (argc == 1 && !std::getline(std::cin, cmd)) {
            cmd = "quit";
        }

        std::istringstream is(cmd);
        cmd.clear();

        token.clear();
        is >> token;

        if (token == "uci") {
            Helper::uciPrint();

#ifdef DO_TUNING
            std::cout << engineParameterToUCI();
#endif
            std::cout << "uciok" << std::endl;
        } else if (token == "stop") {
            stopSearch();
        } else if (token == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (token == "ucinewgame") {
            stopSearch();
            // Reset the board
            board.setFen(STARTPOS);

            // Clear the transposition table
            transpositionTable.clear();

            // Reset the time mangement
            timeManagement.reset();

            // Also reset all the historys
            search->resetHistory();
        } else if (token == "setoption") {
            stopSearch();
            is >> token;

            if (token == "name") {
                is >> token;
#ifdef DO_TUNING
                EngineParameter *param = findEngineParameterByName(token);
                if (param != nullptr) {
                    is >> token;
                    if (token == "value") {
                        is >> token;
                        param->value = std::stoi(token);
                        if (param->name == "lmrBase" || param->name == "lmrDivisor") {
                            search->initLMR();
                        }
                    }
                }
#endif
                if (token == "Hash") {
                    is >> token;
                    if (token == "value") {
                        is >> token;
                        transpositionTableSize = std::stoi(token);
                        transpositionTable.clear();
                        transpositionTable.setSize(transpositionTableSize);
                    }
                }
            }
        } else if (token == "position") {
            stopSearch();
            Helper::handleSetPosition(board, is, token);
        } else if (token == "go") {
            // Stop search
            stopSearch();
            search->shouldStop = false;

            Helper::handleGo(*search, timeManagement, board, is, params);
            searchThread = std::thread([&] {
                search->iterativeDeepening(board, params);
            });
        } else if (token == "d") {
            std::cout << board << std::endl;
        } else if (token == "fen") {
            std::cout << board.getFen() << std::endl;
        } else if (token == "datagen") {
            int numThreads;
            std::uint64_t positionAmount;

            std::cout << "Enter the number of threads to use (press Enter to use half of the available threads): ";
            std::string threadInput;
            std::getline(std::cin, threadInput);

            if (threadInput.empty()) {
                numThreads = std::max(1u, std::thread::hardware_concurrency() / 2);
            } else {
                try {
                    numThreads = std::stoi(threadInput);
                } catch ([[maybe_unused]] const std::invalid_argument &ia) {
                    std::cerr << "Invalid number of threads. Using half of the available threads." << std::endl;
                    numThreads = std::max(1u, std::thread::hardware_concurrency() / 2);
                }
            }

            std::cout << "Enter the number of positions to generate: ";
            std::cin >> positionAmount;
            // Consume the rest of the line
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Starting datagen with " << numThreads << " threads." << std::endl;

            // Open the output file once in append mode
            std::ofstream outputFile("output.txt", std::ios::app);
            if (!outputFile.is_open()) {
                std::cerr << "Error opening output file for datagen!" << std::endl;
                return 1;
            }

            std::vector<std::thread> threads;
            for (int i = 0; i < numThreads; ++i) {
                // Launch each thread to run the 'generate' function
                threads.emplace_back(generate, i, std::ref(outputFile), positionAmount);
            }

            // Periodically print statistics from the main thread
            auto startTime = std::chrono::steady_clock::now();
            while (totalPositionsGenerated < positionAmount) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - startTime).count();
                if (elapsedTime > 0) {
                    // Use .load() for safe reading of atomic variable
                    auto currentPositions = totalPositionsGenerated.load();
                    double pps = static_cast<double>(currentPositions) / elapsedTime;

                    // Ensure ETA is never negative
                    double eta = (pps > 0 && currentPositions < positionAmount)
                                     ? (positionAmount - currentPositions) / pps
                                     : 0.0;

                    // Cap displayed progress at 100%
                    double progress = std::min(100.0, static_cast<double>(currentPositions) / positionAmount * 100.0);

                    std::cout << "\r" << std::fixed << std::setprecision(2)
                            << "Progress: " << progress << "% | "
                            << "Positions: " << currentPositions << "/" << positionAmount << " | "
                            << "PPS: " << static_cast<int>(pps) << " | "
                            << "ETA: " << static_cast<int>(eta) << "s   " << std::flush;
                }
            }

            // Print the progress bar
            std::cout << "\r" << std::fixed << std::setprecision(2)
                    << "Progress: " << 100.00 << "% | "
                    << "Positions: " << positionAmount << "/" << positionAmount << " | "
                    << "PPS: " << 0 << " | "
                    << "ETA: " << 0 << "s   " << std::endl;


            // The program will run indefinitely; this join part is for graceful shutdown logic
            for (std::thread &t: threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
            outputFile.close();
        } else if (token == "bench") {
            Helper::runBenchmark(search.get(), board, params);
        } else if (token == "eval") {
            std::cout << "The raw eval is: " << net.evaluate(board.sideToMove(), board.occ().count()) << std::endl;
            std::cout << "The scaled evaluation is: " << Search::scaleOutput(
                net.evaluate(board.sideToMove(), board.occ().count()), board) << " cp" << std::endl;
        } else if (token == "spsa") {
            std::cout << engineParameterToSpsaInput() << std::endl;
        } else if (token == "stop") {
            search->shouldStop = true;
        } else {
            std::cout << "No valid command: '" << token << "'!" << std::endl;
        }
    } while (token != "quit");

    stopSearch();

    return 0;
}
