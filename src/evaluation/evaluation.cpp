#include "evaluation.h"
#include "../core/types.h"
#include "../core/attacks.h"
#include "psqt.h"
#include <algorithm>

namespace ASTROVE::eval {

    Evaluator board_evaluator;

    Score Evaluator::evaluate_board(const Position& pos) {
        initialize(pos);

        evaluate_material_and_placement(pos);
        evaluate_pawns(pos);
        evaluate_mobility(pos);
        Score result = calculate_final_score(pos);
        
        return result;
    }

    void Evaluator::initialize(const Position& pos) {
        evalData = EvaluationData{};
    }
    /*
    void Evaluator::evaluate_material_and_placement(const Position& pos) {
    for (Square sq = SQ_A1; sq <= SQ_H8; sq = Square(sq + 1)) {
        Piece piece = pos.pieceAt(sq);
        if (piece == None) continue;
        
        Color c = piececolor(piece);
        PieceType pt = piecetype(piece);
        
        if (pt == King) continue;  // Don't evaluate king material
        
        EvalScore materialVal = PieceValues[pt];
        EvalScore psqtVal = PSQT[pt][c][sq];
        
        evalData.add((c == White) ? (materialVal + psqtVal) : -(materialVal + psqtVal));
    }
    }
    */
    void Evaluator::evaluate_material_and_placement(const Position& pos){
        for(PieceType pt=Pawn;pt<=King;pt=PieceType(pt+1)){
            //so white pieces
            Bitboard w=pos.pieces(White,pt);
            while(w){
                Square sq=poplsb(w);
                evalData.add(PieceValues[pt]+PSQT[pt][White][sq]);
            }
            //blck
            Bitboard b=pos.pieces(Black,pt);
            while(b){
                Square sq=poplsb(b);
                evalData.add(-(PieceValues[pt]+PSQT[pt][Black][sq]));
            }
        }
    }

    //function for Evaluate Pawn
    void Evaluator::evaluate_pawns(const Position& pos){
        Bitboard whitePawns = pos.pawns<White>();
        Bitboard blackPawns =pos.pawns<Black>();

        //define static so the it created only once
        static const Bitboard ADJACENT_FILES[8]={
            MASKFILE[1],
            MASKFILE[0]|MASKFILE[2],
            MASKFILE[1]|MASKFILE[3],
            MASKFILE[2]|MASKFILE[4],
            MASKFILE[3]|MASKFILE[5],
            MASKFILE[4]|MASKFILE[6],
            MASKFILE[5]|MASKFILE[7],
            MASKFILE[6]
        };
    
        //white pawns
        Bitboard wp=whitePawns;
        while(wp){
            Square sq=poplsb(wp);
            int f=fileof(sq);
            int r=rankof(sq);

            //isolated pawn
            if((whitePawns & ADJACENT_FILES[f])==0){
                evalData.add(ISOLATED_PAWN_PENALTY);
            }

            //double pawn
            if((whitePawns&MASKFILE[f])^(1ULL<<sq)){
                evalData.add(DOUBLED_PAWN_PENALTY);
            }

            //passed apwn
            if((MASKPASSED[White][sq]&blackPawns)==0){
                evalData.add(PASSED_PAWN_BONUS[r]);
            }
        }

        //black pawn
        Bitboard bp=blackPawns;
        while(bp){
            Square sq=poplsb(bp);
            int f=fileof(sq);
            int relative_rank=7-rankof(sq);

            //isolated pawn
            if((blackPawns & ADJACENT_FILES[f])==0){
                evalData.add(-ISOLATED_PAWN_PENALTY);
            }

            //double pawn
            if((blackPawns&MASKFILE[f])^(1ULL<<sq)){
                evalData.add(-DOUBLED_PAWN_PENALTY);
            }

            //passed apwn
            if((MASKPASSED[Black][sq]&whitePawns)==0){
                evalData.add(-PASSED_PAWN_BONUS[relative_rank]);
            }
        }
    }

    void Evaluator::evaluate_mobility(const Position& pos){
        Bitboard occupancy=pos.occupancy();
        
        //white mobility
        Bitboard knights=pos.knights<White>();
        while(knights){
            Square sq=poplsb(knights);
            Bitboard attacks=Attacks::get_knight_attacks(sq);
            //exclude friendly pieces
            attacks&=~pos.occupancy(White);
            int count=popcount(attacks);
            evalData.add(MobilityBonus_Knight[count]);
        }

        Bitboard bishops=pos.bishops<White>();
        while(bishops){
            Square sq=poplsb(bishops);
            Bitboard attacks=Attacks::get_bishop_attacks(sq,occupancy);
            //exclude friendly pieces
            attacks&=~pos.occupancy(White);
            int count=popcount(attacks);
            evalData.add(MobilityBonus_Bishop[std::min(count,13)]);
        }

        Bitboard rooks=pos.rooks<White>();
        while(rooks){
            Square sq=poplsb(rooks);
            Bitboard attacks=Attacks::get_rook_attacks(sq,occupancy);
            //exclude friendly pieces
            attacks&=~pos.occupancy(White);
            int count=popcount(attacks);
            evalData.add(MobilityBonus_Rook[std::min(count,14)]);
        }

        //black
        knights=pos.knights<Black>();
        while(knights){
            Square sq=poplsb(knights);
            Bitboard attacks=Attacks::get_knight_attacks(sq);
            //exclude friendly pieces
            attacks&=~pos.occupancy(Black);
            int count=popcount(attacks);
            evalData.add(-MobilityBonus_Knight[count]);
        }

        bishops=pos.bishops<Black>();
        while(bishops){
            Square sq=poplsb(bishops);
            Bitboard attacks=Attacks::get_bishop_attacks(sq,occupancy);
            //exclude friendly pieces
            attacks&=~pos.occupancy(Black);
            int count=popcount(attacks);
            evalData.add(-MobilityBonus_Bishop[std::min(count,13)]);
        }

        rooks=pos.rooks<Black>();
        while(rooks){
            Square sq=poplsb(rooks);
            Bitboard attacks=Attacks::get_rook_attacks(sq,occupancy);
            //exclude friendly pieces
            attacks&=~pos.occupancy(Black);
            int count=popcount(attacks);
            evalData.add(-MobilityBonus_Rook[std::min(count,14)]);
        }
    }

    int Evaluator::calculate_game_phase(const Position& pos) const {
    
        int phase = 0;
        phase += popcount(pos.knights<White>()) + popcount(pos.knights<Black>());
        phase += popcount(pos.bishops<White>()) + popcount(pos.bishops<Black>());
        phase += 2 * (popcount(pos.rooks<White>()) + popcount(pos.rooks<Black>()));
        phase += 4 * (popcount(pos.queens<White>()) + popcount(pos.queens<Black>()));
    
        phase = std::clamp(phase, 0, 24);
    
        return phase;
    }

    Score Evaluator::calculate_final_score(const Position& pos) const {
    
        EvalScore score = evalData.currentScore;

        // Add tempo bonus for side to move (FIX: no parentheses!)
        if (pos.sideToMove() == White) {
            score += TEMPO_BONUS;
        }

        int phase = calculate_game_phase(pos);
        constexpr int maxPhase = 24;

        Score opening = openingScore(score);
        Score endgame = endgameScore(score);

        Score finalScore = (opening * phase + endgame * (maxPhase - phase)) / maxPhase;
    
        Score result = (pos.sideToMove() == White) ? finalScore : -finalScore;
    
        return result;
    }
}
