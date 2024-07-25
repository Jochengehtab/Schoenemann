#include "evaluate.h"
#include "psqt.h"


int evaluate(Board& board) {
    int evaluation = 0;
    evaluation = countMaterial(board, Color::WHITE) - countMaterial(board, Color::BLACK);
    evaluation += getMobility(board, Color::WHITE) - getMobility(board, Color::BLACK);
    evaluation += kingSafety(board, Color::WHITE) - kingSafety(board, Color::BLACK);

    int perspective = board.sideToMove() == Color::WHITE ? 1 : -1;

    return evaluation * perspective;
}

int getMobility(Board& borad, Color color)
{
    int mobility = 0;
    for (size_t i = 0; i < 63; i++)
    {
        if (borad.isAttacked(i, color))
        {
            mobility += 5;
        }
    }
    return mobility;
}

int kingSafety(Board& board, Color color)
{
    int safeScore = 0;

    Bitboard pawnBoard = board.pieces(PieceType::PAWN, color);
    int kingIndex = board.kingSq(color).index();

    bool perspective = color == Color::WHITE ? true : false;


    if (perspective)
    {
        for (short i = 7; i < 10; i++)
        {
            if (pawnBoard.check(kingIndex + i))
            {
                safeScore += 80;
            }
        }
    }
    else
    {
        for (short i = 7; i < 10; i++)
        {
            if (pawnBoard.check(kingIndex - i))
            {
                safeScore += 80;
            }
        }
    }

    return safeScore;
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