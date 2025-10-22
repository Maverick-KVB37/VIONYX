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

    // Helpers
    constexpr bool is_capture() const { return (flag() &0x4)!=0; }
    constexpr bool is_promotion() const { return (flag() >= KnightPromotion); }

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

// A list of move
using MoveList = std::vector<Move>;