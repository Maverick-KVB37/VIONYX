#pragma once
#include "types.h"
#include "magic.h"

// This namespace will hold all our attack generation logic.
namespace Attacks {

    // Pre-calculated attack tables
    extern const Bitboard PAWN_ATTACKS[2][64];
    extern const Bitboard KNIGHT_ATTACKS[64];
    extern const Bitboard KING_ATTACKS[64];

    // A one-time initialization function to be called at program startup.
    // This will build the magic bitboard tables.
    void init();

    // Functions to get attack bitboards for sliding pieces.
    // These will use the magic bitboard tables.
    inline Bitboard get_rook_attacks(Square sq, Bitboard occupied){
         return Astrove::magic::GetRookAttacks(sq, occupied);
    }
    inline Bitboard get_bishop_attacks(Square sq, Bitboard occupied){
        return Astrove::magic::GetBishopAttacks(sq, occupied);
    }
    
    // A convenience function for queens.
    inline Bitboard get_queen_attacks(Square sq, Bitboard occupied){
        return Astrove::magic::GetQueenAttacks(sq, occupied);
    }

    inline Bitboard get_pawn_attacks(Color c, Square sq){
        return Attacks::PAWN_ATTACKS[c][sq];
    }

    inline Bitboard get_king_attacks(Square sq){
        return Attacks::KING_ATTACKS[sq];
    }

    inline Bitboard get_knight_attacks(Square sq){
        return Attacks::KNIGHT_ATTACKS[sq];
    }

} // namespace Attacks