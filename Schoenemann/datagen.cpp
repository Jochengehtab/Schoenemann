#include "datagen.h"

void generate(Board& board) 
{
    // Set up the nodes limit
    seracher.hasNodeLimit = true;
    seracher.nodeLimit = 5000;

    // Initialize stuff for randomnes
    std::random_device rd; 
    std::mt19937 gen(rd());

    // Open the outputfile
    std::ofstream outputFile("output.txt", std::ios::app);

    // Check if the file is open
    if (!outputFile.is_open())
    {
        std::cout << "Error opening output file!" << std::endl;
        return;
    }

    // Disable synchronisation
    std::ios::sync_with_stdio(false);

    // Set TT-Size
    transpositionTabel.setSize(4);

    // Needed for logging
    std::uint64_t counter = 0;
    int positions = 0;

    auto startTime = std::chrono::steady_clock::now();

    while (true)
    {
        counter++;

        // Reset the bboard
        board.setFen(STARTPOS);

        bool exitEarly = false;

        // Play random moves
        for (int i = 0; i < 8; i++)
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

            // Make the random move
            board.makeMove(moveList[dis(gen)]);
        }

        // If we got an early exit we continue
        if (exitEarly) 
        {
            continue;
        }

        // Initialize values that are usefull later
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
                
                // We check if it is a win or a lose for whtie
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

            // Generate legal moves
            Movelist moveList;
            movegen::legalmoves(moveList, board);

            // Search for 5000 nodes
            seracher.iterativeDeepening(board, true);

            // Get the best move
            Move bestMove = seracher.rootBestMove;

            // Chck if the move is illegal the we wanna make
            if (board.at(bestMove.from()) == Piece::NONE || !(board.at(bestMove.from()) < Piece::BLACKPAWN) == (board.sideToMove() == Color::WHITE))
            {
                isIllegal = true;
                break;
            }

            // We skip check moves, captures and if the score is to high for any side
            if (board.inCheck() || 
                board.isCapture(bestMove) || 
                (board.sideToMove() == Color::WHITE && seracher.scoreData >= 15000) || 
                (board.sideToMove() == Color::BLACK && seracher.scoreData <= 15000))
            {
                board.makeMove(bestMove);
                continue;
            }

            // We create the output string based on whites perspective
            if (board.sideToMove() == Color::WHITE)
            {
                outputLine[i] = board.getFen() + " | " + std::to_string(seracher.scoreData) + " | ";
            }
            else if (board.sideToMove() == Color::WHITE)
            {
                outputLine[i] = board.getFen() + " | " + std::to_string(-seracher.scoreData) + " | ";
            }

            // Count up the position
            moveCount++;

            // Make the move on the board
            board.makeMove(bestMove);
        }

        // If something has interruped our FEN-Gen we continue
        if (resultString == "none" || isIllegal)
        {
            continue;
        }

        // Write the output to the file
        for (int i = 0; i < std::min(moveCount, 500); i++)
        {   
            // If empty we don't wanna write to the output file
            if (outputLine[i].empty())
            {
                continue;
            }
            
            // Write to the file
            outputFile << outputLine[i] + resultString + "\n";

            // Increment the written positions
            positions++;
        }

        // Every 100 iterations we wanna print stats
        if (counter % 100 == 0)
        {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
            double positionsPerSecond = static_cast<double>(positions) / elapsedTime;
            std::cout << "Generated: " << positions << " positions | " << "PPS: " <<(int) positionsPerSecond << std::endl;
        }
    }
    
    // Reset everything
    transpositionTabel.clear();
    outputFile.close();
    seracher.hasNodeLimit = false;
}
