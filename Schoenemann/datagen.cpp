#include "datagen.h"

void generate() 
{
    Board board;

    seracher.hasNodeLimit = true;
    seracher.nodeLimit = 8000;
    int positions = 0;

    std::random_device rd;  
    std::mt19937 gen(rd());  
    std::ofstream outputFile("output.txt", std::ios::app);

    if (!outputFile.is_open())
    {
        std::cout << "Error opening output file!" << std::endl;
        return;
    }

    transpositionTabel.setSize(16);
    std::uint64_t counter = 0;

    // Accumulate output in the file directly
    while (true)
    {
        counter++;
        board.setFen(STARTPOS);

        for (int i = 0; i < 8; i++)
        {
            Movelist moveList;
            movegen::legalmoves(moveList, board);
            
            if (moveList.size() == 0)
            {
                break;
            }
            
            std::uniform_int_distribution<> dis(0, moveList.size() - 1);

            board.makeMove(moveList[dis(gen)]);
        }

        if (board.fullMoveNumber() < 5) 
        {
            continue;
        }

        std::string outputLine[502];
        std::string resultString = "none";
        int moveCount = 0;

        for (int i = 0; i < 500; i++)
        {
            std::pair<GameResultReason, GameResult> result = board.isGameOver();
            if (result.second != GameResult::NONE)
            {
                if (result.second == GameResult::DRAW)
                {
                    resultString = "0.5";
                }
                
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

            seracher.iterativeDeepening(board, true);

            Move bestMove = seracher.rootBestMove;

            if (board.inCheck() || 
                board.isCapture(bestMove) || 
                (board.sideToMove() == Color::WHITE && seracher.scoreData >= 15000) || 
                (board.sideToMove() == Color::BLACK && seracher.scoreData <= 15000))
            {
                board.makeMove(bestMove);
                continue;
            }

            if (board.sideToMove() == Color::WHITE)
            {
                outputLine[i] = board.getFen() + " | " + std::to_string(seracher.scoreData) + " | ";
            }
            else if (board.sideToMove() == Color::WHITE)
            {
                outputLine[i] = board.getFen() + " | " + std::to_string(-seracher.scoreData) + " | ";
            }
            else 
            {
                break;
            }

            moveCount++;
            board.makeMove(bestMove);
        }

        if (resultString == "none")
        {
            continue;
        }

        for (int i = 0; i < std::min(moveCount, 400); i++)
        {   
            if (outputLine[i].empty())
            {
                continue;
            }
            
            outputFile << outputLine[i] + resultString + "\n";
            positions++;
        }

        if (counter % 100 == 0)
        {
            std::cout << "Generated: " << positions << std::endl; 
        }
    }

    transpositionTabel.clear();

    outputFile.close();
    seracher.hasNodeLimit = false;
}
