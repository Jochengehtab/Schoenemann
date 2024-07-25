#include "evaluate.h"
#include "psqt.h"


int evaluate(Board& board) {
    int evaluation = 0;
    evaluation = countMaterial(board, Color::WHITE) - countMaterial(board, Color::BLACK);
    evaluation += getMobility(board, Color::WHITE) - getMobility(board, Color::BLACK);
    evaluation += realMobility(board, Color::WHITE) - realMobility(board, Color::BLACK);

    int perspective = board.sideToMove() == Color::WHITE ? 1 : -1;

    return evaluation * perspective;
}

int getMobility(Board& borad, Color color)
{
    int mobility = 0;
    for (short i = 0; i < 64; i++)
    {
        if (borad.isAttacked(i, color))
        {
            mobility += 5;
        }
    }
    return mobility;
}

int realMobility(Board& board, Color color)
{
    int mobility = 0;
    const Bitboard allBoard = board.us(color);
    for (short i = 0; i < 64; i++)
    {
        if (allBoard.check(i) == 0 && !board.isAttacked(i, color))
        {
            mobility += 5;
        }
    }
    return mobility;
}

int countMaterial(Board& board, Color color) {
    int material = 0;

    //Pawn
    short amountPawn = countAmount(board, PieceType::PAWN, color);
    int valuePawn = pawnValue + bouns.getPieceBounus(board, PieceType::PAWN, color);
    material += amountPawn * valuePawn;
    //std::cout << "color " << color << " pawnvalue " << count_amount(board, PieceType::PAWN, color) * (pawnValue + bouns.getPieceBounus(board, PieceType::PAWN, color)) << " amount " << count_amount(board, PieceType::PAWN, color) << " bounus " << bouns.getPieceBounus(board, PieceType::PAWN, color) << std::endl;
    
    short amountKnight = countAmount(board, PieceType::KNIGHT, color);
    int valueKnight = knightValue + bouns.getPieceBounus(board, PieceType::KNIGHT, color);
    material += amountKnight * valueKnight;


    short bishopAmount = countAmount(board, PieceType::BISHOP, color);

    if (bishopAmount >= 2)
    {
        material += 50;
    }

    int valueBishop = bishopValue + bouns.getPieceBounus(board, PieceType::BISHOP, color);

    material += bishopAmount * valueBishop;

    short amountRook = countAmount(board, PieceType::ROOK, color);
    int valueRook = rookValue + bouns.getPieceBounus(board, PieceType::ROOK, color);

    material += amountRook * valueRook;

    short amountQueen = countAmount(board, PieceType::QUEEN, color);
    int valueQueen = queenValue + bouns.getPieceBounus(board, PieceType::QUEEN, color);

    material += amountQueen * valueQueen;
    
    //material += bouns.getPieceBounus(board, PieceType::KING, color);
    return material;
}

int countAmount(Board& board, PieceType type, Color color) 
{
    return board.pieces(type, color).count();
}