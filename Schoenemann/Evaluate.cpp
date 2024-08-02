#include "evaluate.h"
#include "psqt.h"


int evaluate(Board& board) {
    int evaluation = 0;
    evaluation = countMaterial(board, Color::WHITE) - countMaterial(board, Color::BLACK);
    evaluation += getMobility(board, Color::WHITE) - getMobility(board, Color::BLACK);
    evaluation += pawnShieldBounus(board, Color::WHITE) - pawnShieldBounus(board, Color::BLACK);

    int perspective = board.sideToMove() == Color::WHITE ? 1 : -1;

    return evaluation * perspective;
}

int getMobility(Board& borad, Color color)
{
    int mobility = 0;
    for (size_t i = 0; i < 64; i++)
    {
        if (borad.isAttacked(i, color))
        {
            mobility += 2;
        }
    }
    return mobility;
}

int pawnShieldBounus(Board& board, Color color)
{
    const Bitboard pawns = board.pieces(PieceType::PAWN, color);
    //std::cout << pawns << std::endl;;
    const Square kingSq = board.kingSq(color);
    //std::cout << "King sq index is: " << kingSq.index() << std::endl;
    if (!kingSq.back_rank(kingSq, color))
    {
        return 0;
    }

    const short kingIndex = board.kingSq(color).index();
    short counter = 0;


    if (color == Color::WHITE)
    {
        for (short i = 1; i < 4; i++)
        {
            counter += pawns.check(kingIndex + 6 + i);
        }
    }
    else
    {
        for (short i = 1; i < 4; i++)
        {
            counter += pawns.check(kingIndex - 6 - i);
        }
    }
    //std::cout << "The counter is: " << counter << std::endl;

    if (counter >= 2)
    {
        return 50;
    }

    return 0;
}

int countMaterial(Board& board, Color color) 
{
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