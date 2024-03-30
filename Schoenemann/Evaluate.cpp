#include <iostream>

#include "Evaluate.h"
#include "movegen/chess.hpp"
#include "psqt.h"

using namespace chess;

const short pawnValue = 100;
const short knightValue = 300;
const short bishopValue = 325;
const short rookValue = 500;
const short queenValue = 900;

int evaluate(Board& board) {
    int evaluation = 0;
    int white_evaluation = count_material(board, Color::WHITE);
    int black_evaluation = count_material(board, Color::BLACK);

    evaluation = white_evaluation - black_evaluation;

    int perspective = board.sideToMove() == Color::WHITE ? 1 : -1;

    return evaluation * perspective;
}

int count_material(Board& board, Color color) {
    int material = 0;
    psqt bouns;
    material += count_amount(board, PieceType::PAWN, color) * (pawnValue + bouns.getPieceBounus(board, PieceType::PAWN));
    material += count_amount(board, PieceType::KNIGHT, color) * (knightValue + bouns.getPieceBounus(board, PieceType::KNIGHT));
    material += count_amount(board, PieceType::BISHOP, color) * (bishopValue + bouns.getPieceBounus(board, PieceType::BISHOP));
    material += count_amount(board, PieceType::ROOK, color) * (rookValue + bouns.getPieceBounus(board, PieceType::ROOK));
    material += count_amount(board, PieceType::QUEEN, color) * (queenValue + bouns.getPieceBounus(board, PieceType::QUEEN));
    return material;
}

int count_amount(Board& board, PieceType type, Color color) {
    Bitboard bitboard = board.pieces(type, color);
    return bitboard.count();
}