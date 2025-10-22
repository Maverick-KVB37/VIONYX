#include "move.h"
#include "types.h" // For squareToString

const Move NO_MOVE = Move();

Move::Move(Square from, Square to, MoveFlag flag) {
    m_data = (static_cast<MoveData>(flag) << 12) | 
             (static_cast<MoveData>(to)   << 6)  |
             (static_cast<MoveData>(from));
}

std::string Move::to_uci_string() const {
    if (m_data == 0) return "0000";
    
    Square fromSq = from();
    Square toSq = to();
    
    char fromFile = 'a' + (fromSq % 8);
    char fromRank = '1' + (fromSq / 8);
    char toFile = 'a' + (toSq % 8);
    char toRank = '1' + (toSq / 8);
    
    std::string moveStr;
    moveStr += fromFile;
    moveStr += fromRank;
    moveStr += toFile;
    moveStr += toRank;
    
    // Add promotion piece if it's a promotion
    if (is_promotion()) {
        MoveFlag f = flag();
        switch (f) {
            case KnightPromotion:
            case KnightPromoCapture:
                moveStr += 'n';
                break;
            case BishopPromotion:
            case BishopPromoCapture:
                moveStr += 'b';
                break;
            case RookPromotion:
            case RookPromoCapture:
                moveStr += 'r';
                break;
            case QueenPromotion:
            case QueenPromoCapture:
                moveStr += 'q';
                break;
            default:
                break;
        }
    }
    
    return moveStr;
}