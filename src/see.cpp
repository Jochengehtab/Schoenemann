#include "see.h"


int getPieceValue(const Board &board, Move &move)
{
    auto moveType = move.typeOf();

    if (moveType == move.CASTLING)
    {
        return 0;
    }

    if (moveType == move.ENPASSANT)
    {
        return SEE_PIECE_VALUES[0];
    }

    int score = SEE_PIECE_VALUES[(int)board.at<PieceType>(move.to())];

    if (moveType == move.PROMOTION)
    {
        score += SEE_PIECE_VALUES[(int)move.promotionType()] - SEE_PIECE_VALUES[0];
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
    score -= SEE_PIECE_VALUES[(int)next];
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
        score = -score - 1 - SEE_PIECE_VALUES[(int)next];
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