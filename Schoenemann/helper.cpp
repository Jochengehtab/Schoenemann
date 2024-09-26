#include "helper.h"

void transpositionTableTest(Board& board)
{
	//Set up a unice position
	board.setFen("3N4/2p5/5K2/k1PB3p/3Pr3/1b5p/6p1/5nB1 w - - 0 1");
	std::uint64_t key = board.hash();

	//Store the information
	transpositionTabel.storeEvaluation(key, 2, LOWER_BOUND, transpositionTabel.scoreToTT(200, 1), uci::uciToMove(board, "d5e4"), 1);

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
	Board testBoard;
	testBoard.setFen("8/4p3/8/8/8/8/8/8 w - - 0 1");
}

//Print the uci info
void uciPrint()
{
	std::cout << "id name Schoenemann" << std::endl
		<< "option name Hash type spin default 64 min 1 max 4096" << std::endl
		<< "uciok" << std::endl;
}

void runBenchmark() {
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
