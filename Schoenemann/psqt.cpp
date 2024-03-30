#include "psqt.h"


int psqt::getPieceBounus(Board& board, PieceType piece)
{
	int bounus = 0;
	int index = getSquareOfPiece(board, board.sideToMove(), piece);

	if (index == -1)
	{
		return 0;
	}

	if (piece == PieceType::PAWN)
	{
		bounus = pawnTable[index];
	}
	else if (piece == PieceType::KNIGHT)
	{
		bounus = knightTable[index];
	}
	else if (piece == PieceType::BISHOP)
	{
		bounus = bishopTable[index];
	}
	else if (piece == PieceType::ROOK)
	{
		bounus = rookTable[index];
	}
	else if (piece == PieceType::QUEEN)
	{
		bounus = queenTable[index];
	}
	
	return bounus;
}


int psqt::getSquareOfPiece(Board& board, Color color, PieceType piece)
{
	if (board.pieces(piece, color).getBits() == 0)
	{
		return -1;
	}

	Square target = board.pieces(piece, color).lsb();

	return target.index();
}