#include "datagen.h"

void generate(int amount) 
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

    std::ios::sync_with_stdio(false);
    transpositionTabel.setSize(16);

    // Accumulate output in the file directly
    for (int j = 0; j < amount; j++)
    {
        board.setFen(STARTPOS);

        for (int i = 0; i < 8; i++)
        {
            Movelist moveList;
            movegen::legalmoves(moveList, board);
            std::uniform_int_distribution<> dis(0, moveList.size() - 1);

            board.makeMove(moveList[dis(gen)]);
        }

        std::string outputLine[400];
        std::string resultString = "none";
        int moveCount = 0;

        for (int i = 0; i < 400; i++)
        {
            seracher.iterativeDeepening(board, true);

            Move bestMove = seracher.rootBestMove;

            std::pair<GameResultReason, GameResult> result = board.isGameOver();
            if (result.second != GameResult::NONE)
            {
                if (result.second == GameResult::DRAW)
                {
                    resultString = "0.5";
                }
                else if (result.second == GameResult::WIN && board.sideToMove() == Color::WHITE)
                {
                    resultString = "1.0";
                }
                else
                {
                    resultString = "0.0";
                }
                
                break;
            }

            if (board.inCheck() || board.isCapture(bestMove))
            {
                board.makeMove(bestMove);
                continue;
            }

            if (board.sideToMove() == Color::WHITE && seracher.scoreData >= 15000)
            {
                board.makeMove(bestMove);
                continue;
            }

            if (board.sideToMove() == Color::BLACK && seracher.scoreData <= 15000)
            {
                board.makeMove(bestMove);
                continue;
            }

            if (board.sideToMove() == Color::WHITE)
            {
                outputLine[i] = board.getFen() + " | " + std::to_string(seracher.scoreData) + " | ";
            }
            else if (board.sideToMove() == Color::BLACK)
            {
                outputLine[i] = board.getFen() + " | " + std::to_string(-seracher.scoreData) + " | ";
            }

            moveCount++;
            positions++;
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
        }

        if (amount % 25 == 0)
        {
            std::cout << "Generated: " << positions << std::endl; 
        }
    }

    transpositionTabel.clear();

    outputFile.close();
    seracher.hasNodeLimit = false;
}
