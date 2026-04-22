#pragma once

#include "types.h"
#include <string>
#include <vector>

// Bits 0-5:   'from' square
// Bits 6-11:  'to' square
// Bits 12-15: flags (promotion, capture, etc)
using MoveData = uint16_t;

// --- Move Flags ---
enum MoveFlag : MoveData {
    QuietMove    = 0,
    DoublePawnPush = 1,
    KingCastle   = 2,
    QueenCastle  = 3,
    Capture      = 4,
    EnPassant    = 5,
    
    // Promotions are flags 8-15
    KnightPromotion = 8,
    BishopPromotion = 9,
    RookPromotion   = 10,
    QueenPromotion  = 11,
    KnightPromoCapture = 12,
    BishopPromoCapture = 13,
    RookPromoCapture   = 14,
    QueenPromoCapture  = 15
};

class Move {
public:
    // Constructors
    Move() : m_data(0) {}
    explicit Move(MoveData data) : m_data(data) {}
    Move(Square from, Square to, MoveFlag flag);

    // Getters
    constexpr Square from() const { return Square(m_data & 0x3f); }
    constexpr Square to() const { return Square((m_data >> 6) & 0x3f); }
    constexpr MoveFlag flag() const { return MoveFlag((m_data >> 12) & 0xf); }
    bool is_valid() const { return m_data != 0; }
    // Helpers
    constexpr bool is_capture() const { return (flag() &0x4)!=0; }
    constexpr bool is_promotion() const { return (flag() >= KnightPromotion); }
    constexpr bool is_enpassant() const { return flag() == EnPassant; }
    constexpr bool is_castle() const { return flag() == KingCastle || flag() == QueenCastle; }

    //for comparing two moves
    constexpr bool operator==(const Move& other) const { return m_data == other.m_data; }
    constexpr bool operator!=(const Move& other) const { return m_data != other.m_data; }
    
    // Utility
    std::string to_uci_string() const;

    // Get the promoted piece type (only valid if is_promotion() returns true)
    constexpr PieceType promoted_piece_type() const {
        if (!is_promotion()) return Nonetype;
        
        MoveFlag f = flag();
        switch (f) {
            case KnightPromotion:
            case KnightPromoCapture:
                return Knight;
            case BishopPromotion:
            case BishopPromoCapture:
                return Bishop;
            case RookPromotion:
            case RookPromoCapture:
                return Rook;
            case QueenPromotion:
            case QueenPromoCapture:
                return Queen;
            default:
                return Nonetype;
        }
    }
private:
    MoveData m_data;
};

extern const Move NO_MOVE;

// Stack-allocated move list — avoids heap allocation in the search hot path
// Maximum legal moves in any chess position is ~218, 256 gives headroom
class MoveList {
public:
    static constexpr int MAX_MOVES = 256;

    MoveList() : count(0) {}

    void clear() { count = 0; }
    void push_back(Move m) { moves[count++] = m; }

    int size() const { return count; }
    bool empty() const { return count == 0; }

    Move& operator[](int i) { return moves[i]; }
    const Move& operator[](int i) const { return moves[i]; }

    Move* begin() { return moves; }
    Move* end() { return moves + count; }
    const Move* begin() const { return moves; }
    const Move* end() const { return moves + count; }

    // no-op for compatibility with old vector code
    void reserve(int) {}

private:
    Move moves[MAX_MOVES];
    int count;
};

