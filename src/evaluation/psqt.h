#pragma once
#include "../core/types.h"

namespace ASTROVE {
    using EvalScore = int32_t;
}

namespace ASTROVE::eval {

    //---------- Piece-Square Tables (Tapered) ------------
    extern EvalScore PSQT[6][2][64];

    //Game Phase Values
    extern const int PiecePhaseValue[12]; //using 12 values for WhitePawn..BlackKing

    // --- Initialization Function ---
    void InitializePieceSquareTable();

}