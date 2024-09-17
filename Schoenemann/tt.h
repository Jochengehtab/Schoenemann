#pragma once

#include <iostream>
#include <cstring>
#include "chess.hpp"

using namespace chess;

const short EXACT = 0;
const short UPPER_BOUND = 1; //Upper bound
const short LOWER_BOUND = 2; //Lower bound
const short infinity = 32767;

struct Hash {
    std::uint64_t key;
    short depth;
    short type; //Ether EXACT, UPPER_BOUND or LOWER_BOUND
    int score;
    Move move;
    int eval;

    void setEntry(std::uint64_t _key, short _depth, short _type, int _score, Move _move, int _eval)
    {
        key = _key;
        depth = _depth;
        type = _type;
        score = _score;
        move = _move;
        eval = _eval;
    }
};

class tt {
public:
    tt(std::uint64_t MB);
    tt(const tt& other) = delete;
    tt& operator=(const tt& other) = delete;
    ~tt();

    void storeEvaluation(std::uint64_t key, short depth, short type, int score, Move move, int eval);
    Hash* getHash(std::uint64_t zobristKey);
    std::uint64_t getSize() const;
    void setSize(std::uint64_t MB);

    void clear();
    int estimateHashfull() const;

    int ScoreToTT(int score, int ply) 
    {
        return score >= infinity ? score + ply
            : score <= -infinity ? score - ply
            : score;
    }

    
    int ScoreFromTT(int score, int ply) 
    {
        return score >= infinity ? score - ply
            : score <= -infinity ? score + ply
            : score;
    }

    bool checkForMoreInformation(short type, int ttScore, int score)
    {
        short tempType;
        if (ttScore >= score)
        {
            tempType = LOWER_BOUND;
        }
        else
        {
            tempType = UPPER_BOUND;
        }

        return type & tempType;
    }

private:
    std::uint64_t size;
    Hash *table;
    void init(std::uint64_t MB);
};