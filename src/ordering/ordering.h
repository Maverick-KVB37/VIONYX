#pragma once
#include "../core/types.h"
#include "../core/move.h"
#include "../board/position.h"
#include "../board/movegen.h"

class MoveOrderer {
public:
    MoveOrderer() = default;

    // Score quiet and non-capture moves for ordering
    void scoreMoves(const Position& pos, MoveList& moves, Move ttMove, Move killers[2]);

    // Score capture moves for ordering using SEE
    void scoreCaptures(const Position& pos, MoveList& captures);

    // Static Exchange Evaluation to order captures
    int see(const Position& pos, Move move);

    bool seeGe(const Position& pos, Move move, int threshold);

    // Storage for scores during sorting
    std::vector<int> scores;
private:
    static constexpr int SEEVALUE[6] = {100, 300, 300, 500, 900, 50000};

    Bitboard minAttacker(const Position& pos, Bitboard attadef, Color color, PieceType& attacker);
    Bitboard considerXRays(const Position& pos, Square sq, Bitboard occupiedBB);
    Bitboard allAttackers(const Position& pos, Square sq, Bitboard occupiedBB);
    Bitboard attackersForSide(const Position& pos, Color attackerColor, Square sq, Bitboard occupiedBB);

};