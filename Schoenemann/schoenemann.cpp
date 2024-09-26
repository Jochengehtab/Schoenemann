#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>
#include <cstring>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "schoenemann.h"
#include "search.h"
#include "consts.h"
#include "helper.h"
#include "nnue.h"
#include "simple-167.h"

using namespace chess;

Search seracher;
tt transpositionTabel(8);

memorystream memoryStream(simple_167_bin, simple_167_bin_len);

// Define & load the network from the stream
network net(memoryStream);

int timeLeft = 0;
int increment = 0;
int newTranspositionTableSize = 8;

// Thread-safe queue for commands
std::queue<std::string> commandQueue;
std::mutex queueMutex;
std::condition_variable queueCondVar;
std::atomic<bool> listenerActive(false);

// Thread function to listen for stop and quit commands during search
void commandListener() 
{
    std::string input;
    while (true) 
	{
        std::getline(std::cin, input);
        std::lock_guard<std::mutex> lock(queueMutex);
        commandQueue.push(input);
        queueCondVar.notify_one();
		if (input == "stop")
		{
			seracher.shouldStop = true;
		}
		

        if (input == "quit") 
		{
			seracher.shouldStop = true;
            break;
        }
    }
}

void processCommand(const std::string& cmd, Board& board) 
{
    std::istringstream is(cmd);
    std::string token;
    is >> std::skipws >> token;

    if (token == "uci") 
	{
        uciPrint();
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
            if (token == "Hash") 
			{
                is >> token;
                if (token == "value") 
				{
                    is >> token;
                    newTranspositionTableSize = std::stoi(token);
                    transpositionTabel.clear();
                    transpositionTabel.setSize(newTranspositionTableSize);
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

        for (const auto& move : moves) 
		{
            board.makeMove(uci::uciToMove(board, move));
        }
    }
    else if (token == "go") 
	{
        int number[4];
        bool hasTime = false;
        seracher.shouldStop = false;
        memset(seracher.countinuationButterfly, 0, sizeof(seracher.countinuationButterfly));

        listenerActive = true; // Activate listener to capture stop/quit during search

        is >> token;
        if (!is.good()) 
		{
            seracher.iterativeDeepening(board, true);
        }
        while (is.good()) {
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
                seracher.pvs(-32767, 32767, std::stoi(token), 0, board);
                std::cout << "bestmove " << uci::moveToUci(seracher.rootBestMove) << std::endl;
            }
            else if (token == "movetime") 
			{
                is >> token;
                timeLeft = std::stoi(token);
                seracher.iterativeDeepening(board, false);
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
                timeLeft = number[0];
                increment = number[2];
            }
            else {
                timeLeft = number[1];
                increment = number[3];
            }
            seracher.iterativeDeepening(board, false);
        }

        listenerActive = false; // Deactivate listener after search is complete
    }
    else if (token == "d") 
	{
        std::cout << board << std::endl;
    }
    else if (token == "bench") 
	{
        runBenchmark();
    }
    else if (token == "nodes") 
	{
        std::cout << seracher.nodes << std::endl;
    }
    else if (token == "ttest") 
	{
        transpositionTableTest(board);
    }
    else if (token == "eval") 
	{
        std::cout << "The evaluation is: " << net.evaluate((int)board.sideToMove()) << " cp" << std::endl;
    }
    else if (token == "test") 
	{
        testCommand();
    }
    else if (token == "stop") 
	{
        seracher.shouldStop = true;
    }
}

int uciLoop(int argc, char* argv[]) 
{

    // The main board
    Board board;

    // UCI-Command stuff
    std::string cmd;

    // Reset the board
    board.setFen(STARTPOS);

    // Disable FRC (Fisher-Random-Chess)
    board.set960(false);

    transpositionTabel.setSize(8);
    if (argc > 1 && strcmp(argv[1], "bench") == 0) 
	{
        runBenchmark();
		return 0;
    }

	// Start the listener thread
    std::thread listener(commandListener);

    // Main UCI-Loop
    while (true) 
	{
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondVar.wait(lock, [] { return !commandQueue.empty(); });
            cmd = commandQueue.front();
            commandQueue.pop();
        }

        if (cmd == "quit") 
		{
            seracher.shouldStop = true;
            break;
        }
        processCommand(cmd, board);
    }

    listener.join(); // Ensure the listener thread completes before exiting
    return 0;
}

int getTime() 
{
    return timeLeft;
}

int getIncrement() 
{
    return increment;
}
