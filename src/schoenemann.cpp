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

#include "consts.h"
#include "helper.h"
#include "nnue.h"
#include "datagen.h"
#include "tune.h"

Search searcher;
tt transpositionTabel(8);
History history;
Time timeManagement;
MoveOrder moveOrder;

network net;

int transpositionTableSize = 8;

int main(int argc, char *argv[])
{
    // The main board
    Board board;

    // UCI-Command stuff
    std::string token, cmd;

    // Reset the board
    board.setFen(STARTPOS);

    // Disable FRC (Fisher-Random-Chess)
    board.set960(false);

    // Init the LMR
    searcher.initLMR();

    transpositionTabel.setSize(8);

    if (argc > 1 && strcmp(argv[1], "bench") == 0)
    {
        runBenchmark();
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "datagen") == 0)
    {
        // Vector to hold threads
        std::vector<std::thread> threads;

        // Launch multiple threads
        for (int i = 0; i < 5; ++i)
        {
            threads.emplace_back(std::thread([&board]()
                                             { generate(board); }));
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
            uciPrint();

#ifdef DO_TUNING
            std::cout << engineParameterToUCI();
#endif
            std::cout << "uciok" << std::endl;
        }
        else if (token == "stop")
        {
            searcher.shouldStop = true;
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
            transpositionTabel.clear();
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
                        transpositionTabel.clear();
                        transpositionTabel.setSize(transpositionTableSize);
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

            for (const auto &move : moves)
            {
                board.makeMove(uci::uciToMove(board, move));
            }
        }
        else if (token == "go")
        {
            int number[4];
            bool hasTime = false;
            searcher.shouldStop = false;

            is >> token;
            if (!is.good())
            {
                searcher.iterativeDeepening(board, true);
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
                    searcher.pvs(-infinity, infinity, std::stoi(token), 0, board, false);
                    std::cout << "bestmove " << uci::moveToUci(searcher.rootBestMove) << std::endl;
                }
                else if (token == "nodes")
                {
                    is >> token;
                    searcher.hasNodeLimit = true;
                    searcher.nodeLimit = std::stoi(token);
                    searcher.iterativeDeepening(board, true);
                }
                else if (token == "movetime")
                {
                    is >> token;
                    timeManagement.timeLeft = std::stoi(token);
                    std::thread t1(std::bind(&Search::iterativeDeepening, &searcher, board, false));
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
                searcher.iterativeDeepening(board, false);
            }
        }
        else if (token == "d")
        {
            std::cout << board << std::endl;
        }
        else if (token == "datagen")
        {
            generate(board);
        }
        else if (token == "bench")
        {
            runBenchmark();
        }
        else if (token == "eval")
        {
            std::cout << "The raw eval is: " << net.evaluate((int)board.sideToMove(), board.occ().count()) << std::endl;
            std::cout << "The scaled evaluation is: " << searcher.scaleOutput(net.evaluate((int)board.sideToMove(), board.occ().count()), board) << " cp" << std::endl;
        }
        else if (token == "test")
        {
            testCommand();
        }
        else if (token == "spsa")
        {
            std::cout << engineParameterToSpsaInput() << std::endl;
        }
        else if (token == "stop")
        {
            searcher.shouldStop = true;
        }
    } while (token != "quit");
    return 0;
}