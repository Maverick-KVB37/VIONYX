#pragma once
#include "types.h"
#include <iostream>
// BITBOARD MACROS

#define setbit(bb, sq)    ((bb) |= (1ULL << (sq)))
#define getbit(bb, sq)    ((bb) & (1ULL << (sq)))
#define clearbit(bb, sq)  ((bb) &= ~(1ULL << (sq)))
#define togglebit(bb, sq) ((bb) ^= (1ULL << (sq)))

// Population count
#define popcount(bb) __builtin_popcountll(bb)

// LSB/MSB
#define getlsb(bb) static_cast<Square>(__builtin_ctzll(bb))
#define getmsb(bb) static_cast<Square>(63 - __builtin_clzll(bb))

// Pop LSB
inline Square poplsb(Bitboard& bb) {
    Square s = Square(getlsb(bb));
    bb &= bb - 1;
    return s;
}

inline Square bsf(Bitboard b) {
  return Square(__builtin_ctzll(b));
}

inline Square bsr(Bitboard b) {
    return Square(63 ^ __builtin_clzll(b));
}

// ================== BITBOARD CONSTANTS =======================
constexpr Bitboard EMPTY_BB = 0ULL;
constexpr Bitboard ALL_SQUARES = ~0ULL;

extern const Bitboard SQUAREBB[64];
extern const Bitboard MASKFILE[8];
extern const Bitboard MASKRANK[8];
extern const Bitboard MASKPASSED[2][64];
extern const Bitboard BBRANKSPAN[8][8];
extern const Bitboard MASKDIAGONAL[15];
extern const Bitboard MASKANTIDIAGONAL[15];

// HELPER FUNCTIONS

// Make piece from color and piece type
constexpr Piece makepiece(Color c, PieceType pt) {
    return Piece((c * 6) + pt);
}

template <Color c>
inline Piece makepiece(PieceType pt){
    return Piece((c * 6) + pt);
}

// Get piece type from piece
constexpr PieceType piecetype(Piece pc) {
    if(pc==None) return Nonetype;
    return PieceType(pc%6);
}

// Get color from piece
constexpr Color piececolor(Piece pc) {
    return Color(pc/6);
}

// Square to file/rank
constexpr File fileof(Square s) {
    return File(s & 7);
}

constexpr Rank rankof(Square s) {
    return Rank(s >> 3);
}

// Make square from file and rank
constexpr Square makesquare(File f, Rank r) {
    return Square((r << 3) + f);
}

// returns diagonal of given square
inline int diagonalof(Square sq) {
    return 7+rankof(sq)-fileof(sq);
}

// returns anti diagonal of given square
inline int antidiagonalof(Square sq) {
    return rankof(sq)+fileof(sq);
}

constexpr Bitboard bb(Square s) {
    return 1ULL << s;
}

inline int squareDistance(Square a, Square b) {
    return std::max(std::abs(fileof(a) - fileof(b)), std::abs(rankof(a) - rankof(b)));
}

// print given bitboard (for debugging purposes)
void printBitboard(Bitboard bb);