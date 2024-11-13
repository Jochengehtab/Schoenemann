#include "datagen.h"

void generate(Board& board) 
{
    seracher.hasNodeLimit = true;
    seracher.nodeLimit = 5000;
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
    std::uint64_t counter = 0;

    auto startTime = std::chrono::steady_clock::now();

    while (true)
    {
        counter++;
        board.setFen(STARTPOS);

        bool exitEarly = false;

        for (int i = 0; i < 8; i++)
        {
            Movelist moveList;
            movegen::legalmoves(moveList, board);
            
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
            
            std::uniform_int_distribution<> dis(0, moveList.size() - 1);

            board.makeMove(moveList[dis(gen)]);
        }

        if (exitEarly) 
        {
            continue;
        }

        std::string outputLine[501];
        std::string resultString = "none";
        int moveCount = 0;
        bool isIllegal = false;

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

            Movelist moveList;
            movegen::legalmoves(moveList, board);

            seracher.iterativeDeepening(board, true);

            Move bestMove = seracher.rootBestMove;

            if (board.at(bestMove.from()) == Piece::NONE || !(board.at(bestMove.from()) < Piece::BLACKPAWN) == (board.sideToMove() == Color::WHITE))
            {
                isIllegal = true;
                break;
            }

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

            moveCount++;
            board.makeMove(bestMove);
        }

        if (resultString == "none" || isIllegal)
        {
            continue;
        }

        for (int i = 0; i < std::min(moveCount, 500); i++)
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
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
            double positionsPerSecond = static_cast<double>(positions) / elapsedTime;
            std::cout << "Generated: " << positions << " positions | " << "PPS: " <<(int) positionsPerSecond << std::endl;
        }

        seracher.nodes = 0;
    }

    transpositionTabel.clear();

    outputFile.close();
    seracher.hasNodeLimit = false;
}
