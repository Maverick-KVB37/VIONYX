#pragma once

#include "../core/types.h"
#include "../core/move.h"
#include "../core/attacks.h"
#include "../core/bitboard.h"
#include "../core/zobrist.h"
#include <string>
#include <vector>
#include <cassert>

const std::string defaultFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// State info for make/unmake
struct StateInfo {
    uint64_t hashKey;
    Square enpassantSquare;
    U8 castlingRights;
    U8 halfMoveClock;
    Piece captured;
    
    // masks for legal move generation
    Bitboard checkers;      // Pieces giving check
    Bitboard pinMaskHV;     // HV pinned pieces
    Bitboard pinMaskD;      // Diagonal pinned pieces
    
    StateInfo* previous;
    
    StateInfo() : hashKey(0), enpassantSquare(NO_SQ), 
                  castlingRights(NO_CASTLING), halfMoveClock(0),
                  captured(None), checkers(EMPTY_BB), 
                  pinMaskHV(EMPTY_BB), pinMaskD(EMPTY_BB),
                  previous(nullptr) {}
};


class Position {
public:

    // constructor
    Position(const std::string& FEN = defaultFEN);
    void parseFEN(const std::string& FEN);
    std::string toFEN() const;
    void print();
    void print_all_bitboards() const;
    // move execution
    template <Color c> void makemove(Move move);
    template <Color c> void unmakemove(Move move);

    // Board queries
    inline Piece pieceAt(Square sq) const { return board[sq]; }
    inline Color sideToMove() const { return stm; }
    inline uint64_t hash() const { return state->hashKey; }
    inline Square epSquare() const { return state->enpassantSquare; }
    inline U8 castling() const { return state->castlingRights; }

    // Attack queries
    template <Color c> bool isSquareAttacked(Square sq) const;
    template <Color c> Square kingsq() const;
    template <Color c> bool inCheck() const;
    
    // Bitboard getters
    template <Color c> constexpr Bitboard pawns()   const { return (c == White) ? PiecesBB[WhitePawn]   : PiecesBB[BlackPawn]; }
    template <Color c> constexpr Bitboard knights()  const { return (c == White) ? PiecesBB[WhiteKnight] : PiecesBB[BlackKnight]; }
    template <Color c> constexpr Bitboard bishops()  const { return (c == White) ? PiecesBB[WhiteBishop] : PiecesBB[BlackBishop]; }
    template <Color c> constexpr Bitboard rooks()    const { return (c == White) ? PiecesBB[WhiteRook]   : PiecesBB[BlackRook]; }
    template <Color c> constexpr Bitboard queens()   const { return (c == White) ? PiecesBB[WhiteQueen]  : PiecesBB[BlackQueen]; }
    template <Color c> constexpr Bitboard kings()    const { return (c == White) ? PiecesBB[WhiteKing]   : PiecesBB[BlackKing]; }
    
    inline Bitboard occupancy(Color c) const { 
        return c == White ? occupancyWhite : occupancyBlack; 
    }
    inline Bitboard occupancy() const { return occupancyAll; }

    // Generate hash from scratch
    uint64_t generateHashKey() const;

    //repetittion draw and 50 move counter
    inline bool isDrawByRepetition(int ply) const;
    inline bool isDrawByFiftyMove() const;

    //define for access to piecesBB[index]
    Bitboard getPiecesBB(int index) const { return PiecesBB[index]; }
    Bitboard pieces(Color c, PieceType pt) const {
        return PiecesBB[c * 6 + pt];
    }
private:
        // Board representation
    Bitboard PiecesBB[12];
    Piece board[64];  // Mailbox
    
    // Occupancy
    Bitboard occupancyWhite;
    Bitboard occupancyBlack;
    Bitboard occupancyAll;
    
    // Current game state
    Color stm;  // Side to move
    StateInfo* state;
    
    // State storage pool
    StateInfo stateStack[1024];
    uint16_t stateCount;
    
    // Position history for repetition detection
    std::vector<uint64_t> positionHistory;
    uint16_t fullMoveCounter;
    uint16_t halfMoveCounter;

    // Helper functions
    void placePiece(Piece piece, Square sq);
    void removePiece(Square sq);
    void movePiece(Square from, Square to);
    
    // Zobrist helpers (inline for speed)
    inline void togglePiece(Piece piece, Square sq) {
        state->hashKey ^= zobrist.pieceKeys[piece][sq];
    }
    
    inline void toggleEnpassant(Square sq) {
        if (sq != NO_SQ) {
            state->hashKey ^= zobrist.enpassantKeys[sq & 7];
        }
    }
    
    inline void toggleCastling(U8 rights) {
        state->hashKey ^= zobrist.castlingKeys[rights];
    }
    
    inline void toggleSide() {
        state->hashKey ^= zobrist.sideKey;
    }
    
};

template <Color c> 
bool Position::isSquareAttacked(Square sq) const {
    if (sq == NO_SQ) return false;
    
    Bitboard occupancy = occupancyAll;
    
    return (pawns<c>()                   & Attacks::get_pawn_attacks(~c,sq))         ||
           (knights<c>()                 & Attacks::get_knight_attacks(sq))           ||
           ((bishops<c>() | queens<c>()) & Attacks::get_bishop_attacks(sq, occupancy)) ||
           ((rooks<c>() | queens<c>())   & Attacks::get_rook_attacks(sq, occupancy))   ||
           (kings<c>()                   & Attacks::get_king_attacks(sq));
}

template <Color c>
bool Position::inCheck() const {
    Square kingSq = getlsb(kings<c>());  // Get king square
    return isSquareAttacked<~c>(kingSq);  // Enemy attacking our king?
}

template <Color c> 
Square Position::kingsq() const {
    if constexpr (c==White) return bsf(kings<White>());
    return bsf(kings<Black>());
}

template <Color c>
void Position::makemove(Move move){
    Square from=move.from();
    Square to=move.to();

    Piece movingpiece = board[from];
    Piece capturedPiece = board[to];

    MoveFlag flag=move.flag();

    // 1. setup new state
    assert(stateCount < 1024);
    StateInfo* newState = &stateStack[stateCount++];
    newState->previous=state;

    //copy current state values
    newState->hashKey=state->hashKey;
    newState->castlingRights=state->castlingRights;
    newState->enpassantSquare=state->enpassantSquare;
    newState->halfMoveClock=state->halfMoveClock;
    newState->captured=capturedPiece;

    state=newState;

    positionHistory.push_back(state->hashKey);

    // 2.REMOVE OLD ENPASSANT FROM HASH
    if(state->previous->enpassantSquare!=NO_SQ){
        toggleEnpassant(state->previous->enpassantSquare);
    }
    state->enpassantSquare=NO_SQ;

    // 3.INCREMENT HALFMOVE CLOCK
    state->halfMoveClock++;

    //4. HANDLE CAPTURES(NORMAL)
    if(move.is_capture()&&flag!=EnPassant){
        state->halfMoveClock=0;
        togglePiece(capturedPiece,to);
        removePiece(to);
    }
    
    // 5.HANDLE ENPASSANT CAPTURE
    if(flag==EnPassant){
        state->halfMoveClock=0;
        U8 offset=(c==White) ? -8 : +8;
        Square capSq=Square(to+offset);
        Piece epPawn=makepiece<~c>(Pawn);

        togglePiece(epPawn,capSq);
        removePiece(capSq);
    }

    // 6. HANDLE DOUBLE PAWN PUSH
    if(flag==DoublePawnPush){
        state->halfMoveClock=0;
        U8 offset=(c==White)?-8:8;
        Square epSq=Square(to+offset);

        //set ep pawn if enemy can capture
        Bitboard enemyPawns=pawns<~c>();
        Bitboard epAttackers=Attacks::get_pawn_attacks(c,epSq);
        if(epAttackers&enemyPawns){
            state->enpassantSquare=epSq;
            toggleEnpassant(epSq);
        }
    }

    // 7. STORE OLD CASTLING FOR HASH UPDATE
    U8 oldCastling=state->castlingRights;

    // 8. HANDLE CASTLE
    if(flag==KingCastle){
        if constexpr (c==White){
            //move king
            togglePiece(WhiteKing,from);
            removePiece(from);
            togglePiece(WhiteKing,to);
            placePiece(WhiteKing,to);

            //move rook
            togglePiece(WhiteRook,SQ_H1);
            removePiece(SQ_H1);
            togglePiece(WhiteRook,SQ_F1);
            placePiece(WhiteRook,SQ_F1);

            state->castlingRights &= ~(WHITE_OO|WHITE_OOO);
        }
        else {
            //move king
            togglePiece(BlackKing,from);
            removePiece(from);
            togglePiece(BlackKing,to);
            placePiece(BlackKing,to);

            //move rook
            togglePiece(BlackRook,SQ_H8);
            removePiece(SQ_H8);
            togglePiece(BlackRook,SQ_F8);
            placePiece(BlackRook,SQ_F8);

            state->castlingRights &= ~(BLACK_OO|BLACK_OOO);
        }
    }
    else if(flag==QueenCastle){
        if constexpr (c==White){
            //move king
            togglePiece(WhiteKing,from);
            removePiece(from);
            togglePiece(WhiteKing,to);
            placePiece(WhiteKing,to);

            //move rook
            togglePiece(WhiteRook,SQ_A1);
            removePiece(SQ_A1);
            togglePiece(WhiteRook,SQ_D1);
            placePiece(WhiteRook,SQ_D1);

            state->castlingRights &= ~(WHITE_OO|WHITE_OOO);
        }
        else {
            //move king
            togglePiece(BlackKing,from);
            removePiece(from);
            togglePiece(BlackKing,to);
            placePiece(BlackKing,to);

            //move rook
            togglePiece(BlackRook,SQ_A8);
            removePiece(SQ_A8);
            togglePiece(BlackRook,SQ_D8);
            placePiece(BlackRook,SQ_D8);

            state->castlingRights &= ~(BLACK_OO|BLACK_OOO);
        }
    }

    // 9. HANDLE PROMOTION
    else if(move.is_promotion()){
        state->halfMoveClock=0;

        if (move.is_capture()) {
            // capturedPiece is already set at the top of the function
            togglePiece(capturedPiece, to);
            removePiece(to);
        }

        Piece promotedPiece;
        switch(flag){
            case KnightPromotion:
            case KnightPromoCapture:
                promotedPiece=makepiece<c>(Knight);
                break;
            case BishopPromotion:
            case BishopPromoCapture:
                promotedPiece=makepiece<c>(Bishop);
                break;
            case RookPromotion:
            case RookPromoCapture:
                promotedPiece=makepiece<c>(Rook);
                break;
            default:
                promotedPiece=makepiece<c>(Queen);
                break;
        }
        //remove pawn from source
        Piece pawn=makepiece<c>(Pawn);
        togglePiece(pawn,from);
        removePiece(from);

        //place promoted piece
        togglePiece(promotedPiece,to);
        placePiece(promotedPiece,to);
    }
    // 10. NORMAL MOVE
    else if(flag!=KingCastle && flag!=QueenCastle){
        //check if pawn move
        if(piecetype(movingpiece)==Pawn){
            state->halfMoveClock=0;
        }

        //move the piece
        togglePiece(movingpiece,from);
        removePiece(from);
        togglePiece(movingpiece,to);
        placePiece(movingpiece,to);
    }

    // 11. UPDATE CASTLE RIGHT(IF ROOK OR KING MOVED)
    if(flag!=KingCastle && flag!=QueenCastle){
        PieceType movedType=piecetype(movingpiece);

        if(movedType==King){
            if constexpr (c==White){
                state->castlingRights &= ~(WHITE_OO | WHITE_OOO);
            }
            else{
                state->castlingRights &= ~(BLACK_OO | BLACK_OOO);
            }
        }
        else if(movedType==Rook){
            if constexpr (c==White){
                if(from==SQ_A1) state->castlingRights &= ~WHITE_OOO;
                else if(from ==SQ_H1) state->castlingRights &= ~WHITE_OO;
            }
            else {
                if(from==SQ_A8) state->castlingRights &= ~BLACK_OOO;
                else if(from==SQ_H8) state->castlingRights &= ~BLACK_OO; 
            }
        }
    }

    // ROOK CAPTURE
    if(to==SQ_A1) state->castlingRights &= ~WHITE_OOO;
    else if(to==SQ_H1) state->castlingRights &= ~WHITE_OO;
    else if(to==SQ_A8) state->castlingRights &= ~BLACK_OOO;
    else if(to==SQ_H8) state->castlingRights &= ~BLACK_OO;

    //UPDATE CASTLING HASH IF CHANGED
    if(oldCastling != state->castlingRights){
        toggleCastling(oldCastling);
        toggleCastling(state->castlingRights);
    }
    
    // 13. SWITCH SIDE TO MOVE
    stm = ~stm;
    toggleSide();
    
    // 14. INCREMENT FULLMOVE
    if (stm == White) {
        fullMoveCounter++;
    }
}

template <Color c>
void Position::unmakemove(Move move){
    Square from=move.from();
    Square to =move.to();
    MoveFlag flag=move.flag();

    //switch side
    stm=~stm;

    //decrement full move
    if(stm==Black){
        fullMoveCounter--;
    }

    //get captured piece from state
    Piece capturedPiece=state->captured;

    //handle castling
    if(flag==KingCastle){
        if constexpr (c==White){
            movePiece(to, from);       // Move king back
            movePiece(SQ_F1, SQ_H1);   // Move rook back
        }
        else{
            movePiece(to, from);
            movePiece(SQ_F8, SQ_H8);
        }
    }
    else if(flag==QueenCastle){
        if constexpr (c == White) {
            movePiece(to, from);
            movePiece(SQ_D1, SQ_A1);
        } else {
            movePiece(to, from);
            movePiece(SQ_D8, SQ_A8);
        }
    }

    //promotion 
    else if(move.is_promotion()){
        // Remove promoted piece from target
        removePiece(to);
        
        // Restore original pawn on source
        placePiece(makepiece<c>(Pawn),from);
        
        // Restore captured piece if promotion was a capture
        if(capturedPiece!=None) {
            placePiece(capturedPiece,to);
        }
    }
    // enpassant
    else if(flag==EnPassant) {
        // Move pawn back
        movePiece(to,from);
        // Restore captured pawn on its original square
        int8_t offset=(c==White)?-8:8;
        Square capSq=Square(to+offset);
        Piece epPawn=makepiece<~c>(Pawn);
        placePiece(epPawn,capSq);
    }
    // normal move
    else {
        Piece movingPiece = board[to];

        removePiece(to);
        placePiece(movingPiece, from);
        
        // Restore captured piece (if any)
        if (capturedPiece!=None) {
            placePiece(capturedPiece,to);
        }
    }
    
    //restore previous state
    state = state->previous;
    stateCount--;
    
    //pop from pos history
    positionHistory.pop_back();
}


inline bool Position::isDrawByRepetition(int ply) const {
    // Need at least 4 half-moves for a repetition to be possible
    if (positionHistory.size() < 4) return false;

    // Calculate ply of last irreversible move (pawn move or capture)
    int irreversibleMovePly = static_cast<int>(positionHistory.size()) - 1 - static_cast<int>(halfMoveCounter);
    if (irreversibleMovePly < 0) irreversibleMovePly = 0;

    int repetitions = 0;
    // Check positions of the same side to move every 2 plies starting 4 plies ago 
    for (int i = static_cast<int>(positionHistory.size()) - 4; i >= irreversibleMovePly; i -= 2) {
        if (positionHistory[i] == state->hashKey) {
            repetitions++;
            // Twofold repetition inside search (ply > 0) treated as draw to avoid infinite loops
            if (ply > 0 && repetitions >= 1) return true;
            // Threefold repetition at root (ply == 0) is draw
            if (ply == 0 && repetitions >= 2) return true;
        }
    }
    return false;
}

inline bool Position::isDrawByFiftyMove() const {
    // no draw possible before 4 halfmove
    if (halfMoveCounter < 4) return false;
    return halfMoveCounter >= 100;
}
