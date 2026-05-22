#pragma once
#include "types.h"
#include "magic.h"

namespace Attacks {

    // pre-calculated attack tables
    extern const Bitboard PAWN_ATTACKS[2][64];
    extern const Bitboard KNIGHT_ATTACKS[64];
    extern const Bitboard KING_ATTACKS[64];

    void init();

    inline Bitboard GetRookAttacks(Square sq, Bitboard occupied){
         return Astrove::magic::GetRookAttacks(sq, occupied);
    }
    inline Bitboard GetBishopAttacks(Square sq, Bitboard occupied){
        return Astrove::magic::GetBishopAttacks(sq, occupied);
    }
    
    inline Bitboard GetQueenAttacks(Square sq, Bitboard occupied){
        return Astrove::magic::GetQueenAttacks(sq, occupied);
    }

    inline Bitboard GetPawnAttacks(Color c, Square sq){
        return Attacks::PAWN_ATTACKS[c][sq];
    }

    inline Bitboard GetKingAttacks(Square sq){
        return Attacks::KING_ATTACKS[sq];
    }

    inline Bitboard GetKnightAttacks(Square sq){
        return Attacks::KNIGHT_ATTACKS[sq];
    }

} // namespace Attacks