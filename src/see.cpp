/*
  This file is part of the Schoenemann chess engine written by Jochengehtab

  Copyright (C) 2024-2025 Jochengehtab

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "see.h"

int getPieceValue(const Board &board, Move &move)
{
    std::uint16_t moveType = move.typeOf();

    if (moveType == move.CASTLING)
    {
        return 0;
    }

    if (moveType == move.ENPASSANT)
    {
        return SEE_PIECE_VALUES[0];
    }

    int score = SEE_PIECE_VALUES[board.at<PieceType>(move.to())];

    if (moveType == move.PROMOTION)
    {
        score += SEE_PIECE_VALUES[move.promotionType()] - SEE_PIECE_VALUES[0];
    }

    return score;
}

// SEE prunning by Starzix
bool see(const Board &board, Move &move, int cutoff)
{
    int score = getPieceValue(board, move) - cutoff;
    if (score < 0)
    {
        return false;
    }

    PieceType next = (move.typeOf() == move.PROMOTION) ? move.promotionType() : board.at<PieceType>(move.from());
    score -= SEE_PIECE_VALUES[next];

    if (score >= 0)
    {
        return true;
    }

    int from = move.from().index();
    int to = move.to().index();

    Bitboard occupancy = board.occ() ^ (1ULL << from) ^ (1ULL << to);
    Bitboard queens = board.pieces(PieceType::QUEEN);
    Bitboard bishops = queens | board.pieces(PieceType::BISHOP);
    Bitboard rooks = queens | board.pieces(PieceType::ROOK);

    Square square = move.to();

    Bitboard attackers = 0;
    attackers |= rooks & attacks::rook(square, occupancy);
    attackers |= bishops & attacks::bishop(square, occupancy);
    attackers |= board.pieces(PieceType::PAWN, Color::BLACK) & attacks::pawn(Color::WHITE, square);
    attackers |= board.pieces(PieceType::PAWN, Color::WHITE) & attacks::pawn(Color::BLACK, square);
    attackers |= board.pieces(PieceType::KNIGHT) & attacks::knight(square);
    attackers |= board.pieces(PieceType::KING) & attacks::king(square);

    Color us = ~board.sideToMove();
    while (true)
    {
        Bitboard ourAttackers = attackers & board.us(us);
        if (ourAttackers == 0)
        {
            break;
        }

        next = getLeastValuableAttacker(board, occupancy, ourAttackers, us);

        if (next == PieceType::PAWN || next == PieceType::BISHOP || next == PieceType::QUEEN)
        {
            attackers |= attacks::bishop(square, occupancy) & bishops;
        }

        if (next == PieceType::ROOK || next == PieceType::QUEEN)
        {
            attackers |= attacks::rook(square, occupancy) & rooks;
        }

        attackers &= occupancy;
        score = -score - 1 - SEE_PIECE_VALUES[next];
        us = ~us;

        if (score >= 0)
        {
            // If our only attacker is our king, but the opponent still has defenders
            if (next == PieceType::KING && (attackers & board.us(us)).getBits() > 0)
            {
                us = ~us;
            }
            break;
        }
    }

    return board.sideToMove() != us;
}

PieceType getLeastValuableAttacker(const Board &board, Bitboard &occ, Bitboard attackers, Color color)
{
    for (int piece = 0; piece <= 5; piece++)
    {
        Bitboard bitboard = attackers & board.pieces((PieceType)piece, color);
        if (bitboard.getBits() > 0)
        {
            occ ^= (1ULL << bitboard.lsb());
            return (PieceType)piece;
        }
    }

    // If no attacker is found we return an empty piece
    return PieceType::NONE;
}