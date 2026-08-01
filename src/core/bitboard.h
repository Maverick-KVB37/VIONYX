#pragma once
#include "types.h"
#include <iostream>

#define setbit(bb, sq) ((bb) |= (1ULL << (sq)))
#define getbit(bb, sq) ((bb) & (1ULL << (sq)))
#define clearbit(bb, sq) ((bb) &= ~(1ULL << (sq)))
#define togglebit(bb, sq) ((bb) ^= (1ULL << (sq)))

#define popcount(bb) __builtin_popcountll(bb)

#define getlsb(bb) static_cast<Square>(__builtin_ctzll(bb))
#define getmsb(bb) static_cast<Square>(63 - __builtin_clzll(bb))

inline Square poplsb(Bitboard &bb) {
  Square s = Square(getlsb(bb));
  bb &= bb - 1;
  return s;
}

inline Square bsf(Bitboard b) { return Square(__builtin_ctzll(b)); }

inline Square bsr(Bitboard b) { return Square(63 ^ __builtin_clzll(b)); }

// --- constants ---
constexpr Bitboard EMPTY_BB = 0ULL;
constexpr Bitboard ALL_SQUARES = ~0ULL;

extern const Bitboard SQUAREBB[64];
extern const Bitboard MASKFILE[8];
extern const Bitboard MASKRANK[8];
extern const Bitboard MASKPASSED[2][64];
extern const Bitboard BBRANKSPAN[8][8];
extern const Bitboard MASKDIAGONAL[15];
extern const Bitboard MASKANTIDIAGONAL[15];

constexpr Piece makepiece(Color c, PieceType pt) { return Piece((c * 6) + pt); }

template <Color c> inline Piece makepiece(PieceType pt) {
  return Piece((c * 6) + pt);
}

constexpr PieceType piecetype(Piece pc) {
  if (pc == None)
    return Nonetype;
  return PieceType(pc % 6);
}

constexpr Color piececolor(Piece pc) { return Color(pc / 6); }

constexpr File fileof(Square s) { return File(s & 7); }

constexpr Rank rankof(Square s) { return Rank(s >> 3); }

constexpr Square makesquare(File f, Rank r) { return Square((r << 3) + f); }

inline int diagonalof(Square sq) { return 7 + rankof(sq) - fileof(sq); }

inline int antidiagonalof(Square sq) { return rankof(sq) + fileof(sq); }

constexpr Bitboard bb(Square s) { return 1ULL << s; }

inline int squareDistance(Square a, Square b) {
  return std::max(std::abs(fileof(a) - fileof(b)),
                  std::abs(rankof(a) - rankof(b)));
}

void printBitboard(Bitboard bb);