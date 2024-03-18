#include "Schoenemann.h"

#include <iostream>
#include <chrono>
#include <sstream>
#include "movegen/tables.h"
#include "movegen/position.h"
#include "movegen/types.h"
#include "Evaluate.h"
#include "Search.h"


//Computes the perft of the position for a given depth, using bulk-counting
//According to the https://www.chessprogramming.org/Perft site:
//Perft is a debugging function to walk the move generation tree of strictly legal moves to count 
//all the leaf nodes of a certain depth, which can be compared to predetermined values and used to isolate bugs
template<Color Us>
unsigned long long perft(Position& p, unsigned int depth) {
	int nmoves = 0;
	unsigned long long nodes = 0;

	MoveList<Us> list(p);

	if (depth == 1) return (unsigned long long) list.size();

	for (Move move : list) {
		p.play<Us>(move);
		nodes += perft<~Us>(p, depth - 1);
		p.undo<Us>(move);
	}

	return nodes;
}

//A variant of perft, listing all moves and for each move, the perft of the decremented depth
//It is used solely for debugging
template<Color Us>
void perftdiv(Position& p, unsigned int depth) {
	unsigned long long nodes = 0, pf;

	MoveList<Us> list(p);

	for (Move move : list) {
		std::cout << move;

		p.play<Us>(move);
		pf = perft<~Us>(p, depth - 1);
		std::cout << ": " << pf << " moves\n";
		nodes += pf;
		p.undo<Us>(move);
	}

	std::cout << "\nTotal: " << nodes << " moves\n";
}

void test_perft() {
	Position p;
	Position::set("rnbqkbnr/pppppppp/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq -", p);
	std::cout << p;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	auto n = perft<WHITE>(p, 7);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	auto diff = end - begin;

	std::cout << "Nodes: " << n << "\n";
	std::cout << "NPS: "
		<< int(n * 1000000.0 / std::chrono::duration_cast<std::chrono::microseconds>(diff).count())
		<< "\n";
	std::cout << "Time difference = "
		<< std::chrono::duration_cast<std::chrono::microseconds>(diff).count() << " [microseconds]\n";
}


int main(int argc, char* argv[]) {
	//Make sure to initialise all databases before using the library!
	initialise_all_databases();
	zobrist::initialise_zobrist_keys();
	Position pos;
	std::string token, cmd;

	pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", pos);

	do
	{
		token.clear();
		std::getline(std::cin, token);
		std::istringstream is(token);

		is >> std::skipws >> cmd;

		if (cmd == "quit") 
		{
			break;
		}
		else if (cmd == "uci") 
		{
			std::cout << "id name Schoenemann" << std::endl << "uciok" << std::endl;
		}
		else if (cmd == "isready")
		{
			std::cout << "readyok" << std::endl;
		}
		else if (cmd == "ucinewgame")
		{
			pos.set(DEFAULT_FEN, pos);
		}
		else if (token == "setoption")
		{

		}
		else if (cmd == "position") 
		{
			std::string position_cmd;
			is >> position_cmd;

			if (position_cmd == "fen")
			{
				std::string fen;
				std::getline(is, fen);  // Liest den Rest der Zeile in 'fen'
				size_t start = fen.find_first_not_of(' ');
				if (start != std::string::npos)
					fen = fen.substr(start);
				std::cout << fen << std::endl;
				pos.set(fen, pos);
				std::cout << pos;
			}
		}
		else if (cmd == "go") 
		{
			Position testPosition;
			testPosition.set("2bqk2r/rp5p/p1n1pn2/2p3p1/2P5/2P2NPP/P2P1PB1/R1BQR2K w k - 2 14", testPosition);
			std::cout << "Evaluation: " << evaluate(testPosition) << "\n" << std::endl;
			std::cout << search(5, -32767, 32767, testPosition) << "\n" << std::endl;
			std::cout << getBestMove() << std::endl;
			std::cout << "success" << std::endl;
		}
		else if (cmd == "bench")
		{
			std::cout << "10000" << std::endl;
		}

	} while (token != "stop");
	


	return 0;
}