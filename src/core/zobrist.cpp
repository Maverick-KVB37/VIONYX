#include "zobrist.h"

Zobrist zobrist;

U32 Zobrist::random32() {
    U32 x = randomState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    randomState = x;
    return x;
}

U64 Zobrist::random64() {
    U64 n1 = (U64)(random32()) & 0xFFFFull;
    U64 n2 = (U64)(random32()) & 0xFFFFull;
    U64 n3 = (U64)(random32()) & 0xFFFFull;
    U64 n4 = (U64)(random32()) & 0xFFFFull;
    
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

void Zobrist::init() {
    randomState = 0x1D2C3A4Full;
    
    for (int piece = 0; piece < 12; ++piece) {
        for (int square = 0; square < 64; ++square) {
            pieceKeys[piece][square] = random64();
        }
    }
    
    for (int file = 0; file < 8; ++file) {
        enpassantKeys[file] = random64();
    }
    
    for (int rights = 0; rights < 16; ++rights) {
        castlingKeys[rights] = random64();
    }
    
    sideKey = random64();
}

U64 Zobrist::computeHash(const U8 board[64], U8 castling, I8 epFile, bool blackToMove) const {
    U64 hash = 0ULL;
    
    for (int sq = 0; sq < 64; ++sq) {
        U8 piece = board[sq];
        if (piece != None) {
            hash ^= pieceKeys[piece][sq];
        }
    }
    
    hash ^= castlingKeys[castling & 0xF];
    
    if (epFile >= 0 && epFile < 8) {
        hash ^= enpassantKeys[epFile];
    }
    
    if (blackToMove) {
        hash ^= sideKey;
    }
    
    return hash;
}
