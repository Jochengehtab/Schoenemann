#include <algorithm>

#include "psqt.h"


int psqt::getPieceBounus(Board& board, PieceType piece, Color color)
{
	const Bitboard pieces = board.pieces(piece, color);
	const bool isWhite = (color == Color::WHITE) ? true : false;

	return bounusCalculation(piece, pieces, isWhite);
}

short psqt::bounusCalculation(PieceType type, Bitboard bitboard, const bool isWhite)
{
	if (type == PieceType::PAWN)
	{
		if (isWhite)
		{
			for (short i = 0; i < 64; i++)
			{
				
				if (bitboard.check(i))
				{
					return pawnTable[63 - i];
				}
			}
		}
		else
		{
			for (short i = 0; i < 64; i++)
			{
				if (bitboard.check(i))
				{
					return pawnTable[i];
				}
			}
		}
	}
	else if(type == PieceType::KNIGHT)
	{
		if (isWhite)
		{
			for (short i = 0; i < 64; i++)
			{

				if (bitboard.check(i))
				{
					return knightTable[63 - i];
				}
			}
		}
		else
		{
			for (short i = 0; i < 64; i++)
			{
				if (bitboard.check(i))
				{
					return knightTable[i];
				}
			}
		}
	}
	else if (type == PieceType::BISHOP)
	{
		if (isWhite)
		{
			for (short i = 0; i < 64; i++)
			{

				if (bitboard.check(i))
				{
					return bishopTable[63 - i];
				}
			}
		}
		else
		{
			for (short i = 0; i < 64; i++)
			{
				if (bitboard.check(i))
				{
					return bishopTable[i];
				}
			}
		}
	}
	else if (type == PieceType::ROOK)
	{
		if (isWhite)
		{
			for (short i = 0; i < 64; i++)
			{

				if (bitboard.check(i))
				{
					return rookTable[i];
				}
			}
		}
		else
		{
			for (short i = 0; i < 64; i++)
			{
				if (bitboard.check(i))
				{
					return rookTable[63 - i];
				}
			}
		}
	}
	else if (type == PieceType::QUEEN)
	{
		if (isWhite)
		{
			for (short i = 0; i < 64; i++)
			{

				if (bitboard.check(i))
				{
					return queenTable[63 - i];
				}
			}
		}
		else
		{
			for (short i = 0; i < 64; i++)
			{
				if (bitboard.check(i))
				{
					return queenTable[i];
				}
			}
		}
	}
	else if (type == PieceType::KING)
	{
		const bool isEndgame = bitboard.count() < 10;
		if (isWhite)
		{
			for (short i = 0; i < 64; i++)
			{

				if (bitboard.check(i))
				{
					if (isEndgame)
					{
						return endgameKingTable[63 - i];
					}
					else
					{
						return middlegameKingTable[63 - i];
					}
				}
			}
		}
		else
		{
			for (short i = 0; i < 64; i++)
			{
				if (bitboard.check(i))
				{
					if (isEndgame)
					{
						return endgameKingTable[i];
					}
					else
					{
						return middlegameKingTable[i];
					}
				}
			}
		}
	}
	return 0;
}