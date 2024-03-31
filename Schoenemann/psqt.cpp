#include "psqt.h"


int psqt::getPieceBounus(Board& board, PieceType piece, Color color)
{
	int bounus = 0;
	Bitboard pieces = board.pieces(piece, color);
	std::vector<int> indexs = getSetBitsEfficient(pieces);

	if (piece == PieceType::PAWN)
	{
		for (int index : indexs)
		{
			bounus += pawnTable[index];
		}
	}
	else if (piece == PieceType::KNIGHT)
	{
		for (int index : indexs)
		{
			bounus += knightTable[index];
		}
	}
	else if (piece == PieceType::BISHOP)
	{
		for (int index : indexs)
		{
			bounus += bishopTable[index];
		}
	}
	else if (piece == PieceType::ROOK)
	{
		for (int index : indexs)
		{
			bounus += rookTable[index];
		}
	}
	else if (piece == PieceType::QUEEN)
	{
		for (int index : indexs)
		{
			bounus += queenTable[index];
		}
	}
	
	return bounus;
}

std::vector<int> psqt::getSetBitsEfficient(Bitboard bitboard) {
	std::vector<int> setBits;
	for (int i = 0; i < 64; ++i) {
		if ((bitboard.getBits() >> i) & 1ULL) {
			setBits.push_back(i);
		}
	}
	return setBits;
}