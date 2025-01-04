#include "datagen.h"

void generate(Board &board)
{
    // Set up the nodes limit
    searcher.hasNodeLimit = true;
    searcher.nodeLimit = 10000;

    // Initialize stuff for random moves
    std::random_device rd;
    std::mt19937 gen(rd());

    // Open the outfile
    std::ofstream outputFile("outputt.txt", std::ios::app);

    // Check if the file is open
    if (!outputFile.is_open())
    {
        std::cout << "Error opening output file!" << std::endl;
        return;
    }

    // Set TT-Size
    transpositionTabel.setSize(16);

    // Needed for logging
    std::uint64_t counter = 0;
    int positions = 0;

    auto startTime = std::chrono::steady_clock::now();

    while (true)
    {
        counter++;

        // Reset the board
        board.setFen(STARTPOS);

        bool exitEarly = false;

        // Play random moves
        for (int i = 0; i < 10; i++)
        {
            // Generate all legal moves
            Movelist moveList;
            movegen::legalmoves(moveList, board);

            // Check if the game ended already
            std::pair<GameResultReason, GameResult> result = board.isGameOver();
            if (result.second != GameResult::NONE)
            {
                exitEarly = true;
                break;
            }

            if (moveList.size() == 0)
            {
                exitEarly = true;
                break;
            }

            // Pick a random move
            std::uniform_int_distribution<> dis(0, moveList.size() - 1);

            // Choose a random move
            Move move = moveList[dis(gen)];
            if (!see(board, move, 0))
            {
                exitEarly = true;
                break;
            }

            // Make the random move
            board.makeMove(move);
        }

        // If we got an early exit we continue
        if (exitEarly)
        {
            continue;
        }

        // Initialize values that are usefully later
        std::string outputLine[501];
        std::string resultString = "none";
        int moveCount = 0;
        bool isIllegal = false;

        for (int i = 0; i < 500; i++)
        {
            // Check if the game is over
            std::pair<GameResultReason, GameResult> result = board.isGameOver();
            if (result.second != GameResult::NONE)
            {
                if (result.second == GameResult::DRAW)
                {
                    resultString = "0.5";
                }

                // We check if it is a win or a loose for white
                if (result.second == GameResult::LOSE && board.sideToMove() == Color::BLACK)
                {
                    resultString = "1.0";
                }
                else
                {
                    resultString = "0.0";
                }

                break;
            }

            // Search for 5000 nodes
            searcher.iterativeDeepening(board, true);

            // Get the best move
            Move bestMove = searcher.rootBestMove;

            // Check if the move is illegal the want to make
            if (board.at(bestMove.from()) == Piece::NONE || !(board.at(bestMove.from()) < Piece::BLACKPAWN) == (board.sideToMove() == Color::WHITE))
            {
                isIllegal = true;
                break;
            }

            // We skip check moves, captures and if the score is to high for any side
            if (bestMove.typeOf() == Move::PROMOTION ||
                board.inCheck() ||
                board.isCapture(bestMove) ||
                (board.sideToMove() == Color::WHITE && searcher.scoreData >= 10000) ||
                (board.sideToMove() == Color::BLACK && searcher.scoreData <= 10000))
            {
                board.makeMove(bestMove);
                continue;
            }

            // We create the output string based on whites perspective
            if (board.sideToMove() == Color::WHITE)
            {
                outputLine[i] = board.getFen() + " | " + std::to_string(searcher.scoreData) + " | ";
            }
            else if (board.sideToMove() == Color::WHITE)
            {
                outputLine[i] = board.getFen() + " | " + std::to_string(-searcher.scoreData) + " | ";
            }

            // Count up the position
            moveCount++;

            // Make the move on the board
            board.makeMove(bestMove);
        }

        // If something has interrupted our FEN-Gen we continue
        if (resultString == "none" || isIllegal)
        {
            continue;
        }

        // Write the output to the file
        for (int i = 0; i < std::min(moveCount, 500); i++)
        {
            // If empty we don't want to write to the output file
            if (outputLine[i].empty())
            {
                continue;
            }

            // Write to the file
            outputFile << outputLine[i] + resultString + "\n";

            // Increment the written positions
            positions++;
        }

        // Every 1000 iterations we want to print stats
        if (counter % 10 == 0)
        {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
            double positionsPerSecond = static_cast<double>(positions) / elapsedTime;
            std::cout << "Generated: " << positions << " positions | " << "PPS: " << (int)positionsPerSecond << std::endl;
        }
    }

    // Reset everything
    transpositionTabel.clear();
    outputFile.close();
    searcher.hasNodeLimit = false;
}
