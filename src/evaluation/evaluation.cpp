#include "evaluation.h"
#include "../core/types.h"
#include "../core/attacks.h"
#include "psqt.h"
#include <algorithm>

namespace ASTROVE::eval {

    Evaluator board_evaluator;

    Score Evaluator::EvaluateBoard(const Position& pos) {
        initialize(pos);

        EvaluateMaterialAndPlacement(pos);
        EvaluatePawns(pos);
        EvaluateMobility(pos);
        EvaluateKingSafety(pos);
        EvaluateRook(pos);
        EvaluatePieceStructure(pos);
        
        return CalculateFinalScore(pos);
    }

    void Evaluator::initialize(const Position& /*pos*/) {
        evalData = EvaluationData{};
    }

    void Evaluator::EvaluateMaterialAndPlacement(const Position& pos){
        evalData.mg += pos.getMgScore();
        evalData.eg += pos.getEgScore();
    }

    //function for Evaluate Pawn
    void Evaluator::EvaluatePawns(const Position& pos){
        // Probe pawn hash table
        int32_t cachedMg, cachedEg;
        if (pawnTable.probe(pos.getPawnKey(), cachedMg, cachedEg)) {
            evalData.mg += cachedMg;
            evalData.eg += cachedEg;
            return;
        }

        // Save current eval to compute delta
        int32_t mgBefore = evalData.mg;
        int32_t egBefore = evalData.eg;

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

            //BACKWARD PAWN(can`t safely advance + can`t be protected by other pawns)
            //1. not a passed pawn(can`t advacnce)
            //2. No friendly pawns on adjacent files at same or lower rank(can`t defended)
            //3. square in front is attacked by an enemy pawn(pawn becomes a long-term weakness)

            bool isPassed=(MASKPASSED[White][sq] & blackPawns)==0;
            if(!isPassed){
                Bitboard ranksBehind=(1ULL<<sq)-1; //all square with index<sq
                Bitboard friendsBehind=whitePawns & ADJACENT_FILES[f] & ranksBehind;

                //square directly infront of pawn
                Square frontSq=Square(sq+8);
                Bitboard frontAttackByEnemy=Attacks::GetPawnAttacks(White,frontSq) & blackPawns;

                if(friendsBehind==0 && frontAttackByEnemy!=0){
                    evalData.add(BACKWARD_PAWN_PENALTY);
                }
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
                evalData.subtract(ISOLATED_PAWN_PENALTY);
            }

            //double pawn
            if((blackPawns & MASKFILE[f])^(1ULL<<sq)){
                evalData.subtract(DOUBLED_PAWN_PENALTY);
            }

            //passed apwn
            if((MASKPASSED[Black][sq]&whitePawns)==0){
                evalData.subtract(PASSED_PAWN_BONUS[relative_rank]);
            }

            bool isPassed=(MASKPASSED[Black][sq] & whitePawns)==0;
            if(!isPassed){
                Bitboard ranksAhead=~((1ULL<<(sq+1))-1); //all square with index>sq
                Bitboard friendsBehind=blackPawns & ADJACENT_FILES[f] & ranksAhead;

                //square directly infront of pawn
                Square frontSq=Square(sq-8);
                Bitboard frontAttackByEnemy=Attacks::GetPawnAttacks(Black,frontSq) & whitePawns;

                if(friendsBehind==0 && frontAttackByEnemy!=0){
                    evalData.subtract(BACKWARD_PAWN_PENALTY);
                }
            }
        }

        // Store pawn eval delta in pawn hash table
        pawnTable.store(pos.getPawnKey(), evalData.mg - mgBefore, evalData.eg - egBefore);
    }

    void Evaluator::EvaluateMobility(const Position& pos){
        Bitboard occupancy=pos.occupancy();
        
        Bitboard blackPawnAtt=0ULL;
        Bitboard bp=pos.pawns<Black>();
        while(bp){
            Square sq = poplsb(bp);
            blackPawnAtt |= Attacks::GetPawnAttacks(Black, sq);
        }

        Bitboard whitePawnAtt = 0ULL;
        Bitboard wp = pos.pawns<White>();
        while(wp){
            Square sq = poplsb(wp);
            whitePawnAtt |= Attacks::GetPawnAttacks(White, sq);
        }

        Bitboard whiteSafe = ~pos.occupancy(White) & ~blackPawnAtt;
        Bitboard blackSafe = ~pos.occupancy(Black) & ~whitePawnAtt;

        //white mobility
        Bitboard knights=pos.knights<White>();
        while(knights){
            Square sq=poplsb(knights);
        
            int count=popcount(Attacks::GetKnightAttacks(sq) & whiteSafe);
            evalData.add(MobilityBonus_Knight[count]);
        }

        Bitboard bishops=pos.bishops<White>();
        while(bishops){
            Square sq=poplsb(bishops);
        
            int count=popcount(Attacks::GetBishopAttacks(sq, occupancy) & whiteSafe);
            evalData.add(MobilityBonus_Bishop[std::min(count,13)]);
        }

        Bitboard rooks=pos.rooks<White>();
        while(rooks){
            Square sq=poplsb(rooks);
            
            int count=popcount(Attacks::GetRookAttacks(sq, occupancy) & whiteSafe);
            evalData.add(MobilityBonus_Rook[std::min(count,14)]);
        }

        //black
        knights=pos.knights<Black>();
        while(knights){
            Square sq=poplsb(knights);
            
            int count=popcount(Attacks::GetKnightAttacks(sq) & blackSafe);
            evalData.subtract(MobilityBonus_Knight[count]);
        }

        bishops=pos.bishops<Black>();
        while(bishops){
            Square sq=poplsb(bishops);
            
            int count=popcount(Attacks::GetBishopAttacks(sq, occupancy) & blackSafe);
            evalData.subtract(MobilityBonus_Bishop[std::min(count,13)]);
        }

        rooks=pos.rooks<Black>();
        while(rooks){
            Square sq=poplsb(rooks);
        
            int count=popcount(Attacks::GetRookAttacks(sq, occupancy) & blackSafe);
            evalData.subtract(MobilityBonus_Rook[std::min(count,14)]);
        }
    }

    //king safety pawn shield
    void Evaluator::EvaluateKingSafety(const Position& pos){
        Bitboard occupancy = pos.occupancy();
        
        //a helper for calculate danger score for a specific side
        auto calculatedanger = [&](Color side)->EvalScore{
            Square ksq = (side == White) ? pos.kingsq<White>() : pos.kingsq<Black>();
            Color enemy=~side;

            //king ring
            Bitboard kingring=Attacks::GetKingAttacks(ksq);

            //now counting attackers
            int attacksunit=0;
            int attackercount=0;
            
            //knight
            Bitboard knights=pos.pieces(enemy,Knight);
            while(knights){
                Square sq=poplsb(knights);
                Bitboard attacks=Attacks::GetKnightAttacks(sq);

                if(attacks & kingring){
                    attacksunit += knightweight;
                    attackercount++;
                }
            }

            //bishop
            Bitboard bishops=pos.pieces(enemy,Bishop);
            while(bishops){
                Square sq=poplsb(bishops);
                Bitboard attacks=Attacks::GetBishopAttacks(sq,occupancy);

                if(attacks & kingring){
                    attacksunit+=bishopweight;
                    attackercount++;
                }
            }

            //rooks
            Bitboard rooks=pos.pieces(enemy,Rook);
            while (rooks){
                Square sq = poplsb(rooks);
                Bitboard attacks = Attacks::GetRookAttacks(sq, occupancy);

                if (attacks & kingring) {
                    attacksunit += rookweight;
                    attackercount++;
                }
            }

            //queen
            Bitboard queens=pos.pieces(enemy,Queen);
            while (queens) {
                Square sq=poplsb(queens);

                Bitboard attacks = Attacks::GetBishopAttacks(sq,occupancy) 
                                 | Attacks::GetRookAttacks(sq,occupancy);
                
                if (attacks & kingring) {
                    attacksunit += queenweight;
                    attackercount++;
                }
            }

            //now calculate penalty
            if(attackercount>=1){
                if(pos.pieces(enemy,Queen)==0){
                    attacksunit/=2;
                }

                int index=std::min(attacksunit+ (attackercount*3),99);
                return SafetyTable[index];
            }
            return composeEval(0,0);
        };

        //now penalty
        EvalScore whitedanger = calculatedanger(White);
        evalData.subtract(whitedanger);

        EvalScore blackdanger=calculatedanger(Black);
        evalData.add(blackdanger);
    }

    void Evaluator::EvaluateRook(const Position& pos) {
        //white rook
        Bitboard wrook=pos.rooks<White>();
        while(wrook){
            Square sq=poplsb(wrook);
            int f=fileof(sq);
            Bitboard fileMask=MASKFILE[f];

            if (!(pos.pawns<White>()&fileMask)){
                if (!(pos.pawns<Black>() & fileMask)) {
                    evalData.add(ROOK_OPEN_FILE_BONUS);
                } else {
                    evalData.add(ROOK_SEMI_OPEN_FILE_BONUS);
                }
            }
        }

        // black rook
        Bitboard brook=pos.rooks<Black>();
        while(brook){
            Square sq=poplsb(brook);
            int f=fileof(sq);
            Bitboard fileMask=MASKFILE[f];

            if (!(pos.pawns<Black>()&fileMask)) {
                if (!(pos.pawns<White>() & fileMask)) {
                    evalData.subtract(ROOK_OPEN_FILE_BONUS);
                } else {
                    evalData.subtract(ROOK_SEMI_OPEN_FILE_BONUS);
                }
            }
        }
    }

    //ouposts
    void Evaluator::EvaluatePieceStructure(const Position& pos){
        //bishop pair
        if(popcount(pos.bishops<White>())>=2){
            evalData.add(BISHOP_PAIR_BONUS);
        }

        if(popcount(pos.bishops<Black>())>=2){
            evalData.subtract(BISHOP_PAIR_BONUS);
        }
        
        //knight outpost
        Bitboard wknight=pos.knights<White>();
        Bitboard wpawns=pos.pawns<White>();
        while(wknight){
            Square sq=poplsb(wknight);
            int r=rankof(sq);

            //so we check outpost on rank3 to6
            if(r>=RANK_3 && r<=RANK_6){
                if(Attacks::GetPawnAttacks(Black,sq)& wpawns){
                    //means it`s supported by pawn
                    evalData.add(KNIGHT_OUTPOST_BONUS[r]);
                }
            }
        }

        Bitboard bknight=pos.knights<Black>();
        Bitboard bpawns=pos.pawns<Black>();
        while(bknight){
            Square sq=poplsb(bknight);
            int rr=7-rankof(sq);

            //so we check outpost on rank3 to6
            if(rr>=RANK_3 && rr<=RANK_6){
                if(Attacks::GetPawnAttacks(White,sq)& bpawns){
                    //means it`s supported by pawn
                    evalData.subtract(KNIGHT_OUTPOST_BONUS[rr]);
                }
            }
        }
    }
    int Evaluator::CalculateGamePhase(const Position& pos) const {
        return std::clamp(pos.getPhase(), 0, 24);
    }

    Score Evaluator::CalculateFinalScore(const Position& pos) const {
        Score opening = evalData.opening();
        Score endgame = evalData.endgame();

        // Add tempo bonus for side to move
        if (pos.sideToMove() == White) {
            opening+=openingScore(TEMPO_BONUS);
            endgame+=endgameScore(TEMPO_BONUS);
        } else {
            opening-=openingScore(TEMPO_BONUS);
            endgame-=endgameScore(TEMPO_BONUS);
        }

        int phase = CalculateGamePhase(pos);

        Score finalScore = (opening * phase + endgame * (24 - phase)) / 24;
    
        Score result = (pos.sideToMove() == White) ? finalScore : -finalScore;
    
        return result;
    }
}
