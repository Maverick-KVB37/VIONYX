#include "magic.h"
#include "types.h"

namespace Astrove::magic{

    // --- Definitions for the extern variables from the header ---
    Bitboard bishop_masks[64];
    Bitboard rook_masks[64];
    Bitboard bishop_attacks[64][512]; // 2^9 = 512, max relevant bits for bishops is 9
    Bitboard rook_attacks[64][4096];  // 2^12 = 4096, max relevant bits for rooks is 12

    // Generates all permutations of blockers for a given mask
    inline Bitboard get_blocker_permutation(int index, int bits, Bitboard mask) {
        Bitboard blockers = 0ULL;
        int bit_indices[12]; // Max 12 bits for rooks
    
        // Extract all bit positions from the mask
        Bitboard temp_mask = mask;
        for (int i = 0; i < bits; ++i) {
            bit_indices[i] = __builtin_ctzll(temp_mask); // Count trailing zeros (LSB position)
            temp_mask &= temp_mask - 1; // Clear the LSB
        }
    
        // Set bits according to the index
        for (int i = 0; i < bits; ++i) {
            if (index & (1 << i)) {
            blockers |= (1ULL << bit_indices[i]);
        }
        }
        return blockers;
    }

    // Generates bishop attack masks, excluding the outer board edges
    Bitboard generate_bishop_mask(Square sq) {
        Bitboard attacks = 0ULL;
        int r, f;
        int tr = static_cast<int>(sq) / 8;
        int tf = static_cast<int>(sq) % 8;
        for (r = tr + 1, f = tf + 1; r < 7 && f < 7; r++, f++) attacks |= (1ULL << (r * 8 + f));
        for (r = tr + 1, f = tf - 1; r < 7 && f > 0; r++, f--) attacks |= (1ULL << (r * 8 + f));
        for (r = tr - 1, f = tf + 1; r > 0 && f < 7; r--, f++) attacks |= (1ULL << (r * 8 + f));
        for (r = tr - 1, f = tf - 1; r > 0 && f > 0; r--, f--) attacks |= (1ULL << (r * 8 + f));
        return attacks;
    }

    // Generates rook attack masks, excluding the outer board edges
    Bitboard generate_rook_mask(Square sq) {
        Bitboard attacks = 0ULL;
        int r, f;
        int tr = static_cast<int>(sq) / 8;
        int tf = static_cast<int>(sq) % 8;
        for (r = tr + 1; r < 7; r++) attacks |= (1ULL << (r * 8 + tf));
        for (r = tr - 1; r > 0; r--) attacks |= (1ULL << (r * 8 + tf));
        for (f = tf + 1; f < 7; f++) attacks |= (1ULL << (tr * 8 + f));
        for (f = tf - 1; f > 0; f--) attacks |= (1ULL << (tr * 8 + f));
        return attacks;
    }

    // Generates bishop attacks "on the fly" to populate the table
    Bitboard generate_bishop_attacks_on_the_fly(Square sq, Bitboard blockers) {
        Bitboard attacks = 0ULL;
        int r, f;
        int tr = static_cast<int>(sq) / 8;
        int tf = static_cast<int>(sq) % 8;
        for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
            attacks |= (1ULL << (r * 8 + f));
            if (blockers & (1ULL << (r * 8 + f))) break;
        }
        for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
            attacks |= (1ULL << (r * 8 + f));
            if (blockers & (1ULL << (r * 8 + f))) break;
        }
        for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
            attacks |= (1ULL << (r * 8 + f));
            if (blockers & (1ULL << (r * 8 + f))) break;
        }
        for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
            attacks |= (1ULL << (r * 8 + f));
            if (blockers & (1ULL << (r * 8 + f))) break;
        }
        return attacks;
    }

    // Generates rook attacks "on the fly" to populate the table
    Bitboard generate_rook_attacks_on_the_fly(Square sq, Bitboard blockers) {
        Bitboard attacks = 0ULL;
        int r, f;
        int tr = static_cast<int>(sq) / 8;
        int tf = static_cast<int>(sq) % 8;
        for (r = tr + 1; r <= 7; r++) {
            attacks |= (1ULL << (r * 8 + tf));
            if (blockers & (1ULL << (r * 8 + tf))) break;
        }
        for (r = tr - 1; r >= 0; r--) {
            attacks |= (1ULL << (r * 8 + tf));
            if (blockers & (1ULL << (r * 8 + tf))) break;
        }
        for (f = tf + 1; f <= 7; f++) {
            attacks |= (1ULL << (tr * 8 + f));
            if (blockers & (1ULL << (tr * 8 + f))) break;
        }
        for (f = tf - 1; f >= 0; f--) {
            attacks |= (1ULL << (tr * 8 + f));
            if (blockers & (1ULL << (tr * 8 + f))) break;
        }
        return attacks;
    }

    // --- Public initialization function ---
    void init() {
        for (int sq_int = 0; sq_int < 64; ++sq_int) {
            Square sq = static_cast<Square>(sq_int);

            // --- Generate masks ---
            bishop_masks[sq_int] = generate_bishop_mask(sq);
            rook_masks[sq_int] = generate_rook_mask(sq);

            // --- Populate bishop attacks ---
            int bishop_bits = bishop_relevant_bits[sq_int];
            int bishop_permutations = 1 << bishop_bits;
            for (int i = 0; i < bishop_permutations; ++i) {
                Bitboard blockers = get_blocker_permutation(i, bishop_bits, bishop_masks[sq_int]);
                Bitboard on_the_fly_attacks = generate_bishop_attacks_on_the_fly(sq, blockers);

                Bitboard magic_index = (blockers * bishop_magic_numbers[sq_int]) >> (64 - bishop_bits);
                bishop_attacks[sq_int][magic_index] = on_the_fly_attacks;
            }

            // --- Populate rook attacks ---
            int rook_bits = rook_relevant_bits[sq_int];
            int rook_permutations = 1 << rook_bits;
            for (int i = 0; i < rook_permutations; ++i) {
                Bitboard blockers = get_blocker_permutation(i, rook_bits, rook_masks[sq_int]);
                Bitboard on_the_fly_attacks = generate_rook_attacks_on_the_fly(sq, blockers);

                Bitboard magic_index = (blockers * rook_magic_numbers[sq_int]) >> (64 - rook_bits);
                rook_attacks[sq_int][magic_index] = on_the_fly_attacks;
            }
        }
    }
}