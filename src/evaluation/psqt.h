#pragma once
#include "../core/types.h"

namespace ASTROVE {
    using EvalScore = int32_t;
}

namespace ASTROVE::eval {

    using Score = int16_t;
    
    struct EvalScore {
        int32_t value;
        constexpr EvalScore(int32_t v = 0) : value(v) {}
        constexpr operator int32_t() const { return value; }
    };

    constexpr EvalScore composeEval(Score opening, Score endgame) {
        return EvalScore((static_cast<uint32_t>(endgame) << 16) | (static_cast<uint16_t>(opening)));
    }

    constexpr Score openingScore(EvalScore score) { return static_cast<Score>(score.value & 0xFFFF); }
    constexpr Score endgameScore(EvalScore score) { return static_cast<Score>((score.value >> 16) & 0xFFFF); }

    //---------- Piece-Square Tables (Tapered) ------------
    extern EvalScore PSQT[6][2][64];

    //Game Phase Values
    extern const int PiecePhaseValue[12]; //using 12 values for WhitePawn..BlackKing

    // --- Initialization Function ---
    void InitializePieceSquareTable();

}