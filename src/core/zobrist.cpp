#include "zobrist.h"

// Global instance
Zobrist zobrist;

/**
 * XOR-shift PRNG with Marsaglia's constants
 * Period: 2^32 - 1
 */
U32 Zobrist::random32() {
    U32 x = randomState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    randomState = x;
    return x;
}

/**
 * Generate 64-bit random by combining four 16-bit chunks
 * This ensures good distribution across all 64 bits
 */
U64 Zobrist::random64() {
    U64 n1 = (U64)(random32()) & 0xFFFFull;
    U64 n2 = (U64)(random32()) & 0xFFFFull;
    U64 n3 = (U64)(random32()) & 0xFFFFull;
    U64 n4 = (U64)(random32()) & 0xFFFFull;
    
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

/**
 * Initialize all Zobrist keys
 * MUST be called once before using position hashing
 */
void Zobrist::init() {
    // Reset PRNG to deterministic seed
    // Using Stockfish-inspired seed for consistency
    randomState = 0x1D2C3A4Full;
    
    // Initialize piece-square keys
    // 12 piece types (6 white + 6 black) × 64 squares
    for (int piece = 0; piece < 12; ++piece) {
        for (int square = 0; square < 64; ++square) {
            pieceKeys[piece][square] = random64();
        }
    }
    
    // Initialize en passant keys (one per file A-H)
    // Only the FILE matters for en passant, not the rank
    for (int file = 0; file < 8; ++file) {
        enpassantKeys[file] = random64();
    }
    
    // Initialize castling keys for all 16 combinations
    // Bit 0: White kingside (K)
    // Bit 1: White queenside (Q)
    // Bit 2: Black kingside (k)
    // Bit 3: Black queenside (q)
    for (int rights = 0; rights < 16; ++rights) {
        castlingKeys[rights] = random64();
    }
    
    // Initialize side-to-move key (XOR when black to move)
    sideKey = random64();
}

/**
 * Compute hash from scratch (for debugging/validation)
 * Normally you'd update hash incrementally during make/unmake
 */
U64 Zobrist::computeHash(const U8 board[64], U8 castling, I8 epFile, bool blackToMove) const {
    U64 hash = 0ULL;
    
    // Hash all pieces
    for (int sq = 0; sq < 64; ++sq) {
        U8 piece = board[sq];
        if (piece != None) {
            hash ^= pieceKeys[piece][sq];
        }
    }
    
    // Hash castling rights
    hash ^= castlingKeys[castling & 0xF];
    
    // Hash en passant file (only if valid)
    if (epFile >= 0 && epFile < 8) {
        hash ^= enpassantKeys[epFile];
    }
    
    // Hash side to move
    if (blackToMove) {
        hash ^= sideKey;
    }
    
    return hash;
}
