#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>

// ========== BASIC TYPE DEFINITION ===============

typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t  U8;

typedef int64_t I64;
typedef int32_t I32;
typedef int16_t I16;
typedef int8_t  I8;

// =========== DEFINITION ============
typedef U64 Bitboard;
typedef U64 Key;
typedef I32 Value;
typedef I32 Depth;

// =========== ENUM =============

enum Color : I8 {
    White, Black
};

enum Piece : I8 {
    WhitePawn, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen, WhiteKing, 
    BlackPawn, BlackKnight, BlackBishop, BlackRook, BlackQueen, BlackKing, 
    None=12
};

enum PieceType : I8 {
    Pawn, Knight, Bishop, Rook, Queen, King, Nonetype
};

enum Square : I8 {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    NO_SQ
};

enum Direction : I8 {
    NORTH =  8,
    EAST  =  1,
    SOUTH = -8,
    WEST  = -1,
    
    NORTH_EAST = 9,
    SOUTH_EAST = -7,
    SOUTH_WEST = -9,
    NORTH_WEST = 7
};

enum File : I8 {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

enum Rank : I8 {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
};

enum CastlingRights : int {
    NO_CASTLING = 0,
    WHITE_OO    = 1,   // White kingside  (1 << 0)
    WHITE_OOO   = 2,   // White queenside (1 << 1)
    BLACK_OO    = 4,   // Black kingside  (1 << 2)
    BLACK_OOO   = 8,   // Black queenside (1 << 3)
    
    WHITE_CASTLING = WHITE_OO | WHITE_OOO,
    BLACK_CASTLING = BLACK_OO | BLACK_OOO,
    ANY_CASTLING   = WHITE_CASTLING | BLACK_CASTLING,
    
    CASTLING_RIGHTS_NB = 16
};


//------ CONSTANTS ---------
const std::string squareToString[65] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "-" // for NO_SQ
};


// ============= OPEERATORS ==============

// Color operators
constexpr Color operator~(Color c) {
    return Color(c ^ Black);
}
constexpr Color operator++(Color c){
    return static_cast<Color>(static_cast<int>(c) + 1);
}

// Square operators
constexpr Square operator+(Square s, int i) { return Square(int(s) + i); }
constexpr Square operator-(Square s, int i) { return Square(int(s) - i); }
inline Square& operator+=(Square& s, int i) { return s = Square(int(s) + i); }
inline Square& operator-=(Square& s, int i) { return s = Square(int(s) - i); }
inline Square& operator++(Square& s) { return s = Square(int(s) + 1); }
inline Square& operator--(Square& s) { return s = Square(int(s) - 1); }

// Direction operators
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square& s, Direction d) { return s = Square(int(s) + int(d)); }
inline Square& operator-=(Square& s, Direction d) { return s = Square(int(s) - int(d)); }

// Piece operators
constexpr Piece operator~(Piece pc) {
    return Piece(pc ^ 8); // Swap color
}

// Increment operators
#define ENABLE_INCR_OPERATORS_ON(T)                     \
inline T& operator++(T& d) { return d = T(int(d) + 1); }\
inline T& operator--(T& d) { return d = T(int(d) - 1); }

ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)

#undef ENABLE_INCR_OPERATORS_ON