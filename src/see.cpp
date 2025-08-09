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
#include "tune.h"

bool SEE::see(const Board &board, const Move &move, const int cutoff) {
    // We get our initial score and check if it is below zero.
    // If that is the case then this is bad for us
    int score = getPieceValue(board, move) - cutoff;
    if (score < 0) {
        return false;
    }

    PieceType next = move.typeOf() == Move::PROMOTION ? move.promotionType() : board.at<PieceType>(move.from());
    score -= *SEE_PIECE_VALUES[next];

    if (score >= 0) {
        return true;
    }

    // Our side is the opposite of the current side to move
    // since we make a move we must revert the side to move
    Color us = ~board.sideToMove();

    // In our occupied bitboard we make the move so we turn off the 'from' bit
    // and turn on the 'to' bit
    Bitboard occupancy = board.occ() ^ 1ULL << move.from().index() ^ 1ULL << move.to().index();

    // Since sliding attacks can cause problems we embed them into the bishops
    // and rooks attack table
    const Bitboard queens = board.pieces(PieceType::QUEEN);
    const Bitboard bishops = queens | board.pieces(PieceType::BISHOP);
    const Bitboard rooks = queens | board.pieces(PieceType::ROOK);

    const Square square = move.to();

    // Initialize the attack table with every attacker from every side to the target square
    Bitboard attackers;
    attackers |= board.pieces(PieceType::PAWN, Color::BLACK) & attacks::pawn(Color::WHITE, square);
    attackers |= board.pieces(PieceType::PAWN, Color::WHITE) & attacks::pawn(Color::BLACK, square);
    attackers |= board.pieces(PieceType::KNIGHT) & attacks::knight(square);
    attackers |= rooks & attacks::rook(square, occupancy);
    attackers |= bishops & attacks::bishop(square, occupancy);
    attackers |= board.pieces(PieceType::KING) & attacks::king(square);

    while (true) {
        // We make a copy of the attack bitboard with only our pieces
        Bitboard ourAttackers = attackers & board.us(us);

        // If we don't have any pieces on the board we finished the SEE loop
        // since one side has no more attackers on the square
        if (ourAttackers == 0) {
            break;
        }

        // We get our attacker that we want to remove from the bitboard
        next = getLeastValuableAttacker(board, occupancy, ourAttackers, us);

        // Remove the attacker from the bitboard
        if (next == PieceType::PAWN || next == PieceType::BISHOP || next == PieceType::QUEEN) {
            attackers |= attacks::bishop(square, occupancy) & bishops;
        }

        if (next == PieceType::ROOK || next == PieceType::QUEEN) {
            attackers |= attacks::rook(square, occupancy) & rooks;
        }

        attackers &= occupancy;

        // Update the score
        score = -score - 1 - *SEE_PIECE_VALUES[next];

        // Flip the side
        us = ~us;

        // Since we negate the score every time in the loop, we are finished if the score is
        // equal or greater to zero
        if (score >= 0) {
            // If our only attacker is our king, but the opponent still has defenders
            if (next == PieceType::KING && attackers & board.us(us)) {
                us = ~us;
            }
            break;
        }
    }

    return board.sideToMove() != us;
}

int SEE::getPieceValue(const Board &board, const Move &move) {
    const std::uint16_t moveType = move.typeOf();

    // If the move is castling we return nothing because two pieces where moved
    if (moveType == Move::CASTLING) {
        return 0;
    }

    // If the move is en passant we know that the piece is a pawn
    if (moveType == Move::ENPASSANT) {
        return *SEE_PIECE_VALUES[static_cast<int>(PieceType::PAWN)];
    }

    // Get our score for the moved piece
    int score = *SEE_PIECE_VALUES[board.at<PieceType>(move.to())];

    // If the move is a promotion we want to add the promoted piece
    // minus a pawn which got promoted
    if (moveType == Move::PROMOTION) {
        score += *SEE_PIECE_VALUES[move.promotionType()] - *SEE_PIECE_VALUES[static_cast<int>(PieceType::PAWN)];
    }

    return score;
}

PieceType SEE::getLeastValuableAttacker(const Board &board, Bitboard &occ, const Bitboard &attackers,
                                        const Color color) {
    for (int piece = 0; piece <= 5; piece++) {
        if (const Bitboard bitboard = attackers & board.pieces(static_cast<PieceType>(piece), color)) {
            occ ^= 1ULL << bitboard.lsb();
            return static_cast<PieceType>(piece);
        }
    }

    // If no attacker is found we return an empty piece
    return PieceType::NONE;
}
