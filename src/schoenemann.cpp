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

#include "consts.h"
#include "helper.h"
#include "NNUE/nnue.h"
#include "datagen.h"
#include "tune.h"
#include "search.h"
#include "history.h"
#include "tt.h"
#include "time.h"
#include "moveorder.h"

int main(int argc, char *argv[])
{
    std::uint32_t transpositionTableSize = 16;

    tt transpositionTable(transpositionTableSize);
    Time timeManagement;
    MoveOrder moveOrder;
    network net;
    Helper helper;

    std::unique_ptr<Search> searcher = std::make_unique<Search>(timeManagement, transpositionTable, moveOrder, net);

    // The main board
    Board board(&net);

    // UCI-Command stuff
    std::string token, cmd;

    // Reset the board
    board.setFen(STARTPOS);

    // Disable FRC (Fisher-Random-Chess)
    board.set960(false);

    // Init the LMR
    searcher->initLMR();

    transpositionTable.setSize(transpositionTableSize);
    timeManagement.reset();
    searcher->resetHistory();

    if (argc > 1 && strcmp(argv[1], "bench") == 0)
    {
        helper.runBenchmark(*searcher, board);
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "datagen") == 0)
    {
        // Vector to hold threads
        std::vector<std::thread> threads;

        // Launch multiple threads
        for (int i = 0; i < 5; ++i)
        {
            // threads.emplace_back(std::thread([&board]()
            //  { generate(board, searcher, transpositionTable); }));
        }

        // Join threads to ensure they complete before exiting main
        for (auto &thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
        return 0;
    }
    // Main UCI-Loop
    do
    {
        if (argc == 1 && !getline(std::cin, cmd))
        {
            cmd = "quit";
        }

        std::istringstream is(cmd);
        cmd.clear();
        is >> std::skipws >> token;

        if (token == "uci")
        {
            helper.uciPrint();

#ifdef DO_TUNING
            std::cout << engineParameterToUCI();
#endif
            std::cout << "uciok" << std::endl;
        }
        else if (token == "stop")
        {
            searcher->shouldStop = true;
        }
        else if (token == "isready")
        {
            std::cout << "readyok" << std::endl;
        }
        else if (token == "ucinewgame")
        {
            // Reset the board
            board.setFen(STARTPOS);

            // Clear the transposition table
            transpositionTable.clear();

            // Reset the time mangement
            timeManagement.reset();

            // Also reset all the historys
            searcher->resetHistory();
        }
        else if (token == "setoption")
        {
            is >> token;

            if (token == "name")
            {
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
                            searcher.initLMR();
                        }
                    }
                }

#endif
                if (token == "Hash")
                {
                    is >> token;
                    if (token == "value")
                    {
                        is >> token;
                        transpositionTableSize = std::stoi(token);
                        transpositionTable.clear();
                        transpositionTable.setSize(transpositionTableSize);
                    }
                }
            }
        }
        else if (token == "position")
        {
            board.setFen(STARTPOS);
            std::string fen;
            std::vector<std::string> moves;
            bool isFen = false;
            while (is >> token)
            {
                if (token == "fen")
                {
                    isFen = true;
                    while (is >> token && token != "moves")
                    {
                        fen += token + " ";
                    }
                    fen = fen.substr(0, fen.size() - 1);
                    board.setFen(fen);
                }
                else if (token != "moves" && isFen)
                {
                    moves.push_back(token);
                }
                else if (token == "startpos")
                {
                    board.setFen(STARTPOS);
                    isFen = true;
                }
            }

            for (const std::string &move : moves)
            {
                board.makeMove(uci::uciToMove(board, move));
            }
        }
        else if (token == "go")
        {
            int number[4];
            bool hasTime = false;
            searcher->shouldStop = false;

            is >> token;
            if (!is.good())
            {
                std::thread t1([&] { searcher->iterativeDeepening(board, true); });
                    t1.detach();
            }
            while (is.good())
            {
                if (token == "wtime")
                {
                    is >> token;
                    number[0] = std::stoi(token);
                    hasTime = true;
                }
                else if (token == "btime")
                {
                    is >> token;
                    number[1] = std::stoi(token);
                    hasTime = true;
                }
                else if (token == "winc")
                {
                    is >> token;
                    number[2] = std::stoi(token);
                }
                else if (token == "binc")
                {
                    is >> token;
                    number[3] = std::stoi(token);
                }
                else if (token == "depth")
                {
                    is >> token;
                    std::thread t1([&] { searcher->pvs(-infinity, infinity, std::stoi(token), 0, board, false); });
                    t1.detach();
                    
                    std::cout << "bestmove " << uci::moveToUci(searcher->rootBestMove) << std::endl;
                }
                else if (token == "nodes")
                {
                    is >> token;
                    searcher->hasNodeLimit = true;
                    searcher->nodeLimit = std::stoi(token);
                    std::thread t1([&]
                                   { searcher->iterativeDeepening(board, true); });
                    t1.detach();
                }
                else if (token == "movetime")
                {
                    is >> token;
                    timeManagement.timeLeft = std::stoi(token);
                    std::thread t1([&]
                                   { searcher->iterativeDeepening(board, false); });
                    t1.detach();
                }
                if (!(is >> token))
                {
                    break;
                }
            }
            if (hasTime)
            {
                if (board.sideToMove() == Color::WHITE)
                {
                    timeManagement.timeLeft = number[0];
                    timeManagement.increment = number[2];
                }
                else
                {
                    timeManagement.timeLeft = number[1];
                    timeManagement.increment = number[3];
                }

                std::thread t1([&]
                               { searcher->iterativeDeepening(board, false); });
                t1.detach();
            }
        }
        else if (token == "d")
        {
            std::cout << board << std::endl;
        }
        else if (token == "datagen")
        {
            // generate(board);
        }
        else if (token == "bench")
        {
            helper.runBenchmark(*searcher, board);
        }
        else if (token == "eval")
        {
            std::cout << "The raw eval is: " << net.evaluate((int)board.sideToMove(), board.occ().count()) << std::endl;
            std::cout << "The scaled evaluation is: " << searcher->scaleOutput(net.evaluate((int)board.sideToMove(), board.occ().count()), board) << " cp" << std::endl;
        }
        else if (token == "spsa")
        {
            std::cout << engineParameterToSpsaInput() << std::endl;
        }
        else if (token == "stop")
        {
            searcher->shouldStop = true;
        }
        else
        {
            std::cout << "No valid command: '" << token << "'!" << std::endl;
        }
    } while (token != "quit");

    return 0;
}