#pragma once
#include "../core/types.h"
#include "../core/move.h"
#include "../core/attacks.h"
#include "position.h"
#include <vector>

// list of moves
using MoveList = std::vector<Move>;

class MoveGenerator {
public:
    template<Color c>
    void generate_all_moves(const Position& pos, MoveList& moves) {
        moves.clear();

        generate_pawn_moves<c>(pos, moves);
        generate_knight_moves<c>(pos, moves);
        generate_king_moves<c>(pos, moves);
        generate_sliding_moves<c>(pos, moves);
        generate_castling_moves<c>(pos, moves);
    }

private:

template <Color c>
void generate_knight_moves(const Position& pos,MoveList& moves){
    //get our knight
    Bitboard knights=pos.knights<c>();
    //friendly piece
    Bitboard friendly=pos.occupancy(c);
    //enemy pieces
    Bitboard enemy=pos.occupancy(~c);

    while(knights){
        Square from=poplsb(knights);

        //get all square on which can attack
        Bitboard attacks=Attacks::get_knight_attacks(from);
        //remove square with friendly
        attacks&=~friendly;
        //split into quietmove and captue

        Bitboard captures=attacks&enemy;
        Bitboard quiets=attacks& ~enemy;

        //generate captured move
        while(captures){
            Square to=poplsb(captures);
            moves.push_back(Move(from,to,Capture));
        }
        //generate quiet move
        while(quiets){
            Square to=poplsb(quiets);
            moves.push_back(Move(from,to,QuietMove));
        }
    }
}

template <Color c>
void generate_sliding_moves(const Position& pos,MoveList& moves){
    const Bitboard occupancy = pos.occupancy();
    const Bitboard friendly = pos.occupancy(c);
    const Bitboard enemy = pos.occupancy(~c);

    auto generate_for_slider = [&](Bitboard pieces, auto get_attacks_func) {
        while (pieces) {
            Square from = poplsb(pieces);
            Bitboard attacks = get_attacks_func(from, occupancy) & ~friendly;

            Bitboard captures = attacks & enemy;
            while (captures) {
                moves.push_back(Move(from, poplsb(captures), Capture));
            }

            Bitboard quiets = attacks & ~enemy;
            while (quiets) {
                moves.push_back(Move(from, poplsb(quiets), QuietMove));
            }
        }
    };

    generate_for_slider(pos.bishops<c>(), Attacks::get_bishop_attacks);
    generate_for_slider(pos.rooks<c>(), Attacks::get_rook_attacks);
    generate_for_slider(pos.queens<c>(), Attacks::get_queen_attacks);
}

template <Color c>
void generate_king_moves(const Position& pos, MoveList& moves){
        //get our knight
    Bitboard kings=pos.kings<c>();
    //friendly piece
    Bitboard friendly=pos.occupancy(c);
    //enemy pieces
    Bitboard enemy=pos.occupancy(~c);

    while(kings){
        Square from=poplsb(kings);

        //get all square on which can attack
        Bitboard attacks=Attacks::get_king_attacks(from);
        //remove square with friendly
        attacks&=~friendly;
        //split into quietmove and captue

        Bitboard captures=attacks&enemy;
        Bitboard quiets=attacks& ~enemy;

        //generate captured move
        while(captures){
            Square to=poplsb(captures);
            moves.push_back(Move(from,to,Capture));
        }
        //generate quiet move
        while(quiets){
            Square to=poplsb(quiets);
            moves.push_back(Move(from,to,QuietMove));
        }
    }
}

template <Color c>
void generate_castling_moves(const Position& pos,MoveList& moves){
    if constexpr (c==White){
        if(pos.castling() & WHITE_OO){
            if(pos.pieceAt(SQ_F1)==None && pos.pieceAt(SQ_G1)==None &&
               !pos.isSquareAttacked<Black>(SQ_E1) &&
               !pos.isSquareAttacked<Black>(SQ_F1) &&
               !pos.isSquareAttacked<Black>(SQ_G1)){
                moves.push_back(Move(SQ_E1,SQ_G1,KingCastle));
            }            
        }
        if(pos.castling() & WHITE_OOO){
            if(pos.pieceAt(SQ_D1)==None && pos.pieceAt(SQ_C1)==None &&
               pos.pieceAt(SQ_B1) == None &&
               !pos.isSquareAttacked<Black>(SQ_E1) &&
               !pos.isSquareAttacked<Black>(SQ_D1) &&
               !pos.isSquareAttacked<Black>(SQ_C1)){
                moves.push_back(Move(SQ_E1,SQ_C1,QueenCastle));
            }            
        }   
    }
    else {
        if (pos.castling() & BLACK_OO) {
            if (pos.pieceAt(SQ_F8) == None && pos.pieceAt(SQ_G8) == None &&
                !pos.isSquareAttacked<White>(SQ_E8) &&
                !pos.isSquareAttacked<White>(SQ_F8) &&
                !pos.isSquareAttacked<White>(SQ_G8)) {
                moves.push_back(Move(SQ_E8, SQ_G8, KingCastle));
            }
        }
        if (pos.castling() & BLACK_OOO) {
            if (pos.pieceAt(SQ_D8) == None && pos.pieceAt(SQ_C8) == None && pos.pieceAt(SQ_B8) == None &&
                !pos.isSquareAttacked<White>(SQ_E8) &&
                !pos.isSquareAttacked<White>(SQ_D8) &&
                !pos.isSquareAttacked<White>(SQ_C8)) {
                moves.push_back(Move(SQ_E8, SQ_C8, QueenCastle));
            }
        }
    }
}

template <Color c>
void generate_pawn_moves(const Position& pos,MoveList& moves){
    Bitboard pawns=pos.pawns<c>();
    Bitboard empty=~pos.occupancy();
    Bitboard enemy=pos.occupancy(~c);

    int forward=(c==White) ? 8: -8;
    int doublepawnpush=(c==White)?1:6;
    int promotionrank=(c==White) ?6:1;

    while(pawns){
        Square from=poplsb(pawns);
        int fromrank=from/8;
        Square to=Square(from+forward);

        //single push
        if(empty&(1ULL<<to)){
            if(fromrank==promotionrank){
                moves.push_back(Move(from,to,QueenPromotion));
                moves.push_back(Move(from,to,RookPromotion));
                moves.push_back(Move(from,to,BishopPromotion));
                moves.push_back(Move(from,to,KnightPromotion));
            }
            else {
                moves.push_back(Move(from,to,QuietMove));
                //double push
                if(fromrank==doublepawnpush){
                    Square to2=Square(from+2*forward);
                    if((empty&(1ULL<<to2))&&(empty & (1ULL<<to))){
                        moves.push_back(Move(from,to2,DoublePawnPush));
                    }
                }
            }
        }
        
        //captures
        Bitboard attacks=Attacks::get_pawn_attacks(c,from) &enemy;
        while(attacks){
            Square capSq=poplsb(attacks);
            if(fromrank==promotionrank){
                moves.push_back(Move(from,capSq,QueenPromoCapture));
                moves.push_back(Move(from,capSq,RookPromoCapture));
                moves.push_back(Move(from,capSq,BishopPromoCapture));
                moves.push_back(Move(from,capSq,KnightPromoCapture));
            }
            else{
                moves.push_back(Move(from,capSq,Capture));
            }
        }

        //enpasaant
        Square epSq=pos.epSquare();
        if(epSq!=NO_SQ){
            Bitboard epAttack=Attacks::get_pawn_attacks(c,from) & (1ULL<<epSq);
            if(epAttack){
                moves.push_back(Move(from,epSq,EnPassant));
            }
        }
        
    }
}

};
