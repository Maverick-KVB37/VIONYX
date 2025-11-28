#pragma once
#include "../core/types.h"
#include "../core/move.h"
#include "../board/position.h"
#include "../board/movegen.h"
#include <vector>

class MoveOrderer {
public:
    MoveOrderer();

    // Score quiet and non-capture moves for ordering
    void scoreMoves(const Position& pos, MoveList& moves, Move ttMove,Move killers[2],const int history[2][64][64]);

    // Score capture moves for ordering using SEE
    void scoreCaptures(const Position& pos, MoveList& captures);

    // Static Exchange Evaluation to order captures
    int see(const Position& pos, Move move);

    bool seeGe(const Position& pos, Move move, int threshold);

    // Storage for scores during sorting
    std::vector<int> scores;
private:
    static constexpr int SEEVALUE[6] = {100, 325, 335, 500, 975, 0};

    Bitboard minAttacker(const Position& pos, Bitboard attadef, Color color, PieceType& attacker);
    Bitboard considerXRays(const Position& pos, Square sq, Bitboard occupiedBB);
    Bitboard allAttackers(const Position& pos, Square sq, Bitboard occupiedBB);
    Bitboard attackersForSide(const Position& pos, Color attackerColor, Square sq, Bitboard occupiedBB);

    //MVV-LVA Table [Attacker][Victim]
    int mvv_lva[6][6];
    //consatnts
    static constexpr int SCORE_TT_MOVE=30000;
    static constexpr int SCORE_CAPTURE_BASE=20000;
    static constexpr int SCORE_KILLER_1=19000;
    static constexpr int SCORE_KILLER_2=18000;
};