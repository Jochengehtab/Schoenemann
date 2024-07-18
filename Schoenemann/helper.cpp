#include "helper.h"

void transpositionTableTest(Board& board)
{
	//Set up a unice position
	board.setFen("3N4/2p5/5K2/k1PB3p/3Pr3/1b5p/6p1/5nB1 w - - 0 1");
	std::uint64_t key = board.hash();

	//Store the information
	transpositionTabel.storeEvaluation(key, 2, LOWER_BOUND, transpositionTabel.ScoreToTT(200, 1), uci::uciToMove(board, "d5e4"), 1);

	//Try to get the information out of the table
	Hash* entry = transpositionTabel.getHash(key);

	if (entry == nullptr)
	{
		std::cout << "The entry is a nullptr" << std::endl;
		return;
	}
	std::uint64_t hashedKey = entry->key;
	short hashedDepth = entry->depth;
	short hashedType = entry->type;
	int hashedScore = entry->score;
	Move hashedMove = entry->move;

	if (hashedKey == key)
	{
		std::cout << "Key PASSED." << std::endl;
	}
	else
	{
		std::cout << "Key FAILED." << "Original key: \n" << key << "\nHash key: \n" << hashedKey << std::endl;
	}

	if (hashedDepth == 2)
	{
		std::cout << "Depth PASSED." << std::endl;
	}
	else
	{
		std::cout << "Depth FAILED." << "Original depth: 2" << "\nHash key: " << hashedDepth << std::endl;
	}

	if (hashedType == LOWER_BOUND)
	{
		std::cout << "Type PASSED." << std::endl;
	}
	else
	{
		std::cout << "Type FAILED." << "Original type: 2" << "\nHash type: " << hashedType << std::endl;
	}

	if (hashedScore == 200)
	{
		std::cout << "Score PASSED." << std::endl;
	}
	else
	{
		std::cout << "Score FAILED." << "Original score: 200" << "\nHash score: " << hashedScore << std::endl;
	}

	if (hashedMove == uci::uciToMove(board, "d5e4"))
	{
		std::cout << "Move PASSED." << std::endl;
	}
	else
	{
		std::cout << "Move FAILED." << "Original move: d5e4" << "\nHash move: " << hashedMove << std::endl;
	}
	board.setFen(STARTPOS);
}

void testCommand()
{
	Board test_board;
	test_board.setFen("8/4R3/6kp/6p1/8/7P/3r4/6K1 w - - 0 28");
}

void nnTest(Board& board)
{
	board.setFen(STARTPOS);
	std::cout << "Evaluation result: " << nnueEvaluation(board) << std::endl;
	board.setFen("8/2q5/8/8/8/4K2k/8/8 w - - 0 1");
	std::cout << "Evaluation result: " << nnueEvaluation(board) << std::endl;
	board.setFen("rnbqkbnr/ppppp3/6p1/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 4");
	std::cout << "Evaluation result: " << nnueEvaluation(board) << std::endl;
	board.setFen(STARTPOS);
}

//Print the uci info
void uciPrint()
{
	std::cout << "id name Schoenemann" << std::endl
		<< "option name Threads type spin default 1 min 1 max 16" << std::endl
		<< "option name Hash type spin default 64 min 1 max 4096" << std::endl
		<< "option name rvpDepth type spin default 6 min 2 max 10" << std::endl

		<< "option name betaReduction type spin default 70 min 20 max 500" << std::endl

		<< "option name alphaReduction type spin default 334 min 100 max 1200" << std::endl

		<< "option name razorDepth type spin default 2 min 1 max 5" << std::endl

		<< "option name depthMultiply type spin default 66 min 10 max 600" << std::endl

		<< "option name pawnValue type spin default 126 min 50 max 600" << std::endl

		<< "option name knightValue type spin default 781 min 300 max 2500" << std::endl

		<< "option name bishopValue type spin default 825 min 300 max 2600" << std::endl

		<< "option name rookValue type spin default 1276 min 500 max 3000" << std::endl

		<< "option name queenValue type spin default 2538 min 1250 max 4500" << std::endl

		<< "uciok" << std::endl;
}

void run_benchmark() {
	//Setting up the bench Board
	Board benchBoard;

	//Setting up the clock 
	auto start = std::chrono::high_resolution_clock::now();

	//Reseting the nodes
	seracher.nodes = 0;

	//Looping over all bench positions
	for (const auto& test : testStrings) {
		benchBoard.setFen(test);
		seracher.pvs(-infinity, infinity, benchDepth, 0, benchBoard);
	}

	auto end = std::chrono::high_resolution_clock::now();

	//Calculates the total time used
	std::chrono::duration<double, std::milli> timeElapsed = end - start;
	int timeInMs = static_cast<int>(timeElapsed.count());

	//calculates the Nodes per Second
	int NPS = static_cast<int>(seracher.nodes / timeElapsed.count() * 1000);

	//Prints out the final bench 
	std::cout << "Time  : " << timeInMs << " ms\nNodes : " << seracher.nodes << "\nNPS   : " << NPS << std::endl;
}
