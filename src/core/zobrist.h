#pragma once

#include "types.h"

/**
 * Zobrist Hashing for Chess Positions
 * 
 * Uses XOR-shift PRNG to generate pseudo-random 64-bit keys.
 * Each unique board feature (piece on square, castling rights, etc.)
 * gets a unique random key. Position hash is XOR of all features.
 */
class Zobrist {
public:
    // Random keys for pieces on squares
    // [piece_type][square] where piece_type is 0-11 (WhitePawn...BlackKing)
    U64 pieceKeys[12][64];
    
    // Random keys for en passant file (only 8 needed, one per file)
    U64 enpassantKeys[8];
    
    // Random keys for castling rights (16 possible combinations)
    // Bits: 0=WK, 1=WQ, 2=BK, 3=BQ
    U64 castlingKeys[16];
    
    // Random key XOR'd when black is to move
    U64 sideKey;
    
    // Constructor with configurable seed
    Zobrist(U32 seed = 0x1D2C3A4Full) : randomState(seed) {}
    
    // Initialize all random keys
    void init();
    
    // Helper: compute full hash from scratch for a position
    U64 computeHash(const U8 board[64], U8 castling, I8 epFile, bool blackToMove) const;

private:
    U32 randomState;
    
    // XOR-shift PRNG with improved constants
    U32 random32();
    U64 random64();
};

// Global zobrist instance
extern Zobrist zobrist;
