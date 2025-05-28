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

#include "consts.h"
#include "helper.h"
#include "NNUE/nnue.h"
#include "datagen.h"
#include "tune.h"
#include "search.h"
#include "tt.h"
#include "time.h"

int main(int argc, char *argv[]) {
    std::uint32_t transpositionTableSize = 16;

    tt transpositionTable(transpositionTableSize);
    Time timeManagement;
    Network net;
    Helper helper;

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

    if (argc > 1 && std::strcmp(argv[1], "bench") == 0) {
        helper.runBenchmark(search.get(), board);
        return 0;
    }

    if (argc > 1 && std::strcmp(argv[1], "datagen") == 0) {
        // Vector to hold threads
        std::vector<std::thread> threads;

        // Launch multiple threads
        for (std::uint16_t i = 0; i < 5; ++i) {
            // threads.emplace_back(std::thread([&board]()
            //  { generate(board, search, transpositionTable); }));
        }

        // Join threads to ensure they complete before exiting main
        for (std::thread &thread: threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
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
            helper.uciPrint();

#ifdef DO_TUNING
            std::cout << engineParameterToUCI();
#endif
            std::cout << "uciok" << std::endl;
        } else if (token == "stop") {
            search->shouldStop = true;
        } else if (token == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (token == "ucinewgame") {
            // Reset the board
            board.setFen(STARTPOS);

            // Clear the transposition table
            transpositionTable.clear();

            // Reset the time mangement
            timeManagement.reset();

            // Also reset all the historys
            search->resetHistory();
        } else if (token == "setoption") {
            is >> token;

            if (token == "name") {
                is >> token;
#ifdef DO_TUNING
                EngineParameter *param = findEngineParameterByName(token);
                if (param != nullptr)
                {
                    is >> token;
                    if (token == "value")
                    {
                        is >> token;
                        param->value = std::stoi(token);
                        if (param->name == "lmrBase" || param->name == "lmrDivisor")
                        {
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
            helper.handleSetPosition(board, is, token);
        } else if (token == "go") {
            helper.handleGo(*search, timeManagement, board, is, token);
        } else if (token == "d") {
            std::cout << board << std::endl;
        } else if (token == "fen") {
            std::cout << board.getFen() << std::endl;
        } else if (token == "datagen") {
            // generate(board);
        } else if (token == "bench") {
            helper.runBenchmark(search.get(), board);
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

    return 0;
}
