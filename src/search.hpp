#ifndef SEARCH_HPP_
#define SEARCH_HPP_

#include "position.hpp"
#include "history.hpp"
#include "killer.hpp"
#include <vector>

const int maxPly = 1200;
const int mateScore = 32767;
const int maxMateScore = 32767 - maxPly;
const int infinity = mateScore + 1;

inline int matedInPly(int ply)
{
    return (-mateScore + ply);
}

inline int mateInPly(int ply)
{
    return (mateScore - ply);
}

inline bool isMateScore(int score)
{
    assert(score < infinity && score > -infinity);
    return (score <= -maxMateScore || score >= maxMateScore);
}

class Search
{
public:
    static int qSearch(Position & pos, int ply, int alpha, int beta);

    // Delete these, only used for tuning.
    static std::array<Move, 32> pv[32];
    static std::array<int, 32> pvLength;
private:
    static HistoryTable historyTable;
    static KillerTable killerTable;

    static const int aspirationWindow;
    static const int nullReduction;
    static const int futilityDepth;
    static const std::array<int, 1 + 4> futilityMargins;
    static const int lmrFullDepthMoves;
    static const int lmrReductionLimit;

    static void orderCaptures(const Position & pos, std::vector<Move> & moveStack);
    static void selectMove(std::vector<Move> & moveStack, size_t currentMove);
};

#endif